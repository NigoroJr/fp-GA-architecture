#ifndef ARCHITECTURE_H_
#define ARCHITECTURE_H_

// #define DEBUG

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>
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
#include <iostream>

#define CRIT_PATH "Final critical path:"
#define LOGIC_AREA "Total used logic block area:"
#define ROUTE_AREA "Total routing area:"
#define SCIENTIFIC_NOTATION "[-]?[0-9]+\\.[0-9]+([e][-+][0-9]+)"
#define NUM_METRICS 3
#define BENCH_ITER 1

class Architecture {
public:
    struct Benchmark {
        static const double FAILED;

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
         * output: a pair, with first representing area in minimum transistor
         *         units and second representing critical path in ns. Returns
         *         pair (FAILED, FAILED) if all metrics are not found.
         */
        std::pair<double, double> parse_results(const std::shared_ptr<FILE>& res);

        double get_crit_path() const;
        double get_area() const;
        const std::string& get_filename() const;
        bool failed() const;
        /**
         * Creates a formatted string of the results that can be printed.
         *
         * \param[in] indent the indentation of the string.
         *
         * \return a string representation of the benchmark results.
         */
        std::string to_s(unsigned indent = 0u) const;

        double crit_path;
        double area;
        std::string benchmark;
        bool is_populated;
    };

    static const unsigned UNSET;
    static const std::pair<unsigned, unsigned> K_RANGE;
    static const std::pair<unsigned, unsigned> N_RANGE;
    static const std::pair<unsigned, unsigned> W_RANGE;

    /* The results used as a reference to measure performance gain */
    static std::vector<Benchmark> reference_results;

    /**
     * \return a randomly generated architecture.
     */
    static Architecture random(const std::vector<Benchmark>& benchmarks = {});

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    Architecture();

    Architecture(const std::vector<Benchmark>& bench);

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
    unsigned N;

    /* Channel Width */
    unsigned W;

    // TODO: switch block type?

    /* All benchmarks to test with */
    std::vector<Benchmark> bench;

    /* Member methods */

    /* Constructs the architecture file */
    std::string make_arch_file();

    /* Run each benchmark and store it in the benchmark object */
    void run_benchmarks(const std::string& vtr_path);

    /**
     * Changes the property of this architecture.
     *
     * \param[in] amount percentage of the change.
     */
    void mutate(const float amount);

    /**
     * The average ratio of the benchmarks compared to the reference results.
     * For example, if the following results are observed,
     *
     *        | Ref | This | Ratio
     * -------+-----+------+------
     * Bench1 | 16  |   12 |  0.75
     * Bench2 | 40  |   32 |  0.80
     *
     * Avg: 0.775
     *
     * \return the average ratio of the benchmarks compared to the reference
     */
    double vs_ref_crit_path() const;

    /**
     * Same as vs_ref_crit_path but for area.
     */
    double vs_ref_area() const;

    /**
     * \return true if all benchmarks have been run already.
     */
    bool already_run() const;

    /**
     * \return true if NONE of the benchmarks failed.
     */
    bool non_failed() const;

    bool operator==(const Architecture& other) const;
    bool operator!=(const Architecture& other) const;

private:
    static std::random_device rd;
    static std::mt19937_64 gen;
    static std::uniform_int_distribution<unsigned> k_rgen;
    static std::uniform_int_distribution<unsigned> w_rgen;
    static std::uniform_int_distribution<unsigned> n_rgen;

    /* Directory to hold related files */
    std::string dir;

    /* The architecture file that represents the architecture */
    std::string arch_file;
};

inline double Architecture::Benchmark::get_crit_path() const {
    return crit_path;
}

inline double Architecture::Benchmark::get_area() const {
    return area;
}

inline const std::string& Architecture::Benchmark::get_filename() const {
    return benchmark;
}

inline bool Architecture::Benchmark::failed() const {
    return crit_path == FAILED || area == FAILED;
}

/* Stream insertion operator */
std::ostream& operator<<(std::ostream& os, const Architecture& a);

#endif /* end of include guard */
