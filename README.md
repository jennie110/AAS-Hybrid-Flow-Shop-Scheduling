# From Full Active to All Active: A Constructive Heuristic Framework for Hybrid Flow Shop Scheduling and Its Automatic Algorithm Design

## Description

This repository provides the C++ source code and benchmark instances used to evaluate constructive heuristics for the hybrid flow shop scheduling problem with makespan minimization. It contains the proposed all-active-schedule heuristic and the comparison algorithms used in the study.

## Dataset information

The `Jose_benchmark` folder contains 480 benchmark instances introduced by Fernandez-Viagas and Framinan (2020):

- `Small_Size_Instances`: 240 instances with 10 to 35 jobs and 5 to 20 stages.
- `Big_Size_Instances`: 240 instances with 80 to 240 jobs and 5 to 20 stages.
- `UpperBounds_01_April_2019.xlsx`: reference upper bounds supplied with the benchmark.

Each text file is named `instancia_n_s_i.txt`, where `n` is the number of jobs, `s` is the number of stages, and `i` is the instance index. The first line gives `n` and `s`. The second line gives the number of identical parallel machines at each stage. The following `s` lines give the processing times of the `n` jobs at each stage. No additional data preprocessing is required.

## Code information

- `AAS`: implementation of the proposed all-active-schedule constructive heuristic.
- `BFH`: implementation of the bottleneck-focused heuristic used for comparison.
- `NEH_Only`: implementations of the priority-rule and NEH-based heuristics used for comparison.

Each code folder contains one `main.cpp` file and the required header files. The active dataset path is defined by `benchmarkDirectory` in each `main.cpp`. By default, AAS reads the small instances, while BFH and NEH_Only read the large instances.

## Requirements

- Windows 10 or later.
- A C++17 compiler, such as MinGW-w64 `g++` or Microsoft Visual C++.
- No external libraries are required.

## Usage instructions

Open PowerShell in the repository root. With MinGW-w64 `g++`, compile the three programs as follows:

```powershell
g++ -std=c++17 -O2 AAS/main.cpp -o AAS.exe
g++ -std=c++17 -O2 BFH/main.cpp -o BFH.exe
g++ -std=c++17 -O2 NEH_Only/main.cpp -o NEH_Only.exe
```

Create the AAS output directory and run the required program from the repository root:

```powershell
New-Item -ItemType Directory -Force results/Jose | Out-Null
.\AAS.exe
.\BFH.exe
.\NEH_Only.exe
```

The programs read all `.txt` instances from the directory specified by `benchmarkDirectory`. To use the other instance set, change this variable to either `Jose_benchmark\\Small_Size_Instances` or `Jose_benchmark\\Big_Size_Instances`, then recompile the program. Makespan values, running times, and detailed schedules are written to the `results` folder.

## Methodology

For each benchmark file, the program reads the number of jobs, stages, machines, and processing times. It then constructs a feasible hybrid flow shop schedule using the selected heuristic, calculates the makespan and computation time, and saves the summary and schedule files. AAS applies the all-active-schedule construction procedure described in the manuscript. BFH and NEH_Only provide the comparison results.

## Citation

When using the benchmark instances, cite:

Fernandez-Viagas, V., and Framinan, J. M. (2020). Design of a testbed for hybrid flow shop scheduling with identical machines. *Computers & Industrial Engineering*, 141, 106288. https://doi.org/10.1016/j.cie.2020.106288

When using this code, please also cite the article associated with this repository.

## License and contributions

The files are provided for research reproducibility. Questions and suggested corrections may be submitted through the GitHub issue tracker.
