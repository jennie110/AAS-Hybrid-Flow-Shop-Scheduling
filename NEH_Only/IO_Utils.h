#pragma once
#include "HFSP_DABC.h"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <direct.h>

using namespace std;

inline bool readInstanceData(const string& filePath) {
    ifstream fin(filePath);
    if (!fin.is_open()) {
        cerr << "Cannot open instance file: " << filePath << endl;
        return false;
    }

    if (!(fin >> pJob >> pStage)) {
        cerr << "Failed to read n, m from: " << filePath << endl;
        fin.close();
        return false;
    }

    pMachines.resize(pStage);
    for (int k = 0; k < pStage; ++k) {
        if (!(fin >> pMachines[k])) {
            cerr << "Failed to read machine count for stage k=" << k << endl;
            fin.close();
            return false;
        }
    }

    pUnitTime.assign(pStage, vector<int>(pJob));
    for (int k = 0; k < pStage; ++k)
        for (int j = 0; j < pJob; ++j)
            if (!(fin >> pUnitTime[k][j])) {
                cerr << "Failed to read p[" << k << "][" << j << "]" << endl;
                fin.close();
                return false;
            }

    pSetupTime.assign(pStage, vector<int>(pJob, 0));
    pTransferTime.assign(pStage, vector<int>(pJob, 0));

    fin.close();
    g_bottleneckStageCached = -1;
    return true;
}

inline void writeScheduleFile(const Schedule& sch, const string& algoFolder,
    const string& instanceBaseName, int runIdx) {
    _mkdir("results");
    string algoPath = "results/" + algoFolder;
    _mkdir(algoPath.c_str());

    char outPath[512];
    sprintf_s(outPath, sizeof(outPath), "%s/%s_%d_schedule.txt",
        algoPath.c_str(), instanceBaseName.c_str(), runIdx);

    ofstream fout(outPath);
    if (!fout.is_open()) return;

    fout << "cJob = " << pJob << ";" << endl;
    fout << "cStage = " << pStage << ";" << endl;

    fout << "PMachine = [";
    for (int k = 0; k < pStage; ++k)
        fout << pMachines[k] << (k + 1 == pStage ? "" : ",");
    fout << "];" << endl;

    for (int k = 0; k < pStage; ++k) {
        fout << "STime" << (k + 1) << "=[";
        for (int j = 0; j < pJob; ++j)
            fout << sch.S[k][j] << (j + 1 == pJob ? "" : ",");
        fout << "];" << endl;
    }

    for (int k = 0; k < pStage; ++k) {
        fout << "CTime" << (k + 1) << "=[";
        for (int j = 0; j < pJob; ++j)
            fout << sch.C[k][j] << (j + 1 == pJob ? "" : ",");
        fout << "];" << endl;
    }

    for (int k = 0; k < pStage; ++k) {
        fout << "chromSMJ" << (k + 1) << "=[" << endl;
        int maxLines = max(5, pMachines[k]);
        for (int i = 0; i < maxLines; ++i) {
            if (i < pMachines[k]) {
                int filled = (int)sch.machineSeq[k][i].size();
                for (int t = 0; t < pJob; ++t) {
                    if (t < filled) fout << sch.machineSeq[k][i][t];
                    else fout << 1000;
                    fout << (t + 1 == pJob ? "" : ",");
                }
            }
            else {
                for (int t = 0; t < pJob; ++t)
                    fout << 1000 << (t + 1 == pJob ? "" : ",");
            }
            fout << ";" << endl;
        }
        fout << "];" << endl;
    }

    fout << "UnitTime = [" << endl;
    for (int k = 0; k < pStage; ++k) {
        fout << "[";
        for (int j = 0; j < pJob; ++j)
            fout << pUnitTime[k][j] << (j + 1 == pJob ? "" : ",");
        fout << "]," << endl;
    }
    fout << "];" << endl;

    fout << "SetupTime = [" << endl;
    for (int k = 0; k < pStage; ++k) {
        for (int j = 0; j < pJob; ++j)
            fout << pSetupTime[k][j] << (j + 1 == pJob ? "" : ",");
        fout << ";" << endl;
    }
    fout << "];" << endl;

    fout.close();
}

