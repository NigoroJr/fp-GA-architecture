#ifndef GENETIC_ALGORITHM_H_
#define GENETIC_ALGORITHM_H_

#include "Architecture.h"

#include <algorithm>
#include <bitset>
#include <functional>
#include <numeric>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

class GeneticAlgorithm {
public:
    using results_t = std::tuple<unsigned, double, double>;
    using Comparator = std::function<bool(const results_t&, const results_t&)>;

    /**
     * Parameters used for the genetic algorithm.
     * See the definition of the default constructor of GeneticAlgorithm for
     * default values.
     */
    struct Params {
        /* Constructors, Destructor, and Assignment operators {{{ */
        // Default constructor
        Params();

        Params(unsigned num_population,
               unsigned elites_preserve,
               float mutation_occurrence_rate,
               float mutation_amount,
               float crossover_occurrence_rate);

        // Copy constructor
        Params(const Params& other);

        // Move constructor
        Params(Params&& other);

        // Destructor
        ~Params();

        // Assignment operator
        Params& operator=(const Params& other);

        // Move assignment operator
        Params& operator=(Params&& other);
        /* }}} */

        unsigned num_population;
        /* How many elites to preserve */
        unsigned elites_preserve;
        /* How often mutation happens */
        float mutation_occurrence_rate;
        /* How much change per mutation */
        float mutation_amount;
        /* How often crossover happens */
        float crossover_occurrence_rate;
    };

    // Comparators for sorting the results
    static const Comparator SPEED_COMP;
    static const Comparator AREA_COMP;

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    GeneticAlgorithm();

    /**
     * Creates a genetic algorithm runner with the given parameters.
     * The number of population must be greater than 1.
     */
    GeneticAlgorithm(const Params& params, const std::string& vtr_path,
        const std::vector<Architecture::Benchmark>& benchmarks = {});

    // Copy constructor
    GeneticAlgorithm(const GeneticAlgorithm& other);

    // Move constructor
    GeneticAlgorithm(GeneticAlgorithm&& other);

    // Destructor
    ~GeneticAlgorithm();

    // Assignment operator
    GeneticAlgorithm& operator=(const GeneticAlgorithm& other);

    // Move assignment operator
    GeneticAlgorithm& operator=(GeneticAlgorithm&& other);
    /* }}} */

    /**
     * Runs one generation of the algorithm.
     * An iteration of a generation involves evaluating the architectures
     * (using VPR), roulette (selecting the best or few of the best),
     * cross-over, and mutation.
     */
    void run_generation();

    /**
     * \return the current best architecture with its parameters populated.
     */
    const Architecture& get_best() const;

    /**
     * \return the current best architecture with its parameters populated.
     */
    const Architecture& get_worst() const;

    /**
     * \return the population.
     */
    const std::vector<Architecture>& population() const;

    /**
     * \return the parameters for the genetic algorithm.
     */
    const Params& parameters() const;

    /**
     * Evaluates and populates performance of the current population by
     * calling VPR.
     */
    void evaluate();

    /**
     * Performs a roulette selection on the population, adding the current
     * population to the next population in a random order, but biased in such
     * a way that the superior entities are added earlier to avoid being
     * eliminated when the generation change happens.
     */
    void select();

    /**
     * Performs a crossover at the rate indicated by the parameters.
     * If a crossover is performed, a new architecture is created with traits
     * from two randomly selected architectures. The new architecture is added
     * to the new population list.
     */
    void crossover();

    /**
     * Performs mutation with probability given by the parameters.
     * If a mutation is performed, a new architecture is created with a new
     * set of architectural parameters and added to the new population.
     */
    void mutate();

    /**
     * Replaces the current generation with the next generation.
     * The current generation needs to be sorted before the call to this
     * method in the order of most superior to least superior (i.e. best
     * performance first, worst performance last). This is important when the
     * next generation contains less entities than the current generation
     * (since that's when some entities from the current generation are used
     * again in the next generation).
     */
    void change_generation();

private:
    static std::random_device rd;
    static std::mt19937_64 gen;
    static std::uniform_real_distribution<float> prob_gen;

    Params params;
    std::vector<Architecture> architectures;
    std::string vtr_path;

    /**
     * The population for the next generation.
     * If the number is less than the population size defined in the params,
     * the lower-ranked entities from the previous generation should be
     * replaced.
     */
    std::vector<Architecture> next_generation;

    /**
     * Used for generating biased random indices for selection.
     * Has length of (architectures.size() - params.elites_preserve).
     */
    std::vector<unsigned> weights;

    // For the get_biased_index method
    std::uniform_real_distribution<float> biased_gen;

    /* Private methods */

    /**
     * Fills in the vector of architectures for the given range.
     */
    template<typename ForwardIter>
    void fill_random_population(const ForwardIter& begin,
                                const ForwardIter& end);

    /**
     * Whether or not to trigger an event, given a probability that it
     * happens.
     * An event could be something like mutation or crossover.
     *
     * \param[in] probability the probability that true is returned.
     *
     * \return true at given probability.
     */
    bool trigger(const float probability);

    /**
     * Randomly selects two distinct architectures.
     *
     * \return two distinct architectures.
     */
    std::pair<Architecture, Architecture> get_two_architectures();

    /**
     * Generates a biased index in the range [offset, architectures.size()).
     *
     * \param[in] offset the lower bound of the random index. Default: 0.
     */
    unsigned get_biased_index(const unsigned offset = 0);

    /**
     * Helper method for the crossover operation.
     * Given two values, does an XOR operation on the bit string and also
     * takes the average of the two. Thus, the parameters are assumed to be
     * addable and divisible by an integer.
     *
     * \param[in] val1 the first value.
     *
     * \param[in] val2 the second value.
     *
     * \return pair of new values.
     */
    template<typename T>
    std::pair<T, T> crossover_helper(const T& val1, const T& val2);
};

#endif /* end of include guard */
