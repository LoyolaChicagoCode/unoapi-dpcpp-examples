class Point {
public:
    Point(double x = 0.0, double y = 0.0, double z = 0.0)
        : coords{x, y, z} {}

    // Copy constructor
    Point(const Point& other)
        : coords{other.coords} {}

    // Move constructor
    Point(Point&& other) noexcept
        : coords {
        std::move(other.coords)
    } {}

    // Copy assignment operator
    Point& operator=(const Point& other) {
        coords = other.coords;
        return *this;
    }

    // Move assignment operator
    Point& operator=(Point&& other) noexcept {
        coords = std::move(other.coords);
        return *this;
    }

    // Accessors
    double x() const {
        return coords.x;
    }
    double y() const {
        return coords.y;
    }
    double z() const {
        return coords.z;
    }

    // Scale the Point by a double factor
    void scale(double factor) {
        coords.x *= factor;
        coords.y *= factor;
        coords.z *= factor;
    }

    // Translate the Point by another Point
    void operator+=(const Point& other) {
        coords.x += other.coords.x;
        coords.y += other.coords.y;
        coords.z += other.coords.z;
    }

private:
    struct Coords {
        double x, y, z;
    };

    Coords coords;
};
