#include <array>
#include <iostream>
#include <cmath>

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

int main() {
    std::cout << "Part 1: " << distance(277678) << '\n';
}
