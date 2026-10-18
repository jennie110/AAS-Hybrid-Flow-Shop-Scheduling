#pragma once
#include <vector>
using namespace std;

// 全局数据
extern int pJob;
extern int pStage;
extern vector<int> pMachines;              // [stage]
extern vector<vector<int>> pUnitTime;      // [stage][job]
extern vector<vector<int>> pSetupTime;     // [stage][job]

// 调度结构
struct Schedule {
    vector<vector<int>> STime;                    // [stage][job]
    vector<vector<int>> CTime;                    // [stage][job]
    vector<vector<vector<int>>> chromSMJ;         // [stage][machineRow][position]
    int cmax = 0;
};
