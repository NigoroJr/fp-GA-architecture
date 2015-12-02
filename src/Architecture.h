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

    // TODO: switch block type?

private:
    void make_arch_file(std::string& filename);
    class Results {
        public:
            Results();
            ~Results();
            void run_benchmark();

            /* Method that parses the output of vpr in the form of a string.
             * res: stream of vpr results.
             * output: a pair, with first representing area in minimum transistor units
             *         and second representing critical path in ns. Return -1, -1 if all
             *         metrics are not found.
             */
            std::pair<double, double> parse_results(FILE* res);

        private:
            double crit_path;
            double area;
            std::string benchmark;
            std::string arch_file;
    };

    // const unsigned Architecture::UNSET = 0;

#endif /* end of include guard */
