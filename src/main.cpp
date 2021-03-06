#include "Architecture.h"
#include "GeneticAlgorithm.h"

#include "cxxopts.hpp"

#include <iostream>
#include <csignal>

bool keep_going = true;

void signal_handler(int sig) {
    std::cout << "Received signal " << sig << std::endl;
    std::cout << "Waiting for VPR processes to finish" << std::endl;
    keep_going = false;
}

int main(int argc, char* argv[]) {
    // Set default values
    unsigned interval = 50;
    unsigned gen_limit = 100;
    unsigned num_population = 10;
    unsigned elites_preserve = 2;
    unsigned num_selection = 5;
    float mutation_occurrence_rate = 0.05;
    float mutation_amount = 0.05;
    float crossover_occurrence_rate = 0.05;
    bool show_help = false;
    bool output_csv = false;

    std::string usage = " <path to vtr> <benchmark1> [benchmark2, benchmark3, ...]";
    cxxopts::Options options{argv[0], usage};
    options.add_options()
        ("p,population-size", "The number of population",
         cxxopts::value(num_population))
        ("i,interval", "Interval to print the intermediate results",
         cxxopts::value(interval))
        ("g,generations", "Number of generations",
         cxxopts::value(gen_limit))
        ("e,elites", "Number of elites to preserve",
         cxxopts::value(elites_preserve))
        ("s,selection", "Number of architectures to crossover/mutate from",
         cxxopts::value(num_selection))
        ("o,mutation-occurrence", "The probability of mutations to occur",
         cxxopts::value(mutation_occurrence_rate))
        ("a,mutation-ammount", "The variation in percentage of the mutation",
         cxxopts::value(mutation_amount))
        ("c,crossover-occurrence", "The probability of crossover to occur",
         cxxopts::value(crossover_occurrence_rate))
        ("csv", "Output in CSV format",
         cxxopts::value(output_csv))
        ("h,help", "Show this help",
         cxxopts::value(show_help));
    options.parse(argc, argv);

    if (show_help) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (argc < 3) {
        std::cerr << "Need at least one benchmark" << std::endl;
        std::cerr << options.help() << std::endl;
        return 1;
    }

    std::string vtr_path{argv[1]};
    if (vtr_path.back() != '/') {
        vtr_path += '/';
    }
    // Add benchmarks
    std::vector<Architecture::Benchmark> benchmarks;
    for (int i = 2; i < argc; i++) {
        benchmarks.emplace_back(argv[i]);
    }

    GeneticAlgorithm::Params params{
        num_population,
        elites_preserve,
        num_selection,
        mutation_occurrence_rate,
        mutation_amount,
        crossover_occurrence_rate
    };
    GeneticAlgorithm ga{params, vtr_path, benchmarks};

    // Output header
    if (output_csv) {
        std::cout << "generation," \
            "best_crit,best_area," \
            "worst_crit,worst_area" << std::endl;
    }

#ifdef _OPENMP
    if (!output_csv) {
#pragma omp parallel
#pragma omp master
    std::cout << "Running program using "
        << omp_get_num_threads() <<
        " threads" << std::endl;
    }
#endif

    std::signal(SIGINT, signal_handler);
    unsigned cnt = 0;
    while (keep_going) {
        ga.run_generation();

        if (cnt % interval == 0) {
            if (output_csv) {
                std::cout << cnt << ","
                    << ga.get_best().vs_ref_crit_path() << ","
                    << ga.get_best().vs_ref_area() << ","
                    << ga.get_worst().vs_ref_crit_path() << ","
                    << ga.get_worst().vs_ref_area() << std::endl;
            }
            else {
                std::cout << "Results from gen " << cnt << std::endl;
                std::cout << ga.get_best() << std::endl;
            }
        }

        if (cnt == gen_limit) {
            break;
        }

        cnt++;
    }

    if (output_csv) {
        std::cerr << ga.get_best() << std::endl;
    }

    return 0;
}
