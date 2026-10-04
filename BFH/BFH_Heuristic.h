#pragma once
#include "HFSP_BFH.h"
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

inline int bottleneckStageBFH() {
    double bestLoad = -1.0;
    int bestStage = 0;
    for (int i = 0; i < pStage; ++i) {
        long long sum = 0;
        for (int j = 0; j < pJob; ++j) sum += pUnitTime[i][j];
        if (pMachines[i] <= 0) continue;
        double load = static_cast<double>(sum) / static_cast<double>(pMachines[i]);
        if (load > bestLoad || (load == bestLoad && i > bestStage)) {
            bestLoad = load;
            bestStage = i;
        }
    }
    return bestStage;
}

inline void buildBFHPermutation(const int bStage, vector<int>& pi) {
    pi.clear();
    pi.resize(pJob);
    vector<int> jobs(pJob);
    for (int j = 0; j < pJob; ++j) jobs[j] = j;

    if (bStage == 0) {
        vector<pair<long long, int>> tmp;
        tmp.reserve(pJob);
        for (int j = 0; j < pJob; ++j) {
            long long s = 0;
            for (int i = 1; i < pStage; ++i) s += pUnitTime[i][j];
            tmp.push_back({ -s, j });
        }
        sort(tmp.begin(), tmp.end());
        for (int k = 0; k < pJob; ++k) pi[k] = tmp[k].second;
        return;
    }

    if (bStage == pStage - 1) {
        vector<pair<long long, int>> tmp;
        tmp.reserve(pJob);
        for (int j = 0; j < pJob; ++j) {
            long long s = 0;
            for (int i = 0; i < pStage - 1; ++i) s += pUnitTime[i][j];
            tmp.push_back({ s, j });
        }
        sort(tmp.begin(), tmp.end());
        for (int k = 0; k < pJob; ++k) pi[k] = tmp[k].second;
        return;
    }

    vector<pair<long long, int>> front;
    vector<pair<long long, int>> back;
    front.reserve(pJob);
    back.reserve(pJob);

    for (int j = 0; j < pJob; ++j) {
        long long sFront = 0;
        for (int i = 0; i <= bStage - 1; ++i) sFront += pUnitTime[i][j];
        long long sBack = 0;
        for (int i = bStage + 1; i < pStage; ++i) sBack += pUnitTime[i][j];
        front.push_back({ sFront, j });
        back.push_back({ -sBack, j });
    }

    sort(front.begin(), front.end());
    sort(back.begin(), back.end());

    vector<int> pi1, pi2;
    pi1.reserve(pJob);
    pi2.reserve(pJob);
    for (auto& p : front) pi1.push_back(p.second);
    for (auto& p : back)  pi2.push_back(p.second);

    vector<int> used(pJob, 0);
    int left = 0;
    int right = pJob - 1;
    int i1 = 0;
    int i2 = static_cast<int>(pi2.size()) - 1;

    while (left <= right && (i1 < (int)pi1.size() || i2 >= 0)) {
        if (i1 < (int)pi1.size()) {
            int j = pi1[i1++];
            if (!used[j]) {
                pi[left++] = j;
                used[j] = 1;
            }
        }
        if (left > right) break;
        if (i2 >= 0) {
            int j = pi2[i2--];
            if (!used[j]) {
                pi[right--] = j;
                used[j] = 1;
            }
        }
    }

    for (int j = 0; j < pJob; ++j) {
        if (!used[j] && left <= right) {
            pi[left++] = j;
            used[j] = 1;
        }
    }
}

inline long long priorityBeforeB(int stage, int job, int bStage) {
    long long s = 0;
    for (int i = stage; i <= bStage - 1; ++i) s += pUnitTime[i][job];
    return s;
}

inline long long priorityAfterB(int stage, int job, int bStage) {
    long long s = 0;
    for (int i = stage; i < pStage; ++i) s += pUnitTime[i][job];
    return s;
}

