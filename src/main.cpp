#include "Architecture.h"
#include "GeneticAlgorithm.h"

#include "cxxopts.hpp"

#include <iostream>
#include <csignal>

bool keep_going = true;

void signal_handler(int sig) {
    std::cout << "Received signal " << sig << std::endl;
    keep_going = false;
}

int main(int argc, char* argv[]) {
    // Set default values
    unsigned interval = 50;
    unsigned num_population = 10;
    unsigned elites_preserve = 2;
    unsigned num_selection = 5;
    float mutation_occurrence_rate = 0.05;
    float mutation_amount = 0.05;
    float crossover_occurrence_rate = 0.05;
    bool show_help = false;

    std::string usage = " <path to vtr> <benchmark1> [benchmark2, benchmark3, ...]";
    cxxopts::Options options{argv[0], usage};
    options.add_options()
        ("p,population-size", "The number of population",
         cxxopts::value(num_population))
        ("i,interval", "Interval to print the intermediate results",
         cxxopts::value(interval))
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

    std::signal(SIGINT, signal_handler);
    unsigned cnt = 0;
    while (keep_going) {
        ga.run_generation();

        if (cnt % interval == 0) {
            std::cout << ga.get_best() << std::endl;
            // TODO: get mean, get worst
        }
        cnt++;
    }

    std::cout << "Results" << std::endl;
    std::cout << ga.get_best() << std::endl;

    return 0;
}
