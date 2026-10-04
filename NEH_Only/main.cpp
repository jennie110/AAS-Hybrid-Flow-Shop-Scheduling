#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <chrono>
#include <direct.h>

#include "HFSP_DABC.h"
#include "HeuristicSolver.h"
#include "IO_Utils.h"

using namespace std;

struct AlgorithmDescriptor {
    string name;
    int flag;
    int needsLambda;

    AlgorithmDescriptor(const string& n, int f, int nl)
        : name(n), flag(f), needsLambda(nl) {
    }
};

vector<AlgorithmDescriptor> createAlgorithmList() {
    vector<AlgorithmDescriptor> algos;

    algos.push_back(AlgorithmDescriptor("SPT", 0, 0));
    algos.push_back(AlgorithmDescriptor("LPT", 0, 0));
    algos.push_back(AlgorithmDescriptor("SPTF", 0, 0));
    algos.push_back(AlgorithmDescriptor("LPTF", 0, 0));
    algos.push_back(AlgorithmDescriptor("SPTB", 0, 0));
    algos.push_back(AlgorithmDescriptor("LPTB", 0, 0));

    algos.push_back(AlgorithmDescriptor("NEH(lamda)_SPT", 0, 1));
    algos.push_back(AlgorithmDescriptor("NEH(lamda)_LPT", 0, 1));
    algos.push_back(AlgorithmDescriptor("NEH(lamda)_SPTF", 0, 1));
    algos.push_back(AlgorithmDescriptor("NEH(lamda)_LPTF", 0, 1));
    algos.push_back(AlgorithmDescriptor("NEH(lamda)_SPTB", 0, 1));
    algos.push_back(AlgorithmDescriptor("NEH(lamda)_LPTB", 0, 1));

    algos.push_back(AlgorithmDescriptor("bSPT", 1, 0));
    algos.push_back(AlgorithmDescriptor("bLPT", 1, 0));
    algos.push_back(AlgorithmDescriptor("bSPTL", 1, 0));
    algos.push_back(AlgorithmDescriptor("bLPTL", 1, 0));
    algos.push_back(AlgorithmDescriptor("bSPTB", 1, 0));
    algos.push_back(AlgorithmDescriptor("bLPTB", 1, 0));

    algos.push_back(AlgorithmDescriptor("bNEH(lamda)_SPT", 1, 1));
    algos.push_back(AlgorithmDescriptor("bNEH(lamda)_LPT", 1, 1));
    algos.push_back(AlgorithmDescriptor("bNEH(lamda)_SPTL", 1, 1));
    algos.push_back(AlgorithmDescriptor("bNEH(lamda)_LPTL", 1, 1));
    algos.push_back(AlgorithmDescriptor("bNEH(lamda)_SPTB", 1, 1));
    algos.push_back(AlgorithmDescriptor("bNEH(lamda)_LPTB", 1, 1));

    return algos;
}

