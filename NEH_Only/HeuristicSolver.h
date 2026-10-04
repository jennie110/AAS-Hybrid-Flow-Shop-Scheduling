#pragma once
#include "HFSP_DABC.h"
#include <algorithm>
#include <numeric>
#include <vector>
#include <string>

using namespace std;

class HeuristicSolver {
private:
    mutable vector<int> _buf_avail;
    mutable vector<pair<int, int>> _buf_rel;
    mutable vector<int> _buf_prevC;

    int bottleneckStage() const {
        if (g_bottleneckStageCached >= 0) return g_bottleneckStageCached;
        int kb = 0;
        double best = -1e100;
        for (int k = 0; k < pStage; k++) {
            long long sum = 0;
            for (int j = 0; j < pJob; j++) sum += pUnitTime[k][j];
            double val = (double)sum / (double)max(1, pMachines[k]);
            if (val > best) { best = val; kb = k; }
        }
        g_bottleneckStageCached = kb;
        return kb;
    }

    int fastEvalPartial(const vector<int>& perm, int flag) const {
        int n_partial = (int)perm.size();
        if (n_partial == 0) return 0;

        if (flag == 0) {
            int m_count = pMachines[0];
            fill(_buf_avail.begin(), _buf_avail.begin() + m_count, 0);

            for (int j : perm) {
                int best_i = 0;
                int best_t = _buf_avail[0];
                for (int i = 1; i < m_count; ++i) {
                    if (_buf_avail[i] < best_t) { best_t = _buf_avail[i]; best_i = i; }
                }
                int c = best_t + pUnitTime[0][j];
                _buf_avail[best_i] = c;
                _buf_prevC[j] = c;
            }

            for (int k = 1; k < pStage; ++k) {
                m_count = pMachines[k];
                int idx = 0;
                for (int j : perm) {
                    _buf_rel[idx++] = { _buf_prevC[j], j };
                }
                stable_sort(_buf_rel.begin(), _buf_rel.begin() + n_partial);
                fill(_buf_avail.begin(), _buf_avail.begin() + m_count, 0);

                for (int i = 0; i < n_partial; ++i) {
                    int r = _buf_rel[i].first;
                    int j = _buf_rel[i].second;
                    int best_i = 0;
                    int best_t = _buf_avail[0];
                    for (int m = 1; m < m_count; ++m) {
                        if (_buf_avail[m] < best_t) { best_t = _buf_avail[m]; best_i = m; }
                    }
                    int c = max(r, best_t) + pUnitTime[k][j];
                    _buf_avail[best_i] = c;
                    _buf_prevC[j] = c;
                }
            }

            int cmx = 0;
            for (int j : perm) {
                if (_buf_prevC[j] > cmx) cmx = _buf_prevC[j];
            }
            return cmx;
        }
        else {
            int rev_k = 0;
            int real_k = pStage - 1 - rev_k;
            int m_count = pMachines[real_k];

            fill(_buf_avail.begin(), _buf_avail.begin() + m_count, 0);

            for (int i = n_partial - 1; i >= 0; --i) {
                int j = perm[i];
                int p_val = pUnitTime[real_k][j];

                int best_i = 0;
                int best_t = _buf_avail[0];
                for (int m = 1; m < m_count; ++m) {
                    if (_buf_avail[m] < best_t) { best_t = _buf_avail[m]; best_i = m; }
                }

                int c = best_t + p_val;
                _buf_avail[best_i] = c;
                _buf_prevC[j] = c;
            }

            for (rev_k = 1; rev_k < pStage; ++rev_k) {
                real_k = pStage - 1 - rev_k;
                m_count = pMachines[real_k];

                int idx = 0;
                for (int j : perm) {
                    _buf_rel[idx++] = { _buf_prevC[j], j };
                }
                stable_sort(_buf_rel.begin(), _buf_rel.begin() + n_partial);
                fill(_buf_avail.begin(), _buf_avail.begin() + m_count, 0);

                for (int i = 0; i < n_partial; ++i) {
                    int r = _buf_rel[i].first;
                    int j = _buf_rel[i].second;
                    int p_val = pUnitTime[real_k][j];

                    int best_i = 0;
                    int best_t = _buf_avail[0];
                    for (int m = 1; m < m_count; ++m) {
                        if (_buf_avail[m] < best_t) { best_t = _buf_avail[m]; best_i = m; }
                    }

                    int c = max(r, best_t) + p_val;
                    _buf_avail[best_i] = c;
                    _buf_prevC[j] = c;
                }
            }

            int cmx = 0;
            for (int j : perm) {
                if (_buf_prevC[j] > cmx) cmx = _buf_prevC[j];
            }
            return cmx;
        }
    }

