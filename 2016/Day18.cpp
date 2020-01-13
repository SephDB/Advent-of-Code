#include <iostream>
#include <string_view>
#include <bitset>

constexpr std::string_view input = "^^^^......^...^..^....^^^.^^^.^.^^^^^^..^...^^...^^^.^^....^..^^^.^.^^...^.^...^^.^^^.^^^^.^^.^..^.^";

int main() {
    using Row = std::bitset<input.size()>;
    Row start(input.data(),input.npos,'.','^');

    auto count = [start](int num) {
        auto c = start;
        std::uint64_t total = c.count();
        for(int i = 1; i < num; ++i) {
            c = (c << 1) ^ (c >> 1);
            total += c.count();
        }
        return c.size()*num - total;
    };

    std::cout << "Part 1: " << count(40) << '\n';
    std::cout << "Part 2: " << count(400'000) << '\n';
}
