#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <complex>

std::vector<std::string_view> split(std::string_view in, char delim) {
    std::vector<std::string_view> ret;
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret.push_back(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    ret.push_back(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

auto parse(std::string_view input) {
    std::vector<std::pair<bool,int>> ret;
    for(auto l : split(input,',')) {
        ret.emplace_back(l[1] == 'R',to_int(l.substr(2)));
    }
    return ret;
}

auto part1(decltype(parse("")) input) {
    std::complex<int> current = {0,0};
    std::complex<int> dir = {1,0};
    std::complex<int> i = {0,1};
    for(auto [d,len] : input) {
        dir *= i*(1-2*d);
        current += dir*len;
    }
    return std::abs(current.real())+std::abs(current.imag());
}

struct Segment {
    Segment(std::complex<int> a, std::complex<int> b) {
        if(a.real() == b.real()) {
            fixed = a.real();
            std::tie(start,end) = std::minmax(a.imag(),b.imag());
        } else {
            fixed = a.imag();
            std::tie(start,end) = std::minmax(a.real(),b.real());
        }
    }
    int fixed;
    int start,end;
    std::optional<std::complex<int>> collision(Segment o) {
        if(fixed < o.start or fixed > o.end or o.fixed < start or o.fixed > end) return {};
        return std::complex<int>{fixed,o.fixed};
    }
};

auto part2(decltype(parse("")) input) {
    std::array<std::vector<Segment>,2> segments;
    int current_dir = 1;
    std::complex<int> current = {0,0};
    std::complex<int> dir = {1,0};
    std::complex<int> i = {0,1};
    for(auto [d,len] : input) {
        current_dir = 1 - current_dir;
        dir *= i*(1-2*d);
        auto next = current + dir*len;
        segments[current_dir].emplace_back(current,next);
        current = next;

        auto& possible_collisions = segments[1-current_dir];
        if(possible_collisions.empty()) continue;

        for(int s = 0; s < possible_collisions.size()-1; ++s) {
            if(auto p = possible_collisions[s].collision(segments[current_dir].back()); p.has_value()) {
                return std::abs(p->real())+std::abs(p->imag());
            }
        }
    }
    return 0;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"( R1, R1, R3, R1, R1, L2, R5, L2, R5, R1, R4, L2, R3, L3, R4, L5, R4, R4, R1, L5, L4, R5, R3, L1, R4, R3, L2, L1, R3, L4, R3, L2, R5, R190, R3, R5, L5, L1, R54, L3, L4, L1, R4, R1, R3, L1, L1, R2, L2, R2, R5, L3, R4, R76, L3, R4, R191, R5, R5, L5, L4, L5, L3, R1, R3, R2, L2, L2, L4, L5, L4, R5, R4, R4, R2, R3, R4, L3, L2, R5, R3, L2, L1, R2, L3, R2, L1, L1, R1, L3, R5, L5, L1, L2, R5, R3, L3, R3, R5, R2, R5, R5, L5, L5, R2, L3, L5, L2, L1, R2, R2, L2, R2, L3, L2, R3, L5, R4, L4, L5, R3, L4, R1, R3, R2, R4, L2, L3, R2, L5, R5, R4, L2, R4, L1, L3, L1, L3, R1, R2, R1, L5, R5, R3, L3, L3, L2, R4, R2, L5, L1, L1, L5, L4, L1, L1, R1)";

int main() {
    solution(input);
}
