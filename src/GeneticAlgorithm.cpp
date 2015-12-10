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
    , architectures{}
    , vpr_path{}
    , selected{}
    , next_generation{}
    , weights{}
{ }

GeneticAlgorithm::GeneticAlgorithm(const Params& params, const std::string& vpr_path,
        const std::vector<Architecture::Benchmark>& benchmarks)
    : params{params}
    , architectures{params.num_population}
    , vpr_path{vpr_path}
    , selected{}
    , next_generation{}
{
    fill_random_population(architectures.begin(), architectures.end());
    for (Architecture& arch : architectures) {
        arch.bench = benchmarks;
    }

    // Make weights for roulette selection
    weights.resize(architectures.size() - params.elites_preserve - 1);

    if (weights.empty()) {
        return;
    }

    // When weights.size() == 3 (thus, has 4 indices to choose from)
    // |------ 0 ------|--- 1 ---|- 2 -|-3->
    // 0               3         5     6
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
    , architectures{other.architectures}
    , vpr_path{other.vpr_path}
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
    , architectures{std::move(other.architectures)}
    , vpr_path{std::move(other.vpr_path)}
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
    architectures = other.architectures;
    next_generation = other.next_generation;
    selected = other.selected;
    vpr_path = other.vpr_path;
    weights = other.weights;
    biased_gen = std::uniform_real_distribution<float>{
        0, static_cast<float>(weights.back())
    };
    return *this;
}

// Move assignment operator
GeneticAlgorithm& GeneticAlgorithm::operator=(GeneticAlgorithm&& other) {
    params = std::move(other.params);
    architectures = std::move(other.architectures);
    next_generation = std::move(other.next_generation);
    selected = std::move(other.selected);
    vpr_path = std::move(other.vpr_path);
    weights = std::move(other.weights);
    biased_gen = std::move(other.biased_gen);
    return *this;
}
/* }}} */

void GeneticAlgorithm::run_generation() {
    next_generation.clear();
    selected.clear();

    evaluate();
    select();
    // Note: architectures sorted after call to select
    crossover();
    mutate();
    change_generation();
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
#pragma omp parallel for
    for (unsigned i = 0; i < architectures.size(); i++) {
        std::string file_name = architectures[i].make_arch_file();
        // Populate the Architecture::Benchmark for each architecture
        architectures[i].run_benchmarks(vpr_path);
        std::remove(file_name.c_str());
    }
}

void GeneticAlgorithm::select() {
    sort_population();

    // Copy the elites
    std::copy(architectures.begin(),
              architectures.begin() + params.elites_preserve,
              std::back_inserter(next_generation));

    // Select what to crossever/mutate from
    for (unsigned i = 0; i < params.num_selection; i++) {
        selected.push_back(architectures[get_biased_index()]);
    }
}

void GeneticAlgorithm::crossover() {
    if (trigger(params.crossover_occurrence_rate)) {
        Architecture a, b;
        std::tie(a, b) = get_two_random(selected);
        Architecture child1, child2;
        std::tie(child1.K, child2.K) = crossover_helper(a.K, b.K);
        std::tie(child1.N, child2.N) = crossover_helper(a.N, b.N);
        std::tie(child1.W, child2.W) = crossover_helper(a.W, b.W);
        // Routing channel width must be even for unidirectional
        child1.W = child1.W % 2 == 0 ? child1.W : child1.W + 1;
        child2.W = child2.W % 2 == 0 ? child2.W : child2.W + 2;

        next_generation.push_back(std::move(child1));
        next_generation.push_back(std::move(child2));
    }
}

void GeneticAlgorithm::mutate() {
    for (const Architecture& arch : selected) {
        if (trigger(params.mutation_occurrence_rate)) {
            Architecture mutant{arch};
            mutant.mutate(params.mutation_amount);

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
    for (unsigned i = 0; i < num_pop - next_generation.size(); i++) {
        next_generation.push_back(Architecture::random());
    }

    architectures = next_generation;
}

/* Private methods */

template<typename ForwardIter>
void GeneticAlgorithm::fill_random_population(const ForwardIter& begin,
                                              const ForwardIter& end) {
    std::generate(begin, end, [](){ return Architecture::random(); });
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
        if (rnd_number > weights[i]) {
            return offset + i;
        }
    }

    return offset + weights.size();
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
