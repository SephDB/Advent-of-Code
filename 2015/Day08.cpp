#include <string_view>
#include <iostream>
#include "Day08-raw.h"

/*
//Basic idea shown here: Use a raw string literal and a regular string literal version of the same input to make C++'s parser do the work for us
std::string_view raw = R"(""
"abc"
"aaa\"aaa"
"\x27")";
std::string_view escaped = ""
"abc"
"aaa\"aaa"
"\x27a";
*/

int main() {
    auto raw_size = raw.size() - std::count(raw.begin(),raw.end(),'\n');
    auto num_lines = std::count(raw.begin(),raw.end(),'\n')+1;
    std::cout << "Part 1: " << raw_size - escaped.size() << '\n';
    std::cout << "Part 2: " << 2*num_lines  //Add two "" in front and at the back of each line
        + std::count_if(raw.begin(),raw.end(),[](char c) {
            return c == '\\' or c == '"';
        });
}
