# HPC-Profiler
`hpc-profiler` is a tool to capture Hardware Performance Counter (HPC) statistics of programs.
To build `hpc-profiler`, PAPI must be installed. Please see the `side-channels/learning-papi/` directory in this git repo for installation instructions or the [official PAPI website](http://icl.utk.edu/papi/software/).

## Build and Run HPC-Profiler
### Build
To build `hpc-profiler`, run:
```
make
```
This builds the `hpc-profiler` executable and a `test` executable.

### Run
To run `hpc-profiler` and capture HPC data, run:
```
./hpc-profiler <run.config> -s
```
where `<run.config>` is a file which contains the command line arguments of programs to profile.
Each line in the `run.config` file is a separate program to profile. Below is an example `run.config` file:
```
./test
/bin/ls
```
The above `run.config` will cause `hpc-profiler` to profile the `test` executable and the `ls` executable.

The `-s` flag in the command line is optional. If `-s` flag is specified, then each of the process data will be placed into a separate `.csv` file.

Running `hpc-profiler` will generate a `run.csv`, which contains the configuration used during the profiling (ex. sampling frequency, total sample time, etc.).
If profiled with the `-s` flag, HPC data with be saved into separate `<program>.csv` files, where `<program>` is the name of the executable that was profiled.
Withou the `-s` flag, all data is compiled into `hpc-data.csv`. 

## Events to Capture
Currently `hpc-profiler` only captures PAPI Preset Events. To see all available preset events on your CPU, download and build PAPI, and then run:
```
papi/src/utils/papi_avail
```
To add events to capture, modify the `profiler.h` file and add preset event codes to the `events[]` array.

## To-Do
* Make it easier to add new event to profile (maybe use [this method](http://www.linux-pages.com/2013/02/how-to-map-enum-to-strings-in-c/)).
* ~~Create an option to place individual runs in the same `.csv` file or separate files.~~
