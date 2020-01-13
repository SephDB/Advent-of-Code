#include <iostream>
#include <bitset>
#include <memory>
#include <algorithm>

template<typename Curve>
int next(Curve& a, int current) {
    auto next_size = current*2+1;
    if(next_size > a.size()) next_size = a.size();

    for(int i = 0; i < next_size - current - 1; ++i) {
        a[current+i+1] = ~a[current-i-1];
    }

    return next_size;
}

template<typename Curve>
void output_checksum(Curve& c, int stride) {
    for(int i = 0; i < c.size(); i += stride) {
        std::cout << c[i];
    }
    std::cout << '\n';
}
/* 110010110100110010110100
1: .1.1.0.1.0.1.1.1.0.1.0.1 ~c^(c>>1), stride 2
2: ...1...0...0...1...0...0 ~c^(c>>2), stride 4
3: .......0.......0.......1 ~c^(c>>4), stride 8
*/
template<typename Curve>
int checksum(Curve& c) {
    int stride = 1;
    //Copy bc it's too expensive to keep on the stack
    auto c_copy = std::make_unique<Curve>(c);
    auto& c2 = *c_copy;
    do {
        c2 >>= stride;
        c ^= c2;
        c.flip();
        c2 = c;
        stride *= 2;
    } while(c.size() / stride % 2 == 0);
    return stride;
}

template<int N>
void solve(std::string_view input) {
    auto sp = std::make_unique<std::bitset<N>>(input.data());
    auto& start = *sp;
    auto s = input.size();
    while(s < start.size()) {
        s = next(start,s);
    }
    auto stride = checksum(start);
    output_checksum(start,stride);
}

int main() {
    std::string input = "10001001100000001";
    std::reverse(input.begin(),input.end());
    std::cout << "Part 1: ";
    solve<272>(input);
    std::cout << "Part 2: ";
    solve<35651584>(input);
}
