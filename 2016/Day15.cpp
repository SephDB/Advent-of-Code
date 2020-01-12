#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

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

template<int N>
auto split_known(std::string_view in, char delim) {
    std::array<std::string_view,N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = in.substr(0,pos);
        in.remove_prefix(pos+1);
    }
    ret[current++] = in;
    return ret;
}

std::int64_t to_int(std::string_view s) {
    std::int64_t ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

/*
function extended_gcd(a, b)
    s := 0;    old_s := 1
    t := 1;    old_t := 0
    r := b;    old_r := a
    
    while r ≠ 0 do
        quotient := old_r div r
        (old_r, r) := (r, old_r − quotient × r)
        (old_s, s) := (s, old_s − quotient × s)
        (old_t, t) := (t, old_t − quotient × t)
    
    output "Bézout coefficients:", (old_s, old_t)*/

std::pair<std::int64_t,std::int64_t> bezout(std::int64_t a, std::int64_t b) {
    std::pair s = {0,1};
    std::pair t = {1,0};
    std::pair r = {b,a};

    while(r.first != 0) {
        auto q = r.second / r.first;
        r = {r.second - q*r.first, r.first};
        s = {s.second - q*s.first, s.first};
        t = {t.second - q*t.first, t.first};
    }

    return {s.second,t.second};
}

std::int64_t abs_modulo(std::int64_t a, std::int64_t n) {
    if(a < 0) {
        return (a%n) + n;
    }
    return a%n;
}

struct Congruence {
    std::int64_t a;
    std::int64_t n;
};

auto solve(Congruence a, Congruence b) {
    auto [m1,m2] = bezout(a.n,b.n);
    auto newn = a.n*b.n;
    auto a12 = (a.a*m2*b.n) + (b.a*m1*a.n);
    return Congruence{abs_modulo(a12,newn),newn};
}

auto parse(std::string_view input) {
    std::vector<Congruence> ret;
    auto current = 0;
    for(auto l : split(input,'\n')) {
        l.remove_suffix(1);
        auto s = split_known<12>(l,' ');
        auto n = to_int(s[3]);
        //If a wheel n is at position s at time n, we need to solve the equation x = (n-s) mod n to have the wheel be at position 0 at time x
        auto a = abs_modulo(n - (to_int(s[11]) + ++current),n);
        
        ret.push_back({a,n});
    }
    return ret;
}

void solution(std::string_view input) {
    auto equations = parse(input);
    auto s = std::accumulate(equations.begin()+1,equations.end(),equations.front(),solve);
    std::cout << "Part 1: " << s.a << '\n';
    auto s2 = solve(s,{11 - equations.size() - 1,11});
    std::cout << "Part 2: " << s2.a  << '\n';
}

std::string_view input = R"(Disc #1 has 17 positions; at time=0, it is at position 5.
Disc #2 has 19 positions; at time=0, it is at position 8.
Disc #3 has 7 positions; at time=0, it is at position 1.
Disc #4 has 13 positions; at time=0, it is at position 7.
Disc #5 has 5 positions; at time=0, it is at position 1.
Disc #6 has 3 positions; at time=0, it is at position 0.)";

int main() {
    solution(input);
}
