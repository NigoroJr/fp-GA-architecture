#include "GeneticAlgorithm.h"

std::random_device GeneticAlgorithm::rd;
std::mt19937_64 GeneticAlgorithm::gen{rd()};
std::uniform_real_distribution<float> GeneticAlgorithm::prob_gen{0, 1};

using results_t = GeneticAlgorithm::results_t;
using Comparator = GeneticAlgorithm::Comparator;
using Params = GeneticAlgorithm::Params;

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Params::Params()
    : Params(10, 1, 10, 0.05, 0.05, 0.01)
{ }

Params::Params(unsigned num_population,
               unsigned elites_preserve,
               unsigned num_selection,
               float mutation_occurrence_rate,
               float mutation_amount,
               float crossover_occurrence_rate)
    : num_population{num_population}
    , elites_preserve{elites_preserve}
    , num_selection{num_selection}
    , mutation_occurrence_rate{mutation_occurrence_rate}
    , mutation_amount{mutation_amount}
    , crossover_occurrence_rate{crossover_occurrence_rate}
{ }

// Copy constructor
Params::Params(const Params& other)
    : num_population{other.num_population}
    , elites_preserve{other.elites_preserve}
    , num_selection{other.num_selection}
    , mutation_occurrence_rate{other.mutation_occurrence_rate}
    , mutation_amount{other.mutation_amount}
    , crossover_occurrence_rate{other.crossover_occurrence_rate}
{ }

// Move constructor
Params::Params(Params&& other)
    : num_population{std::move(other.num_population)}
    , elites_preserve{std::move(other.elites_preserve)}
    , num_selection{std::move(other.num_selection)}
    , mutation_occurrence_rate{std::move(other.mutation_occurrence_rate)}
    , mutation_amount{std::move(other.mutation_amount)}
    , crossover_occurrence_rate{std::move(other.crossover_occurrence_rate)}
{ }

// Destructor
Params::~Params()
{ }

// Assignment operator
Params& Params::operator=(const Params& other) {
    num_population = other.num_population;
    elites_preserve = other.elites_preserve;
    num_selection = other.num_selection;
    mutation_occurrence_rate = other.mutation_occurrence_rate;
    mutation_amount = other.mutation_amount;
    crossover_occurrence_rate = other.crossover_occurrence_rate;
    return *this;
}

// Move assignment operator
Params& Params::operator=(Params&& other) {
    num_population = std::move(other.num_population);
    elites_preserve = std::move(other.elites_preserve);
    num_selection = std::move(other.num_selection);
    mutation_occurrence_rate = std::move(other.mutation_occurrence_rate);
    mutation_amount = std::move(other.mutation_amount);
    crossover_occurrence_rate = std::move(other.crossover_occurrence_rate);
    return *this;
}
/* }}} */

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
GeneticAlgorithm::GeneticAlgorithm()
    : params{}
    , benchmarks{}
    , architectures{}
    , vtr_path{}
    , selected{}
    , next_generation{}
    , weights{}
{ }

GeneticAlgorithm::GeneticAlgorithm(const Params& params, const std::string& vtr_path,
        const std::vector<Architecture::Benchmark>& benchmarks)
    : params{params}
    , benchmarks{benchmarks}
    , architectures{params.num_population}
    , vtr_path{vtr_path}
    , selected{}
    , next_generation{}
    , weights{}
{
    fill_random_population(architectures.begin(), architectures.end());
    for (Architecture& arch : architectures) {
        arch.bench = benchmarks;
    }

    // Make weights for roulette selection
    weights.resize(architectures.size());

    if (weights.empty()) {
        return;
    }

    // When weights.size() == 4
    // |------ 0 ------|--- 1 ---|- 2 -|-3->
    // 4               7         9     10
    weights[0] = weights.size();
    for (unsigned i = 1; i < weights.size(); i++) {
        weights[i] = weights[i - 1] + weights.size() - i;
    }
    biased_gen = std::uniform_real_distribution<float>{
        0, static_cast<float>(weights.back())
    };
}

