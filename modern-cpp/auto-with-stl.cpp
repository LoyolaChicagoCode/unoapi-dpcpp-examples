.. code-block:: cpp

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <fmt/core.h>

int main() {
    auto i{42};
    auto d1{1.23}, d2{4.56};
    auto f1{0.123f}, f2{6.789f};
    auto b1{true}, b2{false};
    auto c1{'c'}, c2{'d'};

    auto v1{std::vector<int>{1, 2, 3, 4, 5}};
    auto l1{std::list<double>{d1, d2, d1, d2, d1}};
    auto m1{std::map<float, bool>{{f1, b1}, {f2, b2}, {f1, b2}}};
    auto s1{std::set<char>{c1, c2, c1, c2}};
    auto str1{std::string{"Hello, world!"}};

    fmt::print("i = {}\n", i);
    fmt::print("d1 = {}\n", d1);
    fmt::print("d2 = {}\n", d2);
    fmt::print("f1 = {}\n", f1);
    fmt::print("f2 = {}\n", f2);
    fmt::print("b1 = {}\n", b1);
    fmt::print("b2 = {}\n", b2);
    fmt::print("c1 = '{}'\n", c1);
    fmt::print("c2 = '{}'\n", c2);
    fmt::print("v1 = {}\n", fmt::join(v1, ", "));
    fmt::print("l1 = {}\n", fmt::join(l1, ", "));
    fmt::print("m1 = {{{}, {}}}\n", m1.begin()->first, m1.begin()->second);
    fmt::print("s1 = {{{}}}\n", fmt::join(s1, ", "));
    fmt::print("str1 = \"{}\"\n", str1);

    return 0;
}


