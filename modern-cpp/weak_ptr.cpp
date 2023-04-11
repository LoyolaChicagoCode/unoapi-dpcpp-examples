#include <iostream>
#include <memory>

int main() {
    // Create a shared pointer to an integer
    std::shared_ptr<int> ptr = std::make_shared<int>(42);

    // Create a weak pointer to the integer
    std::weak_ptr<int> weak_ptr = ptr;

    // Check if the weak pointer is expired
    if (weak_ptr.expired()) {
        std::cout << "The weak pointer is expired." << std::endl;
    } else {
        std::cout << "The weak pointer is not expired." << std::endl;
    }

    // Reset the shared pointer
    ptr.reset();

    // Check if the weak pointer is expired again
    if (weak_ptr.expired()) {
        std::cout << "The weak pointer is expired." << std::endl;
    } else {
        std::cout << "The weak pointer is not expired." << std::endl;
    }

    // Create a new shared pointer to a different integer
    std::shared_ptr<int> new_ptr = std::make_shared<int>(99);

    // Reset the weak pointer to the new shared pointer
    weak_ptr = new_ptr;

    // Check if the weak pointer is expired
    if (weak_ptr.expired()) {
        std::cout << "The weak pointer is expired." << std::endl;
    } else {
        std::cout << "The weak pointer is not expired." << std::endl;
    }

    return 0;
}

