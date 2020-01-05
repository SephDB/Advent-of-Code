#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

struct Entanglement {
    int num_items = 0;
    std::uint64_t entanglement = 1;
    Entanglement add(int n) {
        return {num_items+1,entanglement*n};
    }
    bool operator<(Entanglement o) const {
        return std::tie(num_items,entanglement) < std::tie(o.num_items,o.entanglement);
    }
    static Entanglement max() {
        return {std::numeric_limits<int>::max(),0};
    }
};

template<std::size_t N>
Entanglement min_partition(const std::array<int,N>& in, int goal, int current = 0, Entanglement so_far = {}, Entanglement current_min = Entanglement::max()) {
    if(current == N) return Entanglement::max();

    Entanglement next = so_far.add(in[current]);
    if(next < current_min) {
        int next_goal = goal - in[current];
        if(next_goal == 0) return next;
        current_min = std::min(current_min,min_partition(in,next_goal,current+1,next,current_min));
    }

    current_min = std::min(current_min,min_partition(in,goal,current+1,so_far,current_min));

    return current_min;
}

int main() {
    std::array input = {1,2,3,7,11,13,17,19,23,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113};
    auto total = std::accumulate(input.begin(),input.end(),0);
    std::reverse(input.begin(),input.end());
    std::cout << "Part 1: " << min_partition(input,total/3).entanglement << '\n';
    std::cout << "Part 2: " << min_partition(input,total/4).entanglement << '\n';
}
