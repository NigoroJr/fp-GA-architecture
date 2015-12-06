#include "GeneticAlgorithm.h"

#include <iostream>
#include <csignal>

bool keep_going = true;

void signal_handler(int sig) {
    std::cout << "Received signal " << sig << std::endl;
    keep_going = false;
}

int main(int argc, char const* argv[]) {
    GeneticAlgorithm::Params params{
        10, 2, 0.05, 0.05, 0.05
    };
    GeneticAlgorithm ga{params};

    std::signal(SIGINT, signal_handler);
    unsigned cnt = 0;
    // TODO: command line
    const unsigned interval = 50;
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
