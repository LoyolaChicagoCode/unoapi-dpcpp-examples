#ifndef INTEGRATION_H_
#define INTEGRATION_H_

#include <sycl/sycl.hpp>

// {{UnoAPI:trapezoid-interface:begin}}
SYCL_EXTERNAL double single_trapezoid(double f1, double f2, double dx);
// {{UnoAPI:trapezoid-interface:end}}

SYCL_EXTERNAL double outer_trapezoid(
    const int grain_size,
    const double x_pos,
    const double dx_inner,
    const double half_dx_inner
);

#endif // INTEGRATION_H_
