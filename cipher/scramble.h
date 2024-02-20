#ifndef SCRAMBLE_H
#define SCRAMBLE_H

#include <sycl/sycl.hpp>

std::vector<char> scramble(
	const std::string& key,
	const int key_size,
	const int ascii_a,
	const int range
);

#endif
