#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <array>
#include <bitset>
#include <charconv>
#include <numeric>
#include <algorithm>

uint64_t htonll(uint64_t value) {
    int num = 42;
    if (*(char *)&num == 42) {
        uint32_t high_part = htonl((uint32_t)(value >> 32));
        uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));
        return (((uint64_t)low_part) << 32) | high_part;
    } else {
        return value;
    }
}

auto knot(const std::string& s) {
    std::array<unsigned char,256> nums;
    std::iota(nums.begin(),nums.end(),0);
    int pos = 0, skip = 0;
    for(int i = 0; i < 64; ++i) {
        for(auto length : s) {
            auto end_pos = pos+length-1;
            auto current = pos;
            while(current < end_pos) {
                std::swap(nums[current++%nums.size()],nums[end_pos--%nums.size()]);
            }
            pos += length + skip++;
            pos %= nums.size();
        }
    }

    std::array<unsigned char,16> blocks;
    std::fill(blocks.begin(),blocks.end(),0);
    for(int i = 0; i < 16; ++i) {
        for(int j = 0; j < 16; ++j) {
            blocks[i] ^= nums[i*16+j];
        }
    }
    return std::bitset<128>(htonll(*reinterpret_cast<uint64_t*>(blocks.begin()))) << 64 
         | std::bitset<128>(htonll(*reinterpret_cast<std::uint64_t*>(blocks.begin()+8)));
}

struct DisjointSets {
    struct Set {
        std::size_t parent;
        std::size_t size;
    };
    std::vector<Set> sets;
    std::size_t total_sets = 0;
    std::size_t newSet() {
        sets.push_back({sets.size(),1});
        total_sets++;
        return sets.back().parent;
    }
    //Path compression
    Set findRoot(std::size_t n) {
        auto root = n;
        while(root != sets[root].parent) {
            root = sets[root].parent;
        }
        //Set all intermediate node's parent to root
        while(n != sets[n].parent) {
            auto p = sets[n].parent;
            sets[n].parent = root;
            n = p;
        }
        return sets[root];
    }
    std::size_t merge(std::size_t x, std::size_t y) {
        auto xRoot = findRoot(x);
        auto yRoot = findRoot(y);
        
        if(xRoot.parent == yRoot.parent) return xRoot.parent;

        if(xRoot.size < yRoot.size) std::swap(xRoot,yRoot);
        sets[yRoot.parent].parent = xRoot.parent;
        sets[xRoot.parent].size += yRoot.size;
        total_sets--;
        return xRoot.parent;
    }
};

int main() {
    std::string input = "jzgqcdpd";
    input.push_back('-');
    auto hash = [&input](int i) {
        return knot(input + std::to_string(i) + std::string{17,31,73,47,23});
    };
    std::size_t total = 0;

    DisjointSets sets;
    std::bitset<128> above(0);
    std::array<std::size_t,128> current_sets{0};
    for(int i = 0; i < 128; ++i) {
        auto current = hash(i);
        total += current.count();

        auto left = current << 1;
        
        for(int bit = 0; bit < 128; ++bit) {
            bool l = left.test(bit), a = above.test(bit);
            if(current.test(bit)) {
                if(not(l or a)) {
                    current_sets[bit] = sets.newSet();
                } else if(l and not a) {
                    current_sets[bit] = current_sets[bit-1]; //Only above set is already done by leaving current_sets as it is
                } else if(a and l) {
                    current_sets[bit] = sets.merge(current_sets[bit-1],current_sets[bit]);
                }
            }
        }

        above = current;
    }
    std::cout << "Part 1: " << total << '\n';
    std::cout << "Part 2: " << sets.total_sets << '\n';
}
