#include <iostream>

int main() {
    constexpr size_t mod = 20201227;
    size_t a = 11404017, b = 13768789;
    size_t current = 7;
    size_t res = b;
    while(current != a) {
        current = current*7 % mod;
        res = res*b & mod;
    }
    std::cout << res << '\n';
}
