#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <utility>
#include <vector>
#include <queue>
#include "Architecture.h"

#define CRIT_PATH "Final critical path:"
#define LOGIC_AREA "Total used logic block area:"
#define ROUTE_AREA "Total routing area:"
#define SCIENTIFIC_NOTATION "[-]?[0-9]+\\.[0-9]+([e][-+][0-9]+)"
#define NUM_METRICS 3
#define TEMP_K "num_pins=TEMP_K"

/* Method that parses the output of vpr in the form of a string.
 * res: stream of vpr results.
 * output: a pair, with first representing area in minimum transistor units
 *         and second representing critical path in ns. Return -1, -1 if all
 *         metrics are not found.
 */
auto parse_results(FILE* res) {
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

void make_arch_file(const Architecture& arch, const std::string& out_file) {
    std::string line, temp = "2.690e-10";
    std::queue<std::string> to_replace(std::deque<std::string>({TEMP_K, "TEMP_DELAY"}));
    std::queue<std::string> replace_with(
            std::deque<std::string>({
                std::string("num_pins=\"") + std::to_string(arch.K) + "\""
                }));
    for (size_t i = 0; i < arch.K - 1; i++) {
        temp += "\n2.690e-10";
    }
    replace_with.push(temp);
    std::ifstream is("../arch_template.xml");
    std::ofstream os(out_file);
    std::stringstream ss;
    while (std::getline(is, line)) {
        if (to_replace.size() > 0) {
            ss << line;
            while (ss >> temp) {
                if (to_replace.size() && temp == to_replace.front()) {
                    os << replace_with.front();
                    replace_with.pop();
                    to_replace.pop();
                } else {
                    os << temp;
                }
                os << " ";
            }
            os << std::endl;
            ss.str(std::string());
            ss.clear();
        } else {
            os << line << std::endl;
        }
    }
    if (to_replace.size() != 0) {
        std::cout << "Couldn't find all the template variables." << std::endl;
        exit(-1);
    }
    os.close();
}

int main() {
    /*
       double crit_path, area;
       FILE* res = popen("/home/continnd/ECE487/vtr-verilog-to-routing/vpr/vpr /home/continnd/ECE487/vtr-verilog-to-routing/libarchfpga/arch/sample_arch.xml /home/continnd/ECE487/vtr-verilog-to-routing/vtr_flow/benchmarks/blif/diffeq.blif", "r");
       std::tie(crit_path, area) = parse_results(res);
       std::cout << crit_path << " " << area << std::endl;
       pclose(res);
       */
    Architecture arch;
    arch.K = 62;
    make_arch_file(arch, "test_arch.xml");
    return 0;
}
