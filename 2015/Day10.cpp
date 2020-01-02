#include <iostream>
#include <string>
#include <algorithm>

int next_length(const std::string& s) {
    int ret = 1;
    std::adjacent_find(s.begin(),s.end(),[&](char a, char b) {ret += (a != b); return false;});
    return ret*2;
}

std::string next(const std::string& s) {
    std::string ret;
    int count = 1;
    char current = s[0];
    for(int i = 1; i < s.size(); ++i) {
        if(s[i] == current) {
            ++count;
        } else {
            ret.push_back('0'+count);
            ret.push_back(current);
            count = 1;
            current = s[i];
        }
    }
    ret.push_back('0'+count);
    ret.push_back(current);
    return ret;
}

auto apply(std::string number, int iterations) {
    for(int i = 0; i < iterations-1; i++) {
        number = next(number);
    }
    return next_length(number);
}

int main() {
    std::cout << "Part 1: " << apply("1113122113",40) << '\n';
    std::cout << "Part 2: " << apply("1113122113",50) << '\n';
}
