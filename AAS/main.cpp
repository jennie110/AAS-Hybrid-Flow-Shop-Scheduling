#include <iostream>
#include <windows.h>
#include <string.h>
#include "HFSP_CS.h"
#include "AAS_CCH.h"
#include "Race_noMatlab.h"
//#include "Race.h"
#include <cstdlib>   
#include <ctime>     
#include <chrono>

using namespace std;

int gBottleneckStage=0;

int main1()
{
    int NumberOfInstances = 0;
    //srand((unsigned int)time(NULL));
    for (int j = 0; j < 1; j++)
    {
        for (int s = 0; s < 1; s++)
        {
            for (int level = 0; level < 1; level++)
            {
               
                int InJob = Jobs[j];
                int InStage = Stages[s];
                GenerateInstances(InJob, InStage, NumberOfInstances + 124);
                //OutputInstances(InJob, InStage, level);
                //OutputInstancesName(InJob, InStage, level);

                cout << "Instance:" << NumberOfInstances << endl;

                // 4个权重：SPTB, AfterBN, Total, Current, LastJob
                FAS_CCH cch = FAS_CCH(0.305018, 0.411367, 0.204585, 0.079031, 0.0);

                long initTime, finalTime;
                double costTime;

                initTime = GetTickCount();

                // cch.run_with_reversibility();

                finalTime = GetTickCount();
                costTime = (finalTime - initTime) / 1000.0;

                //把makespan和时间分别输出
                char outfile_makespan[80];
                char outfile_time[80];

                sprintf_s(outfile_makespan, "results//HFSP//%d_%d.txt", InJob, InStage);
                sprintf_s(outfile_time, "results//HFSP//time_%d_%d.txt", InJob, InStage);

                fstream fout_m(outfile_makespan, fstream::out | fstream::app);
                fstream fout_t(outfile_time, fstream::out | fstream::app);

                fout_m << cch.makespan << " ";
                fout_t << costTime << " ";

                fout_m.close();
                fout_t.close();

                //cch.OutputWholeSchedule(level);
                //for (const CriticalStructure& op : cch.Critical_Operations)
                //{
                //    // 打印每个关键操作的阶段和工件编号
                //    cout << "(" << op.Stage + 1 << ", " << op.Critical_Lot + 1 << ")" << endl;
                //}
                //cout << cch.Critical_Operations.size() << endl;
                NumberOfInstances++;
                
            }
        }
    }
    return 0;
}


