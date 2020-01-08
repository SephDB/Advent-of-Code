#include <iostream>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <array>
#include <charconv>

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

template<int N>
auto split_known(std::string_view in, char delim) {
    return split_known<N>(in,delim,[](auto in){return in;});
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

template<std::size_t N>
auto rotate(std::bitset<N> in, int num) {
    if(num > N) num %= N;
    return (in << num) | (in >> (N-num));
}

void solution(std::string_view input) {
    auto lines = split(input,'\n');
    using row = std::bitset<50>;
    using column = std::bitset<6>;
    std::array<row,column{}.size()> screen;

    for(auto l: lines) {
        if(l.starts_with("rect")) {
            auto [w,h] = split_known<2>(l.substr(sizeof("rect")),'x',to_int);
            row insert{(1 << w) - 1};
            for(int i = 0; i < h; ++i) {
                screen[i] |= insert;
            }
        } else {
            auto [rot,direction,which,by,num] = split_known<5>(l,' ');
            auto which_int = to_int(which.substr(2));
            auto num_int = to_int(num);
            if(direction == "row") {
                screen[which_int] = rotate(screen[which_int],num_int);
            } else {
                column c{};
                for(int r = 0; r < c.size(); ++r) {
                    c.set(r,screen[r].test(which_int));
                }
                c = rotate(c,num_int);
                for(int r = 0; r < c.size(); ++r) {
                    screen[r].set(which_int,c.test(r));
                }
            }
        }
    }

    std::cout << "Part 1: " << std::accumulate(screen.begin(),screen.end(),0,[](auto acc, auto row) {return acc + row.count();}) << '\n';
    for(auto r : screen) {
        for(int x = 0; x < r.size(); ++x) {
            if(x % 5 == 0) std::cout << ' ';
            std::cout << (r.test(x) ? '#' : '.');
        }
        std::cout << '\n';
    }
}

std::string_view input = R"(rect 1x1
rotate row y=0 by 2
rect 1x1
rotate row y=0 by 5
rect 1x1
rotate row y=0 by 3
rect 1x1
rotate row y=0 by 3
rect 2x1
rotate row y=0 by 5
rect 1x1
rotate row y=0 by 5
rect 4x1
rotate row y=0 by 2
rect 1x1
rotate row y=0 by 2
rect 1x1
rotate row y=0 by 5
rect 4x1
rotate row y=0 by 3
rect 2x1
rotate row y=0 by 5
rect 4x1
rotate row y=0 by 2
rect 1x2
rotate row y=1 by 6
rotate row y=0 by 2
rect 1x2
rotate column x=32 by 1
rotate column x=23 by 1
rotate column x=13 by 1
rotate row y=0 by 6
rotate column x=0 by 1
rect 5x1
rotate row y=0 by 2
rotate column x=30 by 1
rotate row y=1 by 20
rotate row y=0 by 18
rotate column x=13 by 1
rotate column x=10 by 1
rotate column x=7 by 1
rotate column x=2 by 1
rotate column x=0 by 1
rect 17x1
rotate column x=16 by 3
rotate row y=3 by 7
rotate row y=0 by 5
rotate column x=2 by 1
rotate column x=0 by 1
rect 4x1
rotate column x=28 by 1
rotate row y=1 by 24
rotate row y=0 by 21
rotate column x=19 by 1
rotate column x=17 by 1
rotate column x=16 by 1
rotate column x=14 by 1
rotate column x=12 by 2
rotate column x=11 by 1
rotate column x=9 by 1
rotate column x=8 by 1
rotate column x=7 by 1
rotate column x=6 by 1
rotate column x=4 by 1
rotate column x=2 by 1
rotate column x=0 by 1
rect 20x1
rotate column x=47 by 1
rotate column x=40 by 2
rotate column x=35 by 2
rotate column x=30 by 2
rotate column x=10 by 3
rotate column x=5 by 3
rotate row y=4 by 20
rotate row y=3 by 10
rotate row y=2 by 20
rotate row y=1 by 16
rotate row y=0 by 9
rotate column x=7 by 2
rotate column x=5 by 2
rotate column x=3 by 2
rotate column x=0 by 2
rect 9x2
rotate column x=22 by 2
rotate row y=3 by 40
rotate row y=1 by 20
rotate row y=0 by 20
rotate column x=18 by 1
rotate column x=17 by 2
rotate column x=16 by 1
rotate column x=15 by 2
rotate column x=13 by 1
rotate column x=12 by 1
rotate column x=11 by 1
rotate column x=10 by 1
rotate column x=8 by 3
rotate column x=7 by 1
rotate column x=6 by 1
rotate column x=5 by 1
rotate column x=3 by 1
rotate column x=2 by 1
rotate column x=1 by 1
rotate column x=0 by 1
rect 19x1
rotate column x=44 by 2
rotate column x=40 by 3
rotate column x=29 by 1
rotate column x=27 by 2
rotate column x=25 by 5
rotate column x=24 by 2
rotate column x=22 by 2
rotate column x=20 by 5
rotate column x=14 by 3
rotate column x=12 by 2
rotate column x=10 by 4
rotate column x=9 by 3
rotate column x=7 by 3
rotate column x=3 by 5
rotate column x=2 by 2
rotate row y=5 by 10
rotate row y=4 by 8
rotate row y=3 by 8
rotate row y=2 by 48
rotate row y=1 by 47
rotate row y=0 by 40
rotate column x=47 by 5
rotate column x=46 by 5
rotate column x=45 by 4
rotate column x=43 by 2
rotate column x=42 by 3
rotate column x=41 by 2
rotate column x=38 by 5
rotate column x=37 by 5
rotate column x=36 by 5
rotate column x=33 by 1
rotate column x=28 by 1
rotate column x=27 by 5
rotate column x=26 by 5
rotate column x=25 by 1
rotate column x=23 by 5
rotate column x=22 by 1
rotate column x=21 by 2
rotate column x=18 by 1
rotate column x=17 by 3
rotate column x=12 by 2
rotate column x=11 by 2
rotate column x=7 by 5
rotate column x=6 by 5
rotate column x=5 by 4
rotate column x=3 by 5
rotate column x=2 by 5
rotate column x=1 by 3
rotate column x=0 by 4)";

int main() {
    solution(input);
}
