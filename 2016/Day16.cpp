#include <iostream>
#include <bitset>
#include <algorithm>

using Curve = std::bitset<272>;

int next(Curve& a, int current) {
    auto next_size = current*2+1;
    if(next_size > a.size()) next_size = a.size();

    for(int i = 0; i < next_size - current - 1; ++i) {
        a[current+i+1] = ~a[current-i-1];
    }

    return next_size;
}

void output(Curve c, int size) {
    auto s = c.to_string();
    std::reverse(s.begin(),s.end());
    s = s.substr(0,size);
    std::cout << s << '\n';
}

void output_checksum(Curve c, int stride) {
    for(int i = 0; i < c.size(); i += stride) {
        std::cout << c[i];
    }
    std::cout << '\n';
}
/*
   110010110100110010110100
1: .1.1.0.1.0.1.1.1.0.1.0.1 ~c^(c>>1), stride 2
2: ...1...0...0...1...0...0 ~c^(c>>2), stride 4
3: .......0.......0.......1 ~c^(c>>4), stride 8
*/
int checksum(Curve& c) {
    int stride = 1;
    do {
        c = ~(c ^ (c >> stride));
        stride *= 2;
    } while(c.size() / stride % 2 == 0);
    return stride;
}

int main() {
    std::string input = "10001001100000001";
    std::reverse(input.begin(),input.end());
    Curve start(input.data());
    auto s = input.size();
    while(s < start.size()) {
        s = next(start,s);
    }
    std::cout << '\n';
    auto stride = checksum(start);
    std::cout << "Part 1: ";
    output_checksum(start,stride);
}
