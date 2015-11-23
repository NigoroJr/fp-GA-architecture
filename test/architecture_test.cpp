#define BOOST_TEST_MODULE ArchitectureTest
#include <boost/test/unit_test.hpp>

#include "Architecture.h"

BOOST_AUTO_TEST_CASE(architecture_ctor_test) {
    Architecture a1;
    BOOST_CHECK_EQUAL(a1.K, Architecture::UNSET);
    BOOST_CHECK_EQUAL(a1.I, Architecture::UNSET);
    BOOST_CHECK_EQUAL(a1.W, Architecture::UNSET);

    // TODO: Implement me!
}
