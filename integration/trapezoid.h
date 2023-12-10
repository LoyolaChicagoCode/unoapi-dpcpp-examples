#ifndef INTEGRATION_H_
#define INTEGRATION_H_

#include <sycl/sycl.hpp>

// {{UnoAPI:trapezoid-interface:begin}}
SYCL_EXTERNAL double single_trapezoid(double f1, double f2, double dx);
// {{UnoAPI:trapezoid-interface:end}}

#endif // INTEGRATION_H_
