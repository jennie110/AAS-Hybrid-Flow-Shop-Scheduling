#pragma once

// 必须先包含 Windows.h，避免与 std::byte 冲突
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include<vector>
#include<iostream>
#include<fstream>
#include<algorithm>
#include <numeric>
using namespace std;


//小规模  
int Jobs[6] = { 10,15,20,25,30,35 };
int Stages[4] = { 5,10,15,20};

int Jobs1[6] = { 40,80,120,160,200,240 };
int Stages1[4] = {  5, 10, 15 ,20 };


int pJob;
int pStage;
extern int gBottleneckStage;

vector<int> pMachines;   //每阶段的机器数                    Stages
vector<vector<int>> pUnitTime;  //每阶段的单位加工时间        Stages-Jobs
vector<vector<int>> pSetupTime; //批次在各阶段上的加工时间    Stages-Jobs
vector<vector<int>> pTransferTime;  //子批在相邻阶段的转移时间 Stages-Jobs
vector<int> JobCurrentStage(pJob, 0);//记录每个工件当前所处的阶段

const int IndependentRuns = 10;

void GenerateInstances(int InJob, int InStage, int Seed, int machineRule, int procTimeRule)//Jose规则生成实例
{
	srand(Seed);

	pJob = InJob;
	pStage = InStage;

	// 机器数量
	pMachines.resize(pStage);
	if (machineRule == 1) // M1: All stages 3 machines, except one random stage with 2
	{
		fill(pMachines.begin(), pMachines.end(), 3); // 先全部设为 3
		if (pStage > 0) // 确保至少有一个阶段
		{
			int randomStageIndex = rand() % pStage; // 随机选一个阶段索引
			pMachines[randomStageIndex] = 2;        // 将该阶段机器数设为 2
		}
	}
	else if (machineRule == 2) // M2: All stages 3 machines
	{
		fill(pMachines.begin(), pMachines.end(), 3); // 全部设为 3
	}
	else // M3 (Default or if machineRule == 3): Uniform distribution U[1, 3]
	{
		for (int i = 0; i < pStage; i++)
		{
			pMachines[i] = 1 + rand() % 3; // 生成 1, 2, 或 3
		}
	}

	//加工时间
	pUnitTime.assign(pStage, vector<int>(pJob)); // 使用 assign 调整大小并初始化
	for (int i = 0; i < pStage; i++)
	{
		for (int j = 0; j < pJob; j++)
		{
			if (procTimeRule == 2) // P2: Uniform distribution U[1, 40 * m_i]
			{
				// 确保 m_i (即 pMachines[i]) 至少为 1，避免 rand() % 0 或负数
				int upperBound = 40 * max(1, pMachines[i]);
				if (upperBound < 1) upperBound = 1; // 再次确保上限至少为 1
				// 生成 1 到 upperBound 之间的随机整数
				pUnitTime[i][j] = 1 + rand() % upperBound;
			}
			else // P1 (Default or if procTimeRule == 1): Uniform distribution U[1, 99]
			{
				// 生成 1 到 99 之间的随机整数
				pUnitTime[i][j] = 1 + rand() % 99;
			}
		}
	}

	// 准备时间和转移时间 (保持为 0)
	pSetupTime.assign(pStage, vector<int>(pJob, 0));
	pTransferTime.assign(pStage, vector<int>(pJob, 0));
}
void GenerateInstances(int InJob, int InStage, int Seed)
{
	srand(Seed);

	pJob = InJob;
	pStage = InStage;


	pMachines.resize(pStage);
	for (int i = 0; i < pStage; i++)
	{
		pMachines[i] = rand() % 3 + 1;  //rand() % 5 +1
		//pMachines[i] = 2;
	}

	pUnitTime.resize(InStage);
	for (int i = 0; i < pStage; i++)
	{
		pUnitTime[i].resize(pJob);
	}
	for (int i = 0; i < pStage; i++)
	{
		for (int j = 0; j < pJob; j++)
		{
			pUnitTime[i][j] = 1 + rand() % 99;  //1 + rand() % 10 3
		}
	}

	pSetupTime.resize(pStage);
	for (int i = 0; i < pStage; i++)
	{
		pSetupTime[i].resize(pJob);
	}
	for (int i = 0; i < pStage; i++)
	{
		for (int j = 0; j < pJob; j++)
		{
			pSetupTime[i][j] = 0;  //    50 + rand() % 51  10 11
		}
	}


	pTransferTime.resize(pStage);
	for (int i = 0; i < pStage; i++)
	{
		pTransferTime[i].resize(pJob);
	}

	for (int i = 0; i < pStage; i++)
	{
		for (int j = 0; j < pJob; j++)
		{
			pTransferTime[i][j] = 0; //10 + rand() %11   5  6
		}
	}

}
void GenerateInstances(int Seed)
{
	//  获取目录下所有 .txt 文件列表
	string directoryPath = "Jose_benchmark\\Small_Size_Instances";
	vector<string> instanceFiles;
	string searchPath = directoryPath + "\\*.txt";
	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);

	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			instanceFiles.push_back(directoryPath + "\\" + fd.cFileName);
		}
	} while (FindNextFileA(hFind, &fd) != 0);

	FindClose(hFind);

	if (instanceFiles.empty()) {
		cerr << "错误：目录中没有找到 .txt 实例文件: " << directoryPath << endl;
		return;
	}

	// 2. 使用 Seed 随机选择一个文件
	srand(Seed); // 使用传入的 Seed 初始化随机数生成器
	int randomIndex = rand() % instanceFiles.size();
	string selectedFilePath = instanceFiles[randomIndex];

	// 3. 读取选定文件的数据
	ifstream fin(selectedFilePath);
	if (!fin.is_open()) {
		cerr << "错误：无法打开选定的实例文件: " << selectedFilePath << endl;
		return;
	}

	// 读取 pJob 和 pStage
	if (!(fin >> pJob >> pStage)) {
		cerr << "错误：从文件读取 pJob 和 pStage 失败: " << selectedFilePath << endl;
		fin.close();
		return;
	}

	// 调整大小并读取各阶段机器数
	pMachines.resize(pStage);
	for (int k = 0; k < pStage; ++k) {
		if (!(fin >> pMachines[k])) {
			cerr << "错误：从文件读取阶段 " << k << " 的机器数量失败: " << selectedFilePath << endl;
			fin.close();
			return;
		}
		if (pMachines[k] <= 0) {
			cerr << "错误：阶段 " << k << " 的机器数量无效 (" << pMachines[k] << "): " << selectedFilePath << endl;
			fin.close();
			return;
		}
	}

	// 调整大小并读取加工时间
	pUnitTime.assign(pStage, vector<int>(pJob));
	for (int k = 0; k < pStage; ++k) {
		for (int j = 0; j < pJob; ++j) {
			if (!(fin >> pUnitTime[k][j])) {
				cerr << "错误：从文件读取阶段 " << k << " 工件 " << j << " 的加工时间失败: " << selectedFilePath << endl;
				fin.close();
				return;
			}
			if (pUnitTime[k][j] < 0) {
				cerr << "错误：阶段 " << k << " 工件 " << j << " 的加工时间无效 (" << pUnitTime[k][j] << "): " << selectedFilePath << endl;
				fin.close();
				return;
			}
		}
	}

	// 初始化准备时间和转移时间为 0
	pSetupTime.assign(pStage, vector<int>(pJob, 0));
	pTransferTime.assign(pStage, vector<int>(pJob, 0));

	fin.close();
}
void OutputInstances(int InJob, int InStage, int InType)
{
	char InsFile[80];
	sprintf_s(InsFile, "Large_Instances\\data%d_%d_%d.dat", InJob, InStage, InType);

	fstream fout(InsFile, fstream::out | fstream::app);



	int NumberOfMachines = 0;

	for (int i = 0; i < pStage; i++)
	{
		NumberOfMachines += pMachines[i];
	}



	fout << "nStages = " << pStage << ";" << endl;
	fout << "nJobs = " << pJob << ";" << endl;
	fout << "nMachines = " << NumberOfMachines << ";" << endl;

	fout << "x=[" << endl;


	int cStage;
	for (int j = 1; j <= NumberOfMachines; j++)
	{
		//根据j判断cStage
		int cNumber = 0;
		int cNumber1;
		for (int i = 0; i < pStage; i++)
		{
			cNumber += pMachines[i];
			cNumber1 = cNumber - pMachines[i];

			if (j >= cNumber1 && j <= cNumber)
			{
				cStage = i;
				break;
			}
		}




		fout << "[";



		for (int i = 0; i < pStage; i++)
		{
			if (i == cStage)
			{
				fout << 1 << ",";
			}
			else
			{
				fout << 0 << ",";
			}
		}
		fout << "]," << endl;
	}


	fout << "];" << endl;


	fout << "UnitTime=[" << endl;
	for (int i = 0; i < pStage; i++)
	{
		fout << "[";
		for (int j = 0; j < pJob; j++)
		{
			fout << pUnitTime[i][j] << ",";
		}
		fout << "]," << endl;
	}
	fout << "];" << endl;

	fout.close();
}

void OutputInstancesName(int InJob, int InStage, int InType)
{
	char InsFile[80];
	sprintf_s(InsFile, "FileName.txt");

	fstream fout(InsFile, fstream::out | fstream::app);

	fout << "\"data" << InJob << InStage << InType << ".dat\",";


	fout.close();

}


class PairX
{
public:
	int dim;
	int value;
};

class PairXLess
{
public:
	bool operator () (PairX& a1, PairX& a2)
	{
		return a1.value < a2.value;
	}
};

class PairXGreater
{
public:
	bool operator () (PairX& a1, PairX& a2)
	{
		return a1.value > a2.value;
	}
};



class CriticalStructure
{
public:
	CriticalStructure(int _Stage, int _Lot)
	{
		this->Stage = _Stage;
		this->Critical_Lot = _Lot;
	}
public:
	int Stage;
	int Critical_Lot;

};
