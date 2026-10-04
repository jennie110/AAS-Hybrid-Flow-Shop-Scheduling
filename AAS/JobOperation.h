#pragma once
#include "HFSP_CS.h"
extern int gBottleneckStage;

//class JobOperation
//{
//public:
//
//	JobOperation(int _cStage, int _cLot)
//	{
//		this->cStage = _cStage;
//		this->cJob = _cLot;
//
//		this->current_time = pUnitTime[cStage][cJob];//当前工件本阶段加工时间
//
//		this->remaing_time = 0;
//		for (int i = cStage+1; i < pStage; i++)
//		{
//			this->remaing_time += pUnitTime[i][cJob];//当前工件剩余加工时间
//		}
//	}
//
//public:
//	int cStage;   //操作所属的阶段
//	int cJob;   //操作所属的工序
//
//
//	double current_time;
//	double remaing_time;
//
//
//	double LPT_value;
//	double SPT_value;
//	double LWR_value;
//	double MWR_value;
//	double EPF_value;
//	double LPF_value;
//
//	double combination_value;
//
//public:
//	void get_rule_value(double Max_Current_time, double Min_Current_time,
//		double Max_Remaining_time, double Min_Remaining_time);
//
//
//	void get_combination_value(double LPT_weight, double SPT_weight, double LWR_weight,
//		double MWR_weight, double EPF_weight, double LPF_weight);
//
//};
//
//
//void JobOperation::get_rule_value(double Max_Current_time, double Min_Current_time,
//	double Max_Remaining_time, double Min_Remaining_time)
//{
//
//	if (Max_Current_time - Min_Current_time == 0)
//	{
//		SPT_value = 1;
//		LPT_value = 0;
//	}
//	else
//	{
//		SPT_value = 1 - (current_time - Min_Current_time) * 1.0 / (Max_Current_time - Min_Current_time);
//		LPT_value = (current_time - Min_Current_time) * 1.0 / (Max_Current_time - Min_Current_time);
//	}
//
//
//	if (Max_Remaining_time - Min_Remaining_time == 0)
//	{
//		LWR_value = 1.0;
//		MWR_value = 0.0;
//	}
//	else
//	{
//		LWR_value = 1 - (remaing_time - Min_Remaining_time) * 1.0 / (Max_Remaining_time - Min_Remaining_time);
//		MWR_value = (remaing_time - Min_Remaining_time) * 1.0 / (Max_Remaining_time - Min_Remaining_time);
//	}
//
//
//	EPF_value = 1 - (cStage - 0) * 1.0 / (pStage - 0);
//	LPF_value = (cStage - 0) * 1.0 / (pStage - 0);
//
//}
//
//
//void JobOperation::get_combination_value(double LPT_weight, double SPT_weight, double LWR_weight,
//	double MWR_weight, double EPF_weight, double LPF_weight)
//{
//	combination_value = LPT_weight * LPT_value + SPT_weight * SPT_value + LWR_weight * LWR_value +
//		MWR_weight * MWR_value + EPF_weight * EPF_value + LPF_weight * LPF_value;
//}

class JobOperation
{
public:

