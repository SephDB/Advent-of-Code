#include <iostream>
#include <string_view>
#include <string>
#include <array>
#include <ranges>
#include <map>
#include <bitset>
#include <bit>
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

namespace grid2 {
    constexpr auto flipV(int grid) {
        return ((grid << 2) | (grid >> 2)) & 0xf;
    }

    constexpr auto flipH(int grid) {
        return ((grid << 1) & 0b1010) | ((grid >> 1) & 0b0101);
    }

    constexpr auto flipD(int grid) {
        return 0xf & ((grid & 0b0110) | (grid << 3) | (grid >> 3));
    }

    constexpr auto flipAD(int grid) {
        return (grid & 0b1001) | (grid & 0b0100) >> 1 | (grid & 0b0010) << 1;
    }

    constexpr auto minNum(int num) {
        return std::min({num,flipV(num),flipH(num),flipV(flipH(num)),flipD(num),flipV(flipD(num)),flipAD(num),flipV(flipAD(num))});
    }
}

//Swap bits indicated in mask with bits delta to their left
constexpr int delta_swap(int num, int delta, int mask) {
    int t = (num ^ (num >> delta)) & mask;
    return num ^ t ^ (t << delta);
}

namespace grid3 {
    constexpr int full = 0b111'111'111;

    constexpr auto flipV(int grid) {
        return ((grid & 0b000'111'000) | (grid << 6) | (grid >> 6)) & full;
    }

    constexpr auto flipH(int grid) {
        return ((grid << 2) & 0b100'100'100) | (grid & 0b010'010'010) | ((grid >> 2) & 0b001'001'001);
    }

    constexpr auto flipD(int grid) {
        /*
        -8 -4 +0
        -4 +0 +4
        +0 +4 +8
        */
        grid = delta_swap(grid,8,0b000'000'001);
        grid = delta_swap(grid,4,0b000'001'010);
        return grid;
    }

    constexpr auto flipAD(int grid) {
        /*
        +0 -2 -4
        +2 +0 -2
        +4 +2 +0
        */
        grid = delta_swap(grid,4,0b000'000'100);
        grid = delta_swap(grid,2,0b000'100'010);
        return grid;
    }

    constexpr auto minNum(int num) {
        return std::min({num,flipV(num),flipH(num),flipV(flipH(num)),flipD(num),flipV(flipD(num)),flipAD(num),flipV(flipAD(num))});
    }
}

auto to_int(std::string_view block) {
    int res = 0;
    for(auto c : std::ranges::reverse_view(block)) {
        if(c != '/') res <<= 1;
        res |= (c=='#');
    }
    return res;
}

