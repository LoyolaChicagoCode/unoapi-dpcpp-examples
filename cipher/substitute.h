#ifndef SUBSTITUTE_H_
#define SUBSTITUTE_H_

#include <sycl/sycl.hpp>

template <class Container>
SYCL_EXTERNAL std::byte substitute(
    Container & byte_map,
    Container & input_message,
    const int & decimal_begin,
    const int & i
)
{
    return byte_map[std::to_integer<uint8_t>(input_message[i]) - decimal_begin];
}

#endif
