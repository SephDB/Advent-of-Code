#include <iostream>
#include <string_view>
#include <vector>
#include <ranges>
#include <numeric>
#include <algorithm>
#include <stack>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

template<bool Precedence = false>
auto parse(std::string_view line) {
    struct Element {
        uint64_t value;
        enum {Plus,Mul,Paren} type;
        void process(uint64_t c) {
            value = (type == Plus) ? value+c : value*c;
        }
    };
    std::vector<Element> stack = {{0,Element::Plus}};
    uint64_t current = 0;

    auto collapse_muls = [&stack]() {
        stack.back().type = Element::Mul;
        auto m = stack.back();
        stack.pop_back();
        while(not stack.empty() && stack.back().type == Element::Mul) {
            stack.back().process(m.value);
            m = stack.back();
            stack.pop_back();
        }
        stack.push_back(m);
    };

    for(auto c : line | std::ranges::views::filter([](auto c){return c != ' ';})) {
        switch(c) {
            case '+':
                stack.back().type = Element::Plus;
                break;
            case '*':
                if constexpr (Precedence) {
                    collapse_muls();
                    stack.push_back({0,Element::Plus});
                } else {
                    stack.back().type = Element::Mul;
                }
                break;
            case '(':
                stack.push_back({0,Element::Paren});
                stack.push_back({0,Element::Plus});
                break;
            case ')':
                if(stack.size() > 2) {
                    if constexpr(Precedence)
                        collapse_muls();
                    auto val = stack.back().value;
                    stack.pop_back();
                    stack.pop_back();
                    stack.back().process(val);
                } else {
                    std::cout << "PANIC" << '\n';
                }
                break;
            default:
                stack.back().process(c-'0');
                break;
        }
    }
    if constexpr(Precedence)
        collapse_muls();
    if(stack.size() != 1) std::cout << "Panic " << line << '\n';
    return stack.back().value;
}

auto solution(std::string_view input) {
    uint64_t total1 = 0, total2 = 0;
    split(input,'\n',[&](std::string_view line) {
        total1 += parse<false>(line);
        total2 += parse<true>(line);
    });
    std::cout << "Part 1: " << total1 << '\n';
    std::cout << "Part 2: " << total2 << '\n';
}

