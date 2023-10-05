#include "f.h"

// {{UnoAPI:f-implementation:begin}}
double f(const double x) {
    auto result = 1.0;
    for (auto n = 0; n < 1000; n++) {
        result *= cos(x) * cos(x) + sin(x) * sin(x);
    }
    return result;
}
// {{UnoAPI:f-implementation:end}}