inline void decodeBFH(const vector<int>& pi, int bStage, Schedule& sched) {
    int n = pJob;
    int m = pStage;

    int maxMachines = 0;
    for (int v : pMachines) if (v > maxMachines) maxMachines = v;

    sched.STime.assign(m, vector<int>(n, 0));
    sched.CTime.assign(m, vector<int>(n, 0));
    sched.chromSMJ.assign(m, vector<vector<int>>(maxMachines, vector<int>(n, 1000)));
    sched.cmax = 0;

    if (m == 0 || n == 0) return;

    {
        int machineCount = pMachines[0];
        vector<int> machineTime(machineCount, 0);
        vector<vector<int>> seq(machineCount);

        for (int idx = 0; idx < n; ++idx) {
            int job = pi[idx];
            int bestMachine = 0;
            int bestTime = machineTime[0];
            for (int k = 1; k < machineCount; ++k) {
                if (machineTime[k] < bestTime) {
                    bestTime = machineTime[k];
                    bestMachine = k;
                }
            }
            int start = machineTime[bestMachine];
            int finish = start + pUnitTime[0][job];
            sched.STime[0][job] = start;
            sched.CTime[0][job] = finish;
            machineTime[bestMachine] = finish;
            seq[bestMachine].push_back(job);
        }

        for (int k = 0; k < machineCount; ++k) {
            for (size_t pos = 0; pos < seq[k].size() && pos < (size_t)n; ++pos) {
                sched.chromSMJ[0][k][pos] = seq[k][pos];
            }
        }
    }

    for (int s = 1; s < m; ++s) {
        int machineCount = pMachines[s];
        vector<int> machineTime(machineCount, 0);
        vector<vector<int>> seq(machineCount);
        vector<char> finished(n, 0);
        int remaining = n;

        while (remaining > 0) {
            int bestMachine = 0;
            int bestMachineTime = machineTime[0];
            for (int k = 1; k < machineCount; ++k) {
                if (machineTime[k] < bestMachineTime) {
                    bestMachineTime = machineTime[k];
                    bestMachine = k;
                }
            }

            int currentTime = bestMachineTime;
            int chosenJob = -1;

            while (true) {
                int earliestRelease = numeric_limits<int>::max();
                vector<int> candidates;
                candidates.reserve(n);

                for (int j = 0; j < n; ++j) {
                    if (finished[j]) continue;
                    int r = sched.CTime[s - 1][j];
                    if (r <= currentTime) {
                        candidates.push_back(j);
                    }
                    if (r < earliestRelease) earliestRelease = r;
                }

                if (!candidates.empty()) {
                    long long bestScore = 0;
                    if (s < bStage) bestScore = numeric_limits<long long>::max();
                    else bestScore = numeric_limits<long long>::min();

                    for (int job : candidates) {
                        long long score = 0;
                        if (s < bStage) {
                            score = priorityBeforeB(s, job, bStage);
                            if (score < bestScore ||
                                (score == bestScore && (chosenJob == -1 || job < chosenJob))) {
                                bestScore = score;
                                chosenJob = job;
                            }
                        }
                        else {
                            score = priorityAfterB(s, job, bStage);
                            if (score > bestScore ||
                                (score == bestScore && (chosenJob == -1 || job < chosenJob))) {
                                bestScore = score;
                                chosenJob = job;
                            }
                        }
                    }
                    break;
                }
                else {
                    if (earliestRelease == numeric_limits<int>::max()) break;
                    if (earliestRelease > currentTime) {
                        currentTime = earliestRelease;
                        machineTime[bestMachine] = currentTime;
                    }
                    else {
                        break;
                    }
                }
            }

            if (chosenJob == -1) {
                break;
            }

            int release = sched.CTime[s - 1][chosenJob];
            int start = currentTime;
            if (start < release) start = release;
            int finish = start + pUnitTime[s][chosenJob];

            sched.STime[s][chosenJob] = start;
            sched.CTime[s][chosenJob] = finish;
            machineTime[bestMachine] = finish;
            seq[bestMachine].push_back(chosenJob);
            finished[chosenJob] = 1;
            --remaining;
        }

        for (int k = 0; k < machineCount; ++k) {
            for (size_t pos = 0; pos < seq[k].size() && pos < (size_t)n; ++pos) {
                sched.chromSMJ[s][k][pos] = seq[k][pos];
            }
        }
    }

    int last = m - 1;
    int cmax = 0;
    for (int j = 0; j < n; ++j) {
        if (sched.CTime[last][j] > cmax) cmax = sched.CTime[last][j];
    }
    sched.cmax = cmax;
}


