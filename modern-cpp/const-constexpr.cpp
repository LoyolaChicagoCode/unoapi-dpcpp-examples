#include <iostream>
#include <cmath>

constexpr double pythagorean(double a, double b) {
    return std::sqrt(a * a + b * b);
}

int main() {
    const double a = 3.0;
    const double b = 4.0;
    constexpr double c = pythagorean(a, b);

    std::cout << "The hypotenuse is " << c << std::endl;

    return 0;
}
