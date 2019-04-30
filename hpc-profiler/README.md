# HPC-Profiler
`hpc-profiler` is a tool to capture Hardware Performance Counter (HPC) statistics of programs.
To build `hpc-profiler`, PAPI must be installed. Please see the `side-channels/learning-papi/` directory in this git repo for installation instructions or the [official PAPI website](http://icl.utk.edu/papi/software/).

## Build and Run HPC-Profiler
### Build
To build `hpc-profiler`, run:
```
make
```
This builds the `profiler` executable and a `test` executable.

### Run
To run `hpc-profiler` and capture HPC data, run:
```
./profiler <run.config>
```
where `<run.config>` is a file which contains the command line arguments of programs to profile.
Each line in the `run.config` file is a separate program to profile. Below is an example of a `run.config` file:
```
./test
/bin/ls
```
The above `run.config` will cause `hpc-profiler` to profile the `test` executable and the `ls` executable.

## Events to Capture
Currently `hpc-profiler` only captures PAPI Preset Events. To see all available preset events on your CPU, download and build PAPI, and then run:
```
papi/src/utils/papi_avail
```
To add events to capture, modify the `profiler.h` file and add preset event codes to the `events[]` array.
