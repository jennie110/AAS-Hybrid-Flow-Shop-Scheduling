#include <iostream>
#define NOMINMAX
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <direct.h>
#include <iomanip>
#include <cstdio>
#include "HFSP_BFH.h"
#include "BFH_Heuristic.h"
#include "IO_Utils_BFH.h"

using namespace std;

// 全局变量定义
int pJob = 0;
int pStage = 0;
vector<int> pMachines;
vector<vector<int>> pUnitTime;
vector<vector<int>> pSetupTime;

struct InstanceInfo {
    int job = 0;
    int stage = 0;
    int id = 0;
};

bool parseInstanceFileName(const string& name, InstanceInfo& info) {
    int n = 0, m = 0, id = 0;
    if (sscanf(name.c_str(), "instancia_%d_%d_%d.txt", &n, &m, &id) != 3) {
        return false;
    }
    info.job = n;
    info.stage = m;
    info.id = id;
    return true;
}

void processSingleInstance(const string& dir, const string& fileName) {
    string path = dir + "\\" + fileName;
    if (!readInstanceData(path)) return;

    InstanceInfo info;
    if (!parseInstanceFileName(fileName, info)) {
        cerr << "文件名格式无法解析: " << fileName << endl;
        return;
    }

    if (info.job != pJob || info.stage != pStage) {
        cerr << "文件名与实例内容不一致: " << fileName << endl;
    }

    ensureResultsFolder();
    string summaryFile = "results\\BFH\\" + to_string(pJob) + "_" + to_string(pStage) + ".txt";
    string timeFile = "results\\BFH\\time_" + to_string(pJob) + "_" + to_string(pStage) + ".txt";

    ofstream fout(summaryFile, ios::app);
    ofstream foutTime(timeFile, ios::app);

    // 写入文件名
    if (fout.is_open()) fout << fileName << ": ";
    if (foutTime.is_open()) foutTime << fileName << ": ";

    
    int runTimes = 1;
    for (int runIdx = 0; runIdx < runTimes; ++runIdx) {

        // 运行算法
        int bStage = bottleneckStageBFH();
        vector<int> pi;
        buildBFHPermutation(bStage, pi);

        Schedule sched;
        auto start = chrono::high_resolution_clock::now();
        decodeBFH(pi, bStage, sched);

        // 步骤4: 改进过程（可选）
        // 注释下面这一行可以跳过步骤4，只运行步骤1-3
        //improveBFH(sched, bStage);
        auto end = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(end - start).count();

        // 写入汇总数据
        if (fout.is_open()) fout << sched.cmax << " ";
        if (foutTime.is_open()) foutTime << fixed << elapsed << " ";

        // 生成调度方案文件名
        string scheduleName = to_string(pJob) + "_" + to_string(pStage) + "_" +
            to_string(info.id) + "_" + to_string(runIdx) + "_schedule.txt"; 

        string schedulePath = "results\\BFH\\" + scheduleName;

        // 保存每一次运行的调度文件
        writeBFHScheduleFile(schedulePath, sched);

        cout << "  Run " << runIdx << ": Cmax = " << sched.cmax << ", Time = " << elapsed << "s" << endl;
    }

    if (fout.is_open()) { fout << endl; fout.close(); }
    if (foutTime.is_open()) { foutTime << endl; foutTime.close(); }

    cout << "完成实例: " << fileName << " (共运行 " << runTimes << " 次)" << endl;
}

void processBenchmarkInstances(const string& directory) {
    string pattern = directory + "\\instancia_*.txt";

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "未找到实例文件: " << pattern << endl;
        return;
    }

    int count = 0;
    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            string fileName = ffd.cFileName;
            processSingleInstance(directory, fileName);
            ++count;
        }
    } while (FindNextFileA(hFind, &ffd));

    FindClose(hFind);
    cout << "共处理实例文件数量: " << count << endl;
}

int main() {
    srand((unsigned)time(NULL));

    string benchmarkDirectory = "Jose_benchmark\\Big_Size_Instances";
    processBenchmarkInstances(benchmarkDirectory);

    return 0;
}