// 步骤4: 改进初始调度 (Improvement Phase)
// 可通过注释 main.cpp 中的 improveBFH() 调用来禁用步骤4
// 从 chromSMJ 重新构建调度并计算 Cmax
inline void rebuildScheduleFromChromSMJ(Schedule& sched, int bStage) {
    int n = pJob;
    int m = pStage;

    sched.STime.assign(m, vector<int>(n, 0));
    sched.CTime.assign(m, vector<int>(n, 0));

    // 第一阶段：按 chromSMJ 顺序调度
    {
        int machineCount = pMachines[0];
        vector<int> machineTime(machineCount, 0);

        for (int k = 0; k < machineCount; ++k) {
            for (int pos = 0; pos < n; ++pos) {
                int job = sched.chromSMJ[0][k][pos];
                if (job >= n || job < 0) break; // 1000 或无效值表示结束

                int start = machineTime[k];
                int finish = start + pUnitTime[0][job];
                sched.STime[0][job] = start;
                sched.CTime[0][job] = finish;
                machineTime[k] = finish;
            }
        }
    }

    // 后续阶段：按 chromSMJ 顺序调度，考虑前一阶段完工时间
    for (int s = 1; s < m; ++s) {
        int machineCount = pMachines[s];
        vector<int> machineTime(machineCount, 0);

        for (int k = 0; k < machineCount; ++k) {
            for (int pos = 0; pos < n; ++pos) {
                int job = sched.chromSMJ[s][k][pos];
                if (job >= n || job < 0) break;

                int release = sched.CTime[s - 1][job];
                int start = max(machineTime[k], release);
                int finish = start + pUnitTime[s][job];
                sched.STime[s][job] = start;
                sched.CTime[s][job] = finish;
                machineTime[k] = finish;
            }
        }
    }

    // 计算 Cmax
    int last = m - 1;
    int cmax = 0;
    for (int j = 0; j < n; ++j) {
        if (sched.CTime[last][j] > cmax) cmax = sched.CTime[last][j];
    }
    sched.cmax = cmax;
}

// 获取阶段 s 中机器 k 上的工件列表
inline vector<int> getJobsOnMachine(const Schedule& sched, int stage, int machine) {
    vector<int> jobs;
    for (int pos = 0; pos < pJob; ++pos) {
        int job = sched.chromSMJ[stage][machine][pos];
        if (job >= pJob || job < 0) break;
        jobs.push_back(job);
    }
    return jobs;
}

// 设置阶段 s 中机器 k 上的工件列表
inline void setJobsOnMachine(Schedule& sched, int stage, int machine, const vector<int>& jobs) {
    for (int pos = 0; pos < pJob; ++pos) {
        if (pos < (int)jobs.size()) {
            sched.chromSMJ[stage][machine][pos] = jobs[pos];
        } else {
            sched.chromSMJ[stage][machine][pos] = 1000; // 无效标记
        }
    }
}

// 步骤4.3: 成对交换邻域搜索 (Pairwise Interchange)
// 对阶段 stage 进行同一机器和不同机器之间的工件成对交换
inline bool improveStageBySwap(Schedule& sched, int stage, int bStage) {
    bool improved = true;
    bool anyImproved = false;

    while (improved) {
        improved = false;
        int bestCmax = sched.cmax;
        Schedule bestSched = sched;

        int machineCount = pMachines[stage];

        // 收集该阶段所有机器上的工件信息
        vector<vector<int>> machineJobs(machineCount);
        for (int k = 0; k < machineCount; ++k) {
            machineJobs[k] = getJobsOnMachine(sched, stage, k);
        }

        // 尝试同一机器上的成对交换
        for (int k = 0; k < machineCount; ++k) {
            int jobCount = (int)machineJobs[k].size();
            for (int i = 0; i < jobCount; ++i) {
                for (int j = i + 1; j < jobCount; ++j) {
                    // 交换位置 i 和 j
                    Schedule tempSched = sched;
                    swap(machineJobs[k][i], machineJobs[k][j]);
                    setJobsOnMachine(tempSched, stage, k, machineJobs[k]);
                    rebuildScheduleFromChromSMJ(tempSched, bStage);

                    if (tempSched.cmax < bestCmax) {
                        bestCmax = tempSched.cmax;
                        bestSched = tempSched;
                        improved = true;
                    }

                    // 恢复
                    swap(machineJobs[k][i], machineJobs[k][j]);
                }
            }
        }

        // 尝试不同机器之间的成对交换
        for (int k1 = 0; k1 < machineCount; ++k1) {
            for (int k2 = k1 + 1; k2 < machineCount; ++k2) {
                int cnt1 = (int)machineJobs[k1].size();
                int cnt2 = (int)machineJobs[k2].size();
                for (int i = 0; i < cnt1; ++i) {
                    for (int j = 0; j < cnt2; ++j) {
                        // 交换 k1 的工件 i 和 k2 的工件 j
                        Schedule tempSched = sched;
                        swap(machineJobs[k1][i], machineJobs[k2][j]);
                        setJobsOnMachine(tempSched, stage, k1, machineJobs[k1]);
                        setJobsOnMachine(tempSched, stage, k2, machineJobs[k2]);
                        rebuildScheduleFromChromSMJ(tempSched, bStage);

                        if (tempSched.cmax < bestCmax) {
                            bestCmax = tempSched.cmax;
                            bestSched = tempSched;
                            improved = true;
                        }

                        // 恢复
                        swap(machineJobs[k1][i], machineJobs[k2][j]);
                    }
                }
            }
        }

        if (improved) {
            sched = bestSched;
            anyImproved = true;
        }
    }

    return anyImproved;
}