inline void writeSummaryFiles(const string& algoFolder, const string& instanceFileName,
    int makespan, double timeSeconds) {
    string algoPath = "results/" + algoFolder;
    _mkdir(algoPath.c_str());

    char summaryPath[512];
    sprintf_s(summaryPath, sizeof(summaryPath), "%s/%d_%d.txt",
        algoPath.c_str(), pJob, pStage);

    ofstream fout(summaryPath, ios::app);
    if (fout.is_open()) {
        fout << instanceFileName << ": " << makespan << endl;
        fout.close();
    }

    char timePath[512];
    sprintf_s(timePath, sizeof(timePath), "%s/time_%d_%d.txt",
        algoPath.c_str(), pJob, pStage);

    ofstream fout_time(timePath, ios::app);
    if (fout_time.is_open()) {
        fout_time << instanceFileName << ": " << fixed << setprecision(6)
            << timeSeconds << endl;
        fout_time.close();
    }
}

inline void writeScheduleFile(const Schedule& sch, const string& algoFolder,
    const string& instanceFileName) {
    string baseName = instanceFileName;
    size_t lastDot = baseName.find_last_of('.');
    if (lastDot != string::npos) {
        baseName = baseName.substr(0, lastDot);
    }

    _mkdir("results");
    string algoPath = "results/" + algoFolder;
    _mkdir(algoPath.c_str());

    char outPath[512];
    sprintf_s(outPath, sizeof(outPath), "%s/%s_0_schedule.txt",
        algoPath.c_str(), baseName.c_str());

    ofstream fout(outPath);
    if (!fout.is_open()) return;

    fout << "cJob = " << pJob << ";" << endl;
    fout << "cStage = " << pStage << ";" << endl;

    fout << "PMachine = [";
    for (int k = 0; k < pStage; ++k)
        fout << pMachines[k] << (k + 1 == pStage ? "" : ",");
    fout << "];" << endl;

    for (int k = 0; k < pStage; ++k) {
        fout << "STime" << (k + 1) << "=[";
        for (int j = 0; j < pJob; ++j)
            fout << sch.S[k][j] << (j + 1 == pJob ? "" : ",");
        fout << "];" << endl;
    }

    for (int k = 0; k < pStage; ++k) {
        fout << "CTime" << (k + 1) << "=[";
        for (int j = 0; j < pJob; ++j)
            fout << sch.C[k][j] << (j + 1 == pJob ? "" : ",");
        fout << "];" << endl;
    }

    for (int k = 0; k < pStage; ++k) {
        fout << "chromSMJ" << (k + 1) << "=[" << endl;
        int maxLines = max(5, pMachines[k]);
        for (int i = 0; i < maxLines; ++i) {
            if (i < pMachines[k]) {
                int filled = (int)sch.machineSeq[k][i].size();
                for (int t = 0; t < pJob; ++t) {
                    if (t < filled) fout << sch.machineSeq[k][i][t];
                    else fout << 1000;
                    fout << (t + 1 == pJob ? "" : ",");
                }
            }
            else {
                for (int t = 0; t < pJob; ++t)
                    fout << 1000 << (t + 1 == pJob ? "" : ",");
            }
            fout << ";" << endl;
        }
        fout << "];" << endl;
    }

    fout << "UnitTime = [" << endl;
    for (int k = 0; k < pStage; ++k) {
        fout << "[";
        for (int j = 0; j < pJob; ++j)
            fout << pUnitTime[k][j] << (j + 1 == pJob ? "" : ",");
        fout << "]," << endl;
    }
    fout << "];" << endl;

    fout << "SetupTime = [" << endl;
    for (int k = 0; k < pStage; ++k) {
        for (int j = 0; j < pJob; ++j)
            fout << pSetupTime[k][j] << (j + 1 == pJob ? "" : ",");
        fout << ";" << endl;
    }
    fout << "];" << endl;

    fout.close();
}