void loadConfigurations(
    vector<double>& lpt_weights, vector<double>& spt_weights,
    vector<double>& lwr_weights, vector<double>& mwr_weights,
    vector<double>& epf_weights, vector<double>& lpf_weights
)
{
    // 包含提供的所有180个权重 (30组 * 6个)
    const double all_weights[180] = {
        0.0994549, 0.320633, 0.0716078, 0.320515, 0.0656443, 0.122145,
        0.0602811, 0.270578, 0.0570647, 0.312394, 0.204708, 0.0949743,
        0.132344, 0.360103, 0.0890677, 0.349888, 0.0401219, 0.0284749,
        0.0928067, 0.321942, 0.0724444, 0.324345, 0.137968, 0.0504942,
        0.151424, 0.198329, 0.19817, 0.250018, 0.108977, 0.093082,
        0.0469274, 0.172564, 0.171388, 0.323391, 0.205843, 0.0798863,
        0.196325, 0.3104, 0.0155697, 0.135809, 0.160194, 0.181703,
        0.176717, 0.27341, 0.110364, 0.232406, 0.13136, 0.0757435,
        0.0855556, 0.191038, 0.195119, 0.350299, 0.0412269, 0.136762,
        0.046708, 0.205011, 0.19814, 0.390848, 0.144227, 0.0150655,
        0.073952, 0.241614, 0.117607, 0.375041, 0.0852796, 0.106507,
        0.00107918, 0.188313, 0.0045218, 0.279562, 0.204378, 0.322146,
        0.04, 0.26, 0.07, 0.37, 0.2, 0.06,
        0.0689679, 0.359101, 0.0178455, 0.357673, 0.186644, 0.00976844,
        0.1, 0.2, 0.17, 0.31, 0.11, 0.11,
        0.0486256, 0.177741, 0.0163017, 0.209498, 0.466214, 0.0816199,
        0.13144, 0.233423, 0.163131, 0.300618, 0.149271, 0.022117,
        0.207127, 0.274258, 0.0754932, 0.249095, 0.160641, 0.033387,
        0.0236613, 0.287989, 0.0646856, 0.359837, 0.257257, 0.00656915,
        0.0586285, 0.128939, 0.147064, 0.26008, 0.155262, 0.250026,
        0.0517407, 0.34601, 0.0328414, 0.432185, 0.0867757, 0.0504468,
        0.112523, 0.221279, 0.012428, 0.25325, 0.0798095, 0.320711,
        0.0865148, 0.283264, 0.0643349, 0.297768, 0.226533, 0.0415857,
        0.00725114, 0.325648, 0.0064423, 0.448589, 0.133512, 0.0785579,
        0.0616591, 0.287507, 0.0583972, 0.418015, 0.0213863, 0.153036,
        0.147918, 0.249629, 0.0831873, 0.26965, 0.134029, 0.115587,
        0.0230866, 0.229416, 0.0319283, 0.378211, 0.108617, 0.228742,
        0.040221, 0.113031, 0.200012, 0.310098, 0.27835, 0.0582887,
        0.20225, 0.266761, 0.0958078, 0.163297, 0.189102, 0.0827824,
        0.0545809, 0.23342, 0.1313, 0.396135, 0.05088, 0.133684
    };

    // ("6个30*1的vector")
    // 调整 vector 大小以容纳30个精英配置
    lpt_weights.resize(30);
    spt_weights.resize(30);
    lwr_weights.resize(30);
    mwr_weights.resize(30);
    epf_weights.resize(30);
    lpf_weights.resize(30);

    // 循环30次，填充6个vector
    for (int i = 0; i < 30; ++i)
    {
        int base_index = i * 6; // 计算当前配置的起始索引
        
        lpt_weights[i] = all_weights[base_index + 0]; // LPT weight
        spt_weights[i] = all_weights[base_index + 1]; // SPT weight
        lwr_weights[i] = all_weights[base_index + 2]; // LWR weight
        mwr_weights[i] = all_weights[base_index + 3]; // MWR weight
        epf_weights[i] = all_weights[base_index + 4]; // EPF weight
        lpf_weights[i] = all_weights[base_index + 5]; // LPF weight
    }
}

// Read benchmark instances.
bool readInstanceData(const string& filePath)
{
    ifstream fin(filePath);
    if (!fin.is_open()) {
        cerr << "错误：无法打开实例文件: " << filePath << endl;
        return false;
    }

    // 读取 pJob 和 pStage
    if (!(fin >> pJob >> pStage)) {
        cerr << "错误：从文件读取 pJob 和 pStage 失败: " << filePath << endl;
        fin.close();
        return false;
    }

    // 调整大小并读取各阶段机器数
    pMachines.resize(pStage);
    for (int k = 0; k < pStage; ++k) {
        if (!(fin >> pMachines[k])) {
            cerr << "错误：从文件读取阶段 " << k << " 的机器数量失败: " << filePath << endl;
            fin.close();
            return false;
        }
    }

    // 调整大小并读取加工时间
    pUnitTime.assign(pStage, vector<int>(pJob));
    for (int k = 0; k < pStage; ++k) {
        for (int j = 0; j < pJob; ++j) {
            if (!(fin >> pUnitTime[k][j])) {
                cerr << "错误：从文件读取阶段 " << k << " 工件 " << j << " 的加工时间失败: " << filePath << endl;
                fin.close();
                return false;
            }
        }
    }

    // 初始化准备时间和转移时间为 0
    pSetupTime.assign(pStage, vector<int>(pJob, 0));
    pTransferTime.assign(pStage, vector<int>(pJob, 0));

    fin.close();
    //cout << "实例: " << filePath << " (工件=" << pJob << ", 阶段=" << pStage << ")" << endl;
    return true;
}

