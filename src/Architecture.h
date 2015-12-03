#ifndef ARCHITECTURE_H_
#define ARCHITECTURE_H_

#include <algorithm>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <utility>
#include <vector>
#include <unordered_map>


#define VPR_PATH
#define CRIT_PATH "Final critical path:"
#define LOGIC_AREA "Total used logic block area:"
#define ROUTE_AREA "Total routing area:"
#define SCIENTIFIC_NOTATION "[-]?[0-9]+\\.[0-9]+([e][-+][0-9]+)"
#define NUM_METRICS 3
#define TEMP_K "num_pins=TEMP_K"
#define TEMP_I "num_pb=TEMP_I"
#define BENCH_ITER 10

class Architecture {
public:
    static const unsigned UNSET = 0;

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    Architecture();

    // Copy constructor
    Architecture(const Architecture& other);

    // Move constructor
    Architecture(Architecture&& other);

    // Destructor
    ~Architecture();

    // Assignment operator
    Architecture& operator=(const Architecture& other);

    // Move assignment operator
    Architecture& operator=(Architecture&& other);
    /* }}} */

    /* Member variables */

    /* Number of inputs to LUT */
    unsigned K;

    /* Number of logic elements in a cluster */
    unsigned I;

    /* Channel Width */
    unsigned W;

    /* All benchmarks to test with */
    std::vector<Benchmark> bench;

    /* Constructs the architecture file */
    void make_arch_file();

    /* Run each benchmark and store it in the benchmark object */
    void run_benchmarks();

    // TODO: switch block type?

    /* The architecture file that represents the architecture */
    std::string arch_file;

private:

    class Benchmark {
        public:
            /* Constructors, Destructor, and Assignment operators {{{ */

            // Default constructor
            Benchmark();

            // Copy constructor
            Benchmark(const Benchmark& other);

            // Move constructor
            Benchmark(Benchmark&& other);

            // Filename constructor
            Benchmark(const std::string& filename);

            // Destructor
            ~Benchmark();

            // Assignment operator
            Benchmark& operator=(const Benchmark& other);

            // Move Assignment operator
            Benchmark& operator=(const Benchmark&& other);
            /* }}} */

            /* Method that parses the output of vpr in the form of a string.
             * res: stream of vpr results.
             * output: a pair, with first representing area in minimum transistor units
             *         and second representing critical path in ns. Return -1, -1 if all
             *         metrics are not found.
             */
            std::pair<double, double> parse_results(FILE* res);
            inline double get_crit_path() {return crit_path;}
            inline double get_area() {return area;}
            inline double get_filename() {return benchmark;}

        private:
            double crit_path;
            double area;
            std::string benchmark;
    };

    // const unsigned Architecture::UNSET = 0;

#endif /* end of include guard */