// Copy constructor
GeneticAlgorithm::GeneticAlgorithm(const GeneticAlgorithm& other)
    : params{other.params}
    , benchmarks{other.benchmarks}
    , architectures{other.architectures}
    , vtr_path{other.vtr_path}
    , selected{other.selected}
    , next_generation{other.next_generation}
    , weights{other.weights}
{
    biased_gen = std::uniform_real_distribution<float>{
        0, static_cast<float>(weights.back())
    };
}

// Move constructor
GeneticAlgorithm::GeneticAlgorithm(GeneticAlgorithm&& other)
    : params{std::move(other.params)}
    , benchmarks{std::move(other.benchmarks)}
    , architectures{std::move(other.architectures)}
    , vtr_path{std::move(other.vtr_path)}
    , selected{std::move(other.selected)}
    , next_generation{std::move(other.next_generation)}
    , weights{std::move(other.weights)}
    , biased_gen{std::move(other.biased_gen)}
{ }

// Destructor
GeneticAlgorithm::~GeneticAlgorithm()
{ }

// Assignment operator
GeneticAlgorithm& GeneticAlgorithm::operator=(const GeneticAlgorithm& other) {
    params = other.params;
    benchmarks = other.benchmarks;
    architectures = other.architectures;
    next_generation = other.next_generation;
    vtr_path = other.vtr_path;
    selected = other.selected;
    weights = other.weights;
    biased_gen = std::uniform_real_distribution<float>{
        0, static_cast<float>(weights.back())
    };
    return *this;
}

// Move assignment operator
GeneticAlgorithm& GeneticAlgorithm::operator=(GeneticAlgorithm&& other) {
    params = std::move(other.params);
    benchmarks = std::move(other.benchmarks);
    architectures = std::move(other.architectures);
    next_generation = std::move(other.next_generation);
    vtr_path = std::move(other.vtr_path);
    selected = std::move(other.selected);
    weights = std::move(other.weights);
    biased_gen = std::move(other.biased_gen);
    return *this;
}
/* }}} */

void GeneticAlgorithm::run_generation() {
    if (!next_generation.empty()) {
        change_generation();
        next_generation.clear();
    }
    selected.clear();

    evaluate();

    // Throw away all failed architectures
    auto new_end = std::remove_if(architectures.begin(),
                                  architectures.end(),
                                  [] (const Architecture& a) {
                                  return !a.non_failed();
                                  });
    architectures.resize(std::distance(architectures.begin(), new_end));

    unsigned lim = std::min(params.elites_preserve,
                            static_cast<unsigned>(architectures.size()));

    // Use best of the first generation as reference point
    for (unsigned i = 0; i < get_best().bench.size(); i++) {
        // Save as reference values
        if (!Architecture::reference_results[i].is_populated) {
            Architecture::reference_results[i] = get_best().bench[i];
            Architecture::reference_results[i].is_populated = true;
        }
    }

    // Copy the elites
    sort_population();
    std::copy(architectures.begin(),
              architectures.begin() + lim,
              std::back_inserter(next_generation));

    // Prepare for the next generation (but not replace yet)
    select();
    crossover();
    mutate();
}

const Architecture& GeneticAlgorithm::get_best() const {
    return architectures.front();
}

const Architecture& GeneticAlgorithm::get_worst() const {
    return architectures.back();
}

const std::vector<Architecture>& GeneticAlgorithm::population() const {
    return architectures;
}

const Params& GeneticAlgorithm::parameters() const {
    return params;
}

void GeneticAlgorithm::evaluate() {
#pragma omp parallel for schedule(dynamic)
    for (unsigned i = 0; i < architectures.size(); i++) {
        if (!architectures[i].already_run()) {
            std::string file_name = architectures[i].make_arch_file();
            // Populate the Architecture::Benchmark for each architecture
            architectures[i].run_benchmarks(vtr_path);
            std::remove(file_name.c_str());
        }
    }
}

