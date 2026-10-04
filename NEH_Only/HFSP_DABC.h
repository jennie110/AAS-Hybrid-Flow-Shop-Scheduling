#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <iostream>
#include <climits>
using namespace std;

//  全局问题数据
int pJob = 0;                 // 工件数 n
int pStage = 0;               // 阶段数 m
vector<int> pMachines;        // 各阶段机器数 l_k, size=m
vector<vector<int>> pUnitTime;     // 加工时间 p[k][j], size=m x n
vector<vector<int>> pSetupTime;    // 全为0
vector<vector<int>> pTransferTime; // 全为0

int g_bottleneckStageCached = -1;  // 瓶颈阶段缓存

// 个体表示
struct Solution {
    vector<int> perm; // 0..n-1 的排列
    int flag = 0;     // 0=前向解码；1=反向解码
    int cmax = INT_MAX;

    string key() const { //为结构体生成一个 唯一的、可用于比较的字符串ID
        string s; s.reserve(4 * perm.size() + 4);
        s.push_back(flag ? '1' : '0'); s.push_back(':');
        for (size_t i = 0; i < perm.size(); i++) {
            if (i) s.push_back(',');
            s += to_string(perm[i]);
        }
        return s;
    }
};

// 完整调度
struct Schedule {
    int n = 0, m = 0;
    vector<int> l; // 各阶段机器数
    vector<vector<int>> S; // [m][n] 开工时间
    vector<vector<int>> C; // [m][n] 完工时间
    vector<vector<vector<int>>> machineSeq; // [m][l[k]] -> 各机上加工序列（工件编号1-based）
    int Cmax = 0;

    Schedule() {}
    Schedule(int n_, int m_, const vector<int>& lk) : n(n_), m(m_), l(lk) {
        S.assign(m, vector<int>(n, 0));
        C.assign(m, vector<int>(n, 0));
        machineSeq.resize(m);
        for (int k = 0; k < m; k++) machineSeq[k].assign(l[k], {});
        Cmax = 0;
    }
};

// 瓶颈阶段 
inline int bottleneckStage() {
    if (g_bottleneckStageCached >= 0) return g_bottleneckStageCached;//瓶颈阶段缓存
    int kb = 0;
    double best = -1e100;
    for (int k = 0; k < pStage; k++) {
        long long sum = 0;//该阶段总负荷
        for (int j = 0; j < pJob; j++) sum += pUnitTime[k][j];
        // 最大 (总负荷 / 产能) 的阶段为瓶颈
        double val = (double)sum / (double)max(1, pMachines[k]);
        if (val > best) { best = val; kb = k; }
    }
    g_bottleneckStageCached = kb;
    return kb;//返回瓶颈阶段
}

//前向解码（flag=0）
inline Schedule decodeForward(const vector<int>& perm) {//对一个调度序列进行前向解码
    // 要求：perm.size()==pJob
    Schedule sch(pJob, pStage, pMachines);

    // 阶段0：按perm顺序，每次派到最早空闲的机器（最早完成的机器）
    {
        vector<int> avail(pMachines[0], 0);  //长度为阶段0的机器数，记录各机器的可用时间
        for (int t = 0; t < pJob; ++t) {
            int j = perm[t];//第t个工件的编号
            int best_i = 0, best_t = avail[0];//初始分配到第一个机器上，时间为该机器的可用时间
            for (int i = 1; i < pMachines[0]; ++i) {//为该工件找最早可用的机器
                if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
            }
            int s = best_t;
            int c = s + pUnitTime[0][j];
            sch.S[0][j] = s; sch.C[0][j] = c;
            avail[best_i] = c;
            sch.machineSeq[0][best_i].push_back(j + 1);
        }
    }

    // 后续阶段：按上阶段完工时间升序作为释放时间调度
    for (int k = 1; k < pStage; ++k) {
        vector<pair<int, int>> rel; rel.reserve(pJob);//存储各个工件在上阶段的完工时间及其编号
        for (int j = 0; j < pJob; ++j) rel.emplace_back(sch.C[k - 1][j], j);
        stable_sort(rel.begin(), rel.end());
        vector<int> avail(pMachines[k], 0);
        for (auto& pr : rel) {
            int r = pr.first, j = pr.second;
            int best_i = 0, best_t = avail[0];
            for (int i = 1; i < pMachines[k]; ++i) {
                if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
            }
            int s = max(r, avail[best_i]), c = s + pUnitTime[k][j];
            sch.S[k][j] = s; sch.C[k][j] = c; avail[best_i] = c;
            sch.machineSeq[k][best_i].push_back(j + 1);
        }
    }

    int cmx = 0; for (int j = 0; j < pJob; ++j) cmx = max(cmx, sch.C[pStage - 1][j]);
    sch.Cmax = cmx;
    return sch;
}