	JobOperation(int _cStage, int _cLot)
	{
		this->cStage = _cStage;
		this->cJob = _cLot;

		// 计算原始物理特征值

		//  当前阶段加工时间
		this->raw_Current = pUnitTime[cStage][cJob];
		// this->raw_Current = pStage-cStage;

		// 总加工时间
		this->raw_Total = 0.0;
		for (int i = cStage; i < pStage; i++) {
			this->raw_Total += pUnitTime[i][cJob];
		}

		// 从第一个阶段到瓶颈阶段的时间
		this->raw_SPTB = 0.0;
		// int limit_bn = (gBottleneckStage >= pStage) ? (pStage - 1) : gBottleneckStage;
		int limit_bn = gBottleneckStage;
		for (int i = cStage; i <= limit_bn; i++) {
			this->raw_SPTB += pUnitTime[i][cJob];
		}

		// 从瓶颈阶段到最后阶段的时间
		this->raw_AfterBN = 0.0;
		for (int i = limit_bn; i < pStage; i++) {
			this->raw_AfterBN += pUnitTime[i][cJob];
		}

		// this->raw_lastjob = pStage - cStage - 1;
		this->raw_est=0.0;


	}

public:
	int cStage;   //操作所属的阶段
	int cJob;   //操作所属的工序

	
	double raw_Current;//当前阶段加工时间
	double raw_Total;//总加工时间
	double raw_SPTB;//从第一个阶段到瓶颈阶段的时间
	double raw_AfterBN;//从瓶颈阶段到最后阶段的时间
	// double raw_lastjob;//剩余工序数
	double raw_est;//当前阶段最早开始时间
	// 归一化后的分值
	double SPTB_value;      //SPTB(越小越好)
	double AfterBN_value;   // (越大越好)
	double Total_LPT_value; // LPT (越大越好 )
	double Current_value; //CSPT (越小越好)
	// double lastjob_value; //剩余工序数(越大越好)
	double est_value;//当前阶段最早开始时间
	double combination_value;

public:
	// 更新参数列表以接收特征的极值
	void get_rule_value(
		double Max_SPTB, double Min_SPTB,
		double Max_AfterBN, double Min_AfterBN,
		double Max_Total, double Min_Total,
		double Max_Current, double Min_Current,
		double Max_est, double Min_est);
		// double Max_lastjob, double Min_lastjob);

	// 更新参数列表以接收权重
	void get_combination_value(
		double SPTB_weight, double AfterBN_weight, double Total_weight,
		double Current_weight,double est_weight);

	// 当瓶颈阶段变化时，刷新 raw_SPTB 和 raw_AfterBN
	void RefreshSPTB_AfterBN();

};


void JobOperation::get_rule_value(
	double Max_SPTB, double Min_SPTB,
	double Max_AfterBN, double Min_AfterBN,
	double Max_Total, double Min_Total,
	double Max_Current, double Min_Current,
    double Max_est, double Min_est)
{
	//LPT：总加工时间
	if (Max_Total - Min_Total == 0) Total_LPT_value = 1.0; 
	else Total_LPT_value =1.0- (raw_Total - Min_Total) * 1.0 / (Max_Total - Min_Total);

	//SPTB：从第 1 阶段到瓶颈阶段（含）的所有加工时间之和
	if (Max_SPTB - Min_SPTB == 0) SPTB_value = 1.0;
	else SPTB_value =1.0- (raw_SPTB - Min_SPTB) * 1.0 / (Max_SPTB - Min_SPTB);

	//AfterBN：瓶颈后加工时间
	if (Max_AfterBN - Min_AfterBN == 0) AfterBN_value = 0.0;
	else AfterBN_value = (raw_AfterBN - Min_AfterBN) * 1.0 / (Max_AfterBN - Min_AfterBN);

	//CSPT：当前阶段加工时间
	if (Max_Current - Min_Current == 0) Current_value = 1.0;
	else Current_value = 1.0 - (raw_Current - Min_Current) * 1.0 / (Max_Current - Min_Current);
	// else Current_value = (raw_Current - Min_Current) * 1.0 / (Max_Current - Min_Current);

	if(Max_est - Min_est ==0) est_value=1.0;
	else est_value = 1.0-(raw_est - Min_est) * 1.0 / (Max_est - Min_est);
}


void JobOperation::get_combination_value(
	double SPTB_weight, double AfterBN_weight, double Total_weight,
	double Current_weight,double est_weight)
{
	combination_value =
		SPTB_weight * SPTB_value +
		AfterBN_weight * AfterBN_value +
		Total_weight * Total_LPT_value +
		Current_weight * Current_value +
		// lastjob_weight * lastjob_value;
		est_weight * est_value;
}


void JobOperation::RefreshSPTB_AfterBN()
{
	// 根据当前 gBottleneckStage 重新计算 raw_SPTB 和 raw_AfterBN
	int limit_bn = (gBottleneckStage >= pStage) ? (pStage - 1) : gBottleneckStage;

	// 从第一个阶段到瓶颈阶段的时间
	this->raw_SPTB = 0.0;
	for (int i = cStage; i <= limit_bn; i++) {
		this->raw_SPTB += pUnitTime[i][cJob];
	}

	// 从瓶颈阶段到最后阶段的时间
	this->raw_AfterBN = 0.0;
	for (int i = limit_bn; i < pStage; i++) {
		this->raw_AfterBN += pUnitTime[i][cJob];
	}
}
