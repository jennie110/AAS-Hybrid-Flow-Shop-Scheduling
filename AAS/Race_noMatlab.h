#pragma once
#include "AAS_CCH.h"
#include "Configuration.h"
#include "CompareConfiguration.h"
#include "HFSP_CS.h"

#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

using namespace std;

std::default_random_engine gen;

// 计算正态分布的 CDF (用于 p-value)
double normalCDF(double value)
{
	return 0.5 * erfc(-value * 0.70710678118654752440);
}

// Mann-Whitney U Test 
// 返回双尾 p-value
double MannWhitneyUTest(const vector<double>& sample1, const vector<double>& sample2)
{
	size_t n1 = sample1.size();
	size_t n2 = sample2.size();

	if (n1 == 0 || n2 == 0) return 1.0;

	//  合并样本并记录原始组别
	struct RankNode {
		double value;
		int group; // 1 or 2
	};
	vector<RankNode> combined;
	combined.reserve(n1 + n2);
	for (double v : sample1) combined.push_back({ v, 1 });
	for (double v : sample2) combined.push_back({ v, 2 });

	// 排序
	sort(combined.begin(), combined.end(), [](const RankNode& a, const RankNode& b) {
		return a.value < b.value;
		});

	//计算秩 (处理 Ties/并列值)
	double sumRank1 = 0.0;
	double sumRank2 = 0.0;

	size_t i = 0;
	while (i < combined.size()) {
		size_t j = i + 1;
		while (j < combined.size() && combined[j].value == combined[i].value) {
			j++;
		}

		// 相同值的数量为 (j - i)
		// 它们的秩是从 (i+1) 到 j
		// 平均秩 = ( (i+1) + j ) / 2.0
		double avgRank = ((double)(i + 1) + j) / 2.0;

		for (size_t k = i; k < j; k++) {
			if (combined[k].group == 1) sumRank1 += avgRank;
			else sumRank2 += avgRank;
		}
		i = j;
	}

	// 计算 U 统计量
	double U1 = sumRank1 - (double)n1 * (n1 + 1) / 2.0;
	// double U2 = sumRank2 - (double)n2 * (n2 + 1) / 2.0; // U2 不需要实际计算，U2 = n1*n2 - U1
	double U = min(U1, (double)n1 * n2 - U1);

	//  正态近似 (Normal Approximation)
	double mu = (double)n1 * n2 / 2.0;

	// 简化的标准差计算 (忽略 Ties 的校正，对于高性能计算通常足够精确)
	// 如果需要极致精确，可以加入 Ties 校正公式，但通常差异极小
	double sigma = sqrt((double)n1 * n2 * (n1 + n2 + 1) / 12.0);

	if (sigma == 0.0) return 1.0; // 方差为0，无法区分

	double Z = (U - mu) / sigma;

	// 双尾 P-value
	double p_value = 2.0 * normalCDF(-abs(Z));
	return p_value;
}



//顺序读取且随机生成实例
bool readInstanceData_for_race(const string& filePath)
{
	ifstream fin(filePath);
	if (!fin.is_open()) {
		return false;
	}
	if (!(fin >> pJob >> pStage)) {
		fin.close(); return false;
	}
	pMachines.resize(pStage);
	for (int k = 0; k < pStage; ++k) {
		if (!(fin >> pMachines[k])) {
			fin.close(); return false;
		}
	}
	pUnitTime.assign(pStage, vector<int>(pJob));
	for (int k = 0; k < pStage; ++k) {
		for (int j = 0; j < pJob; ++j) {
			if (!(fin >> pUnitTime[k][j])) {
				fin.close(); return false;
			}
		}
	}
	pSetupTime.assign(pStage, vector<int>(pJob, 0));
	pTransferTime.assign(pStage, vector<int>(pJob, 0));
	fin.close();
	return true;
}

// 仅读取实际实例
static vector<string> g_AllInstanceFiles;
static bool g_FilesLoaded = false;
static int g_CurrentFileIndex = 0;

