#pragma once
#include "AAS_CCH.h"
#include "Configuration.h"
#include "CompareConfiguration.h"
#include "ranksum.h"
#include "myfriedman.h"
#include <iostream>
#include <random>
#include<Windows.h>
#include "HFSP_CS.h"


using namespace std;
std::default_random_engine gen;


//读取实例且随机生成实例
bool readInstanceData_for_race(const string& filePath)
{
	ifstream fin(filePath);
	if (!fin.is_open()) {
		return false;
	}
	if (!(fin >> pJob >> pStage)) {
		fin.close(); return false;
	}
	pMachines.resize(pStage); //
	for (int k = 0; k < pStage; ++k) {
		if (!(fin >> pMachines[k])) {
			fin.close(); return false;
		}
	}
	pUnitTime.assign(pStage, vector<int>(pJob)); //
	for (int k = 0; k < pStage; ++k) {
		for (int j = 0; j < pJob; ++j) {
			if (!(fin >> pUnitTime[k][j])) {
				fin.close(); return false;
			}
		}
	}
	pSetupTime.assign(pStage, vector<int>(pJob, 0)); //
	pTransferTime.assign(pStage, vector<int>(pJob, 0)); //
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
		"data\\Small_Size_Instances",
		"data\\Big_Size_Instances"
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

	// 关键步骤：初始打乱顺序，保证大小实例混合
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(g_AllInstanceFiles.begin(), g_AllInstanceFiles.end(), g);

	g_FilesLoaded = true;
	cout << "Total Benchmark Instances Loaded: " << g_AllInstanceFiles.size() << endl;
}

void GetNextTuningInstance(int Seed)
{
	// 1. 确保文件已加载
	if (!g_FilesLoaded) {
		LoadAllBenchmarkFiles();
	}

	// 2. 获取当前文件路径
	string filePath = g_AllInstanceFiles[g_CurrentFileIndex];

	// 3. 读取数据
	if (!readInstanceData_for_race(filePath)) {
		cerr << "Warning: Failed to read " << filePath << ", skipping..." << endl;
	}

	// 4. 索引递增
	g_CurrentFileIndex++;

	// 5. 如果遍历完了一轮，重新洗牌并重置索引
	if (g_CurrentFileIndex >= g_AllInstanceFiles.size()) {
		g_CurrentFileIndex = 0;
		// 重新洗牌，确保下一轮 Race 即使预算很多，也能遇到不同的顺序
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(g_AllInstanceFiles.begin(), g_AllInstanceFiles.end(), g);
		cout << " --- Benchmark Cycle Completed, Reshuffling --- " << endl;
	}
}
// 核心函数：按顺序提供调优实例
//void GetNextTuningInstance(int Seed)
//{
//	// 使用 static 变量来保存状态
//	static enum RacePhase { INIT, SMALL_INST, BIG_INST, RANDOM_INST };
//	static RacePhase currentPhase = RacePhase::INIT;
//
//	static vector<string> smallInstanceFiles;
//	static vector<string> bigInstanceFiles;
//	static int smallInstanceIndex = 0;
//	static int bigInstanceIndex = 0;
//
//	// 阶段 0: 初始化 (仅在第一次调用时运行)
//	if (currentPhase == RacePhase::INIT)
//	{
//		string smallPath = "data\\Small_Size_Instances";
//		string searchPathSmall = smallPath + "\\*.txt";
//		WIN32_FIND_DATAA fdSmall;
//		// 使用 main.cpp 中展示的文件搜索方法
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
//		string bigPath = "data\\Big_Size_Instances";
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
//		// 设置初始阶段
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
//	// 阶段 1: 顺序处理小实例
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
//	// 阶段 2: 顺序处理大实例
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
//	// 阶段 3: 随机生成 (使用特定重载)
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
//
//		int procTimeRule = 1 + rand() % 2;
//
//		GenerateInstances(InJob, InStage, Seed, machineRule, procTimeRule);
//		return;
//	}
//}

class Race
{
public:
	Race(int NumberOfNP_Integer, int NumberOfNP_Real, vector<int>& MaxForNP_Integer, vector<int>& MinForNP_Integer, vector<double>& MaxForNP_Real, vector<double>& MinForNP_Real, int NumberOfCP, vector<int>& KindsForCP);
	void run();
	void outputBest();

