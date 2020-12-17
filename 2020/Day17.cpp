#include <iostream>
#include <string_view>
#include <array>
#include <map>
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

template<int N>
struct Coord {
    std::array<int,N> coords;
    int& operator[](int n) {
        return coords[N-1-n];
    }
    bool operator<(Coord other) const {
        auto [mis1,mis2] = std::ranges::mismatch(coords,other.coords);
        if(mis1 == coords.end()) return false;
        return *mis1 < *mis2;
    }
    Coord operator+(Coord other) const {
        Coord out;
        std::ranges::transform(coords,other.coords,out.coords.begin(),std::plus<>{});
        return out;
    }
};

template<int N>
constexpr size_t dim() {
    size_t ret = 1;
    for(int i = 0; i < N; ++i) ret *= 3;
    return ret;
}

template<int N>
constexpr auto neighbor_diff() {
    std::array<Coord<N>,dim<N>()-1> neighbors;
    for(size_t i = 0; i < dim<N>(); ++i) {
        if(i == dim<N>()/2) continue;
        auto index = i - (i>dim<N>()/2);
        auto current = i;
        for(auto& dim : neighbors[index].coords) {
            dim = current % 3 - 1;
            current /= 3;
        }
    }
    return neighbors;
}

template<int N>
auto parse(std::string_view input) {
    std::vector<Coord<N>> active;
    Coord<N> current;
    std::ranges::fill(current.coords,0);
    int y = 0;
    split(input,'\n',[&](std::string_view line) {
        current[1] = y++;
        for(int x = 0; x < line.size(); ++x) {
            if(line[x] == '#') {
                current[0] = x;
                active.push_back(current);
            }
        }
    });
    return active;
}

struct neighbor_state {
    int active_neighbors;
    bool active;
};

template<int N>
auto solve(std::vector<Coord<N>> input, int iterations) {
    constexpr auto neighbor_d = neighbor_diff<N>();
    for(int i = 0; i < iterations; ++i) {
        std::map<Coord<N>,neighbor_state> neighbors;
        for(auto c : input) {
            neighbors[c].active = true;
            for(auto n : neighbor_d) {
                neighbors[c+n].active_neighbors++;
            }
        }
        input.clear();
        for(auto [c,s] : neighbors) {
            if(s.active_neighbors == 3 || (s.active && s.active_neighbors == 2)) {
                input.push_back(c);
            }
        }
    }
    return input.size();
}

void solution(std::string_view input) {
    std::cout << "Part 1: " << solve(parse<3>(input),6) << '\n';
    std::cout << "Part 2: " << solve(parse<4>(input),6) << '\n';
}

std::string_view input = R"(...#..#.
..##.##.
..#.....
....#...
#.##...#
####..##
...##.#.
#.#.#...)";

int main() {
    solution(input);
}
