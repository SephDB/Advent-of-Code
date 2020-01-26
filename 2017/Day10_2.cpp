#include <iostream>
#include <string>
#include <array>
#include <numeric>

std::string input = "199,0,255,136,174,254,227,16,51,85,1,2,22,17,7,192";

auto toBlocks(const std::array<int,256>& nums) {
    std::array<unsigned char,16> blocks;
    std::fill(blocks.begin(),blocks.end(),0);
    for(int i = 0; i < 16; ++i) {
        for(int j = 0; j < 16; ++j) {
            blocks[i] ^= nums[i*16+j];
        }
    }
    return blocks;
}

char toHex(unsigned char c) {
    if(c < 10) return '0'+c;
    else return 'a'+c-10;
}

int main() {
    std::array<int,256> nums;
    std::iota(nums.begin(),nums.end(),0);
    input += std::string{17, 31, 73, 47, 23};
    int pos = 0, skip = 0;
    for(int i = 0; i < 64; ++i) {
        for(auto length : input) {
            auto end_pos = pos+length-1;
            auto current = pos;
            while(current < end_pos) {
                std::swap(nums[current++%nums.size()],nums[end_pos--%nums.size()]);
            }
            pos += length + skip++;
            pos %= nums.size();
        }
    }
    std::cout << "Part 2: ";
    for(auto block : toBlocks(nums)) {
        std::cout << toHex(block >> 4) << toHex(block & 0xf);
    }
}
