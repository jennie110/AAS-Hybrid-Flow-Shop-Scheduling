#pragma once

#include <vector>
#include <algorithm>
#include "HFSP_CS.h"
#include "LotCompare.h"
#include "JobOperation.h"


using namespace std;

struct CriticalBlockRange {
	int stage, machine;
	int start_pos, end_pos;
};

struct MandatoryOp {
	int stage;
	int job;
	int machine;
	int pos;
	MandatoryOp(int _stage, int _job, int _machine, int _pos)
		: stage(_stage), job(_job), machine(_machine), pos(_pos) {}
};


class FAS_CCH
{
public:
	/*FAS_CCH(double _LPT_weight, double _SPT_weight, double _LWR_weight,
		double _MWR_weight, double _EPF_weight, double _LPF_weight);*/
	FAS_CCH(double _SPTB_weight, double _AfterBN_weight, double _Total_weight,
		double _Current_weight,double _lastjob_weight);
public:
	double SPTB_weight;
	double AfterBN_weight;
	double Total_weight;
	double Current_weight;
	// double lastjob_weight;
	double est_weight;

	double Max_SPTB, Min_SPTB;
	double Max_AfterBN, Min_AfterBN;
	double Max_Total, Min_Total;
	double Max_Current, Min_Current;
	// double Max_lastjob, Min_lastjob;
	double Max_est, Min_est;

public:
	vector<JobOperation> AS;
	vector<vector<vector<int>>> MachineAssignment;
	vector<vector<int>> STime;
	vector<vector<int>>CTime;
	//vector<vector<int>> LotSplit;


	vector<vector<int>> STime_back;  //backward decoding
	vector<vector<int>> CTime_back;  //backward decoding

	vector<CriticalStructure> Critical_Operations;
	vector<vector<vector<int>>> Critical_Block;
	vector<CriticalBlockRange> CriticalBlocks;
	vector<MandatoryOp> Mandatory_Ops;
	vector<double> StageRemainingLoad;//各阶段剩余负载
	vector<vector<double>> CachedTails;
public:
	int current_Job;    //将要被调度的工件
	int current_Stage;  //将要被调度的阶段
	int operation_position;  //当前current_Job和current_Stage在AS中的位置

public:
	void run();
	void LeftInsert();  //执行Left insert
	void LeftShift_Construct();  //SAS构造版本 ，只追加到末尾，不插入
	void RightInsert();   //执行Right insert
	//利用MachineAssignment在当前阶段tempStage，执行LeftShift
	void LeftShift(int& tempStage, vector<vector<vector<int>>>& tempMachineAssignment, vector<vector<int>>& tempSTime, vector<vector<int>>& tempCTime);
	void LeftShift();
	void RightShift();
	void FAS_Repair(int& tempStage, vector<vector<vector<int>>>& tempMachineAssignment, bool& IsImpro);  //基于lot sequence调整的Destruction and repair

	void Update_Current_Remaining_time();

	void Backward_Decoding();
	void GenerateCriticalBlockRanges();
	void FindMandatoryOperations();
	void MandatoryLocalSearch(bool& IsImpro);

	void Ns1_ReverseEdge(bool& IsImpro);
	void Ns2_MoveInnerToEdge(bool& IsImpro);
	void Ns3_ReinsertEdge(bool& IsImpro);
	void Nc_CrossMachineInsert(bool& IsImpro);

	void OutputWholeSchedule(int instanceIndex, int _run_index);
	void OutputWholeSchedule(int _run_index);
	//void OutputWholeSchedule_temp(vector<vector<int>> &tempSTime, vector<vector<int>> &tempCTime, vector<vector<vector<int>>> &tempMachineAssignment);

	//void OutputWholeSchedule_back();

	void LotSeqDestructAndRepair(bool& IsImpro);

	void ClassicalLotSeqDestructAndRepair(bool& IsImpro);
	void Pair_Swap(bool& IsImpro);
	void Inversion(bool& IsImpro);
	void Displacement(bool& IsImpro);
	void UpdateDynamicBottleneck(int totalStages, int totalJobs, const vector<int>& JobCurrentStage);
	
	void LeftInsert_Global();
	void RightInsert_Global();
	void reverse_schedule();
	void reverse_problem();
	void reinitialize();
	void run_with_reversibility();
	
	vector<int> backup_pMachines;
	vector<vector<int>> backup_pUnitTime;
	vector<vector<int>> backup_pSetupTime;
	vector<vector<int>> backup_pTransferTime;
	int backup_gBottleneckStage;
	
	int makespan;
};


void FAS_CCH::OutputWholeSchedule(int instanceIndex, int _run_index)//指定实例调度
{
	//for scheudle Gantt
	char c[128];
	sprintf_s(c, "results//Jose//%d_%d_%d_%d_schedule.txt", pJob, pStage, instanceIndex, _run_index + 1);
	fstream fout(c, fstream::out);


	//output the schdule
	fout << "cJob = " << pJob << ";" << endl;
	fout << "cStage = " << pStage << ";" << endl;

	fout << "PMachine = [";//输出每个阶段的机器数
	for (int k = 0; k < pStage; k++)
	{
		fout << pMachines[k];
		if (k == pStage - 1)
		{
			fout << "";
		}
		else
		{
			fout << ",";
		}
	}
	fout << "];" << endl;


	for (int k = 0; k < pStage; k++)
	{
		fout << "STime" << k + 1 << "=[";
		for (int j = 0; j < pJob; j++)
		{
			fout << STime[k][j] << ",";
		}
		fout << "];" << endl;
	}

	for (int k = 0; k < pStage; k++)
	{
		fout << "CTime" << k + 1 << "=[";
		for (int j = 0; j < pJob; j++)
		{
			fout << CTime[k][j] << ",";
		}
		fout << "];" << endl;
	}

	for (int k = 0; k < pStage; k++)
	{
		fout << "chromSMJ" << k + 1 << "=[";

		for (int i = 0; i < pMachines[k]; i++)
		{
			for (int j = 0; j < MachineAssignment[k][i].size(); j++)
			{
				fout << MachineAssignment[k][i][j] + 1 << ",";
				if (j == pJob - 1)
				{
					fout << ";" << endl;
				}
			}
			for (int j = MachineAssignment[k][i].size(); j < pJob; j++)//用1000填充空余部分
			{
				fout << 1000;
				if (j == pJob - 1)
				{
					fout << ";" << endl;
				}
				else
				{
					fout << ",";
				}
			}

		}

		for (int i = pMachines[k]; i < 5; i++)
		{
			for (int j = 0; j < pJob; j++)
			{
				fout << 1000;
				if (j == pJob - 1)
				{
					fout << ";" << endl;
				}
				else
				{
					fout << ",";
				}
			}
		}

		fout << "];" << endl;
	}

	fout << "UnitTime = [" << endl;
	for (int k = 0; k < pStage; k++)
	{
		fout << "[";
		for (int j = 0; j < pJob; j++)
		{
			fout << pUnitTime[k][j] << ",";
		}
		fout << "]," << endl;
	}
	fout << "];" << endl;

	fout << "SetupTime = [";
	for (int k = 0; k < pStage; k++)
	{
		for (int j = 0; j < pJob; j++)
		{
			fout << pSetupTime[k][j] << ",";
		}
		fout << ";" << endl;
	}

	fout << "];" << endl;

	fout.close();

}

void FAS_CCH::OutputWholeSchedule(int _run_index)//随机实例调度
{
	//for scheudle Gantt
	char c[128];
	sprintf_s(c, "results//HFSP//%d_%d_%d_schedule.txt", pJob, pStage, _run_index + 1);
	fstream fout(c, fstream::out);


	//output the schdule
	fout << "cJob = " << pJob << ";" << endl;
	fout << "cStage = " << pStage << ";" << endl;

	fout << "PMachine = [";//输出每个阶段的机器数
	for (int k = 0; k < pStage; k++)
	{
		fout << pMachines[k];
		if (k == pStage - 1)
		{
			fout << "";
		}
		else
		{
			fout << ",";
		}
	}
	fout << "];" << endl;


	for (int k = 0; k < pStage; k++)
	{
		fout << "STime" << k + 1 << "=[";
		for (int j = 0; j < pJob; j++)
		{
			fout << STime[k][j] << ",";
		}
		fout << "];" << endl;
	}

	for (int k = 0; k < pStage; k++)
	{
		fout << "CTime" << k + 1 << "=[";
		for (int j = 0; j < pJob; j++)
		{
			fout << CTime[k][j] << ",";
		}
		fout << "];" << endl;
	}

	for (int k = 0; k < pStage; k++)
	{
		fout << "chromSMJ" << k + 1 << "=[";

		for (int i = 0; i < pMachines[k]; i++)
		{
			for (int j = 0; j < MachineAssignment[k][i].size(); j++)
			{
				fout << MachineAssignment[k][i][j] + 1 << ",";
				if (j == pJob - 1)
				{
					fout << ";" << endl;
				}
			}
			for (int j = MachineAssignment[k][i].size(); j < pJob; j++)//用1000填充空余部分
			{
				fout << 1000;
				if (j == pJob - 1)
				{
					fout << ";" << endl;
				}
				else
				{
					fout << ",";
				}
			}

		}

		for (int i = pMachines[k]; i < 5; i++)
		{
			for (int j = 0; j < pJob; j++)
			{
				fout << 1000;
				if (j == pJob - 1)
				{
					fout << ";" << endl;
				}
				else
				{
					fout << ",";
				}
			}
		}

		fout << "];" << endl;
	}

	fout << "UnitTime = [" << endl;
	for (int k = 0; k < pStage; k++)
	{
		fout << "[";
		for (int j = 0; j < pJob; j++)
		{
			fout << pUnitTime[k][j] << ",";
		}
		fout << "]," << endl;
	}
	fout << "];" << endl;

	fout << "SetupTime = [";
	for (int k = 0; k < pStage; k++)
	{
		for (int j = 0; j < pJob; j++)
		{
			fout << pSetupTime[k][j] << ",";
		}
		fout << ";" << endl;
	}

	fout << "];" << endl;

	fout.close();

}
void FAS_CCH::LotSeqDestructAndRepair(bool& IsImpro)
{
	//基于Critical_Block进行操作

	//先随机地选取一个阶段
	int tempStage;
	//再随机地选取一台机器
	int tempMachine;
	//再随机地从Critical_Block中选择一个Lot
	int tempSize;

	do
	{
		tempStage = rand() % pStage;
		//再随机地选取一台机器
		tempMachine = rand() % pMachines[tempStage];
		//再随机地从Critical_Block中选择一个Lot
		tempSize = Critical_Block[tempStage][tempMachine].size();
	} while (tempSize == 0);

	int tempIndex = rand() % tempSize;

	int tempLot = Critical_Block[tempStage][tempMachine][tempIndex];

	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;

	//将tempLot在tempStage中tempMachine中删除
	vector<int>::iterator it = find(tempMachineAssignment[tempStage][tempMachine].begin(), tempMachineAssignment[tempStage][tempMachine].end(), tempLot);

	tempMachineAssignment[tempStage][tempMachine].erase(it);


	//接下来  将tempLot插入到tempStage同机器或不同机器的随机产生的位置上

	//cout << "tempStage:" << tempStage << " " << "tempLot:" << tempLot << endl;

	int rand_no = rand() % 2;

	if (rand_no == 0 && pMachines[tempStage] > 1)
	{
		//插入到不同机器
		int machine_index;
		machine_index = rand() % pMachines[tempStage];

		//将tempLot插入到一个随机的位置
		int temp_temp_index = tempMachineAssignment[tempStage][machine_index].size();

		vector<int>::iterator it = tempMachineAssignment[tempStage][machine_index].begin();

		if (temp_temp_index != 0)
		{
			tempMachineAssignment[tempStage][machine_index].insert(it + rand() % temp_temp_index, tempLot);
		}
		else
		{
			tempMachineAssignment[tempStage][machine_index].push_back(tempLot);
		}
	}
	else
	{
		//插入到同机器
		int temp_temp_index = tempMachineAssignment[tempStage][tempMachine].size();

		//将tempLot插入到一个随机的位置
		vector<int>::iterator it = tempMachineAssignment[tempStage][tempMachine].begin();

		if (temp_temp_index != 0)
		{
			tempMachineAssignment[tempStage][tempMachine].insert(it + rand() % temp_temp_index, tempLot);
		}
		else
		{
			tempMachineAssignment[tempStage][tempMachine].push_back(tempLot);
		}
	}


	//执行修复调度
	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
}

