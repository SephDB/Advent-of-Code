#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <boost/range/counting_range.hpp>

int part1_bruteforce() {

	const auto total_presents = 33100000;
	const auto n_presents = 10;
	const auto total_houses = (total_presents / n_presents);

	// array with 3 mil elements will smash the stack, so vector is used instead
	// every house has at least 10 presents because the first elf visits them all
	auto houses = std::vector(total_houses, n_presents);

	// elf_id is one-based, but house_id is zero-based because it's used to access the vector
	for(const auto elf_id : boost::counting_range(2, (total_houses + 1))) {

		for(auto house_id = (elf_id - 1); house_id < total_houses; house_id += elf_id) {

			houses[house_id] += (elf_id * n_presents);
		}
	}

	const auto lowest_house = std::find_if(houses.begin(), houses.end(), [total_presents] (auto house) {
		return (house >= total_presents);
	});

	// add 1 to account for zero-based indexing of vector
	return std::distance(houses.begin(), lowest_house) + 1;
}

template<typename F>
double time(F&& f, int num_tries=1) {
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < num_tries; ++i) {
        f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    return diff.count()/num_tries;
}

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
