#include <iostream>

constexpr size_t mod = 20201227;

size_t cycles(size_t n) {
    size_t res = 0;
    size_t current = 1;
    while(current != n) {
        current *= 7;
        current %= mod;
        ++res;
    }
    return res;
}

size_t modexp(size_t n, size_t p) {
    size_t r = 1;
    while(p) {
        if(p % 2 == 1) {
            r = r*n % mod;
        }
        p /= 2;
        n = n*n % mod;
    }
    return r;
}

int main() {
    size_t a = 11404017, b = 13768789;
    std::cout << modexp(b,cycles(a)) << '\n';
}