// 步骤4.3: 插入邻域搜索 (Insert)
// 对阶段 stage 进行同一机器和不同机器之间的工件插入操作
inline bool improveStageByInsert(Schedule& sched, int stage, int bStage) {
    bool improved = true;
    bool anyImproved = false;

    while (improved) {
        improved = false;
        int bestCmax = sched.cmax;
        Schedule bestSched = sched;

        int machineCount = pMachines[stage];

        // 收集该阶段所有机器上的工件信息
        vector<vector<int>> machineJobs(machineCount);
        for (int k = 0; k < machineCount; ++k) {
            machineJobs[k] = getJobsOnMachine(sched, stage, k);
        }

        // 尝试同一机器上的插入操作
        for (int k = 0; k < machineCount; ++k) {
            int jobCount = (int)machineJobs[k].size();
            for (int i = 0; i < jobCount; ++i) {
                int job = machineJobs[k][i];
                // 从位置 i 移除工件
                vector<int> temp = machineJobs[k];
                temp.erase(temp.begin() + i);

                // 尝试插入到其他位置
                for (int j = 0; j <= (int)temp.size(); ++j) {
                    if (j == i || j == i + 1) continue; // 跳过相邻位置（效果相同）

                    vector<int> newSeq = temp;
                    newSeq.insert(newSeq.begin() + j, job);

                    Schedule tempSched = sched;
                    setJobsOnMachine(tempSched, stage, k, newSeq);
                    rebuildScheduleFromChromSMJ(tempSched, bStage);

                    if (tempSched.cmax < bestCmax) {
                        bestCmax = tempSched.cmax;
                        bestSched = tempSched;
                        improved = true;
                    }
                }
            }
        }

        // 尝试不同机器之间的插入操作
        for (int k1 = 0; k1 < machineCount; ++k1) {
            for (int k2 = 0; k2 < machineCount; ++k2) {
                if (k1 == k2) continue;

                int cnt1 = (int)machineJobs[k1].size();
                for (int i = 0; i < cnt1; ++i) {
                    int job = machineJobs[k1][i];

                    // 从 k1 移除工件
                    vector<int> seq1 = machineJobs[k1];
                    seq1.erase(seq1.begin() + i);

                    // 尝试插入到 k2 的各个位置
                    int cnt2 = (int)machineJobs[k2].size();
                    for (int j = 0; j <= cnt2; ++j) {
                        vector<int> seq2 = machineJobs[k2];
                        seq2.insert(seq2.begin() + j, job);

                        Schedule tempSched = sched;
                        setJobsOnMachine(tempSched, stage, k1, seq1);
                        setJobsOnMachine(tempSched, stage, k2, seq2);
                        rebuildScheduleFromChromSMJ(tempSched, bStage);

                        if (tempSched.cmax < bestCmax) {
                            bestCmax = tempSched.cmax;
                            bestSched = tempSched;
                            improved = true;
                        }
                    }
                }
            }
        }

        if (improved) {
            sched = bestSched;
            anyImproved = true;
        }
    }

    return anyImproved;
}


// 步骤4 主函数: 改进初始调度
// 遍历所有阶段，对每个阶段依次进行成对交换和插入邻域搜索
inline void improveBFH(Schedule& sched, int bStage) {
    // 步骤4.1: 初始Cmax已在decodeBFH中计算
    // 步骤4.2-4.4: 遍历每个阶段进行改进
    for (int stage = 0; stage < pStage; ++stage) {
        // 步骤4.3: 首先进行成对交换邻域搜索
        improveStageBySwap(sched, stage, bStage);
        // 步骤4.3: 然后进行插入邻域搜索
        improveStageByInsert(sched, stage, bStage);
    }
}