// 函数：处理指定目录下的所有基准实例
void processBenchmarkInstances(const string& directoryPath)
{
    string searchPath = directoryPath + "\\*.txt"; // 搜索 .txt 文件
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "错误：在目录中查找文件失败: " << directoryPath << " (错误码: " << GetLastError() << ")" << endl;
        // 可以添加对目录是否存在的检查
        return;
    }

    int instanceCount = 0;
    do {
        srand((unsigned int)time(NULL));
        // 跳过目录和特殊条目 "." ".."
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            string instanceFileName = fd.cFileName;
            string fullPath = directoryPath + "\\" + instanceFileName;

            cout << "实例文件: " << instanceFileName << endl;

            // 读取实例数据到全局变量
            if (!readInstanceData(fullPath)) {
                cerr << "因读取错误跳过实例: " << instanceFileName << endl;
                continue; // 处理下一个文件
            }

            int instanceIndex = -1; // 初始化为无效值
            size_t lastUnderscore = instanceFileName.find_last_of('_');
            size_t lastDot = instanceFileName.find_last_of('.');

            // 确保找到了下划线和点，并且下划线在点之前
            if (lastUnderscore != string::npos && lastDot != string::npos && lastUnderscore < lastDot)
            {
                // 提取下划线和点之间的子字符串
                string indexStr = instanceFileName.substr(lastUnderscore + 1, lastDot - lastUnderscore - 1);
                // 将子字符串转换为整数
                instanceIndex = std::stoi(indexStr);
            }

            // 汇总文件名 (基于从文件中读取的 pJob, pStage)
            char outfile_makespan[100];
            char outfile_time[100];
            sprintf_s(outfile_makespan, sizeof(outfile_makespan), "results//Jose//%d_%d.txt", pJob, pStage);
            sprintf_s(outfile_time, sizeof(outfile_time), "results//Jose//time_%d_%d.txt", pJob, pStage);

            // 以追加模式打开汇总文件
            fstream fout_m(outfile_makespan, fstream::out | fstream::app);
            fstream fout_t(outfile_time, fstream::out | fstream::app);

            // 写入实例标识符到汇总文件
            if (fout_m.is_open()) {
                fout_m << instanceFileName << ": ";
            }
            else {
                cerr << "错误：无法打开 makespan 汇总文件: " << outfile_makespan << endl;
            }
            if (fout_t.is_open()) {
                fout_t << instanceFileName << ": ";
            }
            else {
                cerr << "错误：无法打开 time 汇总文件: " << outfile_time << endl;
            }

            vector<double> lpt_weights;
            vector<double> spt_weights;
            vector<double> lwr_weights;
            vector<double> mwr_weights;
            vector<double> epf_weights;
            vector<double> lpf_weights;

            // 调用加载函数，填充这6个 vector
            /*loadConfigurations(lpt_weights, spt_weights, lwr_weights,
                mwr_weights, epf_weights, lpf_weights);*/
           // for (int c = 0; c < 30; c++)
            //{
                 // 对当前实例文件运行算法 10 次
                for (int runIdx = 0; runIdx < 1; ++runIdx)
                {
                    cout << "  运行索引: " << runIdx << endl;

                    // 4个权重：SPTB, AfterBN, Total, Current
                    // FAS_CCH cch = FAS_CCH(0.305018, 0.411367, 0.204585, 0.079031);
                    // FAS_CCH cch = FAS_CCH(0.270123, 0.532518, 0.175454, 0.0219042);//原参数
                    // FAS_CCH cch = FAS_CCH(0.383502, 0.49715, 0.0298672, 0.0894804); // AAS_旧动态瓶颈
                    // FAS_CCH cch = FAS_CCH(0.347455, 0.363402, 0.19279, 0.0963537);//AAS_静态瓶颈
                    // FAS_CCH cch = FAS_CCH(0.217901, 0.458936, 0.0494943, 0.273669); // AAS_新动态瓶颈
                    // FAS_CCH cch=FAS_CCH(0.343462, 0.526525, 0.116841, 0.0131725);//AAS current->afterbn/current(越大越好)
                    // FAS_CCH cch=FAS_CCH(0.326898, 0.49688, 0.0621586, 0.114064);//AAS current-下一阶段加工时间(越大越好)
                    // FAS_CCH cch=FAS_CCH(0.178844, 0.484657, 0.294325, 0.042174);
                    // FAS_CCH cch = FAS_CCH(0.257508, 0.534092, 0.00623739, 0.122061, 0.080101);//AAS_5个规则（新增开始时间）
                    // FAS_CCH cch = FAS_CCH(0.232426, 0.436327, 0.0655643, 0.216293, 0.0493893); // 小实例训练结果
                    // FAS_CCH cch = FAS_CCH(0.348567, 0.463361, 0.0774425, 0.0886582, 0.0219707); // 大实例训练结果
                    // FAS_CCH cch = FAS_CCH(0.257508, 0.534092, 0.00623739, 0.122061, 0.080101);//AAS_5个规则（新增开始时间）
                    // FAS_CCH cch = FAS_CCH(0.147619, 0.0236233, 0.158033, 0.109748, 0.560976); //AAS_5个规则（新增开始时间）_开始时间最小化
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0236233, 0.158033, 0.109748, 0.560976); //AAS_5个规则（新增开始时间）_开始时间最小化_去瓶颈前时间
                    // FAS_CCH cch = FAS_CCH(0.147619, 0.0, 0.158033, 0.109748, 0.560976); //AAS_5个规则（新增开始时间）_开始时间最小化_去瓶颈后时间
                    // FAS_CCH cch = FAS_CCH(0.147619, 0.0236233, 0.0, 0.109748, 0.560976); //AAS_5个规则（新增开始时间）_开始时间最小化_去总加工时间
                    // FAS_CCH cch = FAS_CCH(0.147619, 0.0236233, 0.158033, 0.0, 0.560976); //AAS_5个规则（新增开始时间）_开始时间最小化_去当前加工时间
                    // FAS_CCH cch = FAS_CCH(0.147619, 0.0236233, 0.158033, 0.109748, 0.0); //AAS_5个规则（新增开始时间）_开始时间最小化_去最早开始时间

                    // FAS_CCH cch = FAS_CCH(0.138839, 0.0245224, 0.190337, 0.0925064, 0.553795); //AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0245224, 0.190337, 0.0925064, 0.553795); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_去瓶颈前时间
                    // FAS_CCH cch = FAS_CCH(0.138839, 0.0, 0.190337, 0.0925064, 0.553795); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_去瓶颈后时间
                    // FAS_CCH cch = FAS_CCH(0.138839, 0.0245224, 0.0, 0.0925064, 0.553795); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_去总加工时间
                    // FAS_CCH cch = FAS_CCH(0.138839, 0.0245224, 0.190337, 0.0, 0.553795); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_去当前加工时间
                    // FAS_CCH cch = FAS_CCH(0.138839, 0.0245224, 0.190337, 0.0925064, 0.0); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_去最早开始时间

                    // FAS_CCH cch = FAS_CCH(0.0410791, 0.0583478, 0.109861, 0.12463, 0.666082); //AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0583478, 0.109861, 0.12463, 0.666082); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_去瓶颈前时间

                    // FAS_CCH cch = FAS_CCH(0.00584605, 0.0985784, 0.00637815, 0.096444, 0.792753); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_计算时加上瓶颈时间
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0985784, 0.00637815, 0.096444, 0.792753); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_计算时加上瓶颈时间_去瓶颈前时间
                    // FAS_CCH cch = FAS_CCH(0.00584605, 0.0, 0.00637815, 0.096444, 0.792753); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_计算时加上瓶颈时间_去瓶颈后时间
                    // FAS_CCH cch = FAS_CCH(0.00584605, 0.0985784, 0.0, 0.096444, 0.792753); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_计算时加上瓶颈时间_去总加工时间
                    // FAS_CCH cch = FAS_CCH(0.00584605, 0.0985784, 0.00637815, 0.0, 0.792753); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_计算时加上瓶颈时间_去当前加工时间
                    // FAS_CCH cch = FAS_CCH(0.0, 0.140074, 0.0, 0.133297, 0.726629); // AAS_5个规则（新增开始时间）_开始时间最小化_新动态瓶颈_剩余工作量_计算时加上瓶颈时间_去最早开始时间
                   
                    FAS_CCH cch = FAS_CCH(0.0272235, 0.305842, 0.102682, 0.170027, 0.394226);//全局插入时所选序列改变_旧最早开始时间
                    // FAS_CCH cch = FAS_CCH(0.0, 0.305842, 0.102682, 0.170027, 0.394226);//全局插入时所选序列改变_旧最早开始时间_去当前阶段到瓶颈时间
                    // FAS_CCH cch = FAS_CCH(0.0272235, 0.0, 0.102682, 0.170027, 0.394226);//全局插入时所选序列改变_旧最早开始时间_去瓶颈后时间
                    // FAS_CCH cch = FAS_CCH(0.0272235, 0.305842, 0.0, 0.170027, 0.394226);//全局插入时所选序列改变_旧最早开始时间_去剩余工作量
                    // FAS_CCH cch = FAS_CCH(0.0272235, 0.305842, 0.102682, 0.0, 0.394226);//全局插入时所选序列改变_旧最早开始时间_去当前加工时间
                    // FAS_CCH cch = FAS_CCH(0.0272235, 0.305842, 0.102682, 0.170027, 0.0);//全局插入时所选序列改变_旧最早开始时间_去最早开始时间

                    // FAS_CCH cch = FAS_CCH(1.0, 0.0, 0.0, 0.0, 0.0);//全局插入时所选序列改变_旧最早开始时间_仅当前阶段到瓶颈时间
                    // FAS_CCH cch = FAS_CCH(0.0, 1.0, 0.0, 0.0, 0.0); // 全局插入时所选序列改变_新最早开始时间_仅瓶颈后时间
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0, 1.0, 0.0, 0.0); // 全局插入时所选序列改变_新最早开始时间_仅剩余工作量
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0, 0.0, 1.0, 0.0); // 全局插入时所选序列改变_新最早开始时间_仅当前加工时间
                    // FAS_CCH cch = FAS_CCH(0.0, 0.0, 0.0, 0.0, 1.0); // 全局插入时所选序列改变_新最早开始时间_仅最早开始时间
                    // FAS_CCH cch = FAS_CCH(0.2, 0.2, 0.2, 0.2, 0.2); // 全局插入时所选序列改变_新最早开始时间_平均    
                    
                    
                    // FAS_CCH cch = FAS_CCH(0.0832733, 0.199289, 0.142082, 0.0278352, 0.547521);//Carlier_结果
                    double costTime = -1.0; // 初始化为错误值

                    try {
                        auto t0 = chrono::high_resolution_clock::now();
                        cch.run_with_reversibility();
                        auto t1 = chrono::high_resolution_clock::now();
                        costTime = chrono::duration<double>(t1 - t0).count();
                    
                        cch.OutputWholeSchedule(instanceIndex, runIdx); // 保存调度方案
                        // cch.OutputWholeSchedule(99999, 99999); // 保存调度方案
                        // cch.OutputWholeSchedule(99,99);
                    }
                    catch (const std::exception& e) {
                        cerr << "  错误发生在 cch.run() 或 cch.OutputWholeSchedule() 期间，实例 " << instanceFileName << ", 运行 " << runIdx << ": " << e.what() << endl;
                    }
                    catch (...) {
                        cerr << "  未知错误发生在 cch.run() 或 cch.OutputWholeSchedule() 期间，实例 " << instanceFileName << ", 运行 " << runIdx << endl;
                    }
               
                    //  将 makespan 和 time 追加写入汇总文件
                    if (fout_m.is_open()) {
                        fout_m << cch.makespan << " "; // 如果运行出错，makespan 可能是默认值 0
                    }
                    if (fout_t.is_open()) {
                        fout_t << costTime << " ";
                    }
                    //for (const CriticalStructure& op : cch.Critical_Operations)
                    //{
                    //    // 打印每个关键操作的阶段和工件编号
                    //    cout << "(" << op.Stage + 1 << ", " << op.Critical_Lot + 1 << ")" << endl;
                    //}

                } // 结束 10 次运行循环

                // 在处理完一个实例文件的所有运行后，向汇总文件写入换行符
                if (fout_m.is_open()) {
                    fout_m << endl;
                    fout_m.close();
                }
                if (fout_t.is_open()) {
                    fout_t << endl;
                    fout_t.close();
                }

                instanceCount++;
                //break;
           // }
        }
            
    } while (FindNextFileA(hFind, &fd) != 0); // 查找下一个文件

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) { // 检查结束是否因为错误
        cerr << "文件迭代期间发生错误: " << dwError << endl;
    }

    FindClose(hFind); // 关闭搜索句柄
    /*cout << "完成处理目录中找到的 " << instanceCount << " 个实例文件。" << endl;*/
}