void LoadAllBenchmarkFiles() {
	if (g_FilesLoaded) return;

	string basePaths[] = {
		"Jose_benchmark\\Small_Size_Instances",
		"Jose_benchmark\\Big_Size_Instances"
	};

	for (const string& dirPath : basePaths) {
		string searchPath = dirPath + "\\*.txt";
		WIN32_FIND_DATAA fd;
		HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					g_AllInstanceFiles.push_back(dirPath + "\\" + fd.cFileName);
				}
			} while (FindNextFileA(hFind, &fd) != 0);
			FindClose(hFind);
		}
	}

	if (g_AllInstanceFiles.empty()) {
		cerr << "Error: No benchmark files found!" << endl;
		exit(1);
	}

	// 初始打乱顺序，保证大小实例混合
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(g_AllInstanceFiles.begin(), g_AllInstanceFiles.end(), g);

	g_FilesLoaded = true;
	cout << "Total Benchmark Instances Loaded: " << g_AllInstanceFiles.size() << endl;
}

void GetNextTuningInstance(int Seed)
{
	// 确保文件已加载
	if (!g_FilesLoaded) {
		LoadAllBenchmarkFiles();
	}

	// 获取当前文件路径
	string filePath = g_AllInstanceFiles[g_CurrentFileIndex];

	//读取数据
	if (!readInstanceData_for_race(filePath)) {
		cerr << "Warning: Failed to read " << filePath << ", skipping..." << endl;
	}

	//索引递增
	g_CurrentFileIndex++;

	// 如果遍历完了一轮，重新洗牌并重置索引
	if (g_CurrentFileIndex >= g_AllInstanceFiles.size()) {
		g_CurrentFileIndex = 0;
		// 重新洗牌，确保下一轮 Race 即使预算很多，也能遇到不同的顺序
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(g_AllInstanceFiles.begin(), g_AllInstanceFiles.end(), g);
		cout << "Benchmark Cycle Completed, Reshuffling" << endl;
	}
}

//void GetNextTuningInstance(int Seed)
//{
//	static enum RacePhase { INIT, SMALL_INST, BIG_INST, RANDOM_INST };
//	static RacePhase currentPhase = RacePhase::INIT;
//
//	static vector<string> smallInstanceFiles;
//	static vector<string> bigInstanceFiles;
//	static int smallInstanceIndex = 0;
//	static int bigInstanceIndex = 0;
//
//	if (currentPhase == RacePhase::INIT)
//	{
//		string smallPath = "Jose_benchmark\\Small_Size_Instances";
//		string searchPathSmall = smallPath + "\\*.txt";
//		WIN32_FIND_DATAA fdSmall;
//		HANDLE hFindSmall = FindFirstFileA(searchPathSmall.c_str(), &fdSmall);
//		if (hFindSmall != INVALID_HANDLE_VALUE) {
//			do {
//				if (!(fdSmall.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
//					smallInstanceFiles.push_back(smallPath + "\\" + fdSmall.cFileName);
//				}
//			} while (FindNextFileA(hFindSmall, &fdSmall) != 0);
//			FindClose(hFindSmall);
//		}
//
//		string bigPath = "Jose_benchmark\\Big_Size_Instances";
//		string searchPathBig = bigPath + "\\*.txt";
//		WIN32_FIND_DATAA fdBig;
//		HANDLE hFindBig = FindFirstFileA(searchPathBig.c_str(), &fdBig);
//		if (hFindBig != INVALID_HANDLE_VALUE) {
//			do {
//				if (!(fdBig.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
//					bigInstanceFiles.push_back(bigPath + "\\" + fdBig.cFileName);
//				}
//			} while (FindNextFileA(hFindBig, &fdBig) != 0);
//			FindClose(hFindBig);
//		}
//
//		if (!smallInstanceFiles.empty()) {
//			currentPhase = RacePhase::SMALL_INST;
//		}
//		else if (!bigInstanceFiles.empty()) {
//			currentPhase = RacePhase::BIG_INST;
//		}
//		else {
//			currentPhase = RacePhase::RANDOM_INST;
//		}
//	}
//
//	if (currentPhase == RacePhase::SMALL_INST)
//	{
//		if (smallInstanceIndex < smallInstanceFiles.size())
//		{
//			string filePath = smallInstanceFiles[smallInstanceIndex];
//			smallInstanceIndex++;
//			if (!readInstanceData_for_race(filePath)) {
//				GetNextTuningInstance(Seed);
//			}
//			return;
//		}
//		else
//		{
//			currentPhase = RacePhase::BIG_INST;
//			bigInstanceIndex = 0;
//		}
//	}
//
//	if (currentPhase == RacePhase::BIG_INST)
//	{
//		if (!bigInstanceFiles.empty() && bigInstanceIndex < bigInstanceFiles.size())
//		{
//			string filePath = bigInstanceFiles[bigInstanceIndex];
//			bigInstanceIndex++;
//			if (!readInstanceData_for_race(filePath)) {
//				GetNextTuningInstance(Seed);
//			}
//			return;
//		}
//		else
//		{
//			currentPhase = RacePhase::RANDOM_INST;
//		}
//	}
//
//	if (currentPhase == RacePhase::RANDOM_INST)
//	{
//		int f = rand() % 6;
//		int c = rand() % 4;
//		int InJob, InStage;;
//		if (rand() % 2 == 0)
//		{
//			InJob = Jobs[f];
//			InStage = Stages[c];
//		}
//		else
//		{
//			InJob = Jobs1[f];
//			InStage = Stages1[c];
//		}
//
//		int machineRule = 1 + rand() % 3;
//		int procTimeRule = 1 + rand() % 2;
//
//		GenerateInstances(InJob, InStage, Seed, machineRule, procTimeRule);
//		return;
//	}
//}

