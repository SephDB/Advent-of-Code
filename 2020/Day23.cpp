#include <iostream>
#include <string>
#include <numeric>
#include <algorithm>
#include <memory_resource>
#include <list>

auto part1(std::string input) {
    auto next = [](char c) -> char {if(c == '1')return '9'; return c-1;};
    for(int i = 0; i < 100; ++i) {
        char current = next(input[0]);
        auto n = input.begin();
        while((n = std::ranges::find(input,current)) < input.begin()+4) current = next(current);
        std::rotate(input.begin()+1,input.begin()+4,n+1);
        std::rotate(input.begin(),input.begin()+1,input.end());
    }
    auto one = std::ranges::find(input,'1');
    std::rotate(input.begin(),one,input.end());
    return input.substr(1);
}

auto part2(std::string input) {
    std::pmr::monotonic_buffer_resource mem((sizeof(int)+2*sizeof(void*))*1'000'000);
    std::pmr::list<int> cups(&mem);
    std::array<decltype(cups.cbegin()),1'000'001> lookup;
    for(int i = 0; i < 9; ++i) {
        cups.push_back(input[i]-'0');
        lookup[input[i]-'0'] = std::prev(cups.cend());
    }
    for(int i = 10; i <= 1'000'000; ++i) {
        cups.push_back(i);
        lookup[i] = std::prev(cups.cend());
    }

    auto next = [&cups](auto it) {
        it = std::next(it);
        if(it == cups.cend()) return cups.cbegin();
        return it;
    };

    auto current = cups.cbegin();

    for(int i = 0; i < 10'000'000; ++i) {
        std::array<int,3> out;
        auto n = std::next(current);
        for(int i = 0; i < 3; ++i) {
            if(n == cups.cend()) {
                n = cups.cbegin();
                //move elements at the end to the beginning to make the splice later work well
                cups.splice(cups.cbegin(),cups,std::next(current),cups.cend());
            }
            out[i] = *n;
            ++n;
        }

        int insertion = *current - 1;
        if(insertion == 0) insertion = cups.size();
        while(std::ranges::find(out,insertion) != out.end()) {
            --insertion;
            if(insertion == 0) insertion = cups.size();
        }
        cups.splice(next(lookup[insertion]),cups,next(current),n);

        current = next(current);
    }
    current = lookup[1];
    ++current;

    return int64_t{*current} * int64_t{*next(current)};
}


int main() {
    std::cout << "Part 1: " << part1("872495136") << '\n';
    std::cout << "Part 2: " << part2("872495136") << '\n';
}
