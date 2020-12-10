#include <iostream>
#include <string_view>
#include <array>
#include <ranges>
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

unsigned int to_int(std::string_view s) {
    unsigned int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

auto parse(std::string_view input) {
    std::vector<unsigned int> jolts;
    split(input,'\n',[&](std::string_view line) {
        jolts.push_back(to_int(line));
    });
    jolts.push_back(0);
    std::ranges::sort(jolts);
    jolts.push_back(jolts.back()+3);
    return jolts;
}

auto part1(const decltype(parse(""))& input) {
    struct Diff {
        std::array<int,3> diff = {0,0,0};
        Diff(std::array<int,3> d) : diff(d) {}
        Diff(int a, int b) {
            diff[a-b-1] = 1;
        }
        Diff operator+(Diff o) {
            return std::array{diff[0]+o.diff[0],diff[1]+o.diff[1],diff[2]+o.diff[2]};
        }
    };

    auto d = std::inner_product(input.begin()+1,input.end(),input.begin(),Diff({0,0,0}),std::plus<>{},[](int a, int b){return Diff(a,b);});
    return d.diff[0] * d.diff[2];
}

auto part2(const decltype(parse(""))& input) {
    std::array<uint64_t,4> diff = {0,0,0,0};
    auto get = [&diff](unsigned int a) -> uint64_t& {return diff[a%4];};
    get(input.back()+1) = 1;
    int prev = input.back()+1;
    for(auto jolt : std::ranges::reverse_view(input)) {
        for(int i = jolt+1; i < prev; ++i) get(i) = 0;
        get(jolt) = get(jolt+1) + get(jolt+2) + get(jolt+3);
        prev = jolt;
    }
    return diff[0];
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(147
174
118
103
67
33
96
28
43
22
16
138
75
148
35
6
10
169
129
115
21
52
58
79
46
7
139
104
91
51
172
57
49
126
95
149
125
123
112
30
78
44
37
167
157
29
173
98
36
63
111
160
18
8
9
159
179
72
110
2
53
150
17
81
97
108
102
56
135
166
168
163
1
25
3
158
101
132
144
45
140
34
156
178
105
68
153
80
82
59
50
122
69
85
109
40
124
119
94
88
13
180
177
133
66
134
60
141)";

int main() {
    solution(input);
}
