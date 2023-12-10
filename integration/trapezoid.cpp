#include "trapezoid.h"
#include "f.h"

// {{UnoAPI:trapezoid-implementation:begin}}
double single_trapezoid(const double f1, const double f2, const double half_dx) {
  return (f1 + f2) * half_dx;
}
// {{UnoAPI:trapezoid-implementation:end}}

// {{UnoAPI:trapezoid-compute-outer:begin}}
// common function to compute a single outer trapezoid
// from as many inner trapezoids as the grain size
double outer_trapezoid(
    const int grain_size,
    const double x_pos,
    const double dx_inner,
    const double half_dx_inner
) {
    auto area{0.0};
    auto y_left{f(x_pos)};
    for (auto j{0UL}; j < grain_size; j++) {
        auto y_right{f(x_pos + (j + 1) * dx_inner)};
        area += single_trapezoid(y_left, y_right, half_dx_inner);
        y_left = y_right;
    }
    return area;
}
// {{UnoAPI:trapezoid-compute-outer:end}}