Solution runAlgorithm(HeuristicSolver& solver, const AlgorithmDescriptor& algo) {
    Solution sol;
    sol.flag = algo.flag;

    int lambda = algo.needsLambda ? (pJob) : 0;

    if (algo.name == "SPT") {
        sol.perm = solver.seed_SPT();
    }
    else if (algo.name == "LPT") {
        sol.perm = solver.seed_LPT();
    }
    else if (algo.name == "SPTF") {
        sol.perm = solver.seed_SPTF();
    }
    else if (algo.name == "LPTF") {
        sol.perm = solver.seed_LPTF();
    }
    else if (algo.name == "SPTB") {
        sol.perm = solver.seed_SPTB();
    }
    else if (algo.name == "LPTB") {
        sol.perm = solver.seed_LPTB();
    }
    else if (algo.name == "NEH(lamda)_SPT") {
        sol.perm = solver.NEH_SPT(lambda);
    }
    else if (algo.name == "NEH(lamda)_LPT") {
        sol.perm = solver.NEH_LPT(lambda);
    }
    else if (algo.name == "NEH(lamda)_SPTF") {
        sol.perm = solver.NEH_SPTF(lambda);
    }
    else if (algo.name == "NEH(lamda)_LPTF") {
        sol.perm = solver.NEH_LPTF(lambda);
    }
    else if (algo.name == "NEH(lamda)_SPTB") {
        sol.perm = solver.NEH_SPTB(lambda);
    }
    else if (algo.name == "NEH(lamda)_LPTB") {
        sol.perm = solver.NEH_LPTB(lambda);
    }
    else if (algo.name == "bSPT") {
        sol.perm = solver.seed_bSPT();
    }
    else if (algo.name == "bLPT") {
        sol.perm = solver.seed_bLPT();
    }
    else if (algo.name == "bSPTL") {
        sol.perm = solver.seed_bSPTL();
    }
    else if (algo.name == "bLPTL") {
        sol.perm = solver.seed_bLPTL();
    }
    else if (algo.name == "bSPTB") {
        sol.perm = solver.seed_bSPTB();
    }
    else if (algo.name == "bLPTB") {
        sol.perm = solver.seed_bLPTB();
    }
    else if (algo.name == "bNEH(lamda)_SPT") {
        sol.perm = solver.bNEH_SPT(lambda);
    }
    else if (algo.name == "bNEH(lamda)_LPT") {
        sol.perm = solver.bNEH_LPT(lambda);
    }
    else if (algo.name == "bNEH(lamda)_SPTL") {
        sol.perm = solver.bNEH_SPTL(lambda);
    }
    else if (algo.name == "bNEH(lamda)_LPTL") {
        sol.perm = solver.bNEH_LPTL(lambda);
    }
    else if (algo.name == "bNEH(lamda)_SPTB") {
        sol.perm = solver.bNEH_SPTB(lambda);
    }
    else if (algo.name == "bNEH(lamda)_LPTB") {
        sol.perm = solver.bNEH_LPTB(lambda);
    }

    sol.cmax = evalCmax(sol.perm, sol.flag);
    return sol;
}

void processBenchmarkInstances(const string& directoryPath) {
    string searchPath = directoryPath + "\\*.txt";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "Cannot find files in: " << directoryPath
            << " (error=" << GetLastError() << ")" << endl;
        return;
    }

    _mkdir("results");

    vector<AlgorithmDescriptor> algorithms = createAlgorithmList();

    for (const auto& algo : algorithms) {
        string algoPath = "results/" + algo.name;
        _mkdir(algoPath.c_str());
    }

    int instanceCount = 0;

    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            string instanceFileName = fd.cFileName;
            string fullPath = directoryPath + "\\" + instanceFileName;

            cout << "Processing instance: " << instanceFileName << endl;

            if (!readInstanceData(fullPath)) {
                cerr << "Skipping instance: " << instanceFileName << endl;
                continue;
            }

            HeuristicSolver solver;

            for (const auto& algo : algorithms) {
                //cout << "  Running algorithm: " << algo.name << endl;

                auto t0 = chrono::high_resolution_clock::now();

                Solution sol = runAlgorithm(solver, algo);
                Schedule sch = decode(sol);

                auto t1 = chrono::high_resolution_clock::now();
                double timeSeconds = chrono::duration<double>(t1 - t0).count();

                writeSummaryFiles(algo.name, instanceFileName,
                    sch.Cmax, timeSeconds);

                writeScheduleFile(sch, algo.name, instanceFileName);
            }

            instanceCount++;
        }
    } while (FindNextFileA(hFind, &fd) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        cerr << "File iteration error: " << GetLastError() << endl;
    }

    FindClose(hFind);

    cout << "Completed processing " << instanceCount << " instances." << endl;
    cout << "Total algorithms executed: " << algorithms.size() << endl;
}

int main() {
    string benchmarkDirectory = "Jose_benchmark\\Big_Size_Instances";
    processBenchmarkInstances(benchmarkDirectory);

    return 0;
}