std::string_view input = R"(3 + 8
7 * (7 * 6 + 7 * 5 + (9 + 2 * 2 * 4)) + 7 * 3 * 5
3 * 4 + 2 + 7 + (4 * (9 + 9 + 2 + 3 * 8) + 9 + (6 + 4) + 7) + 3
(7 * 9 + 2 + (5 + 4)) + 5 + 9
4 + 5 * 5 + 8 + (3 * (7 * 7 + 5) * 7) * 5
2 + 8 * (5 + 4 * (8 * 3 * 6 * 7 * 8 * 8) + 8 * (7 + 9 + 8 * 9 * 4) * (3 + 9 * 5 + 2 + 5 * 4)) * 4
(8 * (3 + 3 * 9) * 3 * 9 * 9) + 3
(8 * 2 * 4 + 5) * 2 + 2 + (3 + 4 * 2 * 3 + (4 + 7 + 8 * 7 * 5) * (9 + 9 * 8))
(7 + 2 + (2 * 3)) * ((8 + 3 * 4 + 3 + 9 + 4) + 5) + 8 * 7 + 5 * 5
7 + (3 + 6 + 8) + (5 * 9 + 7 + 2 + (2 * 9 + 8) + 2) * 9
8 + 4 * 4 * 6 + 9 + ((5 + 8) + 7)
(6 + (8 + 6 * 8 + 7 + 3 * 3) + 5 + 7 * 6 + 8) * 2 * 2 * 5 * 2 * 4
(4 + (7 * 4 + 7 * 3 * 9 + 2) * 6 * 2 + 6) * (8 + 5 * 6 * (3 + 3 * 8 + 3) * 9) + 9
7 + 9 * ((5 + 8 * 5 * 6) * 2 * 5) + 9 + 9 * (6 * 8 * 4 + 3)
4 + (8 * 7 * 7 + 9) + (7 * 9 + 4) + 5 * 3 + 6
7 + (3 + 6) * (5 * (5 * 7 + 5 * 6 + 3 + 3) + (3 * 4) * 8) * 2 * 4
2 + (6 * 7 * 8 * 6 + (7 + 4 * 3 + 3 * 2 * 9) + 8) * (3 * 4 * (2 * 9 + 9) * 3) + (8 + (4 + 9 * 5 * 2 + 2) + 3 + (9 * 6) + 5 + 9) * 7 + ((2 * 3) + 3 * 8 * 8 * 6 + 5)
9 * 4 + ((5 * 2 + 5 * 5 * 6) * 5 + 9 + 8 * 9)
3 + ((6 * 3 * 9 * 4 + 2) + 6 * 8)
(3 + 4 + 7 + 3) * (6 + 8 * 7 + 8 + (6 + 6 + 8 * 7)) * 5 * 3
2 + 8 + (9 + 4)
((5 * 9 * 5) + 7 * 5) + 3 * 8
((7 + 9 + 5 + 4) * 9) + ((2 * 2 + 5 * 2 * 7) * (6 + 6 * 7 * 2 + 3) * 8 + 5 + 2 * 6) + 7
2 + (9 * 9 + 5 + 6) * 2 * 9 + 3
(5 * 4 * (7 * 9) + 7) * 8 * 5 * 4 + 6
5 + (7 * 4 * 2) * (8 + 7 + 2 * 2 * 7 * 2)
8 * ((5 + 7 + 2 + 9 * 8 * 2) + 9 * 6 * 9 + 7) + 8 * (3 + 5)
3 * 3 * 2 + 9 + (8 * 2 * 5 * 2) * 6
3 + 5 + 5 + 4
2 * 7 + (7 + 2 + 4 * 7 + (9 * 5 + 7 * 6 + 9 + 9) + 2) * 9 + 6 * 6
(2 * 6 + 8) * 3
4 + (7 + (7 * 7) * 3 + (4 * 5 + 4 + 6 + 3) + 5 * 8)
((8 * 9) + 3 * 8 * 7) + ((2 * 4 + 8 * 2 + 8 + 5) + 9 + 4) + 4 + 9 * (7 + 9 * (5 + 7) + 7 * 5 * 8) + 8
((2 + 6 + 9) + 5 * 4) * 6
9 * 5 * (2 * 6) + 6 + (9 * 2 * 8)
3 * 3 * (3 * 9) + 7
9 * ((2 + 7 * 7) * 3 + 8 * 3 + (6 * 2 + 5 + 3 * 3 * 9)) + 3 * 5
(6 + 6 * 6 * 6 * 3 * 3) * 7 * 4 * 9 * 7
2 * ((5 * 4 * 7 + 3) + 6 * (5 * 5 * 2 + 8 * 9 + 5) + 7) + (7 * 8 + 7 + 7 * 9 * 6) * 7 * 8
(5 * 8 * 9 * (6 + 7) + (5 * 3 * 3 * 6 + 4 + 6)) + 2 * 7 + 9 * 8
2 + (7 + 4 + 8) + (7 + 6 + (2 + 9) * 3)
((3 * 2 + 8 + 2) + 7 * 8 + 6) + 9 * 7
6 + (9 + 2) * 8 + ((2 + 6) * 7 * 2) * (9 * 6) * 3
3 * ((5 + 5 * 3 + 8 + 5 * 2) + (5 * 8 * 4 * 5 + 4 + 8) + 6 + 8 + 5)
((4 + 6 + 8 + 8) * 4 * 9 + (6 * 2 + 4 * 5 + 5) + (9 + 4 + 7 + 4 + 5) + 8) * 7 * 5
8 + 9 + (5 * 3 * 9 + 5 * (4 * 7 * 6 + 5 + 9)) * 5 * (9 + 8 + 7 + 4) * 6
4 * 6 + 9 * (9 + 5 + 7 * 4) + 7
(5 * 6) + (2 + 9 + (6 * 2 * 5) + (2 + 8 + 2) + 9) + 7 * 6 + 5
5 * 4 * 7 * (5 * 7 + 2) + 5 + 9
3 * (7 + 2 + 8 + 7 + 5 + 3)
2 * (2 * 5 + 7 + 6 * (8 * 8 * 5 * 4 * 8 + 8)) + 3
(7 * 9 * 7) * 2 * 9 + (7 + 3 * 6 + 3 * 3) * 8 + 9
5 + 8 * 9 * 7 + (4 + 9 * 5 + 2 * 3) + 4
7 + 3 * (3 + 8 + 5 * 6) * (8 + 6 + 8) * (5 + 2 + 8 + 5 + (4 * 6 * 7 * 5 + 4))
((6 * 6 * 7 * 2 + 7) + (3 * 7 + 9 + 7 + 5 + 5) + 2 * 8 * 3 + (3 * 9 + 7 + 7 * 7)) * ((6 + 8) * 3 + 2 * 4) + 3 + (6 * 4 + 7 + 2 * 9 * 2)
(9 * 7 * 6 + 2) + 9 * 6 + 4 + (5 + 3 * 8)
(4 + 2 + 3 + 9) + 6 * 4 + 2 + 9
9 + ((5 * 8 * 8 * 2 * 5) + 8 + 7 * 4) + ((6 + 8 + 4 + 5 * 3 * 5) + 5 + 4 * 6 * 3)
9 + (6 + 3) + 3 * ((2 + 3) + 8 + 4 + 8 + (2 + 9)) * 4 * 2
7 + 8 + (2 + 9 * 9 + 3 * 9) + 6
(8 + (8 * 6 + 9 * 4 + 2) + 4 + 9) + 7
6 + (2 + (4 * 3 * 5 + 5) + 4) + 6 * 6
7 * (3 * 9 * 3) * (7 + (6 + 8 + 4))
8 * ((4 * 3 + 8 * 6) + 7 + 6 * 3) + (4 + 5 * (8 * 2 * 6 * 9) + 7)
5 * ((8 * 4 + 2 * 9 * 8 + 2) * (3 + 5 + 6 * 9 + 5) * (5 + 9 * 9 + 2 * 8) * 2)
(3 * 4 * 5 * 3) * ((8 + 6) + 2 * (6 * 9 + 9 * 9 + 2 * 7) + 6 * 2) * 5 + 4 + 8 + 2
8 * (5 + 2 * 2) * 2
(3 * 8) + 9 * (6 * 2 + (6 + 3 * 6 + 7 + 8 + 4) + 3 * 9)
3 + 6 * (3 + 6 * 8 * 4 * 5 + 2) + (3 + 9 + 2 * 7 * (4 * 3 + 3 + 8) * (8 + 3))
4 * (4 + 3 + 7 + 9 * 6) * 2 + 4 + 8 + (2 + 2)
((5 + 7 * 2 * 9) + (2 + 3 * 6 + 2 * 5 * 8) * 8 * (8 + 8 * 6) + 5 * (7 * 9)) + 7 * 9 * (6 + 8)
5 * 2 * 4 * ((3 * 4) * 6 + 3 + 5 * 5 * 9) + 7 + 8
6 + 8 + (9 * 2 * 9 * 9 * 4 + 3) + 9 + (4 + 2)
9 + 5 * 7 + 2 * 4 + (4 * 8 + 5)
3 + 5 * 8 + 5 * 5
(6 + 6 + 9 * 7 + 4 * 5) + 2 + 4 * 9
((3 + 2) + 2 * 4 * 4 * 7) * (9 + 8 * 6 * 3 + 4 + 6) + 3 + 9 * (7 + 5 + 5) + 3
7 * (8 + 6 * 9 + 8 * 5)
9 * 4 + 8 + 2 * 3
9 + 2
3 + 2 + 9 + 8 + (2 * 9)
4 * 9 * (3 * (2 * 7 + 6) + 9 + 7 * 5) + (9 * 3 * 5) * (6 * 4 * 9 * (5 * 7 + 7 + 2 * 7) * (5 * 5 + 8 + 8) + 8)
5 * 7 * 4 * (3 * 8 + 6 * 2 * (8 * 2 + 4) * 4) + 7
6 * 3 * 2 + (3 * 8) * 4 + 7
6 + ((4 * 4) * 3 + 8 + 3 * (3 * 6 + 9 * 3 * 4) + (7 * 8 + 2 + 7 * 4)) + 7 + 2
8 * 9 * (2 * 5 + 6 * 7 * 9)
9 + (5 + (8 + 2 + 2 * 3)) + 7 * 2
(5 + (3 * 8 + 4 * 3) + (3 + 6 * 2 * 9) + (8 + 7 * 7 * 5 + 2 + 9) + 6) * 5 + 7 * ((6 + 2 + 9) * 4 + 4 * 7 * 3) * 8
4 * (9 * 4 * 9 * 4 + 4) * 6 * 7
3 + 2 * (6 * 5 * 5 * 7 * (7 * 8 + 4)) * 2 * 7 + 4
9 * (4 + 4 + 8 + 5 * (7 * 5 + 2 + 8 + 3 + 4) * (2 + 9 * 3))
9 + ((9 * 7 * 9 + 4) * 3 + 3) * 2 + 2 * (7 + 6 + 2 * (2 + 8 + 7 + 6) * 5)
8 * (7 + 2 + 4 * 9 * 8 * 4) * 8 + 7 * 9 + 2
(9 + 4 + (9 * 5 * 2 + 7 * 9 * 9) + 3 + 5 + 6) + (8 + 5 * 8 * 5 + 8 + 2) + 8 * 2
((7 + 8 * 6) * 7) + (4 + 3 + (8 + 2 * 9 + 8 + 3 * 3) * (6 + 5 * 4 * 8 + 8)) * 5 * 6 + 5 * 9
((8 * 6 + 6) * (6 * 7 * 9) * 3 + (5 * 4 * 9 + 4 + 7 * 2) * 4) + (8 * (8 + 4 + 8 + 2)) + 4
(2 + 2 * 4 * 6 + 6) + 4
((2 * 4 + 9 * 2 * 3 * 4) * 8 * 5 + 8) * 6 * (5 * (6 * 4 * 9 * 3) + (2 * 2 * 2))
(4 + 9 * 2 + 9 + 2 * 7) * (5 * (9 + 7 + 3) + (6 * 4) * (4 + 6) + (3 + 5 + 7))
2 + (9 + 5 + 6) + (4 * 5 * 9 + 5 + 2 * 2) + 3
((4 + 6 * 2) + 4) + 3 + (3 + 7 * 6 + 7 * 6) + 9 + (3 + 4) * 7
6 * 4 + ((6 + 4 + 2 * 6 * 5) + 4 + 4 + 8 + (4 + 2 * 3 + 9) + 9) + 3
2 + (4 + 5 + 6 * 2 * (5 * 5 * 9 * 9)) + ((8 + 3 + 9 * 5) * 3 * 2 * 4 + 9 * 3) + (9 * 8 + (7 * 4 + 6 * 9 + 8)) * 7
5 + (7 + 3 + 6) * 8 * 5 + 9
8 + (5 * (3 * 7 * 9 + 4 + 7)) * 2 + 8 * ((9 + 2 * 9 * 6 + 7 * 7) * 7 * 3 + (4 + 5 + 4 * 7) * (9 + 2) + 6)
5 + (6 + 9 + (2 * 6 * 3 * 2) * 4 + 5 * 3)
(3 * (3 + 7) * 6) + 5 * 6 * (3 * 2 * 3 * 7 + (3 + 4 + 4 * 7))
9 + (2 + 5) * 6 * (5 + 6 + 9 * 6 * 7)
6 * 5 * ((3 + 3 * 6 * 7 * 5 * 6) + (3 + 5 * 4 * 5) * 8 * 3 * 9 * 8)
(6 + 7 * (5 + 6)) * 6 * 9
2 + (3 + 6 + 5 * 3 + 3) + ((6 * 7 + 5 * 8 * 8) + 7 * 8 + 3 * 2) * 9 + 5
((3 * 9 + 4) * 5 * 9 * 7) * 8
(4 + 6 * 6 + 8 + 4) + 8 * 9 + 8 + 3
2 * (8 + (3 * 9 + 2)) * 7
2 + (4 + 6 + 4 * 9)
5 + 9 + (4 * 8 * (3 * 7 * 9 * 2 * 6) + 7)
(3 + 7 * 6 * 9 + (8 * 6 + 8)) + 3 + 3 * 9 + ((9 * 4) + 5)
(6 * (9 * 5 + 9) + 4 * 2 + 5) + 5 * ((2 + 9 * 7) * 4 + (6 * 5 + 2 * 6 + 8 + 7) + 7) + (5 * 6 + (2 * 5) + 7)
7 + 4 + 2 * (6 * (4 * 4 * 9 * 8 + 5) * 7 + 3)
8 + (7 + 8 * 3 * (6 * 8 + 5) * (2 * 9 * 4 * 6 + 7 * 7) + 3)
(7 + 9 * (3 * 6 + 5 + 2) + 5 * 2 + 2) * 6 * (3 + (3 * 8 * 6) * 2 * (4 + 5 * 6) * 5 + 5) + 8
5 + 5 * 7 * (4 * 2 * 5 + 5) + 4 * 5
3 + 4 + 3 + 5 * (2 * (7 + 9) + 2 * 5)
(2 * 7 * 7) * 4 + 4
((4 * 5 + 6) + 7) + 4 * 3 + 5 * (9 * 7 + 9 + 8 + 6) * 6
7 * 6 + 4 + 5 + (5 * 5 * 2) + (5 * 2 + 5 + 6 + (3 + 3 * 4 * 7 * 9))
5 * (2 * (6 + 6 + 4 + 4 + 9 + 5) * 4 + 9 * 8 + 2) * 4 * 4
2 * 6 * 3 + (7 + (9 * 6 * 5 + 6 * 7) + 2 * 2) * 9 + 5
(9 * 6) + (2 * (8 * 4 + 3 + 8 + 8)) * (2 + 4 * 5 + 3 + 4) + (5 * 7 * 7 + 7 * 7)
7 * 2 * ((6 * 5) + (3 + 9 * 3) * (4 + 3 + 9 + 8 + 3))
8 * ((4 * 4) * (6 + 6 + 3 * 5 * 3) * 4 + 3 * 4) + 5 * (6 * 4 * (5 + 7 + 7 * 5 + 4) * 5) + 7 + 5
((6 + 2 * 9 * 2 * 5 * 7) * 7 * 4) * ((4 + 4 + 4) * 8 + 4 * 8) + (6 + 8)
2 * 7 + 3 + 4
(9 * (4 + 7 * 2 * 6) * (5 * 5 + 2 + 2 * 5)) + 8
4 + ((5 + 3) * 4 * 9 + (3 + 5 * 6) * (3 + 8 * 2 + 3)) * 6 + 9
7 * (4 + 6 + 2) + 7 * 2 * 7 * 6
6 * ((4 * 9 * 5 * 4) * 7) + 4
9 * (6 + 4 + 9 * 9) * 4 * 7 + 2 * 9
(9 * (4 * 7)) * 8 * ((7 * 8 * 7 * 3 + 8) + 2 * 2 * (5 + 6) + (2 * 6 * 5 * 5))
8 + 2 + 2 * 6 * 2 + 8
(5 + 3 + 3 * 5 + 4) * (3 * 6 * 7 * (2 + 4 + 3 + 5 * 4) + 8 + 4)
(6 + 6 * 4 + 7 + 4) + 9 * 7 + (5 + 7) * 5 * 4
((3 * 4 * 8 + 8 * 8 + 9) * 2 + 8) * 5 * 8 + 3
2 * 2
(9 + (3 + 2 * 3 + 4) * 4 + 8) * 8 + (7 + 6 * 3 * 7 * 4) * 9 + 4 * 8
7 + 3 + 6 + 6 + 5 * 2
8 + 2 * 3 * 3
7 * 6 * (5 + 2 * 6 * 7 * 6 * 2) + 6 + ((7 * 5 + 9 + 4) * 7 * 6 + 9 * 8 * 2) * 5
(3 * 3) * 2 + 4 * 2 + 3
4 * (5 * 6 * 2 * 4 * (8 + 3 * 5) * 9) + 5
(8 + 5 + 7 + (7 + 6 * 7)) * 7 + 2
9 * 2 + 8 + (3 * 9 + 6 * (8 + 6 + 8 * 3 * 8)) + ((5 * 5 * 5) * 2)
2 * (2 + (8 + 7 * 7) * 9 * 7 * 3 * 9) + 7 + 4 * 3 + 7
8 * 4 + (2 + 3 + 4 * 8)
((6 + 2 * 6 + 8) + 4 + 4 * 4 + 8 * 7) + 5 * (4 * 4 + 7 + 3 + 2)
7 * (2 + 9 * 8 * 2 + 5 + (8 + 9 + 6))
2 + 8 + (5 + 8 + 2 * 6) * 4 + 2
((9 + 6 * 8 * 4 + 2) * 2 + 4 * 6 * 5) * 5 + 6 * 3 + 4 * 6
(9 + (6 * 7 + 2 * 7 * 7) * 6) * 6 * 6 * 4
(4 + 2 + 3 + 3 * (8 * 5)) + 6 * 2 * (8 + 7 + 3 + 9 * (2 * 4 * 8 * 6 + 8 * 5))
7 * 4 + (9 + 6 + (9 * 4 * 7 * 8 * 3 + 4)) + (2 + 8 + 7 + 5 + 7) * 2 + (8 * 6 + 5 * 6 * 9 * 8)
(9 + (3 * 6) * (5 + 2)) + 5 * 7
9 + (8 * 4) * 2 * 6
8 * 3 * (9 * (7 * 6 + 7) * (7 * 8)) + 3
8 * 7 * 6 * (7 * 7 + 3 + 2 * 6) + (8 + 2 + (7 + 9)) + (5 + 5)
(2 * 5 * 7 * 8) + 6 * (5 * 8 + 9 * 8) * 7
(2 + 8 * 6 + (7 + 9 * 8 + 2)) + 6 + (3 + 4 + 4 * 2 * 4) + 8 * 8
7 + ((4 + 7 + 9 * 9 + 7) + (2 * 5 * 8 * 7 * 7 * 8) * 9 + 6 + 9 * 5) * 4 * 4 * (7 + (3 + 7 + 3 + 4) * 6 * (5 + 4))
3 + ((6 * 6 + 8 + 6 * 2) + 6 + 3 + 2 * 5 * 7)
9 + 8 + 9 * (2 + 7 * 5) * 4
9 + (9 * (8 * 7 * 2 * 6 + 2 * 9)) + 7 * 3 * 2 * 5
8 * 9 * (2 + 2 * 6 + 6) + 5
(6 * 5 + 8 + 8) + 7 * ((4 * 6 * 3 * 8 * 3 * 5) + (5 + 6 * 8 * 3) + 3) * 7 + 6
7 + 2 + ((6 * 7) * 8 + 7) + 6 + 3 * 9
6 * (2 * 2 + 8 * 4 + 3 * 9) + (6 * 2 * 4 + (2 * 5 + 4 * 8) + 9) * 7
7 * 5 * (6 * 2 + (4 * 5 + 2) + (2 * 5 + 5 + 5 * 7 * 6) * (6 + 5 * 8 * 2))
9 * ((7 + 7 + 2) + (5 * 5) + (2 + 3 + 2 * 8 * 7) * 7) * 3
(7 + 6) + 3 * 3 * 4 * (2 + 7 * 4 * 4 * (5 * 3 + 8 + 6 * 8))
4 + ((6 * 3 + 8 * 6) * 3 + (8 * 4 + 9 * 6 + 4 + 3) * 8 * 2) + 4 * 8 * 2 * 9
(9 * (3 * 5 + 8 + 3 + 8) * 3 + 9) + 5 + 5 + 8 + 2
(6 + 3 * 7 + 9) * 9 * 6
(5 * 2 + 2 + 2 * 7) + 7 * 7 + (6 + 7 + 9 + 9 + 5) * 7 * 5
5 + 8 * (2 * 2 + (6 * 3 * 6 * 7 + 8) * 4 + 6)
4 + (8 + 2 + 9 + (2 + 4) + 4) * 5
7 + 8 * ((9 + 8 * 6) * 8 * (4 * 2 + 3 * 7 * 9 + 6) * 2)
(8 + (2 * 8 + 6 * 9 * 6 * 4) * (5 + 5) + 6) + 7 * 8 * 8 + 5 * ((5 * 3 * 7 * 2) * 5 + 4 + (2 * 6 + 6 + 2 + 2) * 4 * 2)
6 + (9 + 8 + 5 + 7) + 3 * (7 * 6 * 5 * (2 * 8 * 7 * 5) + 3 * 5) + 3
9 * (6 * (8 + 4 * 8) + 4)
(9 + (5 * 7 * 7 * 8 + 9) + 5 + 5 * (8 * 7 + 2 + 9 + 9) * (9 + 6)) * 3 * ((8 + 3) + 8 * 4 + 2 * (8 + 6 + 4) * 4) + ((8 + 3 + 8 + 7) + 7 * 8 * (7 * 7) + 5) + 9
((3 * 4 * 9) + 2 + 8 + 5 + 2 + 3) * (6 * 8 * 8 + 6 + (4 + 6 + 8)) * 2 * 6 + 5 + 9
2 + 2 + (9 * (2 + 5 * 8 + 2 * 5 * 5) * 3)
(6 * (2 * 8 + 6 * 4 * 9 + 9) + 8) + 3 + (3 + 2 + 2 + 8 * (3 + 4 + 8 * 9 + 2 + 5)) + 3 * 7 + 5
(2 + 5) * (2 + 5) * 3 * (2 + 7 + 8) + 2 + 8
3 + 8 + (7 * 2 + (9 + 2 + 8 * 4) + 8 * 8) + 4 * (5 + (6 * 5 * 8) + 7 * 8 + 9 + 8)
9 + 4 * (4 + (5 * 3 + 7 + 8 + 7 * 4) * 6 + 8 + 9 + 9) * ((4 * 6 + 8) * 9 * 9 * 6)
8 + 7 + ((3 * 6 + 2) + (8 * 2) + 3) + 7
(6 + 4) * 9 + 9
6 * ((8 + 5) + 7 * 3 * 8 * 5 * 5) * 4 + 4
7 + ((4 + 7 * 3 * 3 + 8) * 5 * (8 + 2) * (2 + 6 + 5 * 7 * 4 * 8) + (7 + 5 + 2) + 8)
9 + 7 + (7 * 8 + 9) + 3 * 6 + 6
(5 * 9) + 5 * 2 + 8
(9 + 4 + 7) * ((9 * 7) + 9) + (3 * 9) + ((7 * 5 * 2) * (6 + 9 * 8 * 2 + 5 * 6) + 8 + 9 * 8 + (4 * 9 * 2)) + 6
4 * 8 + 7 * ((6 + 9 * 2) * (4 * 7)) + 9
8 + ((2 + 2 * 6) * 6 + 2 + 4 + 9 + 5) * 3
8 + 7
3 + 7 + (9 + 4 + (8 * 4 * 7 + 7 * 3 + 5) + (9 + 7 + 2) + 9) + 5 + 5 * 2
6 * 7 + (3 + (3 * 3) + 4 + (2 * 7 * 2 * 3) * 7 + 6)
5 * (4 + (7 * 7 + 4 * 5 * 2 + 6) * 3 + 5 + (4 * 3 + 3 + 6 * 6) * 7) + 3 + ((2 * 8 * 4 + 8) + 6 + (5 * 9 + 2 * 7) + 4 * 8) * ((3 * 5 * 3 * 9 * 7) + 5 + (3 + 9 + 9 + 2 + 5)) + (3 * 7 + 9)
(3 * 7) + 5
(2 * 5 * 6 + 6) * 2 * (9 + 3 + 6)
2 * 2 + (6 + (4 + 2) + 4 + 9 * 9) + ((9 + 3 + 6) + 2 * 3 * 5 + 3 * 3) * 8 * (2 + (6 + 3 + 2 + 9 * 6 * 6))
8 + (6 + 8 * 8 + 3 + (4 * 7 + 6 * 3 + 6 * 8) * 6)
(8 * 8 * 4) * 5 * 9 * 9 * ((9 * 2 * 8 * 2) * 4 + (2 + 2) + 8 + 9) * 4
(6 * 3 + (8 + 5 * 9 + 2 + 8) * 3 * 7 * 2) + 9 + 8 + 2
(2 * 4 * 3) * 4 + 6 + ((5 * 7 * 4) + 7 + 4 + 5)
(8 + 5 * 2 + 5 * 3 * 9) * 8 + 8 * 2 * 8 + 5
4 + (6 + 6 * 9 * 5) * (4 + 5 * 6 + 8) + 8 + 8
6 + (5 + 7) * 3 + (7 + 2 + 4 * 5 + 2 * 4) + (6 + 4 * 5 * 7 + 4 + 5) * (5 + 6 + 8 + 8 + 9)
(5 * 7) * 6 * 8 * 9 * 6 * 6
6 + 8 + 7 + 3 * (2 + 9 * 7)
3 + ((8 * 5 + 8 * 8 * 9 + 8) * 2 + 6) * 2 + (2 + 6)
6 + 6 + (3 * 7 * 8 * (3 * 4 + 2 + 2 + 3)) + ((6 * 3 + 9 + 2 + 5 * 9) * 9 + 2 * 7 * 2 + 4) + 9 + 4
9 * 8 + ((6 + 8 * 2 + 7 * 6 * 4) + 4 + 3 + 7 + 5)
((2 + 9 + 2 * 9 + 3 * 5) * 3 + 3 * 4) * 4 + 4
6 * (8 + 7 * 5 * 8 + 2 * 2) + (4 * 4 + 8 + 3) + 3 * 5
7 + (8 * 3 * (5 + 7 + 6) + (9 + 9 * 9 * 6 + 4 * 7)) * 7 + 9 + 4
6 * (9 + 7 * 6 * 5 * 8) + 5
2 * 6 * (4 + (4 * 7 * 2 + 9 * 9) * 5 + 2) + ((5 + 6) * 3) + 8 + 2
5 + 4 * 9 + (5 * (8 * 9 + 2 + 3) * 6) * 3 * 5
5 + 9 + 9 + (5 + 2 + 8) * 3 * (7 + 6 + (7 * 7 + 7 + 7) * 6 + 7 + 2)
(2 + 6 * 4 + 9 * 2 + 2) + ((3 * 8 * 6) + 2 * (9 * 5))
5 + 3 * (3 * 5 * 3 * (7 + 6 * 9 + 3 + 8 + 4) * 9) + 2 + 7 * (6 * 8 * 7)
(4 + (9 * 5 + 3 + 4 * 5) * 6 + 9 * 6) * 5 * 9
4 * (2 + 4 * 4 + (2 + 6 + 8 * 2 * 8 * 8)) * 4 * 6 + 9 + 3
(8 + 9 + 4) + 2 + ((5 * 7 * 3 * 9 + 3) * 6 + 4) * 3 + (2 * 5 * 3 * 3) + (7 + 2 * 9)
8 + (6 + 4 * 8 * (9 + 6) + 5 * (5 * 8 + 9)) + 2 * 5
7 * 5
(4 * 3 * 3) + 7 * (8 * 4 + 3 + 9 + 5 + (6 + 4 * 8 + 3 + 7)) * 6 * 9
9 * 2 * (9 * 9 + 4 + 2 * 9) * (3 + 5 + 8)
3 + (3 + 6 + 8 + 3 * 7) + 6 + (8 + 7 + 5 + (4 + 8 + 5 + 4)) * 6
7 * 2 + 3 + 8 + 2
9 * 6 * 5 * 9 * (3 * (8 + 9 + 5) * (5 + 3 + 8) + 5) + 5
6 + 6 + ((6 + 2 * 7 + 5 + 3) + 4 * (8 * 4 * 2 * 2 + 7)) + 8 + 5 + 3
9 + (2 * 7 * 4 * 5 + 6 + 4) * 5 * 2
7 + ((2 * 4 + 2 + 4 + 3 + 3) * 9 + 5 + 6)
2 * 9 * 8 * (5 + (4 + 5 + 2 + 7))
2 * 6 * 4
(8 * (9 * 6 * 9 * 2 + 4) * 6 + 7 * 6) * 6
8 * (3 * 3 * 4 + 8) * 8 * 7
(8 * 3 * 8 * 8 + 4 + 8) + 4 * (9 * 3) * 7
((8 * 9 + 2 * 4) * (4 + 3 + 8 + 9 + 8) * (3 * 8 + 7) + 6 * 3) + 5
2 + (8 * 9 * (7 + 6) * (4 + 7 + 7 + 7))
4 + (8 + 6 + (8 + 6 * 6 + 7 * 9) + (3 + 6 * 5 * 6)) + 5 * 8 * 7 * 9
(7 * 7 + 8 * 4 + 4) * 3 + 2 * 4 + 5 * (4 * 7)
((4 + 2 * 6 * 8 * 9) + (8 * 5 + 8) * 9) * 9 + 7 * 7 + 9
6 + 3 + ((5 + 6 * 6 * 4 * 3 * 6) + 4) + 4 + (6 * 7 * 4 * 7 + 4 * 5) * 4
3 * 2 + (4 * 4 * 3)
9 + 7 + (8 + 4 * 5 * 7 + 3) * 9 * 6 + 4
(3 * 8 + 9 + 8 + 5) + (2 * 6 * (2 * 4)) * 7 + (7 + 9 * (4 + 9 * 2 * 9 * 9) * 2) + (4 * 2 * 3 * 9)
2 + 3 * 4 * 6 + 6
8 * 4 * 8 + (2 * (5 + 9) * 3) * 6 * 4
8 + (5 + 3 * 3) * 8 * (9 * 9 + 3 * (9 + 9 * 5 + 8 * 4 + 4) + 9) * ((5 * 7 * 7) * (9 + 6) * 6)
2 + 5 + 7 + 8 * (4 + 9 * 4) + 7
(7 + 9 + 7 + 2 * (5 + 3 * 6 * 5) * 4) + 5 + 8 * 3
8 * 7 + (4 * 8 * 9 * 7 * 7 * 7) * 5 + 6 * 4
((8 + 6 * 9 + 7 + 3) + (8 * 2 + 9 + 6) * 3 * 8 + 9 * 5) + (3 * 8 * 2 * 9 * 5) * 3 + (7 + (7 + 4 + 3) + 3) * (9 + 9 + 6 * 9 + 2 * 9)
(9 * 3 + 2 + 5 * 9) * 5 + 6
(3 * 6 + 5 * 9 + 2 * 6) + 2 * ((9 * 3) * 2 + (9 + 7) * 4 + (2 * 8 + 2 * 4 * 9) + 7) + 9 * (5 * (6 * 8 * 9) * 4 * 6 + 2) + 2
2 + 9 + 6 + (6 + 6) * ((4 * 7 * 6 * 8 + 3 + 3) + 9 + 7) + ((2 * 2 * 6) + (9 + 2 + 6 + 2 + 8) + 4 * 7 * (7 + 4 + 4 * 5 + 8) * 4)
7 * 4 + 9 + (5 + 9 + 9 + 4)
2 * 8 * 2 + (4 * 9)
7 * 3 + (5 + 8 * 4 * (9 + 6 * 8 + 2 * 3 + 9) + 2 * 9) * 9 * 5 + 9
2 * (5 * 6 + 8 * 2 + 6) * 3 * 3 + 7
(6 * 5) + 8 * 3 + 9 * (7 * 4 * (2 * 6 * 6 * 2)) + 6
6 * 5 * 2 * 8 + 8 + (6 * 5 + (7 * 7) + 3 * (9 * 8 * 2 * 5 + 4 + 5))
2 + 3 + 7 + (8 * 2 * 2)
6 + 3 * ((9 + 7 * 4 + 4 * 3 * 6) + (3 * 5 + 8 + 3 * 3 + 4) * 5 + 5 * 5) * ((4 + 2) + 9 * 4 * 9 * 5)
8 + 7 + 6 * 2 * ((8 * 3) * 3) * (2 + 5 * 5 + 5 * 8 + 8)
5 * 2 * 8 * 4 * 7 * 2
4 * 9 + 8 * ((9 * 5) + 5 * 2 * (6 * 4 + 2) * (2 + 8 * 8 + 8 * 2))
3 * 2 + ((8 * 8) * 2 + 8 + 5 * 8 + 9) * ((4 * 7 * 4 + 5 + 6) * 6 * 3)
(3 * (3 + 3) * 2 * 3 * (4 * 2 + 9 + 3 * 5 * 3)) * (2 * 2) * 3 + (5 * 5 * 6 + 2 + 2 * 3) + 7 * 2
4 * (3 + 2)
3 * 9 * 2 + (5 * 7 * 5 + 7 * 5 * 9) + 9
2 + (5 * 7) + 3 + 7 * 8 * 8
((4 * 6 * 8 + 4 + 4) * (2 + 3 + 5 * 2)) + (2 * 5) + 3
9 + (5 * (3 + 8 * 7) + 8 + 4 + 8 * (6 * 2 * 7))
9 + 6 + 6 * 3 * (2 + 7 + (5 + 7 + 4) + 3 * 2) + 5
7 + (5 * 5 + (4 * 5 + 7 * 4 + 3 + 3) * 4 + 7)
9 + 5 + 3 + ((7 + 7 + 2) + (6 + 9 + 7 + 4 * 5 + 7)) + 6 * 6
5 * 8 * 2 + ((6 + 6 * 3 + 6 * 8 * 6) + 6 * 3) + 3 * 2
(5 + 5 * 5 * (3 * 7 * 3 + 2) + 6 + 6) + 7 * 4 + (6 * (8 * 8 + 5) * 9 * 8 * 8 + 4) + 6
(7 + 3 + 6 + 2 * 8 + 5) * 6 * (2 * (3 + 7 + 9 + 6) + 5 + 2) + 8
((7 + 4) * 6 * 9) + 5
2 * 7 + 5 + 4 + 7
9 * (2 * 2 * 7 + 4 * (4 * 6 * 5 * 8))
5 * 9 + 8
6 * 2 * 8 + (5 + 3 + (8 + 9 + 6))
3 * 5 + 9 * (4 + 2 + 9 + (5 * 3) * 3 + 6) * 5
4 * 8 + (3 + 9 * 6) * 7 * (7 * 3 * 9 + 8 + 6 + (9 * 9 + 7 + 4)) + 8
(2 * 9) * (6 * 8 * 6 + 5)
9 * 2 * 3
(6 + 2 * 9 + 5) * 6
(8 + 3 + 2 * (2 + 8 * 2 + 6 * 2 + 2)) * 2
(4 + 8 * 6) * 3 * 6 + 4 * (3 * 2 * 5 + 9 + 9) + 8
3 + (9 * (8 + 6 + 8 * 7) * 8 * 7 + 7) * 7 + 8 + (2 + 3 + 6 * 6 + (2 + 4 + 6 + 5 * 3)) * 5
((5 + 2) + (7 * 4 * 9 + 3) * (4 + 5 * 8 + 6 * 6) + (9 + 2) * 4) + 6 + 9 * 3 * 4 * 5
2 + (7 * 5) + 4 + 3 + (3 * (7 + 2 * 6 * 7 + 6)) + 4
2 + 6 + (3 + 2 + (2 + 3) + 7 * 4 * (8 * 7 * 2 * 4 + 8 * 5))
4 * 3 + 6 + 5 * (9 + (2 * 4 * 8 * 8 + 8) + 2 + 4) + 9
4 * 6 + (9 * 8 * 9 * 5 * 9 + 7) + 7
(4 + (9 + 5 + 2 + 4 * 4 + 6) * 5 + 8) + (2 + 8 + (6 + 5 * 8 * 7) + (7 * 5)) + 5
3 + 2 + (6 * 9 + 9 * 2) * (7 * (5 * 8) + 9 + 6 * (7 + 7 * 9 + 2 + 4) * 7) * 9
5 + 9 * 7 * (8 * (6 + 4 * 5 + 3 * 2 * 4) + (5 + 2) * (6 * 3 * 7 * 6 + 8 + 3))
5 * 7 * 2 + 7 * 6 + (2 * 7 + 5 * 5 * (6 + 7 + 5 + 7))
4 + 7 * 2
9 + ((6 * 7) + (9 + 7 + 7)) * 4
5 + 6 * (3 * (8 * 8 * 3 + 4 * 5) + 4 + (2 + 7 + 3))
(2 + 7 + 6) + 5 + 7 * 6 * 8 * 9
2 * (6 + 9) * 8 * (9 + 4) + 7 + 8
5 + 3 * 6 * 9 + (4 * 8 + 3 * 2)
9 * (2 + 5 * 5) * 9 + (8 + 2 * (5 + 2 * 7 + 7 + 4 * 9) + 3 * 6 + 9) + 8
8 + 4 + (2 + (9 * 2) * 5 * (5 * 8)) * 8 * 7
5 * (3 * 5 + 6 * 4) + (7 * 5 * (6 + 6 * 7 * 8 * 6) + 8)
5 + (4 * 6) + 6 * 5 + 6
7 * 9 + ((6 + 3 + 6 * 4 * 4 * 6) + 7 + (6 + 4 * 7 * 3 + 9 * 6) + 4 + 9) + 6
(8 * 6) + (3 * (3 + 2 * 7 * 9 + 9 + 9) * 3 + (2 + 8 * 5))
3 * 4 * 6 * 4 * ((2 + 3 * 6) * 9)
5 * ((8 * 5 + 7) + 4 + 7 + 6 * (9 * 6 * 3 + 9 * 7 + 2) + (4 + 4)) + 8 * (7 * (2 * 8 * 6) + 8 + 9 + 6) + 7
2 * 8 * 5 * ((9 * 4 * 9 + 5 + 2) + (3 * 9) + (6 + 9 * 4 * 9 * 2) * 5 + 5) * 9
5 + (7 * 8 + 7) + 2 * 5
4 * (7 + 4 + 5 + (6 * 2 + 3)) * (7 * 3 * 7 * 8 * 7) * 2 * 2 + 2
(4 + (5 + 5 * 8) + (5 * 5 + 3 * 5) + 6) + 6
6 + (8 + 6 * 4 + 8 + (7 * 6) * 6) + 2 + 3
(6 + 3) + ((8 * 7 * 6 * 6 + 6 * 7) * 4 * 6 * (3 + 3 * 2)) + 6
4 * 2 * 3 * 3 * 7 + 9
3 * 4 * 8 * (6 * 6 * 3 + (3 * 9 * 3 * 3 + 6)) + 5 + 4
((7 * 2 + 7) + 9) * 2 * 8 * 3 + 3 * 3
2 * 3 + ((2 * 5) + 8 * 7 * 5 + (8 * 2 * 4) * 8) * 3 + 7 * 8
9 + (4 + 6) * 3
(9 + 5) * 3 * 2 * 7 * (6 * 4 * 3)
(3 * 3 * (3 * 3 * 5)) * 7 * 8 + (6 * 7 + 4) * 2 * 2
(2 + 4 * (9 + 9 * 7 * 9) + 4) + 4 + 3
(9 + 4 + 3) * 9 * 3 + 7 * 3
6 + 4 + (6 + 2 + 4) + (9 + 7 * 8)
7 + 9 + 5 * (3 + 9 * 7 + (5 + 8 + 9 * 4 + 6) * 5 * 8) + 9 * 6
(3 * 7 * 7 * 3 + 7) + 4 * 3 + 5
(8 + 7 * 2 + 5) * 4 + 7 * 2 + 5 + 3
9 * 5 * ((5 * 8 + 2) * 7) + 2
8 * 4 * 9 + 2
4 + (3 + 5 * 7 * 3 * 8 + 5) * 6 + (3 + 2 + 9) * 5
(3 * 5 + 8 * 8 * (7 * 6 + 2 * 6 + 9 * 7) * 9) * 7 * 9 + 9 + 3
5 * 9 + 6
6 * (8 + 3 + 4)
2 + 5 + (4 + 5 + 8 * (8 + 6 * 9 * 8 + 3) * (3 + 9 * 3) + 3) + 7 + 2
9 * (6 + 4) * 9 * 4 * 2 * ((4 + 3 + 2 * 9) + 6 + 5 + 8 * 7 + (7 * 7 + 6 + 2 + 8))
(7 * 2 * (4 + 2 + 7 * 5 * 6 * 5) * (3 * 4 + 2) + 6) + 9 + 5 + 3
4 * 5 * ((5 + 9 * 7 + 2) + 4 * 7 + 9) + 9 + 7
7 * 2 * 6 + 9 + 5 * 6
3 + 3 * 7 * ((3 * 3 + 2 + 7 * 5 + 2) + 7 * 6 * 2 * 5 * 9) + (4 * 5) + 7
9 + ((5 + 5 * 7) + 7 * 2) + 7 + 6
4 * 9 * 9 + ((9 + 3 + 7 * 4) * 4 + 2 + 9 + 8 + 6) * 6
(6 * 2 * 5 + 4 + 5) + (8 + (8 * 5 + 7 * 7 + 6 * 2) * 6 + 6) + 4 * 8
(5 + 6 * 2) * 5 * 6 * (5 * 5) + 9
(7 + (9 * 4)) * ((4 + 3 + 5) + 7 * 5)
2 * 5 + ((9 * 6 * 3 * 6 + 4) * 9 * 4) * 2 * 3 * (8 + 6)
3 * 6 * 5 + 9 * (6 + 4 * 2 + 5 * 6)
((7 + 8) + 3 + 7 + (8 * 3 * 6 + 8) * 2) * ((3 * 5 * 9) + 5) * 8 + 8
((3 * 4 + 5 + 2) + 6 * 6) * 9
4 + 6 * (9 * 6 * 7 + (2 * 5 + 5 * 9 + 6)) * 8
4 * (2 + 2 + 9) + 5 + 5 * ((5 + 3) + 4 * 7)
6 + 8 * 3 + 4
2 + 8 + (8 + (7 + 5 * 3 * 8 * 7 + 2) * 6) + 3)";

int main() {
    solution(input);
}