#include <iostream>
#include <cstdint>

constexpr std::uint64_t N = (std::uint64_t(1) << 31) - 1;

template<std::uint64_t Mult, std::uint64_t div>
struct Generator {
    std::uint64_t current;
    auto part1_gen() {
        current = current*Mult % N;
        return current;
    }

    auto part2_gen() {
        do {
            part1_gen();
        } while(current % div != 0);
        return current;
    }
};

int main() {

    constexpr std::uint64_t mask = (1 << 16) - 1;

    Generator<16807,4> A1{699};
    auto A2 = A1;
    Generator<48271,8> B1{124};
    auto B2 = B1;
    int total = 0;
    int total2 = 0;
    for(int i = 0; i < 40'000'000; ++i) {
        total += (A1.part1_gen() & mask) == (B1.part1_gen() & mask);
    }
    for(int i = 0; i < 5'000'000; ++i) {
        total2 += (A2.part2_gen() & mask) == (B2.part2_gen() & mask);
    }
    std::cout << "Part 1: " << total << '\n';
    std::cout << "Part 2: " << total2 << '\n';
}
