#include "Architecture.h"

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

void Architecture::make_arch_file() {
    std::string line, temp = "2.690e-10";
    for (size_t i = 0; i < K - 1; i++) {
        temp += "\n2.690e-10";
    }
    std::unordered_map<std::string, std::string> temp_args = {
        {TEMP_K, "num_pins=\"" + std::to_string(K) + "\""},
        {"TEMP_DELAY", temp}};
    std::ifstream is("../arch_template.xml");
    std::ofstream os(arch_files);
    std::stringstream ss;
    while (std::getline(is, line)) {
        ss << line;
        while (ss >> temp) {
            auto it = temp_args.find(temp);
            if (it != temp_args.end()) {
                os << it->second;
            } else {
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

Architecture::Results::Results()
    : crit_path{UNSET}
    , area{UNSET}
    , benchmark{""}
{ }

Architecture::Results::~Results()
{ }

void Architecture::run_benchmark(unsigned K, unsigned I, unsigned W,
        const std::string& arch_file) {
    std::string command = std::string(VPR_PATH) + " " + arch_file + " "
        + benchmark + " -route_chan_width " + W;
    FILE* res;
    double temp_area, temp_crit;
    for (unsigned i = 0; i < BENCH_ITER; i++) {
        res = popen(command.c_str());
        if (crit_path == UNSET) {
            std::tie(area, crit_path) = parse_results(res);
        } else {
            std::tie(temp_area, temp_crit) = parse_results(res);
            area = std::min(area, temp_area);
            crit_path = std::max(crit_path, temp_crit);
        }
    }
}

Architecture::Results::std::pair<double, double> parse_results(FILE* res) {
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
                } else if (temp[0] >= '0' && temp[0] <= '9') {
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

// TODO: Implement me!
