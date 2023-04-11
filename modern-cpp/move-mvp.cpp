#include <iostream>
#include <utility>

class MyClass {
public:
    MyClass() {
        std::cout << "Default constructor" << std::endl;
        data_ = new int[10];
    }

    ~MyClass() {
        std::cout << "Destructor" << std::endl;
        delete[] data_;
    }

    // Move constructor
    MyClass(MyClass&& other) noexcept {
        std::cout << "Move constructor" << std::endl;
        data_ = other.data_;
        other.data_ = nullptr;
    }

    // Move assignment operator
    MyClass& operator=(MyClass&& other) noexcept {
        std::cout << "Move assignment operator" << std::endl;
        delete[] data_;
        data_ = other.data_;
        other.data_ = nullptr;
        return *this;
    }

private:
    int* data_;
};

int main() {
    MyClass a;
    MyClass b(std::move(a)); // move a to b
    MyClass c = std::move(b); // move b to c
    return 0;
}
