#include "Architecture.h"

using Benchmark = Architecture::Benchmark;

const unsigned Architecture::UNSET = 0;
const std::pair<unsigned, unsigned> Architecture::K_RANGE{2, 25};
const std::pair<unsigned, unsigned> Architecture::N_RANGE{1, 50};
const std::pair<unsigned, unsigned> Architecture::W_RANGE{1, 250};
std::random_device Architecture::rd;
std::mt19937_64 Architecture::gen{rd()};

using u_dist_t = std::uniform_int_distribution<unsigned>;
u_dist_t Architecture::k_rgen{K_RANGE.first, K_RANGE.second};
u_dist_t Architecture::w_rgen{W_RANGE.first, W_RANGE.second};
u_dist_t Architecture::n_rgen{N_RANGE.first, N_RANGE.second};

const double Benchmark::FAILED = -1;

/* Static methods */

Architecture Architecture::random() {
    Architecture arch;
    arch.K = k_rgen(gen);
    arch.N = n_rgen(gen);
    arch.W = w_rgen(gen);
    // Routing channel width must be even for unidirectional
    arch.W = arch.W % 2 == 0 ? arch.W : arch.W + 1;

    return arch;
}

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Benchmark::Benchmark()
    : crit_path{FAILED}
    , area{FAILED}
    , benchmark{""}
    , is_populated{false}
{ }

// Copy constructor
Benchmark::Benchmark(const Benchmark& other)
    : crit_path{other.crit_path}
    , area{other.area}
    , benchmark{other.benchmark}
    , is_populated{other.is_populated}
{ }

// Move constructor
Benchmark::Benchmark(Benchmark&& other)
    : crit_path{std::move(other.crit_path)}
    , area{std::move(other.area)}
    , benchmark{std::move(other.benchmark)}
    , is_populated{std::move(other.is_populated)}
{ }

// Filename constructor
Benchmark::Benchmark(const std::string& filename)
    : crit_path{FAILED}
    , area{FAILED}
    , benchmark{filename}
    , is_populated{false}
{ }

// Destructor
Benchmark::~Benchmark()
{ }

// Assignment operator
Benchmark& Benchmark::operator=(const Benchmark& other) {
    crit_path = other.crit_path;
    area = other.area;
    benchmark = other.benchmark;
    is_populated = other.is_populated;
    return *this;
}

// Move assignment operator
Benchmark& Benchmark::operator=(Benchmark&& other) {
    crit_path = std::move(other.crit_path);
    area = std::move(other.area);
    benchmark = std::move(other.benchmark);
    is_populated = std::move(other.is_populated);
    return *this;
}
/* }}} */

std::string Benchmark::to_s(unsigned indent) const {
    std::ostringstream os;
    // Whitespace for indentation
    auto indent_str = std::string(indent, ' ');

    os << benchmark << std::endl;
    os << indent_str << "  " << std::setw(13) << std::left
        << "Crit. Path:" << crit_path << std::endl;
    os << indent_str << "  " << std::setw(13) << std::left
        << "Area:" << area;

    return os.str();
}

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Architecture::Architecture()
    : K{UNSET}
    , N{UNSET}
    , W{UNSET}
    , bench{}
    , reference_results{}
    , speed_penalty{0}
    , area_penalty{0}
    , arch_file{}
{ }

// Copy constructor
Architecture::Architecture(const Architecture& other)
    : K{other.K}
    , N{other.N}
    , W{other.W}
    , bench{other.bench}
    , reference_results{other.reference_results}
    , speed_penalty{other.speed_penalty}
    , area_penalty{other.area_penalty}
    , arch_file{other.arch_file}
{ }

// Move constructor
Architecture::Architecture(Architecture&& other)
    : K{std::move(other.K)}
    , N{std::move(other.N)}
    , W{std::move(other.W)}
    , bench{std::move(other.bench)}
    , reference_results{std::move(other.reference_results)}
    , speed_penalty{std::move(other.speed_penalty)}
    , area_penalty{std::move(other.area_penalty)}
    , arch_file{std::move(other.arch_file)}
{ }

// Destructor
Architecture::~Architecture()
{ }

// Assignment operator
Architecture& Architecture::operator=(const Architecture& other) {
    K = other.K;
    N = other.N;
    W = other.W;
    bench = other.bench;
    reference_results = other.reference_results;
    speed_penalty = other.speed_penalty;
    area_penalty = other.area_penalty;
    arch_file = other.arch_file;
    return *this;
}

// Move assignment operator
Architecture& Architecture::operator=(Architecture&& other) {
    K = std::move(other.K);
    N = std::move(other.N);
    W = std::move(other.W);
    bench = std::move(other.bench);
    reference_results = std::move(other.reference_results);
    speed_penalty = std::move(other.speed_penalty);
    area_penalty = std::move(other.area_penalty);
    arch_file = std::move(other.arch_file);
    return *this;
}
/* }}} */

