#define BOOST_TEST_MODULE GeneticAlgorithmTest
#include <boost/test/unit_test.hpp>

#include "GeneticAlgorithm.h"

BOOST_AUTO_TEST_CASE(ga_ctor_test) {
    GeneticAlgorithm ga1{GeneticAlgorithm::Params{2, 1, 0.1f, 0.2f, 0.3f}, ""};
    BOOST_CHECK_EQUAL(ga1.parameters().num_population, 2);
    BOOST_CHECK_EQUAL(ga1.parameters().elites_preserve, 1);
    BOOST_CHECK_EQUAL(ga1.parameters().mutation_occurrence_rate, 0.1f);
    BOOST_CHECK_EQUAL(ga1.parameters().mutation_amount, 0.2f);
    BOOST_CHECK_EQUAL(ga1.parameters().crossover_occurrence_rate, 0.3f);
}

BOOST_AUTO_TEST_CASE(ga_change_generation_test) {
    GeneticAlgorithm ga1{GeneticAlgorithm::Params{2, 0, 0.0, 0.0, 0.0}, ""};

    auto before = ga1.population();
    ga1.change_generation();
    auto after = ga1.population();
    // Organisms should be replaced (at high probability)
    bool different = before != after;
    BOOST_CHECK(different);
}
