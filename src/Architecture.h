#ifndef ARCHITECTURE_H_
#define ARCHITECTURE_H_

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

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
    static const unsigned UNSET;
    static const std::pair<unsigned, unsigned> K_RANGE;
    static const std::pair<unsigned, unsigned> I_RANGE;
    static const std::pair<unsigned, unsigned> W_RANGE;

    /**
     * \return a randomly generated architecture.
     */
    static Architecture random();

    struct Benchmark {
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
        Benchmark& operator=(Benchmark&& other);
        /* }}} */

        /* Method that parses the output of vpr in the form of a string.
         * res: stream of vpr results.
         * output: a pair, with first representing area in minimum transistor units
         *         and second representing critical path in ns. Return -1, -1 if all
         *         metrics are not found.
         */
        std::pair<double, double> parse_results(FILE* res);

        double get_crit_path();
        double get_area();
        std::string get_filename();

        double crit_path;
        double area;
        std::string benchmark;
    };

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

    /* All benchmarks to test with */
    std::vector<Benchmark> bench;

    /* Member methods */

    /* Constructs the architecture file */
    void make_arch_file();

    /* Run each benchmark and store it in the benchmark object */
    void run_benchmarks();

    /**
     * Changes the property of this architecture.
     *
     * \param[in] amount percentage of the change.
     */
    void mutate(const float amount);

    bool operator==(const Architecture& other) const;
    bool operator!=(const Architecture& other) const;

private:
    static std::random_device rd;
    static std::mt19937_64 gen;
    static std::uniform_int_distribution<unsigned> k_rgen;
    static std::uniform_int_distribution<unsigned> w_rgen;
    static std::uniform_int_distribution<unsigned> i_rgen;

    /* The architecture file that represents the architecture */
    std::string arch_file;
};

inline double Architecture::Benchmark::get_crit_path() {
    return crit_path;
}

inline double Architecture::Benchmark::get_area() {
    return area;
}

inline std::string Architecture::Benchmark::get_filename() {
    return benchmark;
}

/* Stream insertion operator */
std::ostream& operator<<(std::ostream& os, const Architecture& a);

#endif /* end of include guard */
