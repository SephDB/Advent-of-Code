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

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct Scanner {
    int depth;
    int period;
};

auto parse(std::string_view input) {
    std::vector<Scanner> s;
    int total = 0;
    split(input,'\n',[&](std::string_view line) {
        auto pos = line.find(':');
        auto depth = to_int(line.substr(0,pos));
        auto range = to_int(line.substr(pos+2));
        auto period = range*2-2;
        if(depth % period == 0) total += depth*range;
        s.push_back({depth,range*2-2});
    });
    return std::pair{std::move(s),total};
}

auto part2(std::vector<Scanner> input) {
    std::sort(input.begin(),input.end(),[](auto&& a, auto&& b){return a.period < b.period;});
    auto caught = [&](int wait) {
        return std::any_of(input.begin(),input.end(),[wait](const Scanner& s) {return (s.depth+wait) % s.period == 0;});
    };
    int current = 0;
    while(caught(++current));
    return current;
    return 0;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << in.second << '\n';
    std::cout << "Part 2: " << part2(std::move(in.first)) << '\n';
}

std::string_view input = R"(0: 4
1: 2
2: 3
4: 4
6: 8
8: 5
10: 6
12: 6
14: 10
16: 8
18: 6
20: 9
22: 8
24: 6
26: 8
28: 8
30: 12
32: 12
34: 12
36: 12
38: 10
40: 12
42: 12
44: 14
46: 8
48: 14
50: 12
52: 14
54: 14
58: 14
60: 12
62: 14
64: 14
66: 12
68: 12
72: 14
74: 18
76: 17
86: 14
88: 20
92: 14
94: 14
96: 18
98: 18)";

int main() {
    solution(input);
}
