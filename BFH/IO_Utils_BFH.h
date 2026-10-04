#pragma once
#include "HFSP_BFH.h"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <direct.h>

using namespace std;

inline bool readInstanceData(const string& filePath) {
    ifstream fin(filePath);
    if (!fin.is_open()) {
        cerr << "无法打开实例文件: " << filePath << endl;
        return false;
    }

    if (!(fin >> pJob >> pStage)) {
        cerr << "读取 n,m 失败: " << filePath << endl;
        fin.close();
        return false;
    }

    pMachines.assign(pStage, 0);
    for (int i = 0; i < pStage; ++i) {
        if (!(fin >> pMachines[i])) {
            cerr << "读取机器数失败: " << filePath << endl;
            fin.close();
            return false;
        }
    }

    pUnitTime.assign(pStage, vector<int>(pJob, 0));
    for (int i = 0; i < pStage; ++i) {
        for (int j = 0; j < pJob; ++j) {
            if (!(fin >> pUnitTime[i][j])) {
                cerr << "读取加工时间失败: " << filePath << endl;
                fin.close();
                return false;
            }
        }
    }

    pSetupTime.assign(pStage, vector<int>(pJob, 0));

    fin.close();
    return true;
}

inline void ensureResultsFolder() {
    _mkdir("results");
    _mkdir("results\\BFH");
}

inline void writeBFHScheduleFile(const string& filePath, const Schedule& sched) {
    ofstream fout(filePath);
    if (!fout.is_open()) {
        cerr << "无法写入调度文件: " << filePath << endl;
        return;
    }

    int maxMachines = 0;
    for (int m : pMachines) if (m > maxMachines) maxMachines = m;

    fout << "cJob = " << pJob << ";" << endl;
    fout << endl;
    fout << "cStage = " << pStage << ";" << endl;
    fout << endl;

    fout << "PMachine = [";
    for (int i = 0; i < pStage; ++i) {
        fout << pMachines[i];
        if (i + 1 != pStage) fout << ",";
    }
    fout << "];" << endl;
    fout << endl;

    for (int s = 0; s < pStage; ++s) {
        fout << "STime" << (s + 1) << "=[";
        for (int j = 0; j < pJob; ++j) {
            fout << sched.STime[s][j] << ",";
        }
        fout << "];" << endl;
    }
    fout << endl;

    for (int s = 0; s < pStage; ++s) {
        fout << "CTime" << (s + 1) << "=[";
        for (int j = 0; j < pJob; ++j) {
            fout << sched.CTime[s][j] << ",";
        }
        fout << "];" << endl;
    }
    fout << endl;

    for (int s = 0; s < pStage; ++s) {
        fout << "chromSMJ" << (s + 1) << "=[" << endl;
        for (int r = 0; r < maxMachines; ++r) {
            for (int j = 0; j < pJob; ++j) {
                int v = sched.chromSMJ[s][r][j];
                if (v == 1000) {
                    fout << 1000;
                }
                else {
                    fout << (v + 1);
                }
                if (j + 1 != pJob) fout << ",";
            }
            fout << ";" << endl;
        }
        fout << "];" << endl;
        fout << endl;
    }

    fout << "UnitTime = [" << endl;
    for (int i = 0; i < pStage; ++i) {
        fout << "[";
        for (int j = 0; j < pJob; ++j) {
            fout << pUnitTime[i][j] << ",";
        }
        fout << "]," << endl;
    }
    fout << "];" << endl;
    fout << endl;

    fout << "SetupTime = [" << endl;
    for (int i = 0; i < pStage; ++i) {
        for (int j = 0; j < pJob; ++j) {
            fout << pSetupTime[i][j];
            if (j + 1 != pJob) fout << ",";
        }
        fout << ";" << endl;
    }
    fout << "];" << endl;

    fout.close();
}
