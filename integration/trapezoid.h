#ifndef INTEGRATION_H_
#define INTEGRATION_H_

#include <CL/sycl.hpp>

SYCL_EXTERNAL double trapezoid(double f1, double f2, double dx);

#endif // INTEGRATION_H_