void FAS_CCH::ClassicalLotSeqDestructAndRepair(bool& IsImpro)
{
	//基于Critical_Block进行操作

	//先随机地选取一个阶段
	int tempStage;
	//再随机地选取一台机器
	int tempMachine;
	//再随机地从Critical_Block中选择一个Lot
	int tempSize;

	do
	{
		tempStage = rand() % pStage;
		//再随机地选取一台机器
		tempMachine = rand() % pMachines[tempStage];
		//再随机地从Critical_Block中选择一个Lot
		tempSize = MachineAssignment[tempStage][tempMachine].size();
	} while (tempSize == 0);

	int tempIndex = rand() % tempSize;

	int tempLot = MachineAssignment[tempStage][tempMachine][tempIndex];

	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;

	//将tempLot在tempStage中tempMachine中删除
	vector<int>::iterator it = find(tempMachineAssignment[tempStage][tempMachine].begin(), tempMachineAssignment[tempStage][tempMachine].end(), tempLot);

	tempMachineAssignment[tempStage][tempMachine].erase(it);


	//接下来  将tempLot插入到tempStage同机器或不同机器的随机产生的位置上

	//cout << "tempStage:" << tempStage << " " << "tempLot:" << tempLot << endl;

	int rand_no = rand() % 2;

	if (rand_no == 0 && pMachines[tempStage] > 1)
	{
		//插入到不同机器
		int machine_index;
		machine_index = rand() % pMachines[tempStage];

		//将tempLot插入到一个随机的位置
		int temp_temp_index = tempMachineAssignment[tempStage][machine_index].size();

		vector<int>::iterator it = tempMachineAssignment[tempStage][machine_index].begin();

		if (temp_temp_index != 0)
		{
			tempMachineAssignment[tempStage][machine_index].insert(it + rand() % temp_temp_index, tempLot);
		}
		else
		{
			tempMachineAssignment[tempStage][machine_index].push_back(tempLot);
		}
	}
	else
	{
		//插入到同机器
		int temp_temp_index = tempMachineAssignment[tempStage][tempMachine].size();

		//将tempLot插入到一个随机的位置
		vector<int>::iterator it = tempMachineAssignment[tempStage][tempMachine].begin();

		if (temp_temp_index != 0)
		{
			tempMachineAssignment[tempStage][tempMachine].insert(it + rand() % temp_temp_index, tempLot);
		}
		else
		{
			tempMachineAssignment[tempStage][tempMachine].push_back(tempLot);
		}
	}


	//执行修复调度
	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);

}
FAS_CCH::FAS_CCH(double _SPTB_weight, double _AfterBN_weight, double _Total_weight,
	double _Current_weight,double _est_weight)
{
	//接收权重
	this->SPTB_weight = _SPTB_weight;
	this->AfterBN_weight = _AfterBN_weight;
	this->Total_weight = _Total_weight;
	this->Current_weight = _Current_weight;
	// this->lastjob_weight = _lastjob_weight;
	this->est_weight=_est_weight;
	// 重置 JobCurrentStage 全局变量（每个新实例需要重新初始化）
	JobCurrentStage.assign(pJob, 0);

	//计算瓶颈阶段 
	// 总负荷 / 机器数，最大的那个阶段即为瓶颈
	double max_load_ratio = -1.0;
	gBottleneckStage = 0; // 默认为0，防止未初始化
	for (int k = 0; k < pStage; k++)
	{
		double stage_total_time = 0;
		for (int j = 0; j < pJob; j++)
		{
			stage_total_time += pUnitTime[k][j];
		}
		double load_ratio = stage_total_time / pMachines[k]; // 负荷比率
		if (load_ratio > max_load_ratio)
		{
			max_load_ratio = load_ratio;
			gBottleneckStage = k;
		}
	}
	// StageRemainingLoad.resize(pStage, 0.0);
	// for(int i=0;i<pStage;i++)
	// {
	// 	for(int j=0;j<pJob;j++)
	// 	{
	// 		StageRemainingLoad[i]+=pUnitTime[i][j];
	// 	}
	// }

	// CachedTails.assign(pStage, vector<double>(pJob, 0.0));
	// for (int j = 0; j < pJob; j++)
	// {
	// 	double running_tail = 0.0;
	// 	// 倒序遍历阶段
	// 	for (int k = pStage - 1; k >= 0; k--)
	// 	{
	// 		// 对于阶段 k，它的 tail 是阶段 k+1 到最后的和
	// 		// 即上一次循环积累的 running_tail
	// 		CachedTails[k][j] = running_tail;
			
	// 		// 更新 running_tail，把当前阶段的时间加进去，供前一个阶段使用
	// 		running_tail += pUnitTime[k][j];
	// 	}
	// }
	AS.clear();
	for (int i = 0; i < pJob; i++)
	{
		// JobOperation 构造函数会自动计算 raw_SPTB 等物理值
		AS.push_back(JobOperation(0, i));
	}

	// 初始化STime, CTime, MachineAssignment
	STime.resize(pStage);
	for (int k = 0; k < pStage; k++)
	{
		STime[k].resize(pJob);
	}
	CTime = STime;

	STime_back.resize(pStage);
	for (int k = 0; k < pStage; k++)
	{
		STime_back[k].resize(pJob);
	}
	CTime_back = STime_back;

	MachineAssignment.resize(pStage);
	for (int i = 0; i < pStage; i++)
	{
		MachineAssignment[i].resize(pMachines[i]);
	}

	// Initialize the Max and Min values for NEW features
	// 调用 Update 函数来初始化所有4个特征的极值
	Update_Current_Remaining_time();

	// Initialize the priority value 
	for (int k = 0; k < AS.size(); k++)
	{
		AS[k].get_rule_value(Max_SPTB, Min_SPTB,
			Max_AfterBN, Min_AfterBN,
			Max_Total, Min_Total,
			Max_Current, Min_Current,
			Max_est, Min_est);
	}

	//  Initialize the combinaton value
	for (int k = 0; k < AS.size(); k++)
	{
		AS[k].get_combination_value(SPTB_weight, AfterBN_weight, Total_weight,
			Current_weight,est_weight);
	}

	//get the current job and stage 
	double Max_value = -1e9; // 设为一个极小值
	for (int k = 0; k < AS.size(); k++)
	{
		if (AS[k].combination_value > Max_value)
		{
			Max_value = AS[k].combination_value;
			current_Job = AS[k].cJob;
			current_Stage = AS[k].cStage;
			operation_position = k;
		}
	}
}

void FAS_CCH::Update_Current_Remaining_time()
{
	//保存旧瓶颈，更新动态瓶颈
	int oldBottleneck = gBottleneckStage;
	UpdateDynamicBottleneck(pStage, pJob, JobCurrentStage);

	// 如果瓶颈变了，刷新 AS 中所有操作的 raw_SPTB 和 raw_AfterBN
	if (gBottleneckStage != oldBottleneck)
	{
		for (int i = 0; i < AS.size(); i++)
		{
			AS[i].RefreshSPTB_AfterBN();
		}
	}
	
	// 重置 Max 和 Min 值
	Max_SPTB = -1e9; Min_SPTB = INT_MAX;
	Max_AfterBN = -1e9; Min_AfterBN = INT_MAX;
	Max_Total = -1e9; Min_Total = INT_MAX;
	Max_Current = -1e9; Min_Current = INT_MAX;
	// Max_lastjob = -1e9; Min_lastjob = INT_MAX;
	Max_est=-1e9; Min_est=INT_MAX;

	// 遍历当前候选集 AS，找到每一项特征的极值
	for (int i = 0; i < AS.size(); i++)
	{
		// 1. SPTB
		if (AS[i].raw_SPTB > Max_SPTB) Max_SPTB = AS[i].raw_SPTB;
		if (AS[i].raw_SPTB < Min_SPTB) Min_SPTB = AS[i].raw_SPTB;

		// 2. AfterBN
		if (AS[i].raw_AfterBN > Max_AfterBN) Max_AfterBN = AS[i].raw_AfterBN;
		if (AS[i].raw_AfterBN < Min_AfterBN) Min_AfterBN = AS[i].raw_AfterBN;

		// 3. Total
		if (AS[i].raw_Total > Max_Total) Max_Total = AS[i].raw_Total;
		if (AS[i].raw_Total < Min_Total) Min_Total = AS[i].raw_Total;

		// 4. Current
		if (AS[i].raw_Current > Max_Current) Max_Current = AS[i].raw_Current;
		if (AS[i].raw_Current < Min_Current) Min_Current = AS[i].raw_Current;

		// // 5. lastjob
		// if (AS[i].raw_lastjob > Max_lastjob) Max_lastjob = AS[i].raw_lastjob;
		// if (AS[i].raw_lastjob < Min_lastjob) Min_lastjob = AS[i].raw_lastjob;

		// AS[i].raw_est = 1e15;

        // for (int m = 0; m < pMachines[AS[i].cStage]; m++)
        // {
        //     if (MachineAssignment[AS[i].cStage][m].empty())
        //     {
        //         // 机器为空，可以在到达时间开始（第一阶段则为0）
        //         if ((AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]) < AS[i].raw_est)
        //         {
        //             AS[i].raw_est = (AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]);
        //         }
        //     }
        //     else
        //     {
        //         // 1. 检查是否可以插入到机器最前面
        //         if (STime[AS[i].cStage][MachineAssignment[AS[i].cStage][m][0]] - (AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]) >= pUnitTime[AS[i].cStage][AS[i].cJob])
        //         {
        //             if ((AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]) < AS[i].raw_est)
        //             {
        //                 AS[i].raw_est = (AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]);
        //             }
        //         }
                
        //         // 2. 检查工件之间的空隙
        //         for (int pos = 1; pos < (int)MachineAssignment[AS[i].cStage][m].size(); pos++)
        //         {
        //             // 空隙开始时间 = max(前一工件完工时间, 当前工件到达时间)
        //             // 空隙结束时间 = 下一工件开始时间
        //             if (STime[AS[i].cStage][MachineAssignment[AS[i].cStage][m][pos]] - 
        //                 max(CTime[AS[i].cStage][MachineAssignment[AS[i].cStage][m][pos - 1]], 
        //                     AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]) >= pUnitTime[AS[i].cStage][AS[i].cJob])
        //             {
        //                 if (max(CTime[AS[i].cStage][MachineAssignment[AS[i].cStage][m][pos - 1]], 
        //                         AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]) < AS[i].raw_est)
        //                 {
        //                     AS[i].raw_est = max(CTime[AS[i].cStage][MachineAssignment[AS[i].cStage][m][pos - 1]], 
        //                                         AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]);
        //                 }
        //                 break;
        //             }
        //         }
                
        //         // 3. 检查机器末尾
        //         if (max(CTime[AS[i].cStage][MachineAssignment[AS[i].cStage][m].back()], 
        //                 AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]) < AS[i].raw_est)
        //         {
        //             AS[i].raw_est = max(CTime[AS[i].cStage][MachineAssignment[AS[i].cStage][m].back()], 
        //                                 AS[i].cStage == 0 ? 0 : CTime[AS[i].cStage - 1][AS[i].cJob]);
        //         }
        //     }
        // }

		int job = AS[i].cJob;
        int stage = AS[i].cStage;
        
        // 计算工件到达时间 (Arrival Time)
        double arrival_time = 0;
        if (stage > 0) 
        {
            // 利用已调度数据的 CTime，这是 O(1)
            arrival_time = CTime[stage - 1][job]; 
        }
		double min_machine_avail = 1e15;

        for (int m = 0; m < pMachines[stage]; m++)
        {
            double machine_free_time = 0;
            if (MachineAssignment[stage][m].empty())
            {
                machine_free_time = 0;
            }
            else
            {
                int last_job = MachineAssignment[stage][m].back(); 
                machine_free_time = CTime[stage][last_job];       
            }
            
            if (machine_free_time < min_machine_avail)
            {
                min_machine_avail = machine_free_time;
            }
        }
		AS[i].raw_est = max(arrival_time, min_machine_avail);

		//5. est
		if(AS[i].raw_est>Max_est)Max_est=AS[i].raw_est;
		if(AS[i].raw_est<Min_est)Min_est=AS[i].raw_est;
	
	}
}