	vector<Configuration> CandidateConfigurations;
	vector<Configuration> EliteConfigurations;

public:
	int B;  //the total predefiend tuning budeget
	int minSurvival;   //the minimum number of surviving configurations
	int NumberOfElite;   //the number of Elite populations
	int usedB;  //the used budeget
	int usingB;  //the using budeget in the jth budget
	int CurrentIteration;  //the current number of iterations
	int NumberOfIterations;  //the number of iterations
	int NumberOfCandidateConfigurations;  //the number of candidate configurations in this iteration
	int NumberOfCalls;

public:
	int MSUpper;
	int MSLower;

public:
	int NumberOfNP_Integer;  //number of integer numerical parameters
	int NumberOfNP_Real;    //number of real numerical parameters
	int NumberOfCP;        //number of categorical parameters

public:
	vector<int> MaxForNP_Integer;  //parameter setting space for numerical parameters
	vector<int> MinForNP_Integer;

	vector<double> MaxForNP_Real;  //parameter setting space for numerical parameters
	vector<double> MinForNP_Real;

public:
	vector<int> KindsForCP;  //parameter setting space for categorical parameters


public:
	vector<double> stdForNP_Integer; //standard deviations for generating novel integer numerical parameter values
	vector<double> stdForNP_Real;  //standard deviations for generating novel real numerical parameter values
	vector<vector<double>> pForCP; //probability for generating novel categorical parameter values
public:

	fstream fffout;
};