auto split4grid(int grid) {
    /*
    0123
    4567
    890a
    bcde
    */
    grid = delta_swap(grid,2,0b0000'1100'0000'1100);
    return std::array{
        grid & 0xf,
        (grid >> 4) & 0xf,
        (grid >> 8) & 0xf,
        (grid >> 12) & 0xf
    };
}

struct Rule {
    int lhs;
    int actual_rhs;
    std::array<int,4> rhs;
    uint64_t rhs2_full;
    std::array<int,9> rhs2;
    Rule(int l, int r, const auto& small) : lhs(grid3::minNum(l)), actual_rhs(r), rhs(split4grid(r)) {
        std::array<int,4> part2{small[rhs[0]],small[rhs[1]],small[rhs[2]],small[rhs[3]]};
        //put 4 3x3 numbers into a 6x6 grid
        uint64_t n = part2[0]&0x7 | (part2[1]&07) << 3 | (part2[0]&070) << 3 | (part2[1]&070) << 6 | (part2[0]&0700) << 6 | (part2[1]&0700) << 9;
        uint64_t nhigh = part2[2]&0x7 | (part2[3]&07) << 3 | (part2[2]&070) << 3 | (part2[3]&070) << 6 | (part2[2]&0700) << 6 | (part2[3]&0700) << 9;
        n |= nhigh << 18;
        rhs2_full = n;
        //This can prob be done faster with interesting twiddling
        for(int i = 0; i < 3; ++i) {
            for(int j = 0; j < 3; ++j) {
                uint64_t n2 = n >> (i*12+j*2);
                rhs2[3*i+j] = (n2&0b11) | (n2 >> 4)&0b1100;
            }
        }
    }
};

struct RuleSet {
    std::array<int,16> small;
    std::vector<Rule> rules;

    const Rule& find(int n) const {
        return *std::ranges::lower_bound(rules,grid3::minNum(n),{},&Rule::lhs);
    }
};

template<int N>
void out(std::bitset<N*N> output) {
    std::string a = output.to_string('.','#');
    std::ranges::reverse(a);
    std::cout << '\n';
    for(int i = 0; i < N; ++i) {
        std::cout << a.substr(i*N,N) << '\n';
    }
}

auto parse(std::string_view input) {
    std::array<int,16> small{};
    bool seen2s = false;
    std::vector<Rule> rules;
    split(input,'\n',[&](std::string_view line) {
        auto [in,out] = split_known<2>(line,'=',[](auto s) {return s;});
        in.remove_suffix(1);
        out.remove_prefix(2);
        if(in.size() == 5) {
            small[grid2::minNum(to_int(in))] = to_int(out);
        } else {
            if(!seen2s) {
                for(int i = 0; i < 16; ++i) {
                    small[i] = small[grid2::minNum(i)];
                }
                seen2s = true;
            }
            rules.emplace_back(to_int(in),to_int(out), small);
        }
    });
    std::ranges::sort(rules,{},&Rule::lhs);
    return RuleSet{small,std::move(rules)};
}


auto part1(const RuleSet& rules) {
    int start = grid3::minNum(to_int(".#./..#/###"));
    auto& startRule = rules.find(start);

    //step 1: rule->rhs
    //step 2: rule->rhs2
    //step 3: small[rule->rhs2]
    //step 4: small[rule->rhs2]->rhs
    //step 5: small[rule->rhs2]->rhs2
    auto step2 = startRule.rhs2;
    
    std::array<int,step2.size()> step3;
    std::ranges::transform(step2,step3.begin(),[&](int n) {return rules.small[n];});
    
    std::array<int,step2.size()> step5;
    std::ranges::transform(step3,step5.begin(),[&rules](int n) {
        auto s5 = rules.find(n).rhs2;
        return std::accumulate(s5.begin(),s5.end(),0,[](auto acc, int s) {return acc + std::bitset<4>(s).count();});
    });

    return std::accumulate(step5.begin(),step5.end(),0);
}

void solution(std::string_view input) {
    auto rules = parse(input);
    std::cout << "Part 1: " << part1(rules) << '\n';
}

std::string_view input = R"(../.. => .##/..#/##.
#./.. => ##./#../#..
##/.. => ###/#.#/..#
.#/#. => .../#../##.
##/#. => ###/#../###
##/## => .##/.##/#.#
.../.../... => #.##/#.##/###./..##
#../.../... => ##.#/..##/#.#./##.#
.#./.../... => ###./.#.#/.#../.###
##./.../... => ##.#/###./..../##..
#.#/.../... => ##.#/.###/.##./#.#.
###/.../... => #..#/.##./#.../.#.#
.#./#../... => .##./####/#..#/###.
##./#../... => ##../..#./#.##/..##
..#/#../... => #.##/.#.#/##../..##
#.#/#../... => #.../##../..#./.##.
.##/#../... => #.#./.#.#/#.##/#..#
###/#../... => .#../.#../...#/##..
.../.#./... => ..#./..#./##../.#.#
#../.#./... => ##../####/##../.###
.#./.#./... => ..../#..#/#.#./....
##./.#./... => ..##/####/..../##..
#.#/.#./... => #.##/##../#.../..#.
###/.#./... => ..../..../####/#..#
.#./##./... => ..../####/##.#/....
##./##./... => ####/#.../.###/#.##
..#/##./... => .#.#/.#../###./.#..
#.#/##./... => .#.#/###./..../..##
.##/##./... => #.../.#.#/.#.#/...#
###/##./... => #.##/.#../.#../#...
.../#.#/... => ###./..#./.#../..##
#../#.#/... => #..#/#.##/.#../...#
.#./#.#/... => ####/..#./..../..#.
##./#.#/... => #.#./..../.###/..#.
#.#/#.#/... => #..#/.#../#.#./.###
###/#.#/... => .##./#..#/.#.#/..#.
.../###/... => .#../#..#/...#/.##.
#../###/... => .##./##../###./##.#
.#./###/... => ...#/..##/###./...#
##./###/... => .#.#/##.#/.###/.#..
#.#/###/... => #.#./##../#.#./..#.
###/###/... => .#.#/####/###./####
..#/.../#.. => .#../#.##/..../..#.
#.#/.../#.. => ..../.#.#/##../#..#
.##/.../#.. => #.##/.#.#/#..#/.#.#
###/.../#.. => #..#/.#.#/#.#./##.#
.##/#../#.. => ##../##.#/##.#/#..#
###/#../#.. => ..../#..#/###./#.##
..#/.#./#.. => ..../.#../..../.##.
#.#/.#./#.. => #..#/#.##/.###/....
.##/.#./#.. => ###./..../##.#/#.#.
###/.#./#.. => #.../###./.#.#/..#.
.##/##./#.. => ..../.#../..#./#.#.
###/##./#.. => ...#/.###/###./####
#../..#/#.. => ..../.##./..##/..##
.#./..#/#.. => .#.#/#.../#..#/###.
##./..#/#.. => #.#./.##./.##./....
#.#/..#/#.. => #..#/..##/##.#/##..
.##/..#/#.. => ..#./#.../.##./##.#
###/..#/#.. => ##../.##./####/.##.
#../#.#/#.. => ###./#.#./###./.#.#
.#./#.#/#.. => .##./#.#./#..#/..#.
##./#.#/#.. => .#.#/#.#./#.../##.#
..#/#.#/#.. => .##./##.#/.#.#/.#.#
#.#/#.#/#.. => .#../.##./###./#...
.##/#.#/#.. => ####/##../.##./##.#
###/#.#/#.. => ###./.##./##.#/#...
#../.##/#.. => ...#/#.#./..##/####
.#./.##/#.. => #.../##.#/.##./###.
##./.##/#.. => ##.#/.#.#/..../#.#.
#.#/.##/#.. => ..../#.../.#.#/..#.
.##/.##/#.. => ##../..../..#./#.##
###/.##/#.. => ..#./...#/#..#/...#
#../###/#.. => ..../.#../#.../###.
.#./###/#.. => ..../#.#./.#.#/...#
##./###/#.. => ###./###./..#./.###
..#/###/#.. => #.##/..#./..##/#...
#.#/###/#.. => ##.#/.#.#/##../#..#
.##/###/#.. => ###./..##/#.../....
###/###/#.. => .###/###./#.../..#.
.#./#.#/.#. => ..##/##.#/.##./####
##./#.#/.#. => ..../.#.#/#.../###.
#.#/#.#/.#. => ##.#/###./..#./.#..
###/#.#/.#. => .###/##../.###/....
.#./###/.#. => ####/.###/.###/....
##./###/.#. => #.#./#..#/#..#/###.
#.#/###/.#. => #.#./.#.#/#.##/####
###/###/.#. => #.#./.###/..#./#.#.
#.#/..#/##. => ###./.#.#/##../##..
###/..#/##. => #.../.###/#.../..#.
.##/#.#/##. => #..#/.#.#/...#/.#..
###/#.#/##. => ...#/###./..##/.#.#
#.#/.##/##. => ###./...#/..../#...
###/.##/##. => ...#/#.../#.##/##..
.##/###/##. => .###/.###/..#./#...
###/###/##. => #.../##../##.#/.###
#.#/.../#.# => ##../#.##/..#./.###
###/.../#.# => #.#./.##./.##./#..#
###/#../#.# => #.../##../####/..##
#.#/.#./#.# => #.../.#../#.../..##
###/.#./#.# => #..#/###./####/#...
###/##./#.# => ##../..##/#.#./##..
#.#/#.#/#.# => .#../.#.#/#.#./.#.#
###/#.#/#.# => ..##/####/####/.###
#.#/###/#.# => .###/##../#..#/..#.
###/###/#.# => ##../#.../##.#/##..
###/#.#/### => ###./...#/####/..#.
###/###/### => .##./##../..../..#.)";

int main() {
    solution(input);
}
