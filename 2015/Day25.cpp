#include <iostream>

std::uint64_t triangle(std::uint64_t n) {
    return n*(n+1)/2;
}

std::uint64_t n(std::uint64_t i, std::uint64_t j) {
    std::uint64_t ret = triangle(j);
    ret += triangle(i+j-2) - triangle(j-1);
    return ret;
}

std::uint64_t fast_exp(std::uint64_t base, std::uint64_t exp, std::uint64_t mod) {
    std::uint64_t res=1;
    while(exp>0) {
       if(exp%2==1) res=(res*base)%mod;
       base=(base*base)%mod;
       exp/=2;
    }
    return res;
}

int main() {
    for(int i = 1; i < 10; ++i) {
        for(int j = 1; j < 10; ++j) {
            std::cout << n(i,j) << ' ';
        }
        std::cout << '\n';
    }
    constexpr auto mod = 33554393;

    std::cout << "Solution: " << (20151125*fast_exp(252533,n(3010,3019)-1,mod)) % mod;
}