Race::Race(int NumberOfNP_Integer, int NumberOfNP_Real, vector<int>& MaxForNP_Integer, vector<int>& MinForNP_Integer, vector<double>& MaxForNP_Real, vector<double>& MinForNP_Real, int NumberOfCP, vector<int>& KindsForCP)
{
	//赋值操作
	this->NumberOfNP_Integer = NumberOfNP_Integer;
	this->NumberOfNP_Real = NumberOfNP_Real;
	this->NumberOfCP = NumberOfCP;
	this->MaxForNP_Integer = MaxForNP_Integer;
	this->MinForNP_Integer = MinForNP_Integer;
	this->MaxForNP_Real = MaxForNP_Real;
	this->MinForNP_Real = MinForNP_Real;
	this->KindsForCP = KindsForCP;


	fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

	//the number of races (iterations)
	int NOP = NumberOfNP_Integer + NumberOfNP_Real + NumberOfCP;
	NumberOfIterations = (int)(2 + log(NOP) / log(2)); //the number of iterations depends on the number of parameters
	cout << "NumberOfIterations:" << NumberOfIterations << endl;
	fffout << "NumberOfIterations:" << NumberOfIterations << endl;

	//the current iteration
	CurrentIteration = 1;

	//the used budget
	usedB = 0;

	//the total predefiend number of budeget
	B = 8000;
	//B = 1000;

	fffout << "AAD Start: The total budget is:" << B << endl;
	cout << "AAD Start: The total budget is:" << B << endl;

	//the using budeget
	usingB = (B - usedB) / (NumberOfIterations - CurrentIteration + 1);

	cout << "The " << CurrentIteration << " iteration using budeget:" << usingB << endl;

	fffout << "The " << CurrentIteration << " iteration using budeget:" << usingB << endl;

	//the number of Candiate configurations
	NumberOfCandidateConfigurations = (int)(usingB / (10 + min(5, CurrentIteration)));  //改了要和后面统一

	cout << "The" << CurrentIteration << " iteration number of candidate configurations:" << NumberOfCandidateConfigurations << endl;
	fffout << "The" << CurrentIteration << " iteration number of candidate configurations:" << NumberOfCandidateConfigurations << endl;


	//Sample the candidate configurations randomly
	CandidateConfigurations.clear();


	for (int i = 0; i < NumberOfCandidateConfigurations; i++)
	{
		const unsigned int seed = i; // 可以修改为任意种子值
		std::mt19937 gen(i); // 用固定种子初始化随机数引擎

		//Configuration(int iniInd, int decodeInd, int aggreInd, int collaInd, int restartInd, int PopulationSize, int NeighborhoodSize, int P);


		//for the categorical parameters
		vector<int> tempForCP;
		tempForCP.resize(NumberOfCP);
		for (int j = 0; j < NumberOfCP; j++)
		{
			tempForCP[j] = rand() % KindsForCP[j] + 1;
		}

		//for the integer numerical parameters
		vector<int> tempForNP_Integer;
		tempForNP_Integer.resize(NumberOfNP_Integer);
		for (int j = 0; j < NumberOfNP_Integer; j++)
		{
			tempForNP_Integer[j] = MinForNP_Integer[j] + rand() % (MaxForNP_Integer[j] - MinForNP_Integer[j]);
		}

		//for the real numerical parameters
		vector<double> tempForNP_Real;

		int total_aad = 100;
		int n_aad = NumberOfNP_Real;

		// 生成 n-1 个随机断点，将 0~100 切割成 n 段
		std::vector<int> cuts;
		std::uniform_int_distribution<> dis(1, total_aad - 1); // 排除0和100
		while (cuts.size() < n_aad - 1) {
			int val = dis(gen);
			if (std::find(cuts.begin(), cuts.end(), val) == cuts.end()) {
				cuts.push_back(val);
			}
		}
		cuts.push_back(0);
		cuts.push_back(total_aad);
		std::sort(cuts.begin(), cuts.end());

		// 每段长度为一个值，除以100即为所需小数
		for (int i = 1; i <= n_aad; ++i) {
			double val = (cuts[i] - cuts[i - 1]) / 100.0;
			tempForNP_Real.push_back(val);
		}


		/*Configuration(vector<int> &ValueForNumericalParemeter_Integer, vector<double> &ValueForNumericalParemeter_Real, vector<int> &ValueForCategoricalParemeter,
			int ValueForNeighborhoodSize, int ValueForTournamentSize, double ValueForScalingFactor, int ValueForDecomposition, int ValueForIndicator);*/

		CandidateConfigurations.push_back(Configuration(tempForNP_Integer, tempForNP_Real, tempForCP));


	}


	minSurvival = 10;
	NumberOfElite = 5;

	//for the initial standard deviations for integer numerical parameters
	stdForNP_Integer.resize(NumberOfNP_Integer);
	for (int j = 0; j < NumberOfNP_Integer; j++)
	{
		stdForNP_Integer[j] = (MaxForNP_Integer[j] - MinForNP_Integer[j]) * 1.0 / 2;
	}

	//for the initial standard deviations for real numerical parameters
	stdForNP_Real.resize(NumberOfNP_Real);
	for (int j = 0; j < NumberOfNP_Real; j++)
	{
		stdForNP_Real[j] = (MaxForNP_Real[j] - MinForNP_Real[j]) * 1.0 / 2;
	}

	//for the initial probability values for the categorical parameters
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

//void Race::InputInstance(int NumberOfInstance)
//{
//	char InsFile[20];
//	sprintf_s(InsFile, "test\\%d.txt", NumberOfInstance);
//
//	fstream fin;
//
//	fin.open(InsFile, fstream::in);
//
//
//	fin >> MaxMS >> MinMS >> MaxNOS >> MinNOS;
//
//	fin >> pJob;
//	fin >> pStage;
//
//
//
//	pMachines.resize(pStage);
//
//	for (int i = 0; i < pStage; i++)
//	{
//		fin >> pMachines[i];
//	}
//
//
//	pUnitTime.resize(pStage);
//	for (int i = 0; i < pStage; i++)
//	{
//		pUnitTime[i].resize(pJob);
//	}
//	for (int i = 0; i < pStage; i++)
//	{
//		for (int j = 0; j < pJob; j++)
//		{
//			fin >> pUnitTime[i][j];
//		}
//	}
//
//
//
//	pLot.resize(pJob);
//	for (int j = 0; j < pJob; j++)
//	{
//		fin >> pLot[j];
//	}
//
//
//	pSetupTime.resize(pStage);
//	for (int i = 0; i < pStage; i++)
//	{
//		pSetupTime[i].resize(pJob);
//	}
//	for (int i = 0; i < pStage; i++)
//	{
//		for (int j = 0; j < pJob; j++)
//		{
//			fin >> pSetupTime[i][j];
//		}
//	}
//
//
//	pTransferTime.resize(pStage);
//	for (int i = 0; i < pStage; i++)
//	{
//		pTransferTime[i].resize(pJob);
//	}
//
//	for (int i = 0; i < pStage; i++)
//	{
//		for (int j = 0; j < pJob; j++)
//		{
//			fin >> pTransferTime[i][j];
//		}
//	}
//
//
//	fin.close();
//
//
//}

void Race::outputBest()
{

	fffout.open("EliteConfigurations.txt", fstream::out | fstream::app);


	for (int j = 0; j < EliteConfigurations.size(); j++)
	{
		//output the integer numerical parameter
		for (int i = 0; i < NumberOfNP_Integer; i++)
		{
			cout << EliteConfigurations[j].ValueForNumericalParemeter_Integer[i] << " ";
			fffout << EliteConfigurations[j].ValueForNumericalParemeter_Integer[i] << " ";
		}

		//output the real numerical parameter
		for (int i = 0; i < NumberOfNP_Real; i++)
		{
			cout << EliteConfigurations[j].ValueForNumericalParemeter_Real[i] << " ";
			fffout << EliteConfigurations[j].ValueForNumericalParemeter_Real[i] << " ";
		}

		//output the categorical parameter
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
	int NumberOfInstance = 1;  //这个需要更改

	while (usedB < B || CandidateConfigurations.size() > EliteConfigurations.size())
	{
		NumberOfCalls = 0;
		int NOII = 0; //total number of instances used in this race(iteration)

		//conduct the current iteration
		while ((NumberOfCalls + CandidateConfigurations.size()) <= usingB)
		{
			fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

			for (int k = 0; k < IndependentRuns; k++)
			{
				cout << "第" << CurrentIteration << " iteration" << "测试第" << NumberOfInstance << "实例" << endl;
				fffout << "第" << CurrentIteration << " iteration" << "测试第" << NumberOfInstance << "实例" << endl;

				//Generate the test instance
				int f = rand() % 6;
				int c = rand() % 4;

				int InJob = Jobs[f];
				int InStage = Stages[c];
				//GenerateInstances(InJob, InStage, NumberOfInstance + 1234);
				//Jose实例随机生成
				//int machineRule = 1 + rand() % 3;  // 随机生成 1, 2, 或 3
				//int procTimeRule = 1 + rand() % 2;
				//GenerateInstances(InJob, InStage, machineRule, procTimeRule, NumberOfInstance + 1234);
				//GenerateInstances(NumberOfInstance + 1234);
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

                FAS_CCH* cch = new FAS_CCH(SPTB_weight, AfterBN_weight, Total_weight,
						Current_weight);


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
					//fffout << "costTime:" << costTime << endl;

					cout << "CandidateConfigurations[" << i << "]=" << CandidateConfigurations[i].CostValues[k] << endl;


					delete cch;
				}


				//Max-min method 
				for (int i = 0; i < CandidateConfigurations.size(); i++)
				{
					CandidateConfigurations[i].CostValues[k] = (CandidateConfigurations[i].CostValues[k] - LowerBound) * 1.0 / (UpperBound - LowerBound);
				}

				//get the average and rank values
				for (int i = 0; i < CandidateConfigurations.size(); i++)
				{
					CandidateConfigurations[i].getAvgValue();
				}

				NumberOfInstance++;
			}

			NumberOfCalls += CandidateConfigurations.size();
			usedB += CandidateConfigurations.size();

			fffout << "The" << CurrentIteration << " iteration used budeget:" << NumberOfCalls << endl;

			//rank the configurations

			//identify the statistically bad configurations

			if (ranksumInitialize())
			{
				cout << "Matalab的DLL初始化成功！" << endl;
			}
			else
			{
				cout << "Matalab的DLL初始化失败！" << endl;
			}

			for (int i = 0; i < CandidateConfigurations.size(); i++)
			{
				for (int j = 0; j < CandidateConfigurations.size(); j++)
				{
					if (i != j)
					{
						mwArray x(1, IndependentRuns, mxDOUBLE_CLASS);
						mwArray y(1, IndependentRuns, mxDOUBLE_CLASS);

						x.SetData(CandidateConfigurations[i].CostValues, IndependentRuns);
						y.SetData(CandidateConfigurations[j].CostValues, IndependentRuns);

						//cout << "x:" << x << endl;   //调试用
						//cout << "y:" << y << endl;   //调试用

						mwArray p(1, 1, mxDOUBLE_CLASS);
						mwArray h(1, 1, mxLOGICAL_CLASS);

						myranksum(1, p, h, x, y);
						double xx = p.Get(1, 1);
						bool xxx = h.Get(1, 1);

						//cout << "xx:" << xx << " xxx:" << xxx << endl;

						if (xx < 0.05)
						{
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

			//测试用 
			int NumberOfPromisingSolutions = 0;
			for (int i = 0; i < CandidateConfigurations.size(); i++)
			{
				if (CandidateConfigurations[i].IsWorse == false)
				{
					/*cout << "第 " << CurrentIteration << " Iteration测试 " << NumberOfInstance << " 实例：" << "Promising configuration一个" << endl;
					fffout << "第 " << CurrentIteration << " Iteration测试 " << NumberOfInstance << " 实例：" << "Promising configuration一个" << endl;*/
					NumberOfPromisingSolutions++;
				}
			}
			cout << "第 " << CurrentIteration << " Iteration测试 " << NumberOfInstance << " 实例后总共有：" << NumberOfPromisingSolutions << " 个Promising Solutions" << endl;
			fffout << "第 " << CurrentIteration << " Iteration测试 " << NumberOfInstance << " 实例后总共有：" << NumberOfPromisingSolutions << " 个Promising Solutions" << endl;

			ranksumTerminate();

			sort(CandidateConfigurations.begin(), CandidateConfigurations.end(), SortConfiguration());
			//remove the statistically bad configurations
			int NumberOfRemove = 0;
			int UpperNumber = CandidateConfigurations.size() - minSurvival;

			for (int i = CandidateConfigurations.size() - 1; i >= 0; i--)
			{
				if (CandidateConfigurations[i].IsWorse == true)
				{

					/*Configuration(vector<int> &ValueForNumericalParemeter_Integer, vector<double> &ValueForNumericalParemeter_Real, vector<int> &ValueForCategoricalParemeter,
						int ValueForNeighborhoodSize, int ValueForTournamentSize, double ValueForScalingFactor, int ValueForDecomposition, int ValueForIndicator);*/
						//将worse的配置删除
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
				//NumberOfInstance++;
				break;
			}
			else
			{
				//NumberOfInstance++;
				fffout.close();
			}

		}

		//at the end of a race
		fffout.open("EvolutionaryInformation.txt", fstream::out | fstream::app);

		//for constructing the elite configurations
		//sort the configurations according to the rankIndex
		for (int i = 0; i < CandidateConfigurations.size(); i++)
		{
			CandidateConfigurations[i].rankIndex = CandidateConfigurations[i].rankIndex / NOII;
		}
		sort(CandidateConfigurations.begin(), CandidateConfigurations.end(), SortConfigurationRankIndex());


		EliteConfigurations.clear();  //clear the elite configurations


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

		//for conducting the next iteration
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
		NumberOfCandidateConfigurations = (int)(usingB / (1.5 + min(5, CurrentIteration)));  //改动之后需要和前面统一
		cout << "第" << CurrentIteration << " iteration的number of candidate configurations:" << NumberOfCandidateConfigurations << endl;
		fffout << "第" << CurrentIteration << " iteration的number of candidate configurations:" << NumberOfCandidateConfigurations << endl;

		//according to the elite configurations to sample the candidate configurations


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

		//assure the probabilities of the elite configurations to be selected according to the rankingIndex
		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			EliteConfigurations[i].probability = (EliteConfigurations.size() - (i + 1) + 1) * 1.0 / (EliteConfigurations.size() * (EliteConfigurations.size() + 1) / 2);
		}

		//测试用：
		double sumProbability = 0.0;
		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			fffout << "EliteConfigurations[" << i << "]:" << EliteConfigurations[i].probability << endl;
			sumProbability += EliteConfigurations[i].probability;
		}
		fffout << "总的概率为：" << sumProbability << endl;

		CandidateConfigurations.clear();

		//generate the novel configurations
		for (int i = 0; i < (NumberOfCandidateConfigurations - EliteConfigurations.size()); i++)
		{
			//select the configurations as the parent configuration
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

			//调试用
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

			//to sample the categorical method to the new probability

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



			//Configuration(vector<int> &ValueForNumericalParemeter_Integer, vector<double> &ValueForNumericalParemeter_Real, vector<int> &ValueForCategoricalParemeter);


			CandidateConfigurations.push_back(Configuration(ValueForNumericalParemeter_Integer, ValueForNumericalParemeter_Real, ValueForCategoricalParemeter));


		}



		for (int i = 0; i < EliteConfigurations.size(); i++)
		{
			EliteConfigurations[i].probability = 0.0;
			CandidateConfigurations.push_back(EliteConfigurations[i]);
		}

		//update the pForCP for the next iteration according to the elite configurations
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