void GeneticAlgorithm::select() {
    // However many architectures that have successful results
    unsigned successes = std::count_if(architectures.begin(),
                                       architectures.end(),
                                       [] (const Architecture& a) {
                                       return a.non_failed();
                                       });
    unsigned lim = std::min(params.num_selection, successes);
    // Select what to crossever/mutate from
    unsigned cnt = 0;
    while (cnt < lim) {
        unsigned index;
        do {
            index = get_biased_index();
        } while (index >= architectures.size());

        const Architecture& arch = architectures[index];

        // Same architecture shouldn't be selected more than once
        if (arch.non_failed() && std::count(selected.begin(),
                                            selected.end(),
                                            arch) == 0) {
            selected.push_back(arch);
            cnt++;
        }
    }
}

void GeneticAlgorithm::crossover() {
    if (trigger(params.crossover_occurrence_rate)) {
        Architecture a, b;
        std::tie(a, b) = get_two_random(selected);
        Architecture child1{benchmarks};
        Architecture child2{benchmarks};

        std::tie(child1.K, child2.K) = crossover_helper(a.K, b.K);
        std::tie(child1.N, child2.N) = crossover_helper(a.N, b.N);
        std::tie(child1.W, child2.W) = crossover_helper(a.W, b.W);
        // Routing channel width must be even for unidirectional
        child1.W = child1.W % 2 == 0 ? child1.W : child1.W + 1;
        child2.W = child2.W % 2 == 0 ? child2.W : child2.W + 1;

        next_generation.push_back(std::move(child1));
        next_generation.push_back(std::move(child2));
    }
}

void GeneticAlgorithm::mutate() {
    for (const Architecture& arch : selected) {
        if (trigger(params.mutation_occurrence_rate)) {
            Architecture mutant{arch};
            mutant.mutate(params.mutation_amount);
            mutant.bench = benchmarks;

            next_generation.push_back(std::move(mutant));
        }
    }
}

void GeneticAlgorithm::change_generation() {
    const unsigned num_pop = params.num_population;
    if (next_generation.size() > num_pop) {
        // Throw away any excessive entities
        next_generation.resize(num_pop);
    }

    // If there's not enough population, randomly generate some
    unsigned lim = num_pop - next_generation.size();
    for (unsigned i = 0; i < lim; i++) {
        next_generation.push_back(Architecture::random(benchmarks));
    }

    architectures = next_generation;
}

/* Private methods */

template<typename ForwardIter>
void GeneticAlgorithm::fill_random_population(const ForwardIter& begin,
                                              const ForwardIter& end) {
    std::generate(begin, end, [this](){
                  return Architecture::random(benchmarks);
                  });
}

bool GeneticAlgorithm::trigger(const float probability) {
    return prob_gen(gen) <= probability;
}

template<typename T>
std::pair<T, T> GeneticAlgorithm::get_two_random(const std::vector<T>& vec) {
    unsigned a = gen() % vec.size();
    unsigned b;
    do {
        b = gen() % vec.size();
    } while (a == b);

    return std::make_pair(vec[a], vec[b]);
}

unsigned GeneticAlgorithm::get_biased_index(const unsigned offset) {
    float rnd_number = biased_gen(gen);
    for (unsigned i = 0; i < weights.size(); i++) {
        if (weights[i] > rnd_number) {
            return offset + i;
        }
    }

    return offset + weights.size() - 1;
}

template<typename T>
std::pair<T, T> GeneticAlgorithm::crossover_helper(const T& val1, const T& val2) {
    const unsigned res_size = 8 * sizeof(T);
    std::bitset<res_size> set1{static_cast<unsigned long>(val1)};
    std::bitset<res_size> set2{static_cast<unsigned long>(val2)};
    std::bitset<res_size> res;
    res = set1 ^ set2;

    T avg = (val1 + val2) / 2;

    return std::make_pair(static_cast<T>(res.to_ulong()), avg);
}

void GeneticAlgorithm::sort_population() {
    const auto comp = [](const Architecture& a, const Architecture& b) {
        auto a_avg = (a.vs_ref_crit_path() + a.vs_ref_area()) / 2;
        auto b_avg = (b.vs_ref_crit_path() + b.vs_ref_area()) / 2;
        // The smaller the ration compared to ref the better
        return a_avg < b_avg;
    };
    std::sort(architectures.begin(), architectures.end(), comp);
}
