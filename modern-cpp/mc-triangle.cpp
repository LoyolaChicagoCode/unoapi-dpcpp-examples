//
// Created by George K. Thiruvathukal on 3/19/23.
//

#include <iostream>
#include <random>
#include <cmath>

int main() {
    // define the triangle
    double x1 = 0, y1 = 0;
    double x2 = 1, y2 = 1;
    double x3 = 0, y3 = 1;

    // set up random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    // number of random points to generate
    int n = 1000000;

    // initialize the count of points inside the triangle
    int count = 0;

    // generate random points and check if they are inside the triangle
    for (int i = 0; i < n; ++i) {
        double x = dis(gen);
        double y = dis(gen);

        // calculate the cross products
        double cp1 = (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
        double cp2 = (x3 - x2) * (y - y2) - (y3 - y2) * (x - x2);
        double cp3 = (x1 - x3) * (y - y3) - (y1 - y3) * (x - x3);

        // if the cross products have the same sign, the point is inside the triangle
        if ((cp1 >= 0 && cp2 >= 0 && cp3 >= 0) || (cp1 <= 0 && cp2 <= 0 && cp3 <= 0)) {
            ++count;
        }
    }

    // calculate the area of the triangle using Monte Carlo method
    double area = count / static_cast<double>(n) * (x2 - x1) * (y3 - y1);

    // output the estimated area
    std::cout << "Estimated area: " << area << '\n';

    return 0;
}
