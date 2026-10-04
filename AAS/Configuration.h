#pragma once
#include <vector>
#include <cmath>

using namespace std;

class Configuration
{
public:
	Configuration();

	Configuration(vector<int> &ValueForNumericalParemeter_Integer, vector<double> &ValueForNumericalParemeter_Real, vector<int> &ValueForCategoricalParemeter);


	vector<int> ValueForCategoricalParemeter;
	vector<int> ValueForNumericalParemeter_Integer;
	vector<double> ValueForNumericalParemeter_Real;


public:
	double CostValues[IndependentRuns]; //the Cost values obtained by the independent runs
	double avgValue;     //the averaged value obtained through the independent runs
	double devValue;      //the standard deviations obtained through the independent runs

	double rankIndex;     // to sort the elite configurations at the end of a race
	double probability;   //the probability to be selected as the parent configuration

	bool IsWorse;


public:
	void getAvgValue();
};

Configuration::Configuration()
{

}

Configuration::Configuration(vector<int> &ValueForNumericalParemeter_Integer, vector<double> &ValueForNumericalParemeter_Real, vector<int> &ValueForCategoricalParemeter)
{
	this->ValueForNumericalParemeter_Integer = ValueForNumericalParemeter_Integer;
	this->ValueForNumericalParemeter_Real = ValueForNumericalParemeter_Real;
	this->ValueForCategoricalParemeter = ValueForCategoricalParemeter;

	avgValue = 0.0;
	devValue = 0.0;
	rankIndex = 0.0;
	IsWorse = false;

	for (int i = 0; i < IndependentRuns; i++)
	{
		CostValues[i] = 0.0;
	}

	probability = 0.0;
}

void Configuration::getAvgValue()
{
	//求n次独立运行的平均值和方差  averageDistance[i]
	avgValue = 0.0;
	for (int i = 0; i < IndependentRuns; i++)
	{
		avgValue += CostValues[i];
	}
	avgValue = avgValue / IndependentRuns;


	double sum = 0.0;
	for (int i = 0; i < IndependentRuns; i++)
	{
		sum += (CostValues[i] - avgValue)*(CostValues[i] - avgValue);
	}
	devValue = sqrt(sum / (IndependentRuns));

	rankIndex += avgValue;

}

//void Configuration::getCostValue(vector<Individual> &TPF, int MSUpper, int MSLower, int NOSUpper, int NOSLower)
//{
//	//每次独立运行
//	for (int i = 0; i < ParetoFronts.size(); i++)
//	{
//		//存放每个pareto point和TPF之间的minimum Euclidean distance
//		vector<double> d;
//
//		d.resize(TPF.size());
//		for (int j = 0; j < TPF.size(); j++)
//		{
//			d[j] = INT_MAX;
//		}
//
//		for (int j = 0; j < TPF.size(); j++)
//		{
//			double distance;
//			double d1, d2;
//			for (int n = 0; n < ParetoFronts[i].size(); n++)
//			{
//				d1 = ((ParetoFronts[i][n].MS - MSLower) *1.0 / (MSUpper - MSLower) - (TPF[j].MS - MSLower)*1.0 / (MSUpper - MSLower))*((ParetoFronts[i][n].MS - MSLower)*1.0 / (MSUpper
//					- MSLower) - (TPF[j].MS - MSLower) / (MSUpper - MSLower));
//				d2 = ((ParetoFronts[i][n].NOS - NOSLower) * 1.0 / (NOSUpper - NOSLower) - (TPF[j].NOS - NOSLower) * 1.0 / (NOSUpper - NOSLower))*((ParetoFronts[i][n].NOS - NOSLower) *1.0 / (NOSUpper - NOSLower) - (TPF[j].NOS - NOSLower) * 1.0 / (NOSUpper - NOSLower));
//				distance = sqrt(d1 + d2);
//				if (distance < d[j])
//				{
//					d[j] = distance;
//				}
//			}
//		}
//
//		for (int j = 0; j < TPF.size(); j++)
//		{
//			CostValues[i] += d[j];
//		}
//
//		CostValues[i] = CostValues[i] / TPF.size();
//
//	}
//
//
//	//求n次独立运行的平均值和方差  averageDistance[i]
//	avgValue = 0.0;
//	for (int i = 0; i < IndependentRuns; i++)
//	{
//		avgValue += CostValues[i];
//	}
//	avgValue = avgValue / IndependentRuns;
//
//
//	double sum = 0.0;
//	for (int i = 0; i < IndependentRuns; i++)
//	{
//		sum += (CostValues[i] - avgValue)*(CostValues[i] - avgValue);
//	}
//	devValue = sqrt(sum / (IndependentRuns));
//
//
//	rankIndex += avgValue;
//}

