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

template<int N, typename F>
auto split_known(std::string_view in, char delim, F&& apply) {
    std::array<decltype(apply(in)),N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = apply(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    ret[current++] = apply(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

int64_t abs_modulo(int64_t a, int64_t n) {
    if(a < 0) {
        return (a%n) + n;
    }
    return a%n;
}

/*
 function inverse(a, n)
    t := 0;     newt := 1
    r := n;     newr := a

    while newr ≠ 0 do
        quotient := r div newr
        (t, newt) := (newt, t − quotient × newt) 
        (r, newr) := (newr, r − quotient × newr)

    if r > 1 then
        return "a is not invertible"
    if t < 0 then
        t := t + n

    return t
*/
int64_t inverse(int64_t a, int64_t n) {
    std::pair<int64_t,int64_t> t = {0,1};
    std::pair<int64_t,int64_t> r = {n,a};
    while(r.second != 0) {
        int64_t q = r.first/r.second;
        t = {t.second, t.first - q*t.second};
        r = {r.second, r.first - q*r.second};
    }

    if(r.first > 1) std::cout << "PANIC!\n";
    if(t.first < 0) return t.first + n;
    return t.first;
}

struct Departure {
    int64_t id;
    int64_t offset;
};

struct ParseResult {
    std::vector<Departure> ids;
    int estimate;
};

ParseResult parse(std::string_view input) {
    auto [total_s,in] = split_known<2>(input,'\n',[](auto a){return a;});
    int total = to_int(total_s);
    std::vector<Departure> parsed;
    int offset = 0;
    split(in,',',[&](std::string_view num) {
        if(num != "x") {
            parsed.emplace_back(to_int(num),offset);
        }
        ++offset;
    });
    return {parsed,total};
}

int64_t part1(decltype(parse("")) input) {
    auto min = std::ranges::min(input.ids,{},[&](auto i) {return i.id - input.estimate % i.id;}).id;
    return min * (min-input.estimate%min);
}

int64_t part2(decltype(parse("")) input) {
    int64_t N = std::transform_reduce(input.ids.begin(),input.ids.end(),int64_t{1},std::multiplies<>{},[](auto bus){return bus.id;});

    auto c = std::transform_reduce(input.ids.begin(),input.ids.end(),int64_t{0},std::plus<>{},[N](auto bus) {
        int64_t Ni = N/bus.id;
        int64_t Mi = inverse(Ni,bus.id);
        int64_t ai = abs_modulo(bus.id-bus.offset,bus.id);
        return Ni*Mi*ai % N;
    });
    return c % N;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(1000186
17,x,x,x,x,x,x,x,x,x,x,37,x,x,x,x,x,907,x,x,x,x,x,x,x,x,x,x,x,19,x,x,x,x,x,x,x,x,x,x,23,x,x,x,x,x,29,x,653,x,x,x,x,x,x,x,x,x,41,x,x,13)";

int main() {
    solution(input);
}
