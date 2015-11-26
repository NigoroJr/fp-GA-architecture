#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>
#include <utility>

#define CRIT_PATH "Final critical path:"
#define LOGIC_AREA "Total used logic block area:"
#define ROUTE_AREA "Total routing area:"
#define SCIENTIFIC_NOTATION "[-]?[0-9]+\\.[0-9]+([e][-+][0-9]+)"

/* Method that parses the output of vpr in the form of a string.
 * res: stream of vpr results.
 * output: a pair, with first representing critical path in ns and second
 *         representing area in minimum transistor units
 */
std::pair<double, double> parse_results(FILE* res) {
    double metrics[3];
    int num_found = 0;
    std::stringstream ss;
    std::string temp;
    char line[256];
    std::regex reg(LOGIC_AREA);
    while (num_found != 3) {
        if (!fgets(line, 256, res)) {
            return std::pair<double, double>(-1, -1);
        }
        if (std::regex_search(line, reg)) {
            ss << line;
            while (ss >> temp) {
                if (std::regex_match(temp, std::regex(SCIENTIFIC_NOTATION))) {
                    num = atof(temp.substr(0, temp.find('e') + 1).c_str());
                    int exp = atoi(temp.substr(temp.find('e') + 1).c_str());
                    while (exp != 0) {
                        num *= 10;
                        exp--;
                    }
                    break;
                } else if (temp[0] >= '0' && temp[0] <= '9') {
                    num = atof(temp.c_str());
                    break;
                }
            }
            metrics[num_found] = num;
            if (num_found == 0) {
                reg = ROUTE_AREA;
            } else if (num_found == 1) {
                reg = CRIT_PATH;
            }
            num_found++;
            ss.str(std::string());
        }
    }
    return std::pair<double, double>(metrics[2], metrics[0] + metrics[1]);
}

int main() {
    double crit_path, area;
    FILE* res = popen("/home/continnd/ECE487/vtr-verilog-to-routing/vpr/vpr /home/continnd/ECE487/vtr-verilog-to-routing/libarchfpga/arch/sample_arch.xml /home/continnd/ECE487/vtr-verilog-to-routing/vtr_flow/benchmarks/blif/diffeq.blif", "r");
    std::tie(crit_path, area) = parse_results(res);
    std::cout << crit_path << " " << area << std::endl;
    pclose(res);
    return 0;
}
