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

Each code folder contains one `main.cpp` entry file and the required supporting `.cpp` files. The supporting files are included by `main.cpp`, so each program can be compiled directly from its entry file. The source files also use a single-translation-unit guard, which prevents duplicate definitions if all `.cpp` files in one algorithm folder are passed to the compiler together.

The AAS program provides two workflows in `AAS/main.cpp`:

- `main()` runs automatic algorithm design through the iterative I/F-Race procedure. This is the default entry point.
- `main2()` evaluates AAS with the fixed parameter configuration on the benchmark instances.

C++ permits only one function named `main()`. To run the fixed-parameter benchmark workflow, rename the current I/F-Race `main()` function to a non-entry name such as `runIFRace()`, and rename `main2()` to `main()`. Reverse these two names to restore the I/F-Race entry point. The function bodies do not need to be changed.

BFH and NEH_Only directly evaluate their algorithms on benchmark instances. The active dataset path is defined by `benchmarkDirectory` in each corresponding `main.cpp`.

## Requirements

- Windows 10 or later.
- A C++17 compiler, such as MinGW-w64 `g++` or Microsoft Visual C++.
- No external libraries are required.

## Usage instructions

Open PowerShell in the repository root. All commands must be run from this directory so that the relative paths to `Jose_benchmark` are resolved correctly.

### AAS automatic algorithm design with I/F-Race

The default `main()` in `AAS/main.cpp` starts the iterative I/F-Race procedure. It samples candidate parameter configurations, evaluates them on the Jose benchmark instances, removes statistically inferior configurations, and generates the elite configurations used by AAS.

Compile and run it as follows:

```powershell
g++ -std=c++17 -O2 AAS/main.cpp -o AAS.exe
.\AAS.exe
```

The I/F-Race implementation reads both `Jose_benchmark\\Small_Size_Instances` and `Jose_benchmark\\Big_Size_Instances`. The default settings in `AAS/Race_noMatlab.cpp` use 10 iterations and a total evaluation budget of 180,000. The procedure can therefore take considerable time. Progress is written to `EvolutionaryInformation.txt`, and the selected configurations are written to `EliteConfigurations.txt`.

`Race_noMatlab.cpp` contains the native C++ statistical test used by the default build. `Race.cpp` is the earlier MATLAB-dependent implementation and is not included by default.

### AAS evaluation with fixed parameters

The fixed-parameter benchmark workflow is the function currently named `main2()` in `AAS/main.cpp`. Before compilation:

1. Rename the default I/F-Race function from `int main()` to `int runIFRace()`.
2. Rename the fixed-parameter function from `int main2()` to `int main()`.
3. Set `benchmarkDirectory` to the required dataset. Use `Jose_benchmark\\Small_Size_Instances` for the small instances or `Jose_benchmark\\Big_Size_Instances` for the large instances.
4. Compile and run from the repository root.

```powershell
New-Item -ItemType Directory -Force results/Jose | Out-Null
g++ -std=c++17 -O2 AAS/main.cpp -o AAS.exe
.\AAS.exe
```

The program processes every `.txt` instance in the selected directory. It writes makespan values, running times, and detailed schedules under `results/Jose`.

### BFH and NEH-based comparison algorithms

Compile and run the comparison programs from the repository root:

```powershell
g++ -std=c++17 -O2 BFH/main.cpp -o BFH.exe
g++ -std=c++17 -O2 NEH_Only/main.cpp -o NEH_Only.exe

.\BFH.exe
.\NEH_Only.exe
```

BFH and NEH_Only currently read `Jose_benchmark\\Big_Size_Instances`. Change `benchmarkDirectory` in the corresponding `main.cpp` to `Jose_benchmark\\Small_Size_Instances` when the small instances are required, then recompile. Both programs create their output directories under `results` automatically.

## Methodology

For each benchmark file, the evaluation programs read the number of jobs, stages, machines, and processing times. They construct a feasible hybrid flow shop schedule using the selected heuristic, calculate the makespan and computation time, and save the summary and schedule files. The I/F-Race workflow automatically searches the AAS parameter space by repeatedly evaluating candidate configurations and retaining promising configurations. AAS applies the all-active-schedule construction procedure described in the manuscript. BFH and NEH_Only provide the comparison results.

## Citation

When using the benchmark instances, cite:

Fernandez-Viagas, V., and Framinan, J. M. (2020). Design of a testbed for hybrid flow shop scheduling with identical machines. *Computers & Industrial Engineering*, 141, 106288. https://doi.org/10.1016/j.cie.2020.106288

When using this code, please also cite the article associated with this repository.

## License and contributions

The files are provided for research reproducibility. Questions and suggested corrections may be submitted through the GitHub issue tracker.
