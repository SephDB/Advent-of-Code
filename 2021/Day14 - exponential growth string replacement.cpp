#include <iostream>
#include <string_view>
#include <array>
#include <vector>
#include <charconv>
#include <numeric>
#include <optional>
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
auto split_known(std::string_view in, std::string_view delim, F apply) {
    std::array<decltype(apply(in)),N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = apply(in.substr(0,pos));
        in.remove_prefix(pos+delim.size());
    }
    ret[current++] = apply(in);
    return ret;
}

constexpr auto csqrt(size_t n) {
    for(size_t i = 1; i < n; ++i) {
        if(i*i == n) return i;
    }
    return size_t{0};
}

template<size_t InputSize>
struct Polymer {
    static constexpr size_t Size = InputSize;
    std::string_view input;
    std::array<int8_t,26> charToIndex;
    std::array<char,InputSize> indexToChar;
    std::array<int8_t,InputSize*InputSize> rules;

    int8_t& getRule(int8_t left, int8_t right) {
        return rules[left*InputSize + right];
    }

    int8_t getRule(int8_t left, int8_t right) const {
        return rules[left*InputSize+right];
    }
};

template<size_t RuleNum>
auto parse(std::string_view input) {
    Polymer<csqrt(RuleNum)> parsed;
    std::ranges::fill(parsed.charToIndex,-1);

    auto [inputString,rules] = split_known<2>(input,"\n\n",[](auto a){return a;});
    parsed.input = inputString;
    int8_t current = 0;
    auto getIndex = [&](char c) {
        if(parsed.charToIndex[c-'A'] == -1) {
            parsed.charToIndex[c-'A'] = current;
            parsed.indexToChar[current] = c;
            ++current;
        }
        return parsed.charToIndex[c-'A'];
    };
    split(rules,'\n',[&](std::string_view line) {
        int8_t left = getIndex(line[0]);
        int8_t right = getIndex(line[1]);
        int8_t insert = getIndex(line.back());
        parsed.getRule(left,right) = insert;
    });
    return parsed;
}

template<size_t Steps>
auto do_steps(const auto& input) {
    using Result = std::array<uint64_t,input.Size>;
    Result output{};
    using Step = std::array<std::optional<Result>,input.Size*input.Size>;
    std::vector<Step> memoization(Steps);

    auto rec = [&](int8_t left, int8_t right, size_t steps_remaining, auto&& rec) -> Result {
        if(steps_remaining == 0) {
            return {};
        }
        auto& m = memoization.at(steps_remaining-1).at(left*input.Size+right);
        if(m) {
            return *m;
        }
        
        int8_t insert = input.getRule(left,right);
        Result s = rec(left,insert,steps_remaining-1,rec);
        std::ranges::transform(s,rec(insert,right,steps_remaining-1,rec),s.begin(),std::plus<>{});
        s[insert]++;
        m = s;
        return s;
    };
    for(int i = 0; i < input.input.size()-1; ++i) {
        int8_t left = input.charToIndex[input.input[i]-'A'];
        int8_t right = input.charToIndex[input.input[i+1]-'A'];
        output[left]++;
        std::ranges::transform(output,rec(left,right,Steps,rec),output.begin(),std::plus<>{});
    }
    output[input.charToIndex[input.input.back()-'A']]++;

    return output;
}

auto part1(const auto& input) {
    auto result = do_steps<10>(input);
    auto [min,max] = std::ranges::minmax(result);
    return max-min;
}

auto part2(const auto& input) {
    auto result = do_steps<40>(input);
    auto [min,max] = std::ranges::minmax(result);
    return max-min;
}

template<size_t RuleNum>
void solution(std::string_view input) {
    auto in = parse<RuleNum>(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

constexpr std::string_view input = R"(CFFPOHBCVVNPHCNBKVNV

KO -> F
CV -> H
CF -> P
FK -> B
BN -> P
VN -> K
BC -> H
OP -> S
HS -> V
HK -> N
CC -> F
CK -> V
OC -> S
SN -> C
PK -> H
BB -> S
PO -> F
HF -> K
BV -> P
HP -> F
VF -> H
BP -> H
CH -> C
KN -> O
NP -> F
FS -> F
BH -> B
VB -> P
OS -> S
KK -> O
SO -> P
NB -> O
PS -> O
KV -> O
CS -> P
PN -> O
HB -> V
NF -> P
SC -> S
NH -> N
HV -> K
FN -> V
KS -> P
BO -> C
KP -> V
OK -> B
OV -> P
CN -> C
SB -> H
VP -> C
HC -> P
FB -> F
VS -> K
PH -> C
VC -> H
KH -> B
SH -> B
BK -> N
SP -> P
SF -> B
OO -> B
VH -> K
PP -> C
FV -> P
KC -> P
CO -> S
NO -> O
FO -> K
SK -> O
ON -> K
VO -> H
VV -> H
CP -> P
FC -> B
FP -> N
FH -> C
KF -> F
PB -> C
NN -> K
SS -> O
CB -> C
HH -> S
FF -> S
KB -> N
HO -> O
BF -> N
PV -> K
OB -> B
OH -> N
VK -> V
NV -> H
SV -> F
NC -> P
OF -> V
NS -> V
PF -> N
HN -> K
BS -> S
NK -> H
PC -> O)";


int main() {
    solution<std::ranges::count(input,'\n')-1>(input);
}