// 后向解码（flag=1）
inline Schedule decodeBackward(const vector<int>& perm) {
    // 用“反向实例 + 前向解码 + 映射回原实例”的方法实现
    const int rev_m = pStage, rev_n = pJob;

    vector<int> rev_l = pMachines;
    reverse(rev_l.begin(), rev_l.end());

    vector<vector<int>> rev_p(rev_m, vector<int>(rev_n, 0));//新的加工时间矩阵，相当于对原矩阵上下翻转
    for (int r = 0; r < rev_m; ++r) {
        int k = rev_m - 1 - r;
        for (int j = 0; j < rev_n; ++j) rev_p[r][j] = pUnitTime[k][j];
    }

    vector<int> perm_rev = perm;
    reverse(perm_rev.begin(), perm_rev.end());//反转工件顺序

    // 在反向实例上前向解码
    Schedule srev(rev_n, rev_m, rev_l);
    {
        // stage 0
        vector<int> avail(rev_l[0], 0);
        for (int t = 0; t < rev_n; ++t) {
            int j = perm_rev[t];
            int best_i = 0, best_t = avail[0];
            for (int i = 1; i < rev_l[0]; ++i) if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
            int s = best_t, c = s + rev_p[0][j];
            srev.S[0][j] = s; srev.C[0][j] = c; avail[best_i] = c;
            srev.machineSeq[0][best_i].push_back(j + 1);
        }
        for (int k = 1; k < rev_m; ++k) {
            vector<pair<int, int>> rel; rel.reserve(rev_n);
            for (int j = 0; j < rev_n; ++j) rel.emplace_back(srev.C[k - 1][j], j);
            stable_sort(rel.begin(), rel.end());
            vector<int> avail(rev_l[k], 0);
            for (auto& pr : rel) {
                int r = pr.first, j = pr.second;
                int best_i = 0, best_t = avail[0];
                for (int i = 1; i < rev_l[k]; ++i) {
                    if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
                }
                int s = max(r, avail[best_i]), c = s + rev_p[k][j];
                srev.S[k][j] = s; srev.C[k][j] = c; avail[best_i] = c;
                srev.machineSeq[k][best_i].push_back(j + 1);
            }
        }
        int cmx = 0; for (int j = 0; j < rev_n; ++j) cmx = max(cmx, srev.C[rev_m - 1][j]);
        srev.Cmax = cmx;
    }

    // 映回原问题时间与序列
    const int Cmax = srev.Cmax;
    Schedule sch(pJob, pStage, pMachines);
    for (int r = 0; r < pStage; ++r) {
        int k = pStage - 1 - r;
        for (int j = 0; j < pJob; ++j) {
            sch.S[k][j] = Cmax - srev.C[r][j];
            sch.C[k][j] = Cmax - srev.S[r][j];
        }
        sch.machineSeq[k].assign(pMachines[k], {});
        int L = min<int>(srev.machineSeq[r].size(), pMachines[k]);
        for (int i = 0; i < L; ++i) {
            sch.machineSeq[k][i] = srev.machineSeq[r][i];
            reverse(sch.machineSeq[k][i].begin(), sch.machineSeq[k][i].end());
        }
    }
    sch.Cmax = Cmax;
    return sch;
}

