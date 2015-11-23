#include "Architecture.h"

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Architecture::Architecture()
    : K{UNSET}
    , I{UNSET}
    , W{UNSET}
{ }

// Copy constructor
Architecture::Architecture(const Architecture& other)
    : K{other.K}
    , I{other.I}
    , W{other.W}
{ }

// Move constructor
Architecture::Architecture(Architecture&& other)
    : K{std::move(other.K)}
    , I{std::move(other.I)}
    , W{std::move(other.W)}
{ }

// Destructor
Architecture::~Architecture()
{ }

// Assignment operator
Architecture& Architecture::operator=(const Architecture& other) {
    K = other.K;
    I = other.I;
    W = other.W;
    return *this;
}

// Move assignment operator
Architecture& Architecture::operator=(Architecture&& other) {
    K = std::move(other.K);
    I = std::move(other.I);
    W = std::move(other.W);
    return *this;
}
/* }}} */

// TODO: Implement me!
