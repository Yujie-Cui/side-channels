# Sample Programs to Profile
This directory contains sample programs to profile with `hpc-profiler`.

## Build Programs
To build all sample programs, run:
```
make
```

## List of Sample Programs
* `test.c`: A simple program that calls `printf()` and loops for a while
* `spectre.c`: My implementation of Spectre (using the SEED Lab)
* `erik-spectre.c`: [This implementation](https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6)
* `bpoison-spectre.c`: Attempts to manipulate jump targets (does not work... but the Spectre technique is there...)
* `flush-prefetch-spectre.c`: Times access using prefetch instead of directly accessing the probe array
