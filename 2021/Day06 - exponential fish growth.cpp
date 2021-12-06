#include <iostream>
#include <array>
#include <numeric>
#include <utility>

struct Fish {
    std::array<int64_t,7> fish{};
    int zero_index = 0;

    int64_t new_fish = 0;
    int64_t next_fish = 0;

    Fish(auto ages) {
        for(auto i : ages) {
            fish[i]++;
        }
    }

    void advance() {
        int64_t created = fish[zero_index];
        zero_index = (zero_index + 1)%7;
        fish[(zero_index+6)%7] += std::exchange(next_fish,new_fish);
        new_fish = created;
    }

    void print() {
        for(int i = 0; i < 7; ++i) {
            std::cout << '(' << i << ':' << fish[(zero_index+i)%7] << "),";
        }
        std::cout << "(7:" << next_fish << "),(8:" << new_fish << ")\n";
    }

    int64_t total() {
        return std::accumulate(fish.begin(),fish.end(),int64_t{0}) + new_fish + next_fish;
    }
};

int main() {
    std::array input_test = {3,4,3,1,2};
    std::array input_real = {4,3,3,5,4,1,2,1,3,1,1,1,1,1,2,4,1,3,3,1,1,1,1,2,3,1,1,1,4,1,1,2,1,2,2,1,1,1,1,1,5,1,1,2,1,1,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,5,1,4,2,1,1,2,1,3,1,1,2,2,1,1,1,1,1,1,1,1,1,1,4,1,3,2,2,3,1,1,1,4,1,1,1,1,5,1,1,1,5,1,1,3,1,1,2,4,1,1,3,2,4,1,1,1,1,1,5,5,1,1,1,1,1,1,4,1,1,1,3,2,1,1,5,1,1,1,1,1,1,1,5,4,1,5,1,3,4,1,1,1,1,2,1,2,1,1,1,2,2,1,2,3,5,1,1,1,1,3,5,1,1,1,2,1,1,4,1,1,5,1,4,1,2,1,3,1,5,1,4,3,1,3,2,1,1,1,2,2,1,1,1,1,4,5,1,1,1,1,1,3,1,3,4,1,1,4,1,1,3,1,3,1,1,4,5,4,3,2,5,1,1,1,1,1,1,2,1,5,2,5,3,1,1,1,1,1,3,1,1,1,1,5,1,2,1,2,1,1,1,1,2,1,1,1,1,1,1,1,3,3,1,1,5,1,3,5,5,1,1,1,2,1,2,1,5,1,1,1,1,2,1,1,1,2,1};
    Fish fishes(input_real);
    for(int i = 0; i < 256; ++i) {
        fishes.advance();
    }
    fishes.print();
    std::cout << "Part 2: " << fishes.total() << '\n';
}