// 评估
inline int evalCmax(const vector<int>& perm, int flag) {
    return (flag == 0) ? decodeForward(perm).Cmax : decodeBackward(perm).Cmax;
}
inline Schedule decode(const Solution& sol) {
    return (sol.flag == 0) ? decodeForward(sol.perm) : decodeBackward(sol.perm);
}
inline int evalPartialCmax(const vector<int>& perm, int flag) {
    int n_partial = (int)perm.size();
    Schedule sch(pJob, pStage, pMachines);
    if (flag == 0) { // 前向评估
        // 阶段 0
        vector<int> avail(pMachines[0], 0);
        for (int t = 0; t < n_partial; ++t) {
            int j = perm[t];
            int best_i = 0, best_t = avail[0];
            for (int i = 1; i < pMachines[0]; ++i) {
                if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
            }
            int s = best_t;
            int c = s + pUnitTime[0][j];
            sch.C[0][j] = c; // 记录完工时间
            avail[best_i] = c;
        }
        // 后续阶段
        for (int k = 1; k < pStage; ++k) {
            vector<pair<int, int>> rel; rel.reserve(n_partial);
            // 仅对部分序列中的工件排序
            for (int x : perm) rel.emplace_back(sch.C[k - 1][x], x);
            stable_sort(rel.begin(), rel.end());

            vector<int> avail(pMachines[k], 0);
            for (auto& pr : rel) {
                int r = pr.first, j = pr.second;
                int best_i = 0, best_t = avail[0];
                for (int i = 1; i < pMachines[k]; ++i) {
                    if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
                }
                int s = max(r, avail[best_i]);
                int c = s + pUnitTime[k][j];
                sch.C[k][j] = c;
                avail[best_i] = c;
            }
        }
        // 计算这部分工件的最大完工时间
        int cmx = 0;
        for (int x : perm) cmx = max(cmx, sch.C[pStage - 1][x]);
        return cmx;
    }
    else { // 后向评估 
        const int rev_m = pStage;
        vector<int> rev_l = pMachines;
        reverse(rev_l.begin(), rev_l.end());

        // 反向工件序列
        vector<int> perm_rev = perm;
        reverse(perm_rev.begin(), perm_rev.end());

        // 在反向实例上前向解码
        Schedule srev(pJob, rev_m, rev_l);

        // Stage 0 (反向的第一个阶段)
        vector<int> avail(rev_l[0], 0);
        for (int t = 0; t < n_partial; ++t) {
            int j = perm_rev[t];
            // 获取反向加工时间
            int p_val = pUnitTime[pStage - 1][j];

            int best_i = 0, best_t = avail[0];
            for (int i = 1; i < rev_l[0]; ++i) if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
            int s = best_t, c = s + p_val;
            srev.C[0][j] = c; avail[best_i] = c;
        }
        // 后续阶段
        for (int k = 1; k < rev_m; ++k) {
            vector<pair<int, int>> rel; rel.reserve(n_partial);
            for (int x : perm_rev) rel.emplace_back(srev.C[k - 1][x], x);
            stable_sort(rel.begin(), rel.end());

            vector<int> avail(rev_l[k], 0);
            for (auto& pr : rel) {
                int r = pr.first, j = pr.second;
                // 获取反向加工时间
                int p_val = pUnitTime[pStage - 1 - k][j];

                int best_i = 0, best_t = avail[0];
                for (int i = 1; i < rev_l[k]; ++i) if (avail[i] < best_t) { best_t = avail[i]; best_i = i; }
                int s = max(r, avail[best_i]), c = s + p_val;
                srev.C[k][j] = c; avail[best_i] = c;
            }
        }
        int cmx = 0;
        for (int x : perm_rev) cmx = max(cmx, srev.C[rev_m - 1][x]);
        return cmx;
    }
}
