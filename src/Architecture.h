#ifndef ARCHITECTURE_H_
#define ARCHITECTURE_H_

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <random>
#include <type_traits>
#include <utility>

class Architecture {
public:
    static const unsigned UNSET;
    static const std::pair<unsigned, unsigned> K_RANGE;
    static const std::pair<unsigned, unsigned> I_RANGE;
    static const std::pair<unsigned, unsigned> W_RANGE;

    /**
     * \return a randomly generated architecture.
     */
    static Architecture random();

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    Architecture();

    // Copy constructor
    Architecture(const Architecture& other);

    // Move constructor
    Architecture(Architecture&& other);

    // Destructor
    ~Architecture();

    // Assignment operator
    Architecture& operator=(const Architecture& other);

    // Move assignment operator
    Architecture& operator=(Architecture&& other);
    /* }}} */

    /* Member variables */

    /* Number of inputs to LUT */
    unsigned K;

    /* Number of logic elements in a cluster */
    unsigned I;

    /* Channel Width */
    unsigned W;

    // TODO: switch block type?

    /* Member methods */

    /**
     * Changes the property of this architecture.
     *
     * \param[in] amount percentage of the change.
     */
    void mutate(const float amount);

    bool operator==(const Architecture& other) const;
    bool operator!=(const Architecture& other) const;

private:
    static std::random_device rd;
    static std::mt19937_64 gen;
    static std::uniform_int_distribution<unsigned> k_rgen;
    static std::uniform_int_distribution<unsigned> w_rgen;
    static std::uniform_int_distribution<unsigned> i_rgen;
};

/* Stream insertion operator */
std::ostream& operator<<(std::ostream& os, const Architecture& a);

#endif /* end of include guard */
