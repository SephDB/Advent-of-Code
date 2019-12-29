#include <iostream>
#include <string_view>
#include <vector>
#include <charconv>
#include <numeric>
#include <algorithm>

std::vector<std::string_view> split(std::string_view in, char delim) {
    std::vector<std::string_view> ret;
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret.push_back(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    ret.push_back(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

auto parse(std::string_view input) {
    std::vector<int> ret;
    auto lines = split(input,'\n');
    std::transform(lines.begin(),lines.end(),back_inserter(ret),to_int);
    return ret;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::sort(in.begin(),in.end(),std::greater{});
    
    int min_num = in.size();
    int num_finds = 0;

    auto rec = [&](auto&& r, int index, int remaining, int max, int num = 0) -> int {
        if(remaining < 0) return 0;
        if(index == in.size()) return 0;
        if(max < remaining) return 0;
        if(max == remaining or remaining == 0) {
            if(max == remaining)
                num += in.size()-index;
            if(num < min_num) {
                min_num = num;
                num_finds = 1;
            } else if(num == min_num) {
                num_finds++;
            }
            return 1;
        }
        return r(r,index+1,remaining,max-in[index], num) + r(r,index+1,remaining-in[index],max-in[index], num+1);
    };
    std::cout << "Part 1: " << rec(rec,0,150,std::accumulate(in.begin(),in.end(),0)) << '\n';
    std::cout << "Part 2: " << num_finds << '\n';
}

std::string_view input = R"(11
30
47
31
32
36
3
1
5
3
32
36
15
11
46
26
28
1
19
3)";

int main() {
    solution(input);
}
