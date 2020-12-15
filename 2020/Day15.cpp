#include <iostream>
#include <memory>
#include <array>

int solve(auto input, int total) {
    std::unique_ptr<int[]> memory(new int[total]());
    
    int i = 0;
    for(auto in : input) {
        memory[in] = ++i;
    }
    auto num = 0;
    while((++i) < total) {
        auto& mem = memory[num];
        num = mem == 0 ? 0 : i-mem;
        mem = i;
    }
    return num;
}

int main() {
    std::array input{6,4,12,1,20,0,16};
    std::cout << "Part 1: " << solve(input,2020) << '\n';
    std::cout << "Part 2: " << solve(input,30'000'000) << '\n';
}
