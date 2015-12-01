#ifndef ARCHITECTURE_H_
#define ARCHITECTURE_H_

#include <algorithm>

class Architecture {
public:
    static const unsigned UNSET = 0;

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

private:
};

// const unsigned Architecture::UNSET = 0;

#endif /* end of include guard */
