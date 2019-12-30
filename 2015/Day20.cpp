#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

std::pair<int,int> sum_of_dividing_powers(int current, int p) {
    //1 + p + p^2 +... until p^n doesn't divide current
    auto pcopy = p;
    while(current % p == 0) {
        p *= pcopy;
    }
    return {(p-1)/(pcopy-1),current / (p/pcopy)};
}

int part1(int needed) {
    //Sum of factors is (p1^0 + p1^1 + p1^2 + ...)*(p2^0 + p2^1 + p2^2 + ...)*... for all prime factors and their powers.
    //If we find a dividing prime, we can divide away one of the prime factors completely, then take the sum of factors of the number that's left
    //And multiply it with the sum of the remaining prime's powers.
    
    needed /= 10; //Every elf delivers 10 times its number, so dividing everything by 10 makes numbers smaller :p
    std::vector<int> primes{2};
    std::vector<int> totals{0,1,3};
    totals.reserve(needed/2);
    auto t = [&](auto c) {
        //maybe go from largest to smallest prime?
        for(auto p : primes) {
            if(p*p > c) break;
            if(c % p == 0) {
                auto [newt,newc] = sum_of_dividing_powers(c,p);
                return totals[newc]*newt;
            }
        }
        primes.push_back(c);
        return c+1;
    };
    int current = 2;
    while(true) {
        int total = t(++current);
        if(total >= needed) break;
        totals.push_back(total);
    }
    return current;
}

int part2(int needed) {
    //I have no idea how to do this in a mathy way like part 1
    needed /= 11;
    std::vector<int> houses(needed);

    for(int elf = 2; ; ++elf) {
        if(houses[elf]+elf >= needed) return elf;
        for(int i = 1; i <= 50; ++i) {
            if(elf*i >= needed) break;
            houses[elf*i] += elf;
        }
    }
}

int main() {
    std::cout << part1(33100000) << '\n';
    std::cout << part2(33100000) << '\n';
}
