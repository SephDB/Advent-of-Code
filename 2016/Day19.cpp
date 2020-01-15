#include <iostream>

auto highest_bit(unsigned int v) {
    int r = 0;
    while(v >>= 1) {
        ++r;
    }
    return r;
}

auto log(unsigned int v, unsigned int p) {
    unsigned int r = 1;
    while(v /= p) {
        r *= p;
    }
    return r;
}

auto part1(unsigned int input) {
    /**
        Every power of 2 size, position 1 wins
        Every number between two powers, 1+2*(diff) wins
    */
    return 1 + (input - (1 << highest_bit(input)))*2;
}

auto part2(unsigned int input) {
    /**
        This groups every power of 3.
        (3^n) => (3^n)
        (3^n + 1) => 1
        (3^n + 2) => 2
        ...
        (2*3^n) => 3^n
        (2*3^n+1) => 3^n+2
        (2*3^n+2) => 3^n+4
        ...
        (3^(n+1)) => 3^(n+1)
    */
    auto l = log(input,3);
    if(l == input) return l;
    else if(l*2 + 1 > input) return input - l;
    else return l+2*(input-2*l);
}

int main() {
    unsigned int input = 3014603;
    std::cout << "Part 1: " << part1(input) << '\n';
    std::cout << "Part 2: " << part2(input) << '\n';
    /**
    * Brute-force implementation to help spot the pattern
    int last_pos = 0;
    for(unsigned int i = 5; i < 170; ++i) {
        int mid = i/2;
        if(mid > last_pos+1) {
            last_pos++;
        } else {
            last_pos = (last_pos + 2) % i;
        }

        std::cout << i << ' ' << mid << ' ' << last_pos << '\n';
    }
    */

}
