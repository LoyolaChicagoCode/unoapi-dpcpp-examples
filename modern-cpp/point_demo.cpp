#include <iostream>
#include "point.h"

int main() {
    // Create a point object with x=1.0, y=2.0, z=3.0
    Point p1(1.0, 2.0, 3.0);

    // Copy the point object
    Point p2 = p1;

    // Move the point object
    Point p3 = std::move(p1);

    // Output the values of the point objects
    std::cout << "p1: (" << p1.x() << ", " << p1.y() << ", " << p1.z() << ")" << std::endl;
    std::cout << "p2: (" << p2.x() << ", " << p2.y() << ", " << p2.z() << ")" << std::endl;
    std::cout << "p3: (" << p3.x() << ", " << p3.y() << ", " << p3.z() << ")" << std::endl;

    // Update the values of the point objects
    p2 = Point(4.0, 5.0, 6.0);
    p3 = std::move(p2);

    // Output the updated values of the point objects
    std::cout << "p1: (" << p1.x() << ", " << p1.y() << ", " << p1.z() << ")" << std::endl;
    std::cout << "p2: (" << p2.x() << ", " << p2.y() << ", " << p2.z() << ")" << std::endl;
    std::cout << "p3: (" << p3.x() << ", " << p3.y() << ", " << p3.z() << ")" << std::endl;

    return 0;
}
