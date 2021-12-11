#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

auto parse(std::string_view input) {
    std::array<int,12*12> grid;
    std::ranges::fill(grid,std::numeric_limits<int>::min());
    int current = 13;
    split(input,'\n',[&](std::string_view line) {
        for(auto c : line) {
            grid[current++] = c-'0';
        }
        current += 2;
    });
    return grid;
}

int step(decltype(parse(""))& input) {
    int flashes = 0;
    auto run = [&](int loc, auto&& rec) -> void {
        if(++input[loc] == 10) {
            ++flashes;
            rec(loc-13,rec);
            rec(loc-12,rec);
            rec(loc-11,rec);
            rec(loc-1,rec);
            rec(loc+1,rec);
            rec(loc+11,rec);
            rec(loc+12,rec);
            rec(loc+13,rec);
        }
    };
    for(int i = 0; i < input.size(); ++i) {
        run(i,run);
    }
    for(auto& n : input) {
        if(n > 9) n = 0;
    }
    return flashes;
}

auto part1(decltype(parse("")) input) {
    int flashes = 0;
    
    for(int i = 0; i < 100; ++i) {
        flashes += step(input);
    }
    return flashes;
}

auto part2(decltype(parse("")) input) {
    int steps = 1;
    while(step(input) != 100) ++steps;
    return steps;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(6744638455
3135745418
4754123271
4224257161
8167186546
2268577674
7177768175
2662255275
4655343376
7852526168)";

int main() {
    solution(input);
}