// Race 类定义

class Race
{
public:
	Race(int NumberOfNP_Integer, int NumberOfNP_Real, vector<int>& MaxForNP_Integer, vector<int>& MinForNP_Integer, vector<double>& MaxForNP_Real, vector<double>& MinForNP_Real, int NumberOfCP, vector<int>& KindsForCP);
	void run();
	void outputBest();

	vector<Configuration> CandidateConfigurations;
	vector<Configuration> EliteConfigurations;

public:
	int B;
	int minSurvival;
	int NumberOfElite;
	int usedB;
	int usingB;
	int CurrentIteration;
	int NumberOfIterations;
	int NumberOfCandidateConfigurations;
	int NumberOfCalls;

public:
	int MSUpper;
	int MSLower;

public:
	int NumberOfNP_Integer;
	int NumberOfNP_Real;
	int NumberOfCP;

public:
	vector<int> MaxForNP_Integer;
	vector<int> MinForNP_Integer;

	vector<double> MaxForNP_Real;
	vector<double> MinForNP_Real;

public:
	vector<int> KindsForCP;

public:
	vector<double> stdForNP_Integer;
	vector<double> stdForNP_Real;
	vector<vector<double>> pForCP;
public:
	fstream fffout;
};

Race::Race(int NumberOfNP_Integer, int NumberOfNP_Real, vector<int>& MaxForNP_Integer, vector<int>& MinForNP_Integer, vector<double>& MaxForNP_Real, vector<double>& MinForNP_Real, int NumberOfCP, vector<int>& KindsForCP)
{
	this->NumberOfNP_Integer = NumberOfNP_Integer;
	this->NumberOfNP_Real = NumberOfNP_Real;
	this->NumberOfCP = NumberOfCP;
	this->MaxForNP_Integer = MaxForNP_Integer;
	this->MinForNP_Integer = MinForNP_Integer;
	this->MaxForNP_Real = MaxForNP_Real;
	this->MinForNP_Real = MinForNP_Real;
	this->KindsForCP = KindsForCP;

	fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

	int NOP = NumberOfNP_Integer + NumberOfNP_Real + NumberOfCP;
	
	NumberOfIterations = 10;  
	cout << "NumberOfIterations:" << NumberOfIterations << endl;
	fffout << "NumberOfIterations:" << NumberOfIterations << endl;

	CurrentIteration = 1;
	usedB = 0;
	B = 180000; 

	fffout << "AAD Start: The total budget is:" << B << endl;
	cout << "AAD Start: The total budget is:" << B << endl;

	usingB = (B - usedB) / (NumberOfIterations - CurrentIteration + 1);

	cout << "The " << CurrentIteration << " iteration using budeget:" << usingB << endl;
	fffout << "The " << CurrentIteration << " iteration using budeget:" << usingB << endl;

	NumberOfCandidateConfigurations = (int)(usingB / (8 + 2 * CurrentIteration));

	cout << "The" << CurrentIteration << " iteration number of candidate configurations:" << NumberOfCandidateConfigurations << endl;
	fffout << "The" << CurrentIteration << " iteration number of candidate configurations:" << NumberOfCandidateConfigurations << endl;

	CandidateConfigurations.clear();

	for (int i = 0; i < NumberOfCandidateConfigurations; i++)
	{
		const unsigned int seed = i;
		std::mt19937 gen(i);

		vector<int> tempForCP;
		tempForCP.resize(NumberOfCP);
		for (int j = 0; j < NumberOfCP; j++)
		{
			tempForCP[j] = rand() % KindsForCP[j] + 1;
		}

		vector<int> tempForNP_Integer;
		tempForNP_Integer.resize(NumberOfNP_Integer);
		for (int j = 0; j < NumberOfNP_Integer; j++)
		{
			tempForNP_Integer[j] = MinForNP_Integer[j] + rand() % (MaxForNP_Integer[j] - MinForNP_Integer[j]);
		}

		vector<double> tempForNP_Real;
		int total_aad = 100;
		int n_aad = NumberOfNP_Real;
		std::vector<int> cuts;
		std::uniform_int_distribution<> dis(1, total_aad - 1);
		while (cuts.size() < n_aad - 1) {
			int val = dis(gen);
			if (std::find(cuts.begin(), cuts.end(), val) == cuts.end()) {
				cuts.push_back(val);
			}
		}
		cuts.push_back(0);
		cuts.push_back(total_aad);
		std::sort(cuts.begin(), cuts.end());

		for (int i = 1; i <= n_aad; ++i) {
			double val = (cuts[i] - cuts[i - 1]) / 100.0;
			tempForNP_Real.push_back(val);
		}

		CandidateConfigurations.push_back(Configuration(tempForNP_Integer, tempForNP_Real, tempForCP));
	}

	// minSurvival: 每轮保留的最小配置数
	// NumberOfElite: 精英配置数
	minSurvival = 13;
	NumberOfElite = 5;

	stdForNP_Integer.resize(NumberOfNP_Integer);
	for (int j = 0; j < NumberOfNP_Integer; j++)
	{
		stdForNP_Integer[j] = (MaxForNP_Integer[j] - MinForNP_Integer[j]) * 1.0 / 2;
	}

	stdForNP_Real.resize(NumberOfNP_Real);
	for (int j = 0; j < NumberOfNP_Real; j++)
	{
		stdForNP_Real[j] = (MaxForNP_Real[j] - MinForNP_Real[j]) * 1.0 / 2;
	}

	pForCP.resize(NumberOfCP);
	for (int i = 0; i < NumberOfCP; i++)
	{
		pForCP[i].resize(KindsForCP[i]);
	}
	for (int i = 0; i < NumberOfCP; i++)
	{
		for (int j = 0; j < KindsForCP[i]; j++)
		{
			pForCP[i][j] = 1.0 / KindsForCP[i];
		}
	}
	fffout.close();
}