    vector<int> NEH_partial(const vector<int>& beta, int lambda, int flag) {
        const int n = (int)beta.size();
        if (lambda <= 0) return beta;
        if (lambda > n) lambda = n;

        vector<int> pi;
        pi.reserve(n);
        const int fixed = n - lambda;

        for (int i = 0; i < fixed; ++i) pi.push_back(beta[i]);

        for (int h = fixed; h < n; ++h) {
            int job = beta[h];
            vector<int> bestPi;
            int bestC = INT_MAX;

            for (int pos = 0; pos <= (int)pi.size(); ++pos) {
                vector<int> cand = pi;
                cand.insert(cand.begin() + pos, job);
                int c = fastEvalPartial(cand, flag);
                if (c < bestC) { bestC = c; bestPi = cand; }
            }
            pi.swap(bestPi);
        }
        return pi;
    }

public:
    HeuristicSolver() {
        int maxMachines = 0;
        for (int m : pMachines) maxMachines = max(maxMachines, m);
        _buf_avail.resize(maxMachines);
        _buf_rel.resize(pJob);
        _buf_prevC.resize(pJob);
    }

    vector<int> seed_SPT() const {
        vector<pair<long long, int>> v;
        v.reserve(pJob);
        for (int j = 0; j < pJob; ++j) {
            long long s = 0;
            for (int k = 0; k < pStage; ++k) s += pUnitTime[k][j];
            v.emplace_back(s, j);
        }
        stable_sort(v.begin(), v.end());
        vector<int> perm;
        perm.reserve(pJob);
        for (auto& pr : v) perm.push_back(pr.second);
        return perm;
    }

    vector<int> seed_LPT() const {
        auto t = seed_SPT();
        reverse(t.begin(), t.end());
        return t;
    }

    vector<int> seed_SPTF() const {
        vector<pair<int, int>> v;
        v.reserve(pJob);
        for (int j = 0; j < pJob; ++j) v.emplace_back(pUnitTime[0][j], j);
        stable_sort(v.begin(), v.end());
        vector<int> perm;
        for (auto& pr : v) perm.push_back(pr.second);
        return perm;
    }

    vector<int> seed_LPTF() const {
        auto t = seed_SPTF();
        reverse(t.begin(), t.end());
        return t;
    }

    vector<int> seed_SPTB() const {
        int kb = bottleneckStage();
        vector<pair<long long, int>> v;
        v.reserve(pJob);
        for (int j = 0; j < pJob; ++j) {
            long long s = 0;
            for (int k = 0; k <= kb; ++k) s += pUnitTime[k][j];
            v.emplace_back(s, j);
        }
        stable_sort(v.begin(), v.end());
        vector<int> perm;
        for (auto& pr : v) perm.push_back(pr.second);
        return perm;
    }

    vector<int> seed_LPTB() const {
        auto t = seed_SPTB();
        reverse(t.begin(), t.end());
        return t;
    }

    vector<int> seed_bSPT() const { return seed_SPT(); }
    vector<int> seed_bLPT() const { return seed_LPT(); }

    vector<int> seed_bSPTL() const {
        int L = pStage - 1;
        vector<pair<int, int>> v;
        v.reserve(pJob);
        for (int j = 0; j < pJob; ++j) v.emplace_back(pUnitTime[L][j], j);
        stable_sort(v.begin(), v.end());
        vector<int> perm;
        for (auto& pr : v) perm.push_back(pr.second);
        return perm;
    }

    vector<int> seed_bLPTL() const {
        auto t = seed_bSPTL();
        reverse(t.begin(), t.end());
        return t;
    }

    vector<int> seed_bSPTB() const {
        int kb = bottleneckStage();
        vector<pair<long long, int>> v;
        v.reserve(pJob);
        for (int j = 0; j < pJob; ++j) {
            long long s = 0;
            for (int k = kb; k < pStage; ++k) s += pUnitTime[k][j];
            v.emplace_back(s, j);
        }
        stable_sort(v.begin(), v.end());
        vector<int> perm;
        for (auto& pr : v) perm.push_back(pr.second);
        return perm;
    }

    vector<int> seed_bLPTB() const {
        auto t = seed_bSPTB();
        reverse(t.begin(), t.end());
        return t;
    }

    vector<int> NEH_SPT(int lambda) { return NEH_partial(seed_SPT(), lambda, 0); }
    vector<int> NEH_LPT(int lambda) { return NEH_partial(seed_LPT(), lambda, 0); }
    vector<int> NEH_SPTF(int lambda) { return NEH_partial(seed_SPTF(), lambda, 0); }
    vector<int> NEH_LPTF(int lambda) { return NEH_partial(seed_LPTF(), lambda, 0); }
    vector<int> NEH_SPTB(int lambda) { return NEH_partial(seed_SPTB(), lambda, 0); }
    vector<int> NEH_LPTB(int lambda) { return NEH_partial(seed_LPTB(), lambda, 0); }

    vector<int> bNEH_SPT(int lambda) { return NEH_partial(seed_bSPT(), lambda, 1); }
    vector<int> bNEH_LPT(int lambda) { return NEH_partial(seed_bLPT(), lambda, 1); }
    vector<int> bNEH_SPTL(int lambda) { return NEH_partial(seed_bSPTL(), lambda, 1); }
    vector<int> bNEH_LPTL(int lambda) { return NEH_partial(seed_bLPTL(), lambda, 1); }
    vector<int> bNEH_SPTB(int lambda) { return NEH_partial(seed_bSPTB(), lambda, 1); }
    vector<int> bNEH_LPTB(int lambda) { return NEH_partial(seed_bLPTB(), lambda, 1); }
};