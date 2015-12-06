#include "Architecture.h"

using Benchmark = Architecture::Benchmark;

const unsigned Architecture::UNSET = 0;
const std::pair<unsigned, unsigned> Architecture::K_RANGE{2, 50};
const std::pair<unsigned, unsigned> Architecture::I_RANGE{1, 100};
const std::pair<unsigned, unsigned> Architecture::W_RANGE{1, 5000};
std::random_device Architecture::rd;
std::mt19937_64 Architecture::gen{rd()};

using u_dist_t = std::uniform_int_distribution<unsigned>;
u_dist_t Architecture::k_rgen{K_RANGE.first, K_RANGE.second};
u_dist_t Architecture::w_rgen{W_RANGE.first, W_RANGE.second};
u_dist_t Architecture::i_rgen{I_RANGE.first, I_RANGE.second};

/* Static methods */

Architecture Architecture::random() {
    Architecture arch;
    arch.K = k_rgen(gen);
    arch.I = i_rgen(gen);
    arch.W = w_rgen(gen);

    return arch;
}

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Benchmark::Benchmark()
    : crit_path{UNSET}
    , area{UNSET}
    , benchmark{""}
{ }

// Copy constructor
Benchmark::Benchmark(const Benchmark& other)
    : crit_path{other.crit_path}
    , area{other.area}
    , benchmark{other.benchmark}
{ }

// Move constructor
Benchmark::Benchmark(Benchmark&& other)
    : crit_path{std::move(other.crit_path)}
    , area{std::move(other.area)}
    , benchmark{std::move(other.benchmark)}
{ }

// Filename constructor
Benchmark::Benchmark(const std::string& filename)
    : crit_path{UNSET}
    , area{UNSET}
    , benchmark{filename}
{ }

// Destructor
Benchmark::~Benchmark()
{ }

// Assignment operator
Benchmark& Benchmark::operator=(const Benchmark& other) {
    crit_path = other.crit_path;
    area = other.area;
    benchmark = other.benchmark;
    return *this;
}

// Move assignment operator
Benchmark& Benchmark::operator=(Benchmark&& other) {
    crit_path = std::move(other.crit_path);
    area = std::move(other.area);
    benchmark = std::move(other.benchmark);
    return *this;
}
/* }}} */

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Architecture::Architecture()
    : K{UNSET}
    , I{UNSET}
    , W{UNSET}
{ }

// Copy constructor
Architecture::Architecture(const Architecture& other)
    : K{other.K}
    , I{other.I}
    , W{other.W}
{ }

// Move constructor
Architecture::Architecture(Architecture&& other)
    : K{std::move(other.K)}
    , I{std::move(other.I)}
    , W{std::move(other.W)}
{ }

// Destructor
Architecture::~Architecture()
{ }

// Assignment operator
Architecture& Architecture::operator=(const Architecture& other) {
    K = other.K;
    I = other.I;
    W = other.W;
    return *this;
}

// Move assignment operator
Architecture& Architecture::operator=(Architecture&& other) {
    K = std::move(other.K);
    I = std::move(other.I);
    W = std::move(other.W);
    return *this;
}
/* }}} */

void Architecture::mutate(const float amount) {
    std::normal_distribution<float> k_dist(K, K * amount);
    std::normal_distribution<float> w_dist(W, W * amount);
    std::normal_distribution<float> i_dist(I, I * amount);
    K = static_cast<decltype(K)>(k_dist(gen));
    W = static_cast<decltype(W)>(w_dist(gen));
    I = static_cast<decltype(I)>(i_dist(gen));
}

bool Architecture::operator==(const Architecture& other) const {
    return K == other.K
        && I == other.I
        && W == other.W;
}

bool Architecture::operator!=(const Architecture& other) const {
    return K != other.K
        || I != other.I
        || W != other.W;
}

std::ostream& operator<<(std::ostream& os, const Architecture& a) {
    os << std::setw(30) << std::left << "W (channel width): " << a.W << std::endl;
    os << std::setw(30) << std::left << "K (num inputs per LUT): " << a.K << std::endl;
    os << std::setw(30) << std::left << "I (num of LUTs per cluster): " << a.I;

    return os;
}

void Architecture::make_arch_file() {
    // Construct delay matrix based on K
    std::string line, temp = "2.690e-10";
    for (size_t i = 0; i < K - 1; i++) {
        temp += "\n2.690e-10";
    }

    // The unordered map that defined the parts of the template to replace and
    // with what
    std::unordered_map<std::string, std::string> temp_args = {
        {TEMP_K, "num_pins=\"" + std::to_string(K) + "\""},
        {"TEMP_DELAY", temp},
        {TEMP_I, "num_pb=\"" + std::to_string(I) + "\""}
    };

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
            }
            os << " ";
        }
        os << std::endl;
        ss.str(std::string());
        ss.clear();
    }
    os.close();
}

void Architecture::run_benchmarks() {
    std::string command;
    FILE* res;
    double temp_area, temp_crit;

    // Run each benchmark
    for (Benchmark& b : bench) {
        // The command to give to popen
        command = std::string(VPR_PATH) + " " + arch_file + " "
            + b.get_filename() + " -route_chan_width " + std::to_string(W);

        // Run the benchmark multiple times
        for (unsigned i = 0; i < BENCH_ITER; i++) {
            // Run vpr
            res = popen(command.c_str(), "r");

            // Save the results of the benchmark
            if (b.crit_path == UNSET) {
                std::tie(b.area, b.crit_path) = b.parse_results(res);
            }
            else {
                std::tie(temp_area, temp_crit) = b.parse_results(res);
                b.area = std::min(b.area, temp_area);
                b.crit_path = std::max(b.crit_path, temp_crit);
            }
        }
    }
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
            return std::pair<double, double>(-1, -1);
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
