#define BOOST_TEST_MODULE ArchitectureTest
#include <boost/test/unit_test.hpp>

#include "Architecture.h"

#include <vector>

BOOST_AUTO_TEST_CASE(architecture_ctor_test) {
    Architecture a1;
    BOOST_CHECK_EQUAL(a1.K, Architecture::UNSET);
    BOOST_CHECK_EQUAL(a1.I, Architecture::UNSET);
    BOOST_CHECK_EQUAL(a1.W, Architecture::UNSET);

    a1.K = 1;
    a1.I = 2;
    a1.W = 3;

    Architecture a2{a1};
    BOOST_CHECK_EQUAL(a2.K, 1);
    BOOST_CHECK_EQUAL(a2.I, 2);
    BOOST_CHECK_EQUAL(a2.W, 3);
}

BOOST_AUTO_TEST_CASE(architecture_mutate_test) {
    Architecture a1;
    a1.K = 4;
    a1.W = 50;
    a1.I = 10;

    a1.mutate(0);
    BOOST_CHECK_EQUAL(a1.K, 4);
    BOOST_CHECK_EQUAL(a1.W, 50);
    BOOST_CHECK_EQUAL(a1.I, 10);

    Architecture a2{a1};
    bool has_changed = false;
    // 100 mutation attempts should change the values
    for (unsigned i = 0; i < 100; i++) {
        if (a1 != a2) {
            has_changed = true;
            break;
        }
        a1.mutate(0.5);
    }
    BOOST_CHECK(has_changed);
}

BOOST_AUTO_TEST_CASE(architecture_random_test) {
    Architecture sample = Architecture::random();

    std::vector<Architecture> architectures{100};
    for (unsigned i = 0; i < architectures.size(); i++) {
        architectures[i] = std::move(Architecture::random());
    }
    unsigned cnt = 0;
    // Hopefully most of them are different
    for (auto&& arch : architectures) {
        if (arch != sample) {
            cnt++;
        }
    }
    BOOST_CHECK_GE(static_cast<float>(cnt) / architectures.size(), 0.9);
}
