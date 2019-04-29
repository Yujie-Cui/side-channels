# Learning How to Use PAPI

## Installing PAPI
1. Download the latest `papi-.tar.gz` from here (scroll down the web page): [Downloading and Installing PAPI](http://icl.utk.edu/papi/software/)
(cloning the git respository gave me undefined errors ; it's probably not up-to-date).

2. Go to `papi/src` directory
```
cd papi/src
```

3. Run configuration script and build PAPI
```
./configure
make
```

4. Install PAPI
```
sudo make install-all
```

## Compiling C Programs Using PAPI Library
1. Add `#include <papi.h>` to `test.c` file, where `test.c` is the program to compile

2. Run the following command to compile `test.c`:
```
gcc -I/usr/local/include -O0 test.c /usr/local/lib/libpapi.a -o test 
```

## Directories
1. `prac` contains my practice code

## Resources
* [PAPI User's Guide](http://icl.cs.utk.edu/projects/papi/files/documentation/PAPI_USER_GUIDE.htm#INTRODUCTION_TO_PAPI)
* [Original PAPI Paper](https://www.icl.utk.edu/files/publications/1999/icl-utk-58-1999.pdf)
