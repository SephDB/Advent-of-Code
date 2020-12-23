#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>

auto part1(std::string input) {
    auto next = [](char c) -> char {if (c == '1')return '9'; return c - 1; };
    for (int i = 0; i < 100; ++i) {
        char current = next(input[0]);
        auto n = input.begin();
        while ((n = std::ranges::find(input, current)) < input.begin() + 4) current = next(current);
        std::rotate(input.begin() + 1, input.begin() + 4, n + 1);
        std::rotate(input.begin(), input.begin() + 1, input.end());
    }
    auto one = std::ranges::find(input, '1');
    std::rotate(input.begin(), one, input.end());
    return input.substr(1);
}

auto part2(std::string input) {
    std::vector<int> next(1'000'000); //forward-linked list where next[i] is i's successor
    for (int i = 0; i < input.size() - 1; ++i) {
        next[input[i] - '1'] = input[i + 1] - '1';
    }
    next[input.back() - '1'] = input.size();
    for (int i = input.size(); i < 999'999; ++i) {
        next[i] = i + 1;
    }
    next.back() = input[0] - '1';

    int current = next.back();

    auto dec = [&next](int a) -> int {
        if (a == 0) return next.size() - 1;
        return a - 1;
    };

    for (int i = 0; i < 10'000'000; ++i) {
        std::array<int, 3> following{next[current]};
        for (int j = 1; j < 3; ++j) {
            following[j] = next[following[j - 1]];
        }
        auto next_round = dec(current);
        while (std::ranges::find(following, next_round) != following.end()) next_round = dec(next_round);

        //splice the list
        int after = next[next_round];
        next[current] = next[following[2]];
        next[following[2]] = after;
        next[next_round] = following[0];

        current = next[current];
    }

    return int64_t{ next[0] } * int64_t{ next[next[0]] };
}

int main() {
    std::cout << "Part 1: " << part1("872495136") << '\n';
    std::cout << "Part 2: " << part2("872495136") << '\n';
}