void Architecture::mutate(const float amount) {
    std::normal_distribution<float> k_dist(K, K * amount);
    std::normal_distribution<float> w_dist(W, W * amount);
    std::normal_distribution<float> n_dist(N, N * amount);
    K = static_cast<decltype(K)>(k_dist(gen));
    W = static_cast<decltype(W)>(w_dist(gen));
    N = static_cast<decltype(N)>(n_dist(gen));
}

bool Architecture::operator==(const Architecture& other) const {
    return K == other.K
        && N == other.N
        && W == other.W;
}

bool Architecture::operator!=(const Architecture& other) const {
    return K != other.K
        || N != other.N
        || W != other.W;
}

bool Architecture::operator>(const Architecture& other) const {
    return speed_penalty + area_penalty < other.speed_penalty + other.area_penalty;
}

bool Architecture::operator<(const Architecture& other) const {
    return speed_penalty + area_penalty > other.speed_penalty + other.area_penalty;
}

std::ostream& operator<<(std::ostream& os, const Architecture& a) {
    os << std::setw(30) << std::left
        << "W (channel width): " << a.W << std::endl;
    os << std::setw(30) << std::left
        << "K (num inputs per LUT): " << a.K << std::endl;
    os << std::setw(30) << std::left
        << "N (num of LUTs per cluster): " << a.N << std::endl;

    bool print_results = std::any_of(a.bench.begin(), a.bench.end(),
                                     [](const Benchmark& b) {
                                     return b.is_populated;
                                     });
    if (print_results) {
        os << "with results:" << std::endl;
        for (const Architecture::Benchmark& benchmark : a.bench) {
            if (benchmark.is_populated) {
                os << benchmark.to_s(2) << std::endl;
            }
        }
    }

    return os;
}

std::string Architecture::make_arch_file() {
    char folder_buf[80];
    std::sprintf(folder_buf,
            "./%d_%d_%d",
            K, N, W);
#pragma omp critical
    mkdir(folder_buf, 0700);
    dir = std::string{folder_buf};

    // Construct delay matrix based on K
    std::string line, temp = "2.690e-10";
    for (size_t i = 0; i < K - 1; i++) {
        temp += "\n2.690e-10";
    }

    // The unordered map that defined the parts of the template to replace and
    // with what
    std::unordered_map<std::string, std::string> temp_args = {
        {"TEMP_K", "\"" + std::to_string(K) + "\" "},
        {"TEMP_K_RANGE", std::to_string(K - 1) + ":0"},
        {"TEMP_DELAY", temp},
        {"TEMP_N", "\"" + std::to_string(N) + "\" "},
        {"TEMP_N_RANGE", std::to_string(N - 1) + ":0"},
        {"CLB_IN", "\"" + std::to_string((K / 2) * (N + 1)) + "\" "}
    };

    char arch_file_buf[128];
    std::sprintf(arch_file_buf, "%s/%d_%d_%d.xml", dir.c_str(), K, N, W);
    arch_file = std::string{arch_file_buf};
    // Open the files we need to read and write
    std::ifstream is("../arch_template.xml");
    std::ofstream os(arch_file);
    std::stringstream ss;

    // Read to the end of the template
    while (std::getline(is, line)) {
        ss << line;

        // Read each word of the line
        while (ss >> temp) {
            // Check to see if the word need to be replaced
            auto it = temp_args.find(temp);

            // Replace it in the output document if necessary. Otherwise just
            // print the original world from the template.
            if (it != temp_args.end()) {
                os << it->second;
            }
            else {
                os << temp;
                if (temp.back() != '[' && temp.back() != '=') {
                    os << " ";
                }
            }
        }
        os << std::endl;
        ss.str(std::string());
        ss.clear();
    }
    os.close();

    return arch_file;
}

inline std::string get_basename(const std::string& path) {
    size_t start = path.rfind('/') + 1;
    size_t len = path.rfind('.') - start;
    return path.substr(start, len);
}

