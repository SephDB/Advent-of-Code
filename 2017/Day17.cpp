#include <iostream>
#include <vector>

int main() {
    constexpr int input = 394;

    std::vector<int> buffer;
    buffer.reserve(2018);
    buffer.push_back(0);
    int loc = 0;
    for(int i = 1; i <= 2017; ++i) {
        loc += input;
        loc %= buffer.size();
        buffer.insert(buffer.begin()+loc+1,i);
        loc++;
    }
    std::cout << "Part 1: " << buffer[(loc+1) % buffer.size()] << '\n';
    
    loc = 0;
    int size = 1;
    int ret = 1;
    for(int i = 1; i <= 50'000'000; ++i) {
        loc += input;
        loc %= size++;
        if(loc == 0) ret = i;
        loc++;
    }
    std::cout << "Part 2: " << ret << '\n';
}
