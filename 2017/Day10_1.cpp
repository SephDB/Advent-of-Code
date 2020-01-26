#include <iostream>
#include <array>
#include <numeric>

constexpr std::array input{199,0,255,136,174,254,227,16,51,85,1,2,22,17,7,192};

int main() {
    std::array<int,256> nums;
    std::iota(nums.begin(),nums.end(),0);
    int pos = 0, skip = 0;
    for(auto length : input) {
        auto end_pos = pos+length-1;
        auto current = pos;
        while(current < end_pos) {
            std::swap(nums[current++%nums.size()],nums[end_pos--%nums.size()]);
        }
        pos += length + skip++;
    }
    std::cout << "Part 1: " << nums[0]*nums[1] << '\n';
}