void Race::outputBest()
{
	fffout.open("EliteConfigurations.txt", fstream::out | fstream::app);

	for (int j = 0; j < EliteConfigurations.size(); j++)
	{
		for (int i = 0; i < NumberOfNP_Integer; i++)
		{
			cout << EliteConfigurations[j].ValueForNumericalParemeter_Integer[i] << " ";
			fffout << EliteConfigurations[j].ValueForNumericalParemeter_Integer[i] << " ";
		}
		for (int i = 0; i < NumberOfNP_Real; i++)
		{
			cout << EliteConfigurations[j].ValueForNumericalParemeter_Real[i] << " ";
			fffout << EliteConfigurations[j].ValueForNumericalParemeter_Real[i] << " ";
		}
		for (int i = 0; i < NumberOfCP; i++)
		{
			cout << EliteConfigurations[j].ValueForCategoricalParemeter[i] << " ";
			fffout << EliteConfigurations[j].ValueForCategoricalParemeter[i] << " ";
		}
	}
	cout << endl;
	fffout << endl;
	fffout.close();
}

void Race::run()
{
	int NumberOfInstance = 1;

	while (usedB < B || CandidateConfigurations.size() > EliteConfigurations.size())
	{
		NumberOfCalls = 0;
		int NOII = 0;

		while ((NumberOfCalls + CandidateConfigurations.size()) <= usingB)
		{
			fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

			for (int k = 0; k < IndependentRuns; k++)
			{
				cout << "第" << CurrentIteration << " iteration" << "测试第" << NumberOfInstance << "实例" << endl;
				fffout << "第" << CurrentIteration << " iteration" << "测试第" << NumberOfInstance << "实例" << endl;

				GetNextTuningInstance(NumberOfInstance + 1234);
				cout << "Instances:" << pJob << " " << pStage << endl;
				NOII++;

				int UpperBound = 0;
				int LowerBound = INT_MAX;

				for (int i = 0; i < CandidateConfigurations.size(); i++)
				{
					double SPTB_weight = CandidateConfigurations[i].ValueForNumericalParemeter_Real[0];
					double AfterBN_weight = CandidateConfigurations[i].ValueForNumericalParemeter_Real[1];
					double Total_weight = CandidateConfigurations[i].ValueForNumericalParemeter_Real[2];
					double Current_weight = CandidateConfigurations[i].ValueForNumericalParemeter_Real[3];
					double lastjob_weight = CandidateConfigurations[i].ValueForNumericalParemeter_Real[4];
                FAS_CCH* cch = new FAS_CCH(SPTB_weight, AfterBN_weight, Total_weight,
						Current_weight,lastjob_weight);

					long initTime, finalTime;
					double costTime;
					initTime = GetTickCount();

					cch->run_with_reversibility();					
					CandidateConfigurations[i].CostValues[k] = cch->makespan;

					if (CandidateConfigurations[i].CostValues[k] > UpperBound)
					{
						UpperBound = CandidateConfigurations[i].CostValues[k];
					}

					if (CandidateConfigurations[i].CostValues[k] < LowerBound)
					{
						LowerBound = CandidateConfigurations[i].CostValues[k];
					}

					finalTime = GetTickCount();
					costTime = (finalTime - initTime) / 1000.0;
					cout << "costTime:" << costTime << endl;

					cout << "CandidateConfigurations[" << i << "]=" << CandidateConfigurations[i].CostValues[k] << endl;
					delete cch;
				}

				// Max-min method 
				for (int i = 0; i < CandidateConfigurations.size(); i++)
				{
					if (UpperBound - LowerBound != 0)
					{
						CandidateConfigurations[i].CostValues[k] = (CandidateConfigurations[i].CostValues[k] - LowerBound) * 1.0 / (UpperBound - LowerBound);
					}
					else
					{
						CandidateConfigurations[i].CostValues[k] = 0.0; // 避免除以零
					}
				}

				for (int i = 0; i < CandidateConfigurations.size(); i++)
				{
					CandidateConfigurations[i].getAvgValue();
				}

				NumberOfInstance++;
			}

			NumberOfCalls += CandidateConfigurations.size();
			usedB += CandidateConfigurations.size();

			fffout << "The" << CurrentIteration << " iteration used budeget:" << NumberOfCalls << endl;

			// =====================================================
			// 原生 C++ 统计检验替换 MATLAB 部分
			// =====================================================

			cout << "开始进行统计检验 (C++ 原生 Mann-Whitney U)..." << endl;

			for (int i = 0; i < CandidateConfigurations.size(); i++)
			{
				// 如果 i 已经被标记为 worse，是否可以跳过？原代码未跳过，这里保持原样以确保全量比较

				for (int j = 0; j < CandidateConfigurations.size(); j++)
				{
					if (i != j)
					{
						// 提取数据到 vector 供 Mann-Whitney U 使用
						vector<double> x(IndependentRuns);
						vector<double> y(IndependentRuns);

						// 假设 CostValues 是数组或支持下标访问
						for (int r = 0; r < IndependentRuns; ++r) {
							x[r] = CandidateConfigurations[i].CostValues[r];
							y[r] = CandidateConfigurations[j].CostValues[r];
						}

						// 调用原生 U 检验
						double p_value = MannWhitneyUTest(x, y);

						// 原有逻辑：p < 0.05 则认为有显著差异
						if (p_value < 0.05)
						{
							// 均值越小越好（Makespan）
							// 如果 i 的均值大于 j，说明 i 比 j 差 (IsWorse)
							if (CandidateConfigurations[i].avgValue > CandidateConfigurations[j].avgValue)
							{
								CandidateConfigurations[i].IsWorse = true;
							}
							else
							{
								CandidateConfigurations[j].IsWorse = true;
							}
						}
					}
				}
			}
			// =====================================================
			// 统计检验结束
			// =====================================================

			int NumberOfPromisingSolutions = 0;
			for (int i = 0; i < CandidateConfigurations.size(); i++)
			{
				if (CandidateConfigurations[i].IsWorse == false)
				{
					NumberOfPromisingSolutions++;
				}
			}
			cout << "第 " << CurrentIteration << " Iteration测试 " << NumberOfInstance << " 实例后总共有：" << NumberOfPromisingSolutions << " 个Promising Solutions" << endl;
			fffout << "第 " << CurrentIteration << " Iteration测试 " << NumberOfInstance << " 实例后总共有：" << NumberOfPromisingSolutions << " 个Promising Solutions" << endl;

			// ranksumTerminate(); // 移除

			sort(CandidateConfigurations.begin(), CandidateConfigurations.end(), SortConfiguration());

			int NumberOfRemove = 0;
			int UpperNumber = CandidateConfigurations.size() - minSurvival;

			for (int i = CandidateConfigurations.size() - 1; i >= 0; i--)
			{
				if (CandidateConfigurations[i].IsWorse == true)
				{
					CandidateConfigurations.erase(CandidateConfigurations.begin() + i);
					NumberOfRemove++;
					if (NumberOfRemove >= UpperNumber)
					{
						break;
					}
				}
			}
			cout << "一共删除了" << NumberOfRemove << "个configurations" << endl;
			fffout << "一共删除了" << NumberOfRemove << "个configurations" << endl;

			for (int i = 0; i < CandidateConfigurations.size(); i++)
			{
				CandidateConfigurations[i].avgValue = 0.0;
				CandidateConfigurations[i].devValue = 0.0;
				CandidateConfigurations[i].IsWorse = false;

				for (int j = 0; j < IndependentRuns; j++)
				{
					CandidateConfigurations[i].CostValues[j] = 0.0;
				}
			}

			if (CandidateConfigurations.size() <= minSurvival)
			{
				cout << "因候选配置数目小于minSurvival,第" << CurrentIteration << " iteration结束" << endl;
				fffout << "因候选配置数目小于minSurvival,第" << CurrentIteration << " iteration结束" << endl;
				fffout.close();
				break;
			}
			else
			{
				fffout.close();
			}
		}

		// 迭代结束
		fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

		for (int i = 0; i < CandidateConfigurations.size(); i++)
		{
			CandidateConfigurations[i].rankIndex = CandidateConfigurations[i].rankIndex / NOII;
		}
		sort(CandidateConfigurations.begin(), CandidateConfigurations.end(), SortConfigurationRankIndex());

		EliteConfigurations.clear();

		int minOf;
		if (minSurvival < NumberOfElite)
		{
			minOf = minSurvival;
		}
		else
		{
			minOf = NumberOfElite;
		}

		for (int i = 0; i < minOf; i++)
		{
			EliteConfigurations.push_back(CandidateConfigurations[i]);
		}

		cout << "第" << CurrentIteration << "Iteration 结束后生成的Elite Configurations个数：" << EliteConfigurations.size() << endl;
		fffout << "第" << CurrentIteration << "Iteration 结束后生成的Elite Configurations个数：" << EliteConfigurations.size() << endl;

		fffout.close();

		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			EliteConfigurations[i].avgValue = 0.0;
			EliteConfigurations[i].devValue = 0.0;
			EliteConfigurations[i].rankIndex = 0.0;
			EliteConfigurations[i].IsWorse = false;

			for (int j = 0; j < IndependentRuns; j++)
			{
				EliteConfigurations[i].CostValues[j] = 0.0;
			}
		}

		outputBest();
		fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

		CurrentIteration++;
		cout << "执行" << CurrentIteration << " iteration" << endl;
		fffout << "执行" << CurrentIteration << " iteration" << endl;
		if (NumberOfIterations - CurrentIteration + 1 == 0)
		{
			break;
			fffout.close();
		}

		usingB = (B - usedB) / (NumberOfIterations - CurrentIteration + 1);
		cout << "第" << CurrentIteration << " iteration的usingB:" << usingB << endl;
		fffout << "第" << CurrentIteration << " iteration的usingB:" << usingB << endl;
		
		// 后续轮次候选配置数计算 (与第一轮使用相同公式保持一致性)
		NumberOfCandidateConfigurations = (int)(usingB / (8 + 2 * CurrentIteration));
		
		// 确保至少有 minSurvival + 几个新配置
		if (NumberOfCandidateConfigurations < minSurvival + 5) {
			NumberOfCandidateConfigurations = minSurvival + 5;
		}
		
		cout << "第" << CurrentIteration << " iteration的number of candidate configurations:" << NumberOfCandidateConfigurations << endl;
		fffout << "第" << CurrentIteration << " iteration的number of candidate configurations:" << NumberOfCandidateConfigurations << endl;

		if (CurrentIteration != 2)
		{
			for (int i = 0; i < stdForNP_Integer.size(); i++)
			{
				stdForNP_Integer[i] = stdForNP_Integer[i] * pow(1.0 / (NumberOfCandidateConfigurations - EliteConfigurations.size()), 1.0 / (NumberOfCP + NumberOfNP_Integer + NumberOfNP_Real));
			}
			for (int i = 0; i < stdForNP_Real.size(); i++)
			{
				stdForNP_Real[i] = stdForNP_Real[i] * pow(1.0 / (NumberOfCandidateConfigurations - EliteConfigurations.size()), 1.0 / (NumberOfCP + NumberOfNP_Integer + NumberOfNP_Real));
			}
		}

		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			EliteConfigurations[i].probability = (EliteConfigurations.size() - (i + 1) + 1) * 1.0 / (EliteConfigurations.size() * (EliteConfigurations.size() + 1) / 2);
		}

		double sumProbability = 0.0;
		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			fffout << "EliteConfigurations[" << i << "]:" << EliteConfigurations[i].probability << endl;
			sumProbability += EliteConfigurations[i].probability;
		}
		fffout << "总的概率为：" << sumProbability << endl;

		CandidateConfigurations.clear();

		for (int i = 0; i < (NumberOfCandidateConfigurations - EliteConfigurations.size()); i++)
		{
			double random = rand() * 1.0 / (RAND_MAX + 1);
			fffout << "the selected " << i << " probability:" << random << endl;
			double sumRandom = 0.0;
			Configuration Parent;
			for (int j = 0; j < EliteConfigurations.size(); j++)
			{
				if ((random >= sumRandom) && (random < (sumRandom + EliteConfigurations[j].probability)))
				{
					Parent = EliteConfigurations[j];
					fffout << "the selected Elite configuration:" << j << endl;
					break;
				}
				sumRandom += EliteConfigurations[j].probability;
			}

			vector<int> ValueForCategoricalParemeter;
			ValueForCategoricalParemeter.resize(NumberOfCP);
			vector<int> ValueForNumericalParemeter_Integer;
			ValueForNumericalParemeter_Integer.resize(NumberOfNP_Integer);
			vector<double> ValueForNumericalParemeter_Real;
			ValueForNumericalParemeter_Real.resize(NumberOfNP_Real);

			for (int j = 0; j < NumberOfNP_Integer; j++)
			{
				std::normal_distribution<double> dis(Parent.ValueForNumericalParemeter_Integer[j], sqrt(stdForNP_Integer[j]));
				ValueForNumericalParemeter_Integer[j] = round(dis(gen));

				if (ValueForNumericalParemeter_Integer[j] < MinForNP_Integer[j])
				{
					ValueForNumericalParemeter_Integer[j] = MinForNP_Integer[j];
				}
				if (ValueForNumericalParemeter_Integer[j] > MaxForNP_Integer[j])
				{
					ValueForNumericalParemeter_Integer[j] = MaxForNP_Integer[j];
				}
			}

			vector<double> sampled;
			sampled.resize(NumberOfNP_Real);
			double sum = 0.0;
			for (int j = 0; j < NumberOfNP_Real; j++)
			{
				std::normal_distribution<double> dis(Parent.ValueForNumericalParemeter_Real[j], sqrt(stdForNP_Real[j]));
				double val;
				do
				{
					val = dis(gen);
				} while (val < 0 || val > 1);

				sampled[j] = val;
				sum += val;
			}

			for (int j = 0; j < NumberOfNP_Real; j++)
			{
				ValueForNumericalParemeter_Real[j] = sampled[j] / sum;
			}

			cout << "已生成了第" << i << "个configuration" << endl;

			vector<vector<double>> temppForCP = pForCP;

			for (int j = 0; j < NumberOfCP; j++)
			{
				double Increment = (CurrentIteration - 1) * 1.0 / NumberOfIterations;
				for (int k = 0; k < KindsForCP[j]; k++)
				{
					temppForCP[j][k] = temppForCP[j][k] * (1 - (CurrentIteration - 1) * 1.0 / NumberOfIterations);
				}
				temppForCP[j][Parent.ValueForCategoricalParemeter[j] - 1] += Increment;
			}

			for (int j = 0; j < NumberOfCP; j++)
			{
				double sumForP = 0.0;
				for (int k = 0; k < KindsForCP[j]; k++)
				{
					cout << " The " << i << " pForCP_" << j << "_" << k << ":" << pForCP[j][k];
					fffout << " The " << i << " pForCP_" << j << "_" << k << ":" << pForCP[j][k];
					sumForP += pForCP[j][k];
				}
				fffout << endl << " The " << i << " sumpForCP_" << j << ":" << sumForP << endl;
				cout << endl << " The " << i << " sumpForCP_" << j << ":" << sumForP << endl;
			}

			for (int j = 0; j < NumberOfCP; j++)
			{
				double randomP = rand() * 1.0 / RAND_MAX;
				double sumP = 0.0;
				for (int k = 0; k < KindsForCP[j]; k++)
				{
					if ((randomP >= sumP) && (randomP < (sumP + temppForCP[j][k])))
					{
						ValueForCategoricalParemeter[j] = k + 1;
						break;
					}
					sumP += temppForCP[j][k];
				}
			}

			CandidateConfigurations.push_back(Configuration(ValueForNumericalParemeter_Integer, ValueForNumericalParemeter_Real, ValueForCategoricalParemeter));
		}

		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			EliteConfigurations[i].probability = 0.0;
			CandidateConfigurations.push_back(EliteConfigurations[i]);
		}

		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			for (int j = 0; j < NumberOfCP; j++)
			{
				double Increment = (CurrentIteration - 1) * 1.0 / NumberOfIterations;
				for (int k = 0; k < KindsForCP[j]; k++)
				{
					pForCP[j][k] = pForCP[j][k] * (1 - (CurrentIteration - 1) * 1.0 / NumberOfIterations);
				}
				pForCP[j][EliteConfigurations[i].ValueForCategoricalParemeter[j] - 1] += Increment;
			}
		}

		cout << "第" << CurrentIteration << " iteration的number of candidate configurations:" << NumberOfCandidateConfigurations << endl;
		fffout << "第" << CurrentIteration << " iteration的number of candidate configurations:" << NumberOfCandidateConfigurations << endl;
		fffout.close();
	}
	outputBest();
}
