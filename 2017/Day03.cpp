#include <array>
#include <iostream>
#include <cmath>
#include <numeric>
#include <deque>

auto distance(int i) {
    //Numbers on the diagonals are of the form n*(n-1)+1
    //Finding the n of i, rounded down, gives us the last diagonal number
    //this will be at coordinates {n/2,n/2}(*-1 depending on n%2 but we don't care)
    //The first n numbers have the y coordinate move straight up or down, the next n move x
    int n = (1 + std::sqrt(1+4*(i-1)))/2;
    int x = n/2;
    int y = n/2;
    int start = n*(n-1)+1;
    if(i-start <= n) {
        y -= (i-start);
    } else {
        y -= n;
        x -= (i-start-n);
    }
    return std::abs(x) + std::abs(y);
}

auto sums(int input) {
    //Example starting at position 26, which locations to sum together
    /*
    26=10,25
    27=10,11,25,26
    28=10,11,12,27
    29=11,12,13,28
    30=12,13,29
    31=13,30
    32=13,14,30,31
    33=13,14,15,32
    34=14,15,16,33
    35=15,16,17,34
    36=16,17,35
    37=17,36
    38=17,18,36,37
    ...
    */
    std::deque<int> nums{1,1,2,4,5,10,11};
    for(int iteration = 6; ; ++iteration) {
        nums.push_back(nums.back()*2+nums[1]);
        if(nums.back() >= input) return nums.back();

        int len = iteration/2 - 1;
        for(int len = iteration/2 - 1; len > 0; --len) {
            auto next = std::accumulate(nums.begin(),nums.begin()+std::min(3,len),nums.back(),std::plus{});
            if(next >= input) return next;
            nums.push_back(next);
            if(len > 1) nums.pop_front();
        }
    }
}

int main() {
    constexpr auto input = 277678;
    std::cout << "Part 1: " << distance(input) << '\n';
    std::cout << "Part 2: " << sums(input) << '\n';
}