int main()
{
    srand((unsigned int)time(NULL));
    string benchmarkDirectory = "data\\Small_Size_Instances";
    // string benchmarkDirectory = "data\\Big_Size_Instances";
    // 调用函数处理目录中的所有实例
    processBenchmarkInstances(benchmarkDirectory);

    return 0;
}

//for AAD
int main2()
{
    srand((unsigned int)time(NULL));
    int NumberOfNP_Integer = 0;
    vector<int> MaxForNP_Integer;
    vector<int> MinForNP_Integer;
    MaxForNP_Integer.resize(NumberOfNP_Integer);
    MinForNP_Integer.resize(NumberOfNP_Integer);

    int NumberOfNP_Real = 5;
    vector<double> MaxForNP_Real;
    vector<double> MinForNP_Real;
    MaxForNP_Real.resize(NumberOfNP_Real);
    MinForNP_Real.resize(NumberOfNP_Real);

    //for the non-delay factor
    MaxForNP_Real[0] = 1.00;
    MinForNP_Real[0] = 0.00;
    MaxForNP_Real[1] = 1.00;
    MinForNP_Real[1] = 0.00;
    MaxForNP_Real[2] = 1.00;
    MinForNP_Real[2] = 0.00;
    MaxForNP_Real[3] = 1.00;
    MinForNP_Real[3] = 0.00;
    MaxForNP_Real[4] = 1.00;
    MinForNP_Real[4] = 0.00;
    //MaxForNP_Real[5] = 1.00;
    //MinForNP_Real[5] = 0.00;


    int NumberOfCP = 0;
    vector<int> KindsForCP;
    KindsForCP.resize(NumberOfCP);


    int NumberOfNP_Sub = 0;
    int NumberOfCP_Sub = 0;

    Race* race = new Race(NumberOfNP_Integer, NumberOfNP_Real, MaxForNP_Integer, MinForNP_Integer, MaxForNP_Real, MinForNP_Real, NumberOfCP, KindsForCP);
    race->run();
    delete race;

    return 0;
}