void Architecture::run_benchmarks(const std::string& vtr_path) {
    // If this is the first generation, also save the results as reference
    if (reference_results.empty()) {
        reference_results.resize(bench.size());
        // Fill with unpopulated results
        std::generate(reference_results.begin(),
                      reference_results.end(),
                      []() { return Benchmark(); });
    }

    std::string path;

    // Run each benchmark
    for (unsigned i = 0; i < bench.size(); i++) {
        Benchmark& b = bench[i];
        // Don't need to rerun VPR if already run
        if (b.is_populated) {
            continue;
        }

        path = dir + '/' + get_basename(b.get_filename());
        mkdir(path.c_str(), 0700);
        path += '/';
        // The command to give to popen
        char command_abc[512];
        char command_vpr[512];
        std::sprintf(command_abc,
                "%svtr_flow/scripts/run_vtr_flow.pl %s %s " \
                "-starting_stage abc -ending_stage abc -keep_intermediate_files " \
                "-keep_result_files -temp_dir %s 1>/dev/null 2>&1",
                vtr_path.c_str(),
                b.get_filename().c_str(),
                arch_file.c_str(),
                path.c_str());

#ifdef DEBUG
#pragma omp critical(print)
        std::cout << "Running ABC with: " << command_abc << std::endl;
#endif

        system(command_abc);

        std::string new_blif = path + get_basename(b.get_filename()) + ".abc.blif";
        if (access(new_blif.c_str(), F_OK) == -1) {
            b.crit_path = Benchmark::FAILED;
            b.area = Benchmark::FAILED;
            continue;
        }

        // Run the benchmark multiple times
        for (unsigned i = 0; i < BENCH_ITER; i++) {
            int seed = rd();
            if (seed < 0) {
                seed = -1 * seed;
            } else if (seed == 0) {
                seed++;
            }
            std::sprintf(command_vpr,
                    "%s/vpr/vpr %s %s -route_chan_width %d -seed %d",
                    vtr_path.c_str(),
                    arch_file.c_str(),
                    new_blif.c_str(),
                    W, seed);
            std::string command{command_vpr};

#ifdef DEBUG
#pragma omp critical(print)
            {
                std::cout << "Running " << b.benchmark << std::endl;
                std::cout << "with parameters:" << std::endl;
                std::cout << *this << std::endl;
                std::cout << command << std::endl;
            }
#endif

            // Run vpr
            FILE* res = popen(command.c_str(), "r");

            double res_area, res_crit;
            std::tie(res_area, res_crit) = b.parse_results(res);

            // Finish process
            pclose(res);

            // Save the results of the benchmark
            if (b.is_populated) {
                b.area = std::min(b.area, res_area);
                b.crit_path = std::min(b.crit_path, res_crit);
            }
            else if (!b.failed()) {
                b.area = res_area;
                b.crit_path = res_crit;

                b.is_populated = true;

                // Save as reference values
                if (!reference_results[i].is_populated) {
                    reference_results[i] = b;
                    reference_results[i].is_populated = true;
                }
            }

#ifdef DEBUG
#pragma omp critical(print)
            {
                std::cout << "Finished running:" << std::endl;
                std::cout << *this << std::endl;
            }
#endif

            if (b.failed()) {
                break;
            }
        }
    }
    system(("rm -rf " + dir).c_str());
}

std::pair<double, double> Benchmark::parse_results(FILE* res) {
    double metrics[NUM_METRICS];
    std::regex reg[NUM_METRICS] = {std::regex(LOGIC_AREA),
        std::regex(ROUTE_AREA), std::regex(CRIT_PATH)};
    std::stringstream ss;
    std::string temp;
    char line[256];
    // Search stream until all metrics are found
    for (size_t i = 0; i < NUM_METRICS;) {
        // If you get to end of stream, output failure
        if (!fgets(line, 256, res)) {
            return std::pair<double, double>(FAILED, FAILED);
        }
        // If the stat we are looking for is found, parse the line
        if (std::regex_search(line, reg[i])) {
            ss << line;
            while (ss >> temp) {
                // If the number is represented in scientific notation,
                // parse it to output a double
                if (std::regex_match(temp, std::regex(SCIENTIFIC_NOTATION))) {
                    metrics[i] = atof(temp.substr(0, temp.find('e') + 1).c_str());
                    int exp = atoi(temp.substr(temp.find('e') + 1).c_str());
                    while (exp != 0) {
                        metrics[i] *= 10;
                        exp--;
                    }
                    break;
                }
                else if (temp[0] >= '0' && temp[0] <= '9') {
                    metrics[i] = atof(temp.c_str());
                    break;
                }
            }
            ss.str(std::string());
            i++;
        }
    }
    return std::pair<double, double>(metrics[0] + metrics[1], metrics[2]);
}

float Architecture::vs_ref_crit_path() const {
    float sum = 0;
    for (unsigned i = 0; i < bench.size(); i++) {
        // If reference value is not set, then there's no performance change
        if (!reference_results[i].is_populated) {
            return 1.0;
        }

        sum += static_cast<float>(bench[i].crit_path) / reference_results[i].crit_path;
    }

    return sum / bench.size();
}

float Architecture::vs_ref_area() const {
    float sum = 0;
    for (unsigned i = 0; i < bench.size(); i++) {
        // If reference value is not set, then there's no performance change
        if (!reference_results[i].is_populated) {
            return 1.0;
        }

        sum += static_cast<float>(bench[i].area) / reference_results[i].area;
    }

    return sum / bench.size();
}
