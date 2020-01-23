#include <iostream>
#include <string_view>
#include <array>
#include <unordered_map>
#include <charconv>
#include <numeric>
#include <algorithm>

char to_char(std::string_view s) {
    char ret = 0;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

auto split(std::string_view in, char delim) {
    std::string ret;
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret.push_back(to_char(in.substr(0,pos)));
        in.remove_prefix(pos+1);
    }
    ret.push_back(to_char(in));
    return ret;
}

auto parse(std::string_view input) {
    return split(input,'\t');
}

auto next(std::string& in) {
    auto m = std::max_element(in.begin(),in.end());
    auto val = *m;
    *m = 0;
    int num_each = val/in.size();
    int left = val - num_each*in.size();
    auto current = m+1;
    while(current != in.end()) {
        *current++ += (left-- > 0)+num_each;
    }
    for(current = in.begin(); current <= m; ++current) {
        *current += (left-- > 0)+num_each;
    }
    return in;
}

void solution(std::string_view input) {
    auto in = parse(input);
    int total = 1;
    std::unordered_map<std::string,int> seen;
    seen.insert({in,0});
    while(seen.insert({next(in),total}).second) ++total;
    std::cout << "Part 1: " << total << '\n';
    std::cout << "Part 2: " << total - seen.at(in) << '\n';
}

std::string_view input = R"(4	1	15	12	0	9	9	5	5	8	7	3	14	5	12	3)";

int main() {
    solution(input);
}
