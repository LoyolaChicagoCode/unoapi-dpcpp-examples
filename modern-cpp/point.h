#ifndef POINT_H
#define POINT_H

class Point {
public:
    Point(double x = 0.0, double y = 0.0, double z = 0.0)
        : x_{x}, y_{y}, z_{z} {}

    // Copy constructor
    Point(const Point& other)
        : x_{other.x_}, y_{other.y_}, z_{other.z_} {}

    // Move constructor
    Point(Point&& other) noexcept
        : x_{std::exchange(other.x_, 0.0)},
          y_{std::exchange(other.y_, 0.0)},
          z_{std::exchange(other.z_, 0.0)} {}

    // Copy assignment operator
    Point& operator=(const Point& other) {
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
        return *this;
    }

    // Move assignment operator
    Point& operator=(Point&& other) noexcept {
        x_ = std::exchange(other.x_, 0.0);
        y_ = std::exchange(other.y_, 0.0);
        z_ = std::exchange(other.z_, 0.0);
        return *this;
    }

    // Accessors
    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }

private:
    double x_, y_, z_;
};
#endif // POINT_H
 
