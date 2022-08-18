#include "trapezoid.h"

// {{UnoAPI:trapezoid-implementation:begin}}
double trapezoid(const double f1, const double f2, const double half_dx) {
  return (f1 + f2) * half_dx;
}
// {{UnoAPI:trapezoid-implementation:end}}
