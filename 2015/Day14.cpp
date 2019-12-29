#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <unordered_map>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <span>

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

struct Reindeer {
    std::string_view name;
    int speed;
    int time;
    int rest_time;
};

auto parse(std::string_view input) {
    std::vector<Reindeer> ret;
    for(auto l : split(input,'\n')) {
        auto comp = split(l,' ');
        auto name = comp[0];
        auto speed = to_int(comp[3]);
        auto time = to_int(comp[6]);
        auto rest = to_int(comp[comp.size()-2]);
        ret.push_back({name,speed,time,rest});
        std::cout << name << ' ' << speed << ' ' << time << ' ' << rest << '\n';
    }
    return ret;
}

int part1(Reindeer r, int T) {
    return T/(r.rest_time+r.time)*r.speed*r.time + std::min(T % (r.rest_time+r.time),r.time)*r.speed;
}

void solution(std::string_view input) {
    auto reindeer = parse(input);
    int max = 0;
    for(auto r: reindeer) {
        max = std::max(part1(r,2503),max);
    }
    std::cout << "Part 1: " << max << '\n';
    //Silly brute-force
    std::vector<int> scores(reindeer.size());
    for(int i = 1; i <= 2503; ++i) {
        auto pos = std::max_element(reindeer.begin(),reindeer.end(),[i](auto& a, auto& b){return part1(a,i) < part1(b,i);}) - reindeer.begin();
        scores[pos]++;
    }
    std::cout << "Part 2: " << *std::max_element(scores.begin(),scores.end()) << '\n';
}

std::string_view input = R"(Vixen can fly 8 km/s for 8 seconds, but then must rest for 53 seconds.
Blitzen can fly 13 km/s for 4 seconds, but then must rest for 49 seconds.
Rudolph can fly 20 km/s for 7 seconds, but then must rest for 132 seconds.
Cupid can fly 12 km/s for 4 seconds, but then must rest for 43 seconds.
Donner can fly 9 km/s for 5 seconds, but then must rest for 38 seconds.
Dasher can fly 10 km/s for 4 seconds, but then must rest for 37 seconds.
Comet can fly 3 km/s for 37 seconds, but then must rest for 76 seconds.
Prancer can fly 9 km/s for 12 seconds, but then must rest for 97 seconds.
Dancer can fly 37 km/s for 1 seconds, but then must rest for 36 seconds.)";

int main() {
    solution(input);
}
