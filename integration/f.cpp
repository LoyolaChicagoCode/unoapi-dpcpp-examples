#include "f.h"

#include <cstdlib>

// {{UnoAPI:f-implementation:begin}}
double f(const double x) {
    // return cos(x) * cos(x) + sin(x) * sin(x);
    return 3 * pow(x, 2);
}
// {{UnoAPI:f-implementation:end}}
