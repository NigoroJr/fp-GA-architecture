#include "Architecture.h"

const unsigned Architecture::UNSET = 0;
const std::pair<unsigned, unsigned> Architecture::K_RANGE{2, 50};
const std::pair<unsigned, unsigned> Architecture::I_RANGE{1, 100};
const std::pair<unsigned, unsigned> Architecture::W_RANGE{1, 5000};
std::random_device Architecture::rd;
std::mt19937_64 Architecture::gen{rd()};

using u_dist_t = std::uniform_int_distribution<unsigned>;
u_dist_t Architecture::k_rgen{K_RANGE.first, K_RANGE.second};
u_dist_t Architecture::w_rgen{W_RANGE.first, W_RANGE.second};
u_dist_t Architecture::i_rgen{I_RANGE.first, I_RANGE.second};

/* Static methods */

Architecture Architecture::random() {
    Architecture arch;
    arch.K = k_rgen(gen);
    arch.I = i_rgen(gen);
    arch.W = w_rgen(gen);

    return arch;
}

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

void Architecture::mutate(const float amount) {
    std::normal_distribution<float> k_dist(K, K * amount);
    std::normal_distribution<float> w_dist(W, W * amount);
    std::normal_distribution<float> i_dist(I, I * amount);
    K = static_cast<decltype(K)>(k_dist(gen));
    W = static_cast<decltype(W)>(w_dist(gen));
    I = static_cast<decltype(I)>(i_dist(gen));
}

bool Architecture::operator==(const Architecture& other) const {
    return K == other.K
        && I == other.I
        && W == other.W;
}

bool Architecture::operator!=(const Architecture& other) const {
    return K != other.K
        || I != other.I
        || W != other.W;
}

std::ostream& operator<<(std::ostream& os, const Architecture& a) {
    os << std::setw(30) << std::left << "W (channel width): " << a.W << std::endl;
    os << std::setw(30) << std::left << "K (num inputs per LUT): " << a.K << std::endl;
    os << std::setw(30) << std::left << "I (num of LUTs per cluster): " << a.I;

    return os;
}
