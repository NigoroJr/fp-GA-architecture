# fp-GA-architecture

[![Build Status](https://travis-ci.org/NigoroJr/fp-GA-architecture.svg)](https://travis-ci.org/NigoroJr/fp-GA-architecture)

## Description
Program that generates FPGA architectures using Genetic Algorithm. In the
current implementation, K (number of inputs to a LUT), N (number of LUTs in a
cluster), and W (channel width) are varied to find the optimal combination for
a given set of benchmarks. Combinations of parameters are evaluated using
[VTR](https://github.com/verilog-to-routing/vtr-verilog-to-routing).

## Requirements
* C++ compiler that supports C++11 standards
* [VTR](https://github.com/verilog-to-routing/vtr-verilog-to-routing)
* GNU/Linux or Unix-like operating system that uses `/` as the path separator
* CMake 2.8 or later

When compiling with debug on (i.e. with `-DCMAKE_BUILD_TYPE=Debug`)
* Boost.Test framework

## Recommended
* C++ compiler that has support for OpenMP

## Compiling and Running
Clone this repository and go to the top-level directory. Assuming VTR is
installed in `~/src/vtr-verilog-to-routing`, the program can be run with:

```
$ cd ./build
$ cmake ..
$ export OMP_NUM_THREADS=8
$ ./src/fp-GA-architecture ~/src/vtr-verilog-to-routing /path/to/benchmark/file
```

Various parameters for the genetic algorithm are described in the help
message, which is displayed when the program is invoked with the `-h` or
`--help` command line option.

## Caveats
This program was developed and checked on a gluster file system.  There seems
to be an
[issue](https://www.gluster.org/pipermail/gluster-users.old/2015-February/020809.html)
where directories cannot be removed. Thus, after running the program, there
may be directories that are formatted as `NUM1_NUM2_NUM3`. You can remove
these directories once the program has finished.

## Thanks
[cxxopts](https://github.com/jarro2783/cxxopts) for the command line option
parsing.