// LeftShift构造版本,用于生成SAS
// 与LeftInsert不同，此函数只追加到机器末尾，不尝试在中间位置插入
void FAS_CCH::LeftShift_Construct()
{
	// 选择完成时间最早的机器，将工序追加到末尾
	int Earliest_CTime = INT_MAX;
	int Machine_Index = 0;
	
	for (int k = 0; k < pMachines[current_Stage]; k++)
	{
		int temp_ctime;
		
		if (MachineAssignment[current_Stage][k].size() == 0)
		{
			// 机器为空
			if (current_Stage == 0)
			{
				temp_ctime = pUnitTime[current_Stage][current_Job];
			}
			else
			{
				temp_ctime = CTime[current_Stage - 1][current_Job] + pUnitTime[current_Stage][current_Job];
			}
		}
		else
		{
			// 机器非空，追加到末尾
			int last_job = MachineAssignment[current_Stage][k].back();
			int start_time;
			
			if (current_Stage == 0)
			{
				start_time = CTime[current_Stage][last_job];
			}
			else
			{
				start_time = max(CTime[current_Stage - 1][current_Job], CTime[current_Stage][last_job]);
			}
			temp_ctime = start_time + pUnitTime[current_Stage][current_Job];
		}
		
		if (temp_ctime < Earliest_CTime)
		{
			Earliest_CTime = temp_ctime;
			Machine_Index = k;
		}
	}
	
	// 将工序追加到选定机器的末尾
	if (MachineAssignment[current_Stage][Machine_Index].size() == 0)
	{
		// 机器为空
		if (current_Stage == 0)
		{
			STime[current_Stage][current_Job] = 0;
			CTime[current_Stage][current_Job] = pUnitTime[current_Stage][current_Job];
		}
		else
		{
			STime[current_Stage][current_Job] = CTime[current_Stage - 1][current_Job];
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
	}
	else
	{
		// 追加到末尾
		int last_job = MachineAssignment[current_Stage][Machine_Index].back();
		
		if (current_Stage == 0)
		{
			STime[current_Stage][current_Job] = CTime[current_Stage][last_job];
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		else
		{
			STime[current_Stage][current_Job] = max(CTime[current_Stage - 1][current_Job], CTime[current_Stage][last_job]);
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
	}
	
	MachineAssignment[current_Stage][Machine_Index].push_back(current_Job);
}

void FAS_CCH::LeftInsert()//允许第一阶段插入
{
	//选定机器和位置
	int Earliest_Start = INT_MAX;
	int Machine_Index;
	int Position_Index;
	for (int k = 0; k < pMachines[current_Stage]; k++)
	{
		if (MachineAssignment[current_Stage][k].size() == 0)
		{
			int temp_start_time;

			if (current_Stage == 0)
			{
				temp_start_time =0 ;
			}
			else
			{
				temp_start_time = CTime[current_Stage - 1][current_Job];
			}


			if (temp_start_time < Earliest_Start)
			{
				Earliest_Start = temp_start_time;
				Machine_Index = k;
				Position_Index = 0;
			}
		}
		else
		{
			int current_job_num = MachineAssignment[current_Stage][k].size();
			//其它阶段就要考虑插入的情况
			for (int position = 0; position <= current_job_num; position++)
			{
				if (position == 0)
				{
					//先进行初步判断

					int OSTime, OCTime;
					if (current_Stage == 0)
					{
						OSTime = 0;
						OCTime = OSTime + pUnitTime[current_Stage][current_Job];
					}
					else
					{
						OSTime = CTime[current_Stage - 1][current_Job] ;
						OCTime = OSTime + pUnitTime[current_Stage][current_Job];
					}


					//子批调度完之后，判断是否可实行插入
					if (OCTime <= STime[current_Stage][MachineAssignment[current_Stage][k][position]] - pSetupTime[current_Stage][MachineAssignment[current_Stage][k][position]])
					{
						int temp_start_time = OSTime;

						if (temp_start_time < Earliest_Start)
						{
							Earliest_Start = temp_start_time;
							Machine_Index = k;
							Position_Index = position;
						}

						break;//如果执行了插入，就不用往下接着遍历了，直接跳出循环就行了
					}



				}
				else if (position == current_job_num)
				{
					//插入到最后
					int OSTime, OCTime;
					if (current_Stage == 0)
					{
						OSTime = CTime[current_Stage][MachineAssignment[current_Stage][k][current_job_num - 1]] ;
						OCTime = OSTime + pUnitTime[current_Stage][current_Job];
					}
					else
					{
						OSTime = max(CTime[current_Stage - 1][current_Job], CTime[current_Stage][MachineAssignment[current_Stage][k][current_job_num - 1]] );
						OCTime = OSTime + pUnitTime[current_Stage][current_Job];
					}
					int temp_start_time = OSTime;

					if (temp_start_time < Earliest_Start)
					{
						Earliest_Start = temp_start_time;
						Machine_Index = k;
						Position_Index = current_job_num;
					}
				}
				else
				{
					//先进行初步判断

					int OSTime, OCTime;
					if (current_Stage == 0)
					{
						OSTime = CTime[current_Stage][MachineAssignment[current_Stage][k][position - 1]] ;
						OCTime = OSTime + pUnitTime[current_Stage][current_Job];
					}
					else
					{
						OSTime = max(CTime[current_Stage - 1][current_Job] , CTime[current_Stage][MachineAssignment[current_Stage][k][position - 1]] );
						OCTime = OSTime + pUnitTime[current_Stage][current_Job];
					}



					//子批调度完之后，判断是否可实行插入
					if (OCTime <= STime[current_Stage][MachineAssignment[current_Stage][k][position]] - pSetupTime[current_Stage][MachineAssignment[current_Stage][k][position]])
					{
						int temp_start_time = OSTime;

						if (temp_start_time < Earliest_Start)
						{
							Earliest_Start = temp_start_time;
							Machine_Index = k;
							Position_Index = position;
						}

						break;  //直接跳出循环，不用接着往下遍历了
					}

				}
			}

		}
	}



	if (Position_Index == MachineAssignment[current_Stage][Machine_Index].size() && MachineAssignment[current_Stage][Machine_Index].size() > 0)
	{
		//最后一个位置
		int previous_job = MachineAssignment[current_Stage][Machine_Index][Position_Index - 1];
		if (current_Stage == 0)
		{
			STime[current_Stage][current_Job] = CTime[current_Stage][previous_job] ;
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		else
		{
			STime[current_Stage][current_Job] = max(CTime[current_Stage - 1][current_Job] ,  CTime[current_Stage][previous_job]);
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		MachineAssignment[current_Stage][Machine_Index].push_back(current_Job);
	}
	else if (Position_Index == MachineAssignment[current_Stage][Machine_Index].size() && MachineAssignment[current_Stage][Machine_Index].size() == 0)
	{
		if (current_Stage == 0)
		{
			STime[current_Stage][current_Job] = 0 ;
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		else
		{
			STime[current_Stage][current_Job] = CTime[current_Stage - 1][current_Job] ;
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		MachineAssignment[current_Stage][Machine_Index].push_back(current_Job);
	}
	else if (Position_Index == 0)
	{
		if (current_Stage == 0)
		{
			STime[current_Stage][current_Job] = 0;
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		else
		{
			STime[current_Stage][current_Job] = CTime[current_Stage - 1][current_Job];
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		MachineAssignment[current_Stage][Machine_Index].insert(MachineAssignment[current_Stage][Machine_Index].begin() + Position_Index, current_Job);
	}
	else
	{
		int previous_job = MachineAssignment[current_Stage][Machine_Index][Position_Index - 1];
		if (current_Stage == 0)
		{
			STime[current_Stage][current_Job] = CTime[current_Stage][previous_job] ;
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		else
		{
			STime[current_Stage][current_Job] = max(CTime[current_Stage][previous_job] , CTime[current_Stage - 1][current_Job] );
			CTime[current_Stage][current_Job] = STime[current_Stage][current_Job] + pUnitTime[current_Stage][current_Job];
		}
		MachineAssignment[current_Stage][Machine_Index].insert(MachineAssignment[current_Stage][Machine_Index].begin() + Position_Index, current_Job);
	}

}

void FAS_CCH::RightInsert()
{
	if (current_Stage >= 1)
	{
		for (int stage = current_Stage - 1; stage >= 0; stage--)
		{
			vector<vector<int>> tempMachineAssignment = MachineAssignment[stage];

			//先将current_Job在MachineAssignment中删除
			for (int k = 0; k < pMachines[stage]; k++)
			{
				vector<int>::iterator it = find(MachineAssignment[stage][k].begin(), MachineAssignment[stage][k].end(), current_Job);
				if (it != MachineAssignment[stage][k].end())
				{
					//说明找到了
					MachineAssignment[stage][k].erase(it);
					break;
				}
			}


			bool isSuccess = false;

			//定义update_STime, update_CTime
			int update_STime, update_CTime;

			//选定机器和位置
			int Machine_Index;
			int Position_Index;

			int current_time = STime[stage][current_Job];  //当前的开始加工时间

			for (int k = 0; k < pMachines[stage]; k++)
			{
				if (MachineAssignment[stage][k].size() == 0)
				{
					int temp_STime, temp_CTime;

					temp_CTime = STime[stage + 1][current_Job] ;
					temp_STime = temp_CTime - pUnitTime[stage][current_Job];

					if (temp_STime > current_time)
					{
						current_time = temp_STime;
						Machine_Index = k;
						Position_Index = 0;

						update_STime = temp_STime;
						update_CTime = temp_CTime;

						isSuccess = true;
					}

				}
				else
				{
					int current_job_num = MachineAssignment[stage][k].size();
					for (int position = current_job_num; position >= 0; position--)
					{
						if (position == current_job_num)
						{
							int temp_STime, temp_CTime;

							temp_CTime = STime[stage + 1][current_Job] ;
							temp_STime = temp_CTime - pUnitTime[stage][current_Job];

							if ((temp_STime > current_time) && (temp_STime >= CTime[stage][MachineAssignment[stage][k][current_job_num - 1]]))
							{
								current_time = temp_STime;
								Machine_Index = k;
								Position_Index = position;
								update_STime = temp_STime;
								update_CTime = temp_CTime;
								isSuccess = true;
								break;
							}
						}
						else
						{
							int temp_STime, temp_CTime;

							temp_CTime = min(STime[stage][MachineAssignment[stage][k][position]], STime[stage + 1][current_Job] );
							temp_STime = temp_CTime - pUnitTime[stage][current_Job];

							if (position > 0)
							{
								if (temp_STime > current_time && temp_STime >= CTime[stage][MachineAssignment[stage][k][position - 1]])
								{
									current_time = temp_STime;
									Machine_Index = k;
									Position_Index = position;
									update_STime = temp_STime;
									update_CTime = temp_CTime;
									isSuccess = true;
									break;
								}
							}
							else
							{
								if (temp_STime > current_time)
								{
									current_time = temp_STime;
									Machine_Index = k;
									Position_Index = position;
									update_STime = temp_STime;
									update_CTime = temp_CTime;
									isSuccess = true;
									break;
								}
							}

						}
					}
				}
			}

			//选定好机器和位置之后，开始更新STime, CTime以及MachineAssignment
			//更新MachineAssignment
			if (isSuccess)
			{
				vector<int>::iterator it1 = MachineAssignment[stage][Machine_Index].begin();
				MachineAssignment[stage][Machine_Index].insert(it1 + Position_Index, current_Job);

				STime[stage][current_Job] = update_STime;
				CTime[stage][current_Job] = update_CTime;
			}
			else
			{
				MachineAssignment[stage] = tempMachineAssignment;
			}


		}
	}
}

void FAS_CCH::FAS_Repair(int& tempStage, vector<vector<vector<int>>>& tempMachineAssignment, bool& IsImpro)// 只修复 tempStage 及其之后的所有阶段
{
	vector<vector<int>> tempSTime = STime;
	vector<vector<int>> tempCTime = CTime;

	// 修复被破坏的阶段
	LeftShift(tempStage, tempMachineAssignment, tempSTime, tempCTime);

	// 只修复 tempStage 之后的所有阶段
	for (int i = tempStage + 1; i < pStage; i++)
	{
		for (int m = 0; m < pMachines[i]; m++)
		{
			int tempsize = tempMachineAssignment[i][m].size();
			vector<LotCompare> tempSet;

			for (int k = 0; k < tempsize; k++)
			{
				int c_job = tempMachineAssignment[i][m][k];
				// 根据 刚刚更新完 的前一阶段 (i-1) 的 CTime 来排序
				tempSet.push_back(LotCompare(c_job, tempCTime[i - 1][c_job]));
			}

			sort(tempSet.begin(), tempSet.end(), CompareLot());

			for (int k = 0; k < tempsize; k++)
			{
				tempMachineAssignment[i][m][k] = tempSet[k].cLot;
			}
		}
		// 重新调度当前阶段 i
		LeftShift(i, tempMachineAssignment, tempSTime, tempCTime);
	}

	int tempmakespan = 0;
	for (int j = 0; j < pJob; j++)
	{
		if (tempCTime[pStage - 1][j] > tempmakespan)
		{
			tempmakespan = tempCTime[pStage - 1][j];
		}
	}

	// 贪婪接受
	if (tempmakespan < makespan)
	{
		STime = tempSTime;
		CTime = tempCTime;
		makespan = tempmakespan;
		MachineAssignment = tempMachineAssignment;
		IsImpro = true;
	}
	else
	{
		IsImpro = false;
	}
}

void FAS_CCH::LeftShift(int& tempStage, vector<vector<vector<int>>>& tempMachineAssignment, vector<vector<int>>& tempSTime, vector<vector<int>>& tempCTime)
{

	for (int m = 0; m < pMachines[tempStage]; m++)
	{
		for (int k = 0; k < tempMachineAssignment[tempStage][m].size(); k++)
		{
			int tempLot = tempMachineAssignment[tempStage][m][k];

			if (tempStage == 0)
			{
				//第一阶段
				if (k == 0)
				{
					//机器上第一个位置
					tempSTime[tempStage][tempLot] = pSetupTime[tempStage][tempLot];
					tempCTime[tempStage][tempLot] = tempSTime[tempStage][tempLot] + pUnitTime[tempStage][tempLot];

				}
				else
				{
					//机器上其它位置
					int previousLot = tempMachineAssignment[tempStage][m][k - 1];
					tempSTime[tempStage][tempLot] = tempCTime[tempStage][previousLot] + pSetupTime[tempStage][tempLot];
					tempCTime[tempStage][tempLot] = tempSTime[tempStage][tempLot] + pUnitTime[tempStage][tempLot];

				}
			}
			else
			{
				//后续阶段
				if (k == 0)
				{
					//机器上第一个位置
					tempSTime[tempStage][tempLot] = max(tempCTime[tempStage - 1][tempLot] + pTransferTime[tempStage - 1][tempLot], pSetupTime[tempStage][tempLot]);
					tempCTime[tempStage][tempLot] = tempSTime[tempStage][tempLot] + pUnitTime[tempStage][tempLot];


				}
				else
				{
					//机器上其它位置
					int previousLot = tempMachineAssignment[tempStage][m][k - 1];
					tempSTime[tempStage][tempLot] = max(tempCTime[tempStage - 1][tempLot] + pTransferTime[tempStage - 1][tempLot], tempCTime[tempStage][previousLot] + pSetupTime[tempStage][tempLot]);
					tempCTime[tempStage][tempLot] = tempSTime[tempStage][tempLot] + pUnitTime[tempStage][tempLot];
				}
			}
		}
	}

}

void FAS_CCH::LeftShift()//从第一阶段开始左移调度
{
	for (int i = 0; i < pStage; i++)
	{
		for (int k = 0; k < MachineAssignment[i].size(); k++)
		{
			for (int j = 0; j < MachineAssignment[i][k].size(); j++)
			{
				if (i == 0)
				{
					if (j == 0)
					{
						STime[i][MachineAssignment[i][k][j]] = 0;
						CTime[i][MachineAssignment[i][k][j]] = STime[i][MachineAssignment[i][k][j]] + pUnitTime[i][MachineAssignment[i][k][j]];
					}
					else
					{
						int previousLot = MachineAssignment[i][k][j - 1];
						STime[i][MachineAssignment[i][k][j]] = CTime[i][previousLot];
						CTime[i][MachineAssignment[i][k][j]] = STime[i][MachineAssignment[i][k][j]] + pUnitTime[i][MachineAssignment[i][k][j]];
					}
				}
				else
				{
					if (j == 0)
					{
						int c_job = MachineAssignment[i][k][j];
						STime[i][c_job] = CTime[i - 1][c_job];
						CTime[i][c_job] = STime[i][c_job] + pUnitTime[i][c_job];
					}
					else
					{
						int c_job = MachineAssignment[i][k][j];
						int p_job = MachineAssignment[i][k][j - 1];
						//更新STime和CTime
						STime[i][c_job] = max(CTime[i - 1][c_job], CTime[i][p_job]);
						CTime[i][c_job] = STime[i][c_job] + pUnitTime[i][c_job];
					}
				}

			}
		}
	}
}

void FAS_CCH::Backward_Decoding()
{
	//基于生成的MachineAssignment
	for (int k = pStage - 1; k >= 0; k--)
	{
		if (k == pStage - 1)
		{
			//最后一个阶段
			for (int m = 0; m < pMachines[k]; m++)
			{
				//基于机器逐一进行反向调度
				for (int j = MachineAssignment[k][m].size() - 1; j >= 0; j--)
				{
					if (j == MachineAssignment[k][m].size() - 1)
					{
						int c_job = MachineAssignment[k][m][j];
						CTime_back[k][c_job] = makespan;
						STime_back[k][c_job] = CTime_back[k][c_job] - pUnitTime[k][c_job];
					}
					else
					{
						int c_job = MachineAssignment[k][m][j];
						int sub_job = MachineAssignment[k][m][j + 1];
						CTime_back[k][c_job] = STime_back[k][sub_job];
						STime_back[k][c_job] = CTime_back[k][c_job] - pUnitTime[k][c_job];
					}

				}

			}

		}
		else
		{
			//其它阶段
			for (int m = 0; m < pMachines[k]; m++)
			{
				//基于机器逐一进行反向调度
				for (int j = MachineAssignment[k][m].size() - 1; j >= 0; j--)
				{
					if (j == MachineAssignment[k][m].size() - 1)
					{
						int c_job = MachineAssignment[k][m][j];
						CTime_back[k][c_job] = STime_back[k + 1][c_job];
						STime_back[k][c_job] = CTime_back[k][c_job] - pUnitTime[k][c_job];
					}
					else
					{
						int c_job = MachineAssignment[k][m][j];
						int sub_job = MachineAssignment[k][m][j + 1];
						CTime_back[k][c_job] = min(STime_back[k][sub_job], STime_back[k + 1][c_job]);//计算关键路径的时候要注意！
						STime_back[k][c_job] = CTime_back[k][c_job] - pUnitTime[k][c_job];
					}
				}
			}
		}
	}


	//生成Critical blocks  
	//以下代码可以放在构造函数中
	Critical_Operations.clear();
	Critical_Block.clear();
	Critical_Block.resize(pStage);
	for (int i = 0; i < pStage; i++)
	{
		Critical_Block[i].resize(pMachines[i]);
	}

	for (int k = 0; k < pStage; k++)
	{
		for (int j = 0; j < pJob; j++)
		{
			//判断一下 j 分配到了哪台机器上
			int machine_index;
			for (int m = 0; m < pMachines[k]; m++)
			{
				vector<int>::iterator it;

				it = find(MachineAssignment[k][m].begin(), MachineAssignment[k][m].end(), j);

				if (it != MachineAssignment[k][m].end())
				{
					//说明找到了
					machine_index = m;
					break;  //终止循环
				}
			}

			int sizee = Critical_Block[k][machine_index].size();
			if (STime[k][j] == STime_back[k][j])
			{

				Critical_Operations.push_back(CriticalStructure(k, j));

				if (sizee == 0)
				{
					Critical_Block[k][machine_index].push_back(j);
					sizee = Critical_Block[k][machine_index].size();
				}
				else
				{
					if (j != Critical_Block[k][machine_index][sizee - 1])
					{
						Critical_Block[k][machine_index].push_back(j);
						sizee = Critical_Block[k][machine_index].size();
					}
				}

			}
		}
	}
}

void FAS_CCH::GenerateCriticalBlockRanges()
{
	CriticalBlocks.clear();

	for (int k = 0; k < pStage; k++)
	{
		for (int m = 0; m < pMachines[k]; m++)
		{
			int pos = 0;
			while (pos < MachineAssignment[k][m].size())
			{
				int lot = MachineAssignment[k][m][pos];
				if (STime[k][lot] == STime_back[k][lot])
				{
					int start_pos = pos;
					while (pos < MachineAssignment[k][m].size())
					{
						lot = MachineAssignment[k][m][pos];
						if (STime[k][lot] != STime_back[k][lot])
							break;
						pos++;
					}
					int end_pos = pos - 1;
					if (end_pos - start_pos >= 1)
					{
						CriticalBlockRange block;
						block.stage = k;
						block.machine = m;
						block.start_pos = start_pos;
						block.end_pos = end_pos;
						CriticalBlocks.push_back(block);
					}
				}
				else
				{
					pos++;
				}
			}
		}
	}
}

void FAS_CCH::FindMandatoryOperations()
{
	Mandatory_Ops.clear();
	
	if (Critical_Operations.empty())
	{
		return;
	}
	
	int critical_count = Critical_Operations.size();//结点总数
	
	vector<int> op_machine(critical_count);//关键工序所在机器
	vector<int> op_pos(critical_count);//关键工序在机器上的位置
	vector<bool> is_visit(critical_count, false);//是否被访问过
	vector<int> dfn(critical_count + 2, 0);
	vector<int> low(critical_count + 2, 0);//+2是因为多了两个虚拟结点
	vector<vector<int>> adj(critical_count + 2);
	
	int VIRTUAL_START = critical_count;
	int VIRTUAL_END = critical_count + 1;
	
	for (int i = 0; i < critical_count; i++)//找每个关键工序所在的机器和位置
	{
		int k = Critical_Operations[i].Stage;
		int j = Critical_Operations[i].Critical_Lot;
		
		for (int m = 0; m < pMachines[k]; m++)
		{
			for (int p = 0; p < (int)MachineAssignment[k][m].size(); p++)
			{
				if (MachineAssignment[k][m][p] == j)
				{
					op_machine[i] = m;
					op_pos[i] = p;
					break;
				}
			}
		}
	}
	
	auto find_critical_index = [&](int stage, int job) -> int {//寻找关键工序在Critical_Operations中的索引
		for (int i = 0; i < critical_count; i++)
		{
			if (Critical_Operations[i].Stage == stage && Critical_Operations[i].Critical_Lot == job)
			{
				return i;
			}
		}
		return -1;
	};
	
	for (int i = 0; i < critical_count; i++)//构建无向图
	{
		int k = Critical_Operations[i].Stage;
		int j = Critical_Operations[i].Critical_Lot;
		int m = op_machine[i];
		int p = op_pos[i];
		
		if (p > 0)
		{
			int prev_job = MachineAssignment[k][m][p - 1];
			if (CTime[k][prev_job] == STime[k][j])//前一道工序的完成时间等于当前工序的开始时间
			{
				int prev_idx = find_critical_index(k, prev_job);
				if (prev_idx >= 0)
				{
					adj[i].push_back(prev_idx);
					adj[prev_idx].push_back(i);
				}
			}
		}
		
		if (k > 0)
		{
			if (CTime[k - 1][j] == STime[k][j])
			{
				int prev_idx = find_critical_index(k - 1, j);
				if (prev_idx >= 0)
				{
					adj[i].push_back(prev_idx);
					adj[prev_idx].push_back(i);
				}
			}
		}
	}
	
	for (int i = 0; i < critical_count; i++)
	{
		int k = Critical_Operations[i].Stage;
		int j = Critical_Operations[i].Critical_Lot;
		
		if (k == pStage - 1 && CTime[k][j] == makespan)
		{
			adj[VIRTUAL_START].push_back(i);
			adj[i].push_back(VIRTUAL_START);
		}
		
		if (k == 0 && STime[k][j] == 0)
		{
			adj[VIRTUAL_END].push_back(i);
			adj[i].push_back(VIRTUAL_END);
		}
	}
	
	if (adj[VIRTUAL_START].empty() || adj[VIRTUAL_END].empty())
	{
		return;
	}
	
	vector<pair<int, int>> node_stack;
	vector<int> adj_ptr(critical_count + 2, 0);
	node_stack.push_back(make_pair(VIRTUAL_START, -1));
	int time_stamp = 1;
	
	while (!node_stack.empty())
	{
		int cur_node = node_stack.back().first;
		int parent_node = node_stack.back().second;
		
		if (dfn[cur_node] == 0)
		{
			dfn[cur_node] = low[cur_node] = time_stamp;
			time_stamp++;
		}
		
		bool found_child = false;
		
		while (adj_ptr[cur_node] < (int)adj[cur_node].size())
		{
			int child_node = adj[cur_node][adj_ptr[cur_node]];
			adj_ptr[cur_node]++;
			
			if (child_node == parent_node)
			{
				continue;
			}
			
			if (dfn[child_node] == 0)
			{
				node_stack.push_back(make_pair(child_node, cur_node));
				found_child = true;
				break;
			}
			else
			{
				low[cur_node] = min(low[cur_node], dfn[child_node]);
			}
		}
		
		if (!found_child)
		{
			node_stack.pop_back();
			
			if (!node_stack.empty())
			{
				int p_node = node_stack.back().first;
				
				if (dfn[p_node] <= low[cur_node] && p_node != VIRTUAL_START && p_node < critical_count)
				{
					int k = Critical_Operations[p_node].Stage;
					int j = Critical_Operations[p_node].Critical_Lot;
					int m = op_machine[p_node];
					int pos = op_pos[p_node];
					Mandatory_Ops.push_back(MandatoryOp(k, j, m, pos));
				}
				
				low[p_node] = min(low[p_node], low[cur_node]);
			}
		}
	}
}

void FAS_CCH::Ns1_ReverseEdge(bool& IsImpro)
{
	if (CriticalBlocks.empty())
	{
		IsImpro = false;
		return;
	}

	int blockIndex = rand() % CriticalBlocks.size();
	CriticalBlockRange block = CriticalBlocks[blockIndex];

	int blockLength = block.end_pos - block.start_pos + 1;
	if (blockLength < 2)
	{
		IsImpro = false;
		return;
	}

	if (block.start_pos >= MachineAssignment[block.stage][block.machine].size() || 
		block.end_pos >= MachineAssignment[block.stage][block.machine].size())
	{
		IsImpro = false;
		return;
	}

	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;
	int tempStage = block.stage;
	int tempMachine = block.machine;

	if (rand() % 2 == 0)
	{
		if (block.start_pos + 1 <= block.end_pos && block.start_pos + 1 < tempMachineAssignment[tempStage][tempMachine].size())
		{
			swap(tempMachineAssignment[tempStage][tempMachine][block.start_pos],
				 tempMachineAssignment[tempStage][tempMachine][block.start_pos + 1]);
		}
		else
		{
			IsImpro = false;
			return;
		}
	}
	else
	{
		if (block.end_pos - 1 >= block.start_pos && block.end_pos < tempMachineAssignment[tempStage][tempMachine].size())
		{
			swap(tempMachineAssignment[tempStage][tempMachine][block.end_pos - 1],
				 tempMachineAssignment[tempStage][tempMachine][block.end_pos]);
		}
		else
		{
			IsImpro = false;
			return;
		}
	}

	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
}

void FAS_CCH::Ns2_MoveInnerToEdge(bool& IsImpro)
{
	if (CriticalBlocks.empty())
	{
		IsImpro = false;
		return;
	}

	int blockIndex = rand() % CriticalBlocks.size();
	CriticalBlockRange block = CriticalBlocks[blockIndex];

	int blockLength = block.end_pos - block.start_pos + 1;
	if (blockLength < 3)
	{
		IsImpro = false;
		return;
	}

	if (block.end_pos >= MachineAssignment[block.stage][block.machine].size())
	{
		IsImpro = false;
		return;
	}

	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;
	int tempStage = block.stage;
	int tempMachine = block.machine;

	int relativeInnerPos = 1 + (rand() % (blockLength - 2));
	int innerPos = block.start_pos + relativeInnerPos;

	if (innerPos >= tempMachineAssignment[tempStage][tempMachine].size())
	{
		IsImpro = false;
		return;
	}

	int lot = tempMachineAssignment[tempStage][tempMachine][innerPos];

	// 统一先删除再插入，避免先插入导致索引漂移
	tempMachineAssignment[tempStage][tempMachine].erase(
		tempMachineAssignment[tempStage][tempMachine].begin() + innerPos);

	int seqSizeAfterErase = (int)tempMachineAssignment[tempStage][tempMachine].size();
	if (rand() % 2 == 0)
	{
		int insertPos = block.start_pos;
		// 夹持到当前序列范围
		if (insertPos < 0) insertPos = 0;
		if (insertPos > seqSizeAfterErase) insertPos = seqSizeAfterErase;
		tempMachineAssignment[tempStage][tempMachine].insert(
			tempMachineAssignment[tempStage][tempMachine].begin() + insertPos, lot);
	}
	else
	{
		// 原 block.end_pos 是针对原始序列，删除后可能需向左收缩
		int insertPos = min(block.end_pos, seqSizeAfterErase);
		if (insertPos < 0) insertPos = 0;
		tempMachineAssignment[tempStage][tempMachine].insert(
			tempMachineAssignment[tempStage][tempMachine].begin() + insertPos, lot);
	}

	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
}

void FAS_CCH::Ns3_ReinsertEdge(bool& IsImpro)
{
	if (CriticalBlocks.empty())
	{
		IsImpro = false;
		return;
	}

	int blockIndex = rand() % CriticalBlocks.size();
	CriticalBlockRange block = CriticalBlocks[blockIndex];

	int blockLength = block.end_pos - block.start_pos + 1;
	if (blockLength < 3)
	{
		IsImpro = false;
		return;
	}

	if (block.end_pos >= MachineAssignment[block.stage][block.machine].size())
	{
		IsImpro = false;
		return;
	}

	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;
	int tempStage = block.stage;
	int tempMachine = block.machine;
	int machineSeqSize = tempMachineAssignment[tempStage][tempMachine].size();

	int action = rand() % 4;
	int lot, erasePos, insertPos;

	if (action == 0)
	{
		lot = tempMachineAssignment[tempStage][tempMachine][block.start_pos];
		erasePos = block.start_pos;
		insertPos = block.start_pos + 1 + (rand() % max(1, blockLength - 1));
		if (insertPos > block.end_pos)
		{
			insertPos = block.end_pos;
		}
	}
	else if (action == 1)
	{
		lot = tempMachineAssignment[tempStage][tempMachine][block.end_pos];
		erasePos = block.end_pos;
		insertPos = block.start_pos + (rand() % max(1, blockLength - 1));
	}
	else if (action == 2)
	{
		int relativeInternalPos = 1 + (rand() % max(1, blockLength - 2));
		lot = tempMachineAssignment[tempStage][tempMachine][block.start_pos + relativeInternalPos];
		erasePos = block.start_pos + relativeInternalPos;
		insertPos = block.start_pos;
	}
	else
	{
		int relativeInternalPos = 1 + (rand() % max(1, blockLength - 2));
		lot = tempMachineAssignment[tempStage][tempMachine][block.start_pos + relativeInternalPos];
		erasePos = block.start_pos + relativeInternalPos;
		insertPos = block.end_pos;
	}

	if (erasePos < 0 || insertPos < 0 || erasePos >= machineSeqSize)
	{
		IsImpro = false;
		return;
	}
	// 统一先删除再插入，避免先插入导致索引错位
	tempMachineAssignment[tempStage][tempMachine].erase(
		tempMachineAssignment[tempStage][tempMachine].begin() + erasePos);
	int seqSizeAfterErase = (int)tempMachineAssignment[tempStage][tempMachine].size();
	if (insertPos > seqSizeAfterErase) insertPos = seqSizeAfterErase;
	if (insertPos < 0) insertPos = 0;
	tempMachineAssignment[tempStage][tempMachine].insert(
		tempMachineAssignment[tempStage][tempMachine].begin() + insertPos, lot);

	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
}

void FAS_CCH::Nc_CrossMachineInsert(bool& IsImpro)
{
	if (CriticalBlocks.empty())
	{
		IsImpro = false;
		return;
	}

	CriticalBlockRange block = CriticalBlocks[rand() % CriticalBlocks.size()];
	int blockLength = block.end_pos - block.start_pos + 1;

	if (blockLength < 2 || pMachines[block.stage] < 2)
	{
		IsImpro = false;
		return;
	}

	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;
	int tempStage = block.stage;

	int relativePos = rand() % blockLength;
	int posInMachine = block.start_pos + relativePos;
	int seqSize = (int)tempMachineAssignment[tempStage][block.machine].size();
	if (posInMachine < 0 || posInMachine >= seqSize)
	{
		IsImpro = false;
		return;
	}
	int lot = tempMachineAssignment[tempStage][block.machine][posInMachine];

	int targetMachine;
	do
	{
		targetMachine = rand() % pMachines[tempStage];
	} while (targetMachine == block.machine);

	tempMachineAssignment[tempStage][block.machine].erase(
		tempMachineAssignment[tempStage][block.machine].begin() + posInMachine);

	int targetMachineSize = tempMachineAssignment[tempStage][targetMachine].size();
	int insertPos = (targetMachineSize == 0) ? 0 : (rand() % (targetMachineSize + 1));

	tempMachineAssignment[tempStage][targetMachine].insert(
		tempMachineAssignment[tempStage][targetMachine].begin() + insertPos, lot);

	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
}

void FAS_CCH::MandatoryLocalSearch(bool& IsImpro)
{
	FindMandatoryOperations();

	if (Mandatory_Ops.empty())
	{
		IsImpro = false;
		return;
	}

	int originalSpan = makespan;

	for (const auto& mandatoryOp : Mandatory_Ops)
	{
		int opStage = mandatoryOp.stage;
		int opJob = mandatoryOp.job;
		int opMachine = mandatoryOp.machine;
		int opPos = mandatoryOp.pos;

		if (opPos >= (int)MachineAssignment[opStage][opMachine].size() ||
			MachineAssignment[opStage][opMachine][opPos] != opJob)
		{
			continue;
		}

		int f_pre_stage = (opStage == 0) ? 0 : CTime[opStage - 1][opJob];
		int b_suc_stage = 0;
		if (opStage < pStage - 1)
		{
			b_suc_stage = originalSpan - (STime_back[opStage + 1][opJob] - pTransferTime[opStage][opJob]);
		}
		int p_del = pUnitTime[opStage][opJob];

		int bestPathLength = originalSpan;
		int bestTargetMachine = -1;
		int bestInsertPos = -1;

		for (int targetMachine = 0; targetMachine < pMachines[opStage]; targetMachine++)
		{
			vector<int>& seq = MachineAssignment[opStage][targetMachine];
			int seqSize = seq.size();

			if (targetMachine == opMachine)
			{
				int virtualSize = seqSize - 1;
				if (virtualSize < 0) continue;

				int alphaPos = -1;
				int f_at_alpha = 0;
				for (int vp = 0; vp < virtualSize; vp++)
				{
					int rp = (vp < opPos) ? vp : vp + 1;
					int f_complete = CTime[opStage][seq[rp]];
					if (f_complete <= f_pre_stage)
					{
						alphaPos = vp;
						f_at_alpha = f_complete;
					}
					else
					{
						break;
					}
				}

				int betaPos = virtualSize;
				int b_at_beta = 0;
				for (int vp = virtualSize - 1; vp >= 0; vp--)
				{
					int rp = (vp < opPos) ? vp : vp + 1;
					int next_job = seq[rp];
					int b_duration = originalSpan - (STime_back[opStage][next_job] - pSetupTime[opStage][next_job]);
					if (b_duration <= b_suc_stage)
					{
						betaPos = vp + 1;
						b_at_beta = b_duration;
						break;
					}
				}

				if (betaPos <= alphaPos + 1)
				{
					int longestValue = f_pre_stage + p_del + b_suc_stage;
					if (longestValue < bestPathLength)
					{
						bestPathLength = longestValue;
						bestTargetMachine = targetMachine;
						bestInsertPos = alphaPos + 1;
					}
				}
				else
				{
					int f_time = f_at_alpha;
					for (int insertPos = alphaPos + 1; insertPos <= betaPos && insertPos <= virtualSize; insertPos++)
					{
						int f_before = 0;
						if (insertPos > 0)
						{
							int prevVp = insertPos - 1;
							int prevRp = (prevVp < opPos) ? prevVp : prevVp + 1;
							f_before = CTime[opStage][seq[prevRp]];
						}

						int b_after = 0;
						if (insertPos < virtualSize)
						{
							int nextVp = insertPos;
							int nextRp = (nextVp < opPos) ? nextVp : nextVp + 1;
							int next_job = seq[nextRp];
							b_after = originalSpan - (STime_back[opStage][next_job] - pSetupTime[opStage][next_job]);
						}

						int longestValue = max(f_pre_stage, f_before) + p_del + max(b_suc_stage, b_after);
						if (longestValue < bestPathLength)
						{
							bestPathLength = longestValue;
							bestTargetMachine = targetMachine;
							bestInsertPos = insertPos;
						}
					}
				}
			}
			else
			{
				if (seqSize == 0)
				{
					int longestValue = f_pre_stage + p_del + b_suc_stage;
					if (longestValue < bestPathLength)
					{
						bestPathLength = longestValue;
						bestTargetMachine = targetMachine;
						bestInsertPos = 0;
					}
					continue;
				}

				int f_machine_end = CTime[opStage][seq[seqSize - 1]];
				int first_job = seq[0];
				int b_machine_start = originalSpan - (STime_back[opStage][first_job] - pSetupTime[opStage][first_job]);

				if (f_machine_end - f_pre_stage >= b_machine_start - b_suc_stage)
				{
					bool foundAlpha = false;
					for (int insertPos = 0; insertPos <= seqSize; insertPos++)
					{
						if (!foundAlpha && insertPos < seqSize)
						{
							if (CTime[opStage][seq[insertPos]] > f_pre_stage)
							{
								foundAlpha = true;
							}
						}

						if (insertPos < seqSize && (originalSpan - (STime_back[opStage][seq[insertPos]] - pSetupTime[opStage][seq[insertPos]])) <= b_suc_stage)
						{
							int f_before = (insertPos == 0) ? 0 : CTime[opStage][seq[insertPos - 1]];
							int longestValue = max(f_pre_stage, f_before) + p_del + b_suc_stage;
							if (longestValue < bestPathLength)
							{
								bestPathLength = longestValue;
								bestTargetMachine = targetMachine;
								bestInsertPos = insertPos;
							}
							break;
						}

						if (foundAlpha)
						{
							int f_before = (insertPos == 0) ? 0 : CTime[opStage][seq[insertPos - 1]];
							int b_after = (insertPos >= seqSize) ? 0 : (originalSpan - (STime_back[opStage][seq[insertPos]] - pSetupTime[opStage][seq[insertPos]]));
							int longestValue = max(f_pre_stage, f_before) + p_del + max(b_suc_stage, b_after);
							if (longestValue < bestPathLength)
							{
								bestPathLength = longestValue;
								bestTargetMachine = targetMachine;
								bestInsertPos = insertPos;
							}
						}
					}
				}
				else
				{
					bool foundBeta = false;
					for (int insertPos = seqSize; insertPos >= 0; insertPos--)
					{
						if (!foundBeta && insertPos > 0)
						{
							int prev_job = seq[insertPos - 1];
							if ((originalSpan - (STime_back[opStage][prev_job] - pSetupTime[opStage][prev_job])) > b_suc_stage)
							{
								foundBeta = true;
							}
						}

						if (insertPos > 0 && CTime[opStage][seq[insertPos - 1]] <= f_pre_stage)
						{
							int b_after = (insertPos >= seqSize) ? 0 : (originalSpan - (STime_back[opStage][seq[insertPos]] - pSetupTime[opStage][seq[insertPos]]));
							int longestValue = f_pre_stage + p_del + max(b_suc_stage, b_after);
							if (longestValue < bestPathLength)
							{
								bestPathLength = longestValue;
								bestTargetMachine = targetMachine;
								bestInsertPos = insertPos;
							}
							break;
						}

						if (foundBeta)
						{
							int f_before = (insertPos == 0) ? 0 : CTime[opStage][seq[insertPos - 1]];
							int b_after = (insertPos >= seqSize) ? 0 : (originalSpan - (STime_back[opStage][seq[insertPos]] - pSetupTime[opStage][seq[insertPos]]));
							int longestValue = max(f_pre_stage, f_before) + p_del + max(b_suc_stage, b_after);
							if (longestValue < bestPathLength)
							{
								bestPathLength = longestValue;
								bestTargetMachine = targetMachine;
								bestInsertPos = insertPos;
							}
						}
					}
				}
			}
		}

		if (bestPathLength < originalSpan && bestTargetMachine >= 0)
		{
			MachineAssignment[opStage][opMachine].erase(
				MachineAssignment[opStage][opMachine].begin() + opPos);

			int actualInsertPos = bestInsertPos;
			if (bestTargetMachine == opMachine && bestInsertPos > opPos)
			{
				actualInsertPos = bestInsertPos;
			}

			MachineAssignment[opStage][bestTargetMachine].insert(
				MachineAssignment[opStage][bestTargetMachine].begin() + actualInsertPos, opJob);

			LeftShift();
			Backward_Decoding();

			int new_makespan = 0;
			for (int j = 0; j < pJob; j++)
			{
				if (CTime[pStage - 1][j] > new_makespan)
				{
					new_makespan = CTime[pStage - 1][j];
				}
			}

			if (new_makespan < originalSpan)
			{
				makespan = new_makespan;
				IsImpro = true;
				return;
			}
			else
			{
				MachineAssignment[opStage][bestTargetMachine].erase(
					MachineAssignment[opStage][bestTargetMachine].begin() + actualInsertPos);
				MachineAssignment[opStage][opMachine].insert(
					MachineAssignment[opStage][opMachine].begin() + opPos, opJob);

				LeftShift();
				Backward_Decoding();
				makespan = 0;
				for (int j = 0; j < pJob; j++)
				{
					if (CTime[pStage - 1][j] > makespan)
					{
						makespan = CTime[pStage - 1][j];
					}
				}
				// 不返回，继续尝试其他强制操作
			}
		}
	}

	IsImpro = false;
}

void FAS_CCH::Pair_Swap(bool& IsImpro)
{
	int tempStage;
	int tempMachine1, tempMachine2;
	int tempSize1, tempSize2;
	int criticalLot1, criticalLot2;
	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;

	do
	{
		//先随机地选取一个阶段
		tempStage = rand() % pStage;
		//再随机地选取一台机器
		tempMachine1 = rand() % pMachines[tempStage];
		//再随机选择一个Lot
		tempSize1 = Critical_Block[tempStage][tempMachine1].size();
	} while (tempSize1 == 0);
	int criticalIndex1 = rand() % tempSize1;
	criticalLot1 = Critical_Block[tempStage][tempMachine1][criticalIndex1];

	do
	{
		//再随机地选取一台机器
		tempMachine2 = rand() % pMachines[tempStage];
		//再随机选择一个Lot
		tempSize2 = Critical_Block[tempStage][tempMachine2].size();
	} while (tempSize2 == 0);
	int criticalIndex2 = rand() % tempSize2;
	criticalLot2 = Critical_Block[tempStage][tempMachine2][criticalIndex2];


	vector<int>::iterator it1 = find(tempMachineAssignment[tempStage][tempMachine1].begin(),
		tempMachineAssignment[tempStage][tempMachine1].end(),
		criticalLot1);

	// 找到 criticalLot2 在 tempMachineAssignment 中的迭代器
	vector<int>::iterator it2 = find(tempMachineAssignment[tempStage][tempMachine2].begin(),
		tempMachineAssignment[tempStage][tempMachine2].end(),
		criticalLot2);

	if (it1 != tempMachineAssignment[tempStage][tempMachine1].end() &&
		it2 != tempMachineAssignment[tempStage][tempMachine2].end())
	{
		// 执行交换。
		swap(*it1, *it2);
	}

	//执行修复调度
	FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
}

void FAS_CCH::Inversion(bool& IsImpro)
{
	int tempStage;
	int tempMachine;
	int tempSize;
	int exist = 0;
	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;
	for (int i = 0; i < pStage; i++)
	{
		//判断某机器上是否有两个及以上的Lot在关键路径上
		for (int m = 0; m < pMachines[i]; m++)
		{
			int critical_size = Critical_Block[i][m].size();
			if (critical_size >= 2)
			{
				exist = 1;
				break;
			}
		}
	}
	if (exist == 0)
	{
		//选定一个阶段和机器
		do
		{
			//先随机地选取一个阶段
			tempStage = rand() % pStage;
			//再随机地选取一台机器
			tempMachine = rand() % pMachines[tempStage];
			//再随机选择一个Lot
			tempSize = MachineAssignment[tempStage][tempMachine].size();
		} while (tempSize < 2);
		int index1 = rand() % tempSize;
		int index2;
		do
		{
			index2 = rand() % tempSize;
		} while (index1 == index2);
		if (index1 > index2)
		{
			swap(index1, index2);
		}
		reverse(tempMachineAssignment[tempStage][tempMachine].begin() + index1, tempMachineAssignment[tempStage][tempMachine].begin() + index2 + 1);
		//执行修复调度
		FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
	}
	else
	{
		do
		{
			// 随机选取一个阶段
			tempStage = rand() % pStage;
			// 随机选取一台机器
			tempMachine = rand() % pMachines[tempStage];
			// 检查这台机器上 关键工件 的数量
			tempSize = Critical_Block[tempStage][tempMachine].size();
		} while (tempSize < 2); // 至少有2个关键工件才能定义一个序列

		// 从关键工件列表中随机选择两个不同的工件
		int criticalIndex1 = rand() % tempSize;
		int criticalIndex2;
		do
		{
			criticalIndex2 = rand() % tempSize;
		} while (criticalIndex1 == criticalIndex2); // 确保是两个不同的关键工件索引

		// 获取工件ID
		int lot1 = Critical_Block[tempStage][tempMachine][criticalIndex1];
		int lot2 = Critical_Block[tempStage][tempMachine][criticalIndex2];

		// 在 MachineAssignment 的实际序列中找到这两个工件的位置
		// 获取实际的工件序列
		vector<int>& machine_seq = tempMachineAssignment[tempStage][tempMachine];

		// 查找 lot1 和 lot2 的迭代器
		vector<int>::iterator it1 = find(machine_seq.begin(), machine_seq.end(), lot1);
		vector<int>::iterator it2 = find(machine_seq.begin(), machine_seq.end(), lot2);

		if (it1 == machine_seq.end() || it2 == machine_seq.end())
		{
			IsImpro = false;
			return;
		}

		int index1 = distance(machine_seq.begin(), it1);
		int index2 = distance(machine_seq.begin(), it2);


		if (index1 > index2)
		{
			swap(index1, index2);
		}

		reverse(tempMachineAssignment[tempStage][tempMachine].begin() + index1,
			tempMachineAssignment[tempStage][tempMachine].begin() + index2 + 1);

		FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
	}
}

//子序列移动：在tempStage，随机选1台机器，从中取出一个子序列（比如2个或3个工件），并将其插入到该机器的另一个位置。
void FAS_CCH::Displacement(bool& IsImpro)
{
	int tempStage;
	int tempMachine;
	int tempSize;
	int criticalSize;
	int exist = 0;
	vector<vector<vector<int>>> tempMachineAssignment = MachineAssignment;

	for (int i = 0; i < pStage; i++)
	{
		for (int m = 0; m < pMachines[i]; m++)
		{
			// 至少2个关键工件、有3个工件才能使移动有意义
			if (Critical_Block[i][m].size() >= 2 && MachineAssignment[i][m].size() >= 3) //
			{
				exist = 1;
				break;
			}
		}
		if (exist == 1) break;
	}
	if (exist == 0)
	{
		//选定一个阶段和机器
		do
		{
			//先随机地选取一个阶段
			tempStage = rand() % pStage;
			//再随机地选取一台机器
			tempMachine = rand() % pMachines[tempStage];
			//再随机选择一个Lot
			tempSize = MachineAssignment[tempStage][tempMachine].size();
			criticalSize = Critical_Block[tempStage][tempMachine].size();
		} while (tempSize < 3 || criticalSize == 0);
		int index1 = rand() % tempSize;
		int index2;
		do
		{
			index2 = rand() % tempSize;
		} while (index1 == index2);
		if (index1 > index2)
		{
			swap(index1, index2);
		}
		//取出子序列
		vector<int> sub_sequence(tempMachineAssignment[tempStage][tempMachine].begin() + index1, tempMachineAssignment[tempStage][tempMachine].begin() + index2 + 1);
		//删除子序列
		tempMachineAssignment[tempStage][tempMachine].erase(tempMachineAssignment[tempStage][tempMachine].begin() + index1, tempMachineAssignment[tempStage][tempMachine].begin() + index2 + 1);
		//插入到新的位置
		int insert_position = rand() % (tempMachineAssignment[tempStage][tempMachine].size() + 1);
		tempMachineAssignment[tempStage][tempMachine].insert(tempMachineAssignment[tempStage][tempMachine].begin() + insert_position, sub_sequence.begin(), sub_sequence.end());
		//执行修复调度
		FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
	}
	else
	{
		do
		{
			tempStage = rand() % pStage;
			tempMachine = rand() % pMachines[tempStage];
		} while (Critical_Block[tempStage][tempMachine].size() < 2 || MachineAssignment[tempStage][tempMachine].size() < 3); // 找到满足条件的机器

		tempSize = Critical_Block[tempStage][tempMachine].size();

		// 从关键工件列表中随机选择两个不同的工件
		int criticalIndex1 = rand() % tempSize;
		int criticalIndex2;
		do
		{
			criticalIndex2 = rand() % tempSize;
		} while (criticalIndex1 == criticalIndex2);

		int lot1 = Critical_Block[tempStage][tempMachine][criticalIndex1];
		int lot2 = Critical_Block[tempStage][tempMachine][criticalIndex2];

		// 在 MachineAssignment的实际序列中找到这两个工件的位置
		vector<int>& machine_seq = tempMachineAssignment[tempStage][tempMachine];
		vector<int>::iterator it1 = find(machine_seq.begin(), machine_seq.end(), lot1);
		vector<int>::iterator it2 = find(machine_seq.begin(), machine_seq.end(), lot2);

		if (it1 == machine_seq.end() || it2 == machine_seq.end())
		{
			IsImpro = false;
			return;
		}

		int index1 = distance(machine_seq.begin(), it1);
		int index2 = distance(machine_seq.begin(), it2);

		if (index1 > index2)
		{
			swap(index1, index2);
		}

		vector<int> sub_sequence(machine_seq.begin() + index1, machine_seq.begin() + index2 + 1);
		machine_seq.erase(machine_seq.begin() + index1, machine_seq.begin() + index2 + 1);

		int insert_position = rand() % (machine_seq.size() + 1);
		machine_seq.insert(machine_seq.begin() + insert_position, sub_sequence.begin(), sub_sequence.end());

		FAS_Repair(tempStage, tempMachineAssignment, IsImpro);
	}
}



// 动态瓶颈
void FAS_CCH::UpdateDynamicBottleneck(int totalStages, int totalJobs, const vector<int>& JobCurrentStage) {
	// 对于每个阶段 k，计算尚未完成该阶段的工件在该阶段的剩余负载之和，除以该阶段的机器数
	double maxScore = -1.0;
	int newBottleneck = 0;

	for (int k = 0; k < totalStages; ++k) {
		double sumLoad = 0.0;
		for (int j = 0; j < totalJobs; ++j) {
			// 如果工件 j 还未完成 stage k（包含正在该阶段或尚未到达该阶段），则计入该阶段的负载
			if (JobCurrentStage[j] <= k) {
				sumLoad += pUnitTime[k][j];
			}
		}

		double score = sumLoad / pMachines[k];
		if (score > maxScore) {
			maxScore = score;
			newBottleneck = k;
		}
	}

	// 更新全局瓶颈阶段
	gBottleneckStage = newBottleneck;
}

//逐工件进行操作
void FAS_CCH::LeftInsert_Global() {
	// 原来的排序：按第一阶段完工时间升序排序
	// vector<pair<int, int>> jobSortByFirstStageCTime;
	// for (int j = 0; j < pJob; j++) {
	// 	jobSortByFirstStageCTime.push_back(make_pair(CTime[0][j], j));
	// }
	// sort(jobSortByFirstStageCTime.begin(), jobSortByFirstStageCTime.end());
	
	// 原来的瓶颈阶段排序
	// // 重新计算瓶颈阶段：总加工时间/机器数 最大的阶段
	// int bottleneck = 0;
	// double max_load = 0;
	// for (int k = 0; k < pStage; k++) {
	// 	double total_time = 0;
	// 	for (int j = 0; j < pJob; j++) {
	// 		total_time += pUnitTime[k][j];
	// 	}
	// 	if (total_time / pMachines[k] > max_load) {
	// 		max_load = total_time / pMachines[k];
	// 		bottleneck = k;
	// 	}
	// }
	// // 新的排序：按瓶颈阶段开始时间升序排序
	// vector<pair<int, int>> jobSortByBottleneckSTime;
	// for (int j = 0; j < pJob; j++) {
	// 	jobSortByBottleneckSTime.push_back(make_pair(STime[bottleneck][j], j));
	// }
	// sort(jobSortByBottleneckSTime.begin(), jobSortByBottleneckSTime.end());
	// for (int sortIdx = 0; sortIdx < jobSortByBottleneckSTime.size(); sortIdx++) {
	// 	int c_job = jobSortByBottleneckSTime[sortIdx].second;
	// 	for (int stage = 0; stage < pStage; stage++) {
	
	// 新的策略：外层遍历阶段，每个阶段根据前一阶段完工时间排序
	for (int stage = 0; stage < pStage; stage++) {
		// 根据前一阶段完工时间排序（第一阶段用最后阶段完工时间）
		vector<pair<int, int>> jobSort;
		for (int j = 0; j < pJob; j++) {
			if (stage == 0) {
				jobSort.push_back(make_pair(CTime[pStage - 1][j], j)); // 第一阶段用最后阶段完工时间
			} else {
				jobSort.push_back(make_pair(CTime[stage - 1][j], j)); // 其他阶段用前一阶段完工时间
			}
		}
		sort(jobSort.begin(), jobSort.end());
		
		for (int sortIdx = 0; sortIdx < jobSort.size(); sortIdx++) {
			int c_job = jobSort[sortIdx].second;
			int current_machine = -1;
			int current_idx = -1;
			for (int k = 0; k < MachineAssignment[stage].size(); k++) { //寻找该工件在该阶段该机器上的位置
				for (int idx = 0; idx < MachineAssignment[stage][k].size(); idx++) {
					if (MachineAssignment[stage][k][idx] == c_job) {
						current_machine = k;
						current_idx = idx;
						break;
					}
				}
				if (current_machine >= 0) break;
			}
			if (current_machine < 0) continue;

			int old_STime = STime[stage][c_job];
			int best_machine = -1;
			int best_position = -1;
			int best_STime = old_STime;

			for (int m = 0; m < MachineAssignment[stage].size(); m++) { //逐机器寻找位置插入
				vector<int> temp_seq = MachineAssignment[stage][m];
				if (m == current_machine) {
					temp_seq.erase(temp_seq.begin() + current_idx);
				}

				int job_num = temp_seq.size();

				for (int position = 0; position <= job_num; position++) {
					int OSTime, OCTime;

					if (job_num == 0) {
						if (stage == 0) {
							OSTime = 0;
						} else {
							OSTime = CTime[stage - 1][c_job];
						}
						OCTime = OSTime + pUnitTime[stage][c_job];

						if (stage < pStage - 1 && OCTime > STime[stage + 1][c_job]) {
							continue;
						}

					} else if (position == 0) {
						if (stage == 0) {
							OSTime = 0;
						} else {
							OSTime = CTime[stage - 1][c_job];
						}
						OCTime = OSTime + pUnitTime[stage][c_job];

						int next_job = temp_seq[0];
						if (OCTime > STime[stage][next_job]) {
							continue;
						}
					} else if (position == job_num) {
						if (stage == 0) {
							OSTime = CTime[stage][temp_seq[job_num - 1]];
						} else {
							OSTime = max(CTime[stage - 1][c_job], CTime[stage][temp_seq[job_num - 1]]);
						}
						OCTime = OSTime + pUnitTime[stage][c_job];
					} else {
						int prev_job = temp_seq[position - 1];
						int next_job = temp_seq[position];
						if (stage == 0) {
							OSTime = CTime[stage][prev_job];
						} else {
							OSTime = max(CTime[stage - 1][c_job], CTime[stage][prev_job]);
						}
						OCTime = OSTime + pUnitTime[stage][c_job];

						if (OCTime > STime[stage][next_job]) {
							continue;
						}
					}

					if (OSTime < best_STime) {
						best_STime = OSTime;
						best_machine = m;
						best_position = position;
						break;
					}
				}
			}

			if (best_machine >= 0 && best_STime < old_STime) {
				MachineAssignment[stage][current_machine].erase(MachineAssignment[stage][current_machine].begin() + current_idx);
				MachineAssignment[stage][best_machine].insert(MachineAssignment[stage][best_machine].begin() + best_position, c_job);

				STime[stage][c_job] = best_STime;
				CTime[stage][c_job] = best_STime + pUnitTime[stage][c_job];
			}
		}
	}
}

//逐工件进行操作
void FAS_CCH::RightInsert_Global() {
	// 原来的排序：按最后一阶段完工时间降序排序
	// vector<pair<int, int>> jobSortByLastStageCTime;
	// for (int j = 0; j < pJob; j++) {
	// 	jobSortByLastStageCTime.push_back(make_pair(CTime[pStage - 1][j], j));
	// }
	// sort(jobSortByLastStageCTime.rbegin(), jobSortByLastStageCTime.rend());
	
	// 原来的瓶颈阶段排序
	// // 重新计算瓶颈阶段：总加工时间/机器数 最大的阶段
	// int bottleneck = 0;
	// double max_load = 0;
	// for (int k = 0; k < pStage; k++) {
	// 	double total_time = 0;
	// 	for (int j = 0; j < pJob; j++) {
	// 		total_time += pUnitTime[k][j];
	// 	}
	// 	if (total_time / pMachines[k] > max_load) {
	// 		max_load = total_time / pMachines[k];
	// 		bottleneck = k;
	// 	}
	// }
	// // 新的排序：按瓶颈阶段完工时间降序排序
	// vector<pair<int, int>> jobSortByBottleneckCTime;
	// for (int j = 0; j < pJob; j++) {
	// 	jobSortByBottleneckCTime.push_back(make_pair(CTime[bottleneck][j], j));
	// }
	// sort(jobSortByBottleneckCTime.rbegin(), jobSortByBottleneckCTime.rend());
	// for (int sortIdx = 0; sortIdx < jobSortByBottleneckCTime.size(); sortIdx++) {
	// 	for (int stage = pStage - 1; stage >= 0; stage--) {
	// 		int c_job = jobSortByBottleneckCTime[sortIdx].second;
	
	// 新的策略：外层遍历阶段（倒序），每个阶段根据后一阶段开始时间排序
	for (int stage = pStage - 1; stage >= 0; stage--) {
		// 根据后一阶段开始时间排序（最后阶段用第一阶段开始时间）
		vector<pair<int, int>> jobSort;
		for (int j = 0; j < pJob; j++) {
			if (stage == pStage - 1) {
				jobSort.push_back(make_pair(STime[0][j], j)); // 最后阶段用第一阶段开始时间
			} else {
				jobSort.push_back(make_pair(STime[stage + 1][j], j)); // 其他阶段用后一阶段开始时间
			}
		}
		sort(jobSort.rbegin(), jobSort.rend()); // 降序排序
		
		for (int sortIdx = 0; sortIdx < jobSort.size(); sortIdx++) {
			int c_job = jobSort[sortIdx].second;
			int current_machine = -1;
			int current_idx = -1;
			// 寻找该工件在该阶段该机器上的位置
			for (int k = 0; k < MachineAssignment[stage].size(); k++) {
				for (int idx = 0; idx < MachineAssignment[stage][k].size(); idx++) {
					if (MachineAssignment[stage][k][idx] == c_job) {
						current_machine = k;
						current_idx = idx;
						break;
					}
				}
				if (current_machine >= 0) break;
			}
			if (current_machine < 0) continue;

			int old_STime = STime[stage][c_job];
			int best_machine = -1;
			int best_position = -1;
			int best_STime = old_STime;

			// 逐机器寻找位置插入
			for (int m = 0; m < MachineAssignment[stage].size(); m++) {
				vector<int> temp_seq = MachineAssignment[stage][m];
				if (m == current_machine) {
					// 模拟将工件移除
					temp_seq.erase(temp_seq.begin() + current_idx);
				}

				int job_num = temp_seq.size();

				// 倒序遍历插入位置
				for (int position = job_num; position >= 0; position--) {
					int temp_STime, temp_CTime;

					if (job_num == 0) {
						// 机器上没有其他工件
						if (stage == pStage - 1) {
							temp_CTime = makespan;
						} else {
							temp_CTime = STime[stage + 1][c_job];
						}
						temp_STime = temp_CTime - pUnitTime[stage][c_job];

						// 判断是否优于当前解且合法
						if (stage == 0) { // 是第0阶段，无上一阶段约束
							if (temp_STime > best_STime && temp_STime >= 0) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = 0;
							}
						} else {
							if (temp_STime > best_STime && temp_STime >= CTime[stage - 1][c_job]) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = 0;
							}
						}
					} else if (position == job_num) {
						// 插入到队尾
						if (stage == pStage - 1) {
							temp_CTime = makespan;
						} else {
							temp_CTime = STime[stage + 1][c_job];
						}
						temp_STime = temp_CTime - pUnitTime[stage][c_job];

						// 检查是否优于当前解且满足左侧约束（上一工件完工时间）
						// 此时只需看 temp_seq[job_num - 1]
						if (temp_STime > best_STime && temp_STime >= CTime[stage][temp_seq[job_num - 1]]) {
							// 还要检查上一阶段约束
							if (stage == 0) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = position;
							} else if (temp_STime >= CTime[stage - 1][c_job]) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = position;
							}
						}
					} else if (position == 0) {
						// 插入到队头
						if (stage == pStage - 1) {
							temp_CTime = min(makespan, STime[stage][temp_seq[0]]);
						} else {
							temp_CTime = min(STime[stage + 1][c_job], STime[stage][temp_seq[0]]);
						}
						temp_STime = temp_CTime - pUnitTime[stage][c_job];

						// 检查是否优于当前解 且 满足上一阶段约束
						if (stage == 0) {
							if (temp_STime > best_STime && temp_STime >= 0) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = position;
							}
						} else {
							if (temp_STime > best_STime && temp_STime >= CTime[stage - 1][c_job]) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = position;
							}
						}
					} else {
						// 插入到中间
						int prev_job = temp_seq[position - 1];
						int next_job = temp_seq[position];

						if (stage == pStage - 1) {
							temp_CTime = min(makespan, STime[stage][next_job]);
						} else {
							temp_CTime = min(STime[stage + 1][c_job], STime[stage][next_job]);
						}
						temp_STime = temp_CTime - pUnitTime[stage][c_job];

						// 检查是否优于当前解且满足左侧所有约束（前一工件 + 上一阶段）
						if (temp_STime > best_STime && temp_STime >= CTime[stage][prev_job] ) {
							if (stage == 0) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = position;
							} else if (temp_STime >= CTime[stage - 1][c_job]) {
								best_STime = temp_STime;
								best_machine = m;
								best_position = position;
							}
						}
					}
				}
			}

			// 如果找到了更好的位置
			if (best_machine >= 0 && best_STime > old_STime) {
				MachineAssignment[stage][current_machine].erase(MachineAssignment[stage][current_machine].begin() + current_idx);
				MachineAssignment[stage][best_machine].insert(MachineAssignment[stage][best_machine].begin() + best_position, c_job);

				STime[stage][c_job] = best_STime;
				CTime[stage][c_job] = best_STime + pUnitTime[stage][c_job];
			}
		}
	}
}

void FAS_CCH::RightShift()
{
	for (int i = pStage - 1; i >= 0; i--)
	{
		for (int k = pMachines[i] - 1; k >= 0; k--)
		{
			for (int j = MachineAssignment[i][k].size() - 1; j >= 0; j--)
			{
				int c_job = MachineAssignment[i][k][j];
				
				if (i == pStage - 1)
				{
					if (j == MachineAssignment[i][k].size() - 1)
					{
						CTime[i][c_job] = makespan;
						STime[i][c_job] = CTime[i][c_job] - pUnitTime[i][c_job];
					}
					else
					{
						int nextLot = MachineAssignment[i][k][j + 1];
						CTime[i][c_job] = STime[i][nextLot];
						STime[i][c_job] = CTime[i][c_job] - pUnitTime[i][c_job];
					}
				}
				else
				{
					if (j == MachineAssignment[i][k].size() - 1)
					{
						CTime[i][c_job] = STime[i + 1][c_job];
						STime[i][c_job] = CTime[i][c_job] - pUnitTime[i][c_job];
					}
					else
					{
						int nextLot = MachineAssignment[i][k][j + 1];
						CTime[i][c_job] = min(STime[i + 1][c_job], STime[i][nextLot]);
						STime[i][c_job] = CTime[i][c_job] - pUnitTime[i][c_job];
					}
				}
			}
		}
	}
	
	int minSTime = INT_MAX;
	for (int j = 0; j < pJob; j++)
	{
		if (STime[0][j] < minSTime)
		{
			minSTime = STime[0][j];
		}
	}
	
	if (minSTime > 0)
	{
		for (int i = 0; i < pStage; i++)
		{
			for (int j = 0; j < pJob; j++)
			{
				STime[i][j] -= minSTime;
				CTime[i][j] -= minSTime;
			}
		}
		makespan -= minSTime;
	}
}


void FAS_CCH::reverse_schedule()
{
	reverse(MachineAssignment.begin(), MachineAssignment.end());
	for (int k = 0; k < pStage; k++)
	{
		for (int m = 0; m < MachineAssignment[k].size(); m++)
		{
			reverse(MachineAssignment[k][m].begin(), MachineAssignment[k][m].end());
		}
	}
}


void FAS_CCH::reverse_problem()
{
	backup_pMachines = pMachines;
	backup_pUnitTime = pUnitTime;
	backup_pSetupTime = pSetupTime;
	backup_pTransferTime = pTransferTime;
	backup_gBottleneckStage = gBottleneckStage;
	
	reverse(pMachines.begin(), pMachines.end());
	reverse(pUnitTime.begin(), pUnitTime.end());
	
	double max_load_ratio = -1.0;
	gBottleneckStage = 0;
	for (int k = 0; k < pStage; k++)
	{
		double stage_total_time = 0;
		for (int j = 0; j < pJob; j++)
		{
			stage_total_time += pUnitTime[k][j];
		}
		double load_ratio = stage_total_time / pMachines[k];
		if (load_ratio > max_load_ratio)
		{
			max_load_ratio = load_ratio;
			gBottleneckStage = k;
		}
	}
}



void FAS_CCH::reinitialize()
{
	JobCurrentStage.assign(pJob, 0);
	
	AS.clear();
	for (int i = 0; i < pJob; i++)
	{
		AS.push_back(JobOperation(0, i));
	}
	
	STime.assign(pStage, vector<int>(pJob, 0));
	CTime.assign(pStage, vector<int>(pJob, 0));
	STime_back.assign(pStage, vector<int>(pJob, 0));
	CTime_back.assign(pStage, vector<int>(pJob, 0));
	
	StageRemainingLoad.resize(pStage, 0.0);
	for(int i=0;i<pStage;i++)
	{
		double total_load = 0.0;
		for(int j=0;j<pJob;j++)
		{
			total_load += pUnitTime[i][j];
		}
		StageRemainingLoad[i] = total_load;
	}

	CachedTails.assign(pStage, vector<double>(pJob, 0.0));
	for (int j = 0; j < pJob; j++)
	{
		double running_tail = 0.0;
		// 倒序遍历阶段
		for (int k = pStage - 1; k >= 0; k--)
		{
			// 对于阶段 k，它的 tail 是阶段 k+1 到最后的和
			// 即上一次循环积累的 running_tail
			CachedTails[k][j] = running_tail;
			
			// 更新 running_tail，把当前阶段的时间加进去，供前一个阶段使用
			running_tail += pUnitTime[k][j];
		}
	}

	MachineAssignment.clear();
	MachineAssignment.resize(pStage);
	for (int i = 0; i < pStage; i++)
	{
		MachineAssignment[i].resize(pMachines[i]);
	}
	
}


void FAS_CCH::run()
{
	while (AS.size() != 0)
	{
		Update_Current_Remaining_time(); 
        for (int i = 0; i < AS.size(); i++)
        {
            AS[i].get_rule_value(Max_SPTB, Min_SPTB,
                Max_AfterBN, Min_AfterBN,
                Max_Total, Min_Total,
                Max_Current, Min_Current,
				Max_est, Min_est);
            AS[i].get_combination_value(SPTB_weight, AfterBN_weight, Total_weight,
                Current_weight,est_weight); 
        }

        double Max_value = -1e9;
        current_Job = -1; 
        current_Stage = -1;
        operation_position = -1;

        for (int i = 0; i < AS.size(); i++)
        {
            if (AS[i].combination_value > Max_value)
            {
                Max_value = AS[i].combination_value;
                current_Job = AS[i].cJob;
                current_Stage = AS[i].cStage;
                operation_position = i;
            }
        }
		// StageRemainingLoad[current_Stage] -= pUnitTime[current_Stage][current_Job];
        if (current_Stage == 0)
        {
            // LeftShift_Construct();  // SAS,使用LS
			LeftInsert();
			// OutputWholeSchedule(current_Stage,current_Job-1);
        }
        else
        {
            // LeftShift_Construct();  // SAS,使用LS
			LeftInsert();
			// OutputWholeSchedule(current_Stage,current_Job-1);
            RightInsert();
			// OutputWholeSchedule(current_Stage+999,current_Job);
        }
		// cout<<"已调度阶段:"<<current_Stage<<"，工件："<<current_Job<<endl;
		// for(int i=0;i<AS.size();i++)
		// {
		// 	cout<<"阶段:"<<AS[i].cStage<<"，工件:"<<AS[i].cJob<<", SPTB:"<<AS[i].SPTB_value<<", AfterBN:"<<AS[i].AfterBN_value<<", Total:"<<AS[i].Total_LPT_value<<", Iflow:"<<AS[i].Iflow_value<<", Value:"<<AS[i].combination_value<<endl;
		// }
		AS.erase(AS.begin() + operation_position);

		JobCurrentStage[current_Job]++;
		
		if (current_Stage + 1 < pStage)
		{
			JobOperation joo = JobOperation(current_Stage + 1, current_Job);
			AS.push_back(joo);
		}
		
		// OutputWholeSchedule(current_Stage,current_Job-1);
	}

	LeftShift();
	makespan = 0;
	for (int j = 0; j < pJob; j++)
	{
		if (CTime[pStage - 1][j] > makespan)
		{
			makespan = CTime[pStage - 1][j];
		}
	}
	// OutputWholeSchedule(4567,99999);
	int no_improve_count = 0;
	int kkk=1;
	while (no_improve_count < 2)
	{
		int old_makespan = makespan;
		LeftInsert_Global();//左插
		// OutputWholeSchedule(3000+kkk,99999);
		// kkk++;
		LeftShift();//左移
		// OutputWholeSchedule(3000+kkk,99999);
		// kkk++;
		makespan = 0;
		for (int j = 0; j < pJob; j++)
		{
			if (CTime[pStage - 1][j] > makespan)
			{
				makespan = CTime[pStage - 1][j];
			}
		}
		RightInsert_Global();//右插
		// OutputWholeSchedule(3000+kkk,99999);
		// kkk++;
		RightShift();//右移
		// OutputWholeSchedule(3000+kkk,99999);
		// kkk++;
		if (makespan < old_makespan)
		{
			no_improve_count = 0;
		}
		else
		{
			no_improve_count++;
		}
	}
	LeftShift();
}


void FAS_CCH::run_with_reversibility()
{
	run();
	// OutputWholeSchedule(2,0);
	//反向求解
	// int makespan_forward = makespan;
	// vector<vector<vector<int>>> MA_forward = MachineAssignment;
	// vector<vector<int>> STime_forward = STime;
	// vector<vector<int>> CTime_forward = CTime;
	
	// vector<int> saved_pMachines = pMachines;
	// vector<vector<int>> saved_pUnitTime = pUnitTime;
	
	// reverse_problem();
	// reinitialize();
	// run();
	// reverse_schedule();
	
	// pMachines = saved_pMachines;
	// pUnitTime = saved_pUnitTime;
	
	// LeftShift();
	
	// int makespan_reverse = 0;
	// for (int j = 0; j < pJob; j++)
	// {
	// 	if (CTime[pStage - 1][j] > makespan_reverse)
	// 	{
	// 		makespan_reverse = CTime[pStage - 1][j];
	// 	}
	// }
	// // OutputWholeSchedule(2,1);
	// if (makespan_forward <= makespan_reverse)
	// {
	// 	makespan = makespan_forward;
	// 	MachineAssignment = MA_forward;
	// 	STime = STime_forward;
	// 	CTime = CTime_forward;
	// }
	// else
	// {

	// 	// cout<<"反向更优"<<endl;
	// 	makespan = makespan_reverse;
	// }
	
	// int no_improve_count = 0;
	// while (no_improve_count < 2)
	// {
	// 	int old_makespan = makespan;
		
	// 	LeftInsert_Global();//左插
	// 	LeftShift();//左移
	// 	RightInsert_Global();//右插
	// 	RightShift();//右移
	// 	makespan = 0;
	// 	for (int j = 0; j < pJob; j++)
	// 	{
	// 		if (CTime[pStage - 1][j] > makespan)
	// 		{
	// 			makespan = CTime[pStage - 1][j];
	// 		}
	// 	}
		
	// 	if (makespan < old_makespan)
	// 	{
	// 		no_improve_count = 0;
	// 	}
	// 	else
	// 	{
	// 		no_improve_count++;
	// 	}
	// }
	// LeftShift();

	// bool IsImprove = false;
	// int count_fails = 0;
	// int total_fails = 0;
	// int neighborhood_index = 0; 

	// Backward_Decoding();
	// GenerateCriticalBlockRanges();

	// while (total_fails < 1000)
	// {
	// 	if (neighborhood_index == 0)
	// 	{
	// 		// 优先使用基于强制操作的搜索
	// 		MandatoryLocalSearch(IsImprove);
	// 		if(!IsImprove)
	// 		{
	// 			count_fails=60;
	// 		}
	// 	}
	// 	else if (neighborhood_index == 1)
	// 	{
	// 		Ns1_ReverseEdge(IsImprove);
	// 	}
	// 	else if (neighborhood_index == 2)
	// 	{
	// 		Ns2_MoveInnerToEdge(IsImprove);
	// 	}
	// 	else if (neighborhood_index == 3)
	// 	{
	// 		Ns3_ReinsertEdge(IsImprove);
	// 	}
	// 	else if (neighborhood_index == 4)
	// 	{
	// 		Nc_CrossMachineInsert(IsImprove);
	// 	}
		
	// 	if (IsImprove)
	// 	{
	// 		cout << "成功更新 (使用邻域 " << neighborhood_index << ")" << endl;
	// 		IsImprove = false;
	// 		neighborhood_index = 0; // 重置为0，优先尝试强制操作搜索
			
	// 		count_fails = 0;
	// 		total_fails = 0;
	// 		LeftShift();
	// 		makespan=0;
	// 		for (int j = 0; j < pJob; j++)
	// 		{
	// 			if (CTime[pStage - 1][j] > makespan)
	// 			{
	// 				makespan = CTime[pStage - 1][j];
	// 			}
	// 		}
	// 		// 后续的左移插入优化
	// 		int no_improve_count = 0;
	// 		while (no_improve_count < 2)
	// 		{
	// 			int old_makespan = makespan;
				
	// 			LeftInsert_Global();
	// 			LeftShift();
	// 			RightInsert_Global();
	// 			RightShift();
	// 			makespan = 0;

	// 			for (int j = 0; j < pJob; j++)
	// 			{
	// 				if (CTime[pStage - 1][j] > makespan)
	// 				{
	// 					makespan = CTime[pStage - 1][j];
	// 				}
	// 			}
				
	// 			if (makespan < old_makespan)
	// 			{
	// 				no_improve_count = 0;
	// 			}
	// 			else
	// 			{
	// 				no_improve_count++;
	// 			}
	// 		}
	// 		Backward_Decoding();
	// 		GenerateCriticalBlockRanges();
	// 	}
	// 	else
	// 	{
	// 		count_fails++;
	// 		total_fails++;
	// 	}
		
	// 	if (count_fails > 60)
	// 	{
	// 		neighborhood_index++;
	// 		count_fails = 0;

	// 		if (neighborhood_index > 4)
	// 		{
	// 			neighborhood_index = 0; // 循环回到强制操作搜索
	// 		}
	// 	}
	// }
	// LeftShift();
	// makespan = 0;
	// for (int j = 0; j < pJob; j++)
	// {
	// 	if (CTime[pStage - 1][j] > makespan)
	// 	{
	// 		makespan = CTime[pStage - 1][j];
	// 	}
	// }
}