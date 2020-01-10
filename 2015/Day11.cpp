#include <iostream>
#include <string>
#include <string_view>
#include <algorithm>

bool two_pairs(std::string_view p) {
    auto a = std::adjacent_find(p.begin(),p.end());
    return a < p.end()-3 and std::adjacent_find(a+2,p.end()) != p.end();
}

bool incrementing_triple(std::string_view p) {
    for(int i = 2; i < p.size(); ++i) {
        if(p[i] == p[i-1]+1 and p[i-1] == p[i-2]+1) return true;
    }
    return false;
}

bool valid(std::string_view p) {
    return two_pairs(p) and incrementing_triple(p) and p.find_first_of("iol") == p.npos;
}

void inc(std::string& s) {
    auto index = s.size()-1;
    while(s[index] == 'z') s[index--] = 'a';
    s[index]++;
}

int main() {
    std::string start = "hxbxwxba";
    
    while(not valid(start)) inc(start);
    std::cout << "Part 1: " << start << '\n';
    inc(start);
    while(not valid(start)) inc(start);
    std::cout << "Part 2: " << start << '\n';
}
