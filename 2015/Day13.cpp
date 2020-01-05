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

auto triangle(std::size_t n) {
    return n*(n-1)/2;
}

template<typename T>
class Triangular {
public:
    Triangular(std::size_t size) : contents(triangle(size),0) {}

    std::span<T> operator[](std::size_t index) {
        return {&contents[triangle(index)],index};
    }

    std::span<const T> operator[](std::size_t index) const {
        return {&contents[triangle(index)],index};
    }

private:
    std::vector<T> contents;
};

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

struct Pairs {
    Pairs(std::size_t n) : total(n), content(n) {};

    int& operator[](std::pair<int,int> p) {
        return content[std::max(p.first,p.second)][std::min(p.first,p.second)];
    }

    int operator[](std::pair<int,int> p) const {
        return content[std::max(p.first,p.second)][std::min(p.first,p.second)];
    }

    std::size_t total;
    Triangular<int> content;
};

Pairs parse(std::string_view input) {
    std::map<std::pair<int,int>,int> pairs;
    int total = 0;
    auto lookup = [&,m=std::unordered_map<std::string_view,int>{}](std::string_view n) mutable {
        if(not m.contains(n)) {
            m[n] = total++;
        }
        return m.at(n);
    };
    for(auto l : split(input,'\n')) {
        auto comp = split(l,' ');
        comp.back().remove_suffix(1);
        pairs[{lookup(comp.front()),lookup(comp.back())}] = to_int(comp[3]) * (comp[2] == "lose" ? -1 : 1);
    }
    Pairs ps(total);
    for(auto [pair,val] : pairs) {
        ps[pair] += val;
    }
    return ps;
}

int happiness(const std::vector<int>& arrangement, const Pairs& ps) {
    int total = 0;
    for(int i = 0; i < arrangement.size()-1; ++i) {
        total += ps[{arrangement[i],arrangement[i+1]}];
    }
    total += ps[{arrangement.front(),arrangement.back()}];
    return total;
}

int happiness2(const std::vector<int>& arrangement, const Pairs& ps) {
    //Inserting yourself into an arrangement by increasing the happiness the most(or decreasing it the least) means breaking up the lowest happiness connection
    int total = 0;
    int min = ps[{arrangement.front(),arrangement.back()}];
    for(int i = 0; i < arrangement.size()-1; ++i) {
        total += ps[{arrangement[i],arrangement[i+1]}];
        min = std::min(min,ps[{arrangement[i],arrangement[i+1]}]);
    }
    total += ps[{arrangement.front(),arrangement.back()}];
    total -= min;
    return total;
}

void solution(std::string_view input) {
    auto pairs = parse(input);

    std::vector<int> seating(pairs.total,0);
    int max = 0;
    int max2 = 0;

    //Each combination of two neighbors for the first person gives a unique set of seatings
    for(int i = 1; i < pairs.total-1; ++i) {
        seating[1] = 0;
        seating[0] = i;
        for(int j = i+1; j < pairs.total; ++j) {
            seating[2] = j;
            auto current = 0;
            std::generate(seating.begin()+3,seating.end(),[&]{while(++current == i or current == j);return current;});
            do {
                max = std::max(max,happiness(seating,pairs));
                max2 = std::max(max2,happiness2(seating,pairs));
            } while(std::next_permutation(seating.begin()+3,seating.end()));
        }
    }
    std::cout << "Part 1: " << max << '\n';
    std::cout << "Part 2: " << max2 << '\n';
}

std::string_view input = R"(Alice would gain 2 happiness units by sitting next to Bob.
Alice would gain 26 happiness units by sitting next to Carol.
Alice would lose 82 happiness units by sitting next to David.
Alice would lose 75 happiness units by sitting next to Eric.
Alice would gain 42 happiness units by sitting next to Frank.
Alice would gain 38 happiness units by sitting next to George.
Alice would gain 39 happiness units by sitting next to Mallory.
Bob would gain 40 happiness units by sitting next to Alice.
Bob would lose 61 happiness units by sitting next to Carol.
Bob would lose 15 happiness units by sitting next to David.
Bob would gain 63 happiness units by sitting next to Eric.
Bob would gain 41 happiness units by sitting next to Frank.
Bob would gain 30 happiness units by sitting next to George.
Bob would gain 87 happiness units by sitting next to Mallory.
Carol would lose 35 happiness units by sitting next to Alice.
Carol would lose 99 happiness units by sitting next to Bob.
Carol would lose 51 happiness units by sitting next to David.
Carol would gain 95 happiness units by sitting next to Eric.
Carol would gain 90 happiness units by sitting next to Frank.
Carol would lose 16 happiness units by sitting next to George.
Carol would gain 94 happiness units by sitting next to Mallory.
David would gain 36 happiness units by sitting next to Alice.
David would lose 18 happiness units by sitting next to Bob.
David would lose 65 happiness units by sitting next to Carol.
David would lose 18 happiness units by sitting next to Eric.
David would lose 22 happiness units by sitting next to Frank.
David would gain 2 happiness units by sitting next to George.
David would gain 42 happiness units by sitting next to Mallory.
Eric would lose 65 happiness units by sitting next to Alice.
Eric would gain 24 happiness units by sitting next to Bob.
Eric would gain 100 happiness units by sitting next to Carol.
Eric would gain 51 happiness units by sitting next to David.
Eric would gain 21 happiness units by sitting next to Frank.
Eric would gain 55 happiness units by sitting next to George.
Eric would lose 44 happiness units by sitting next to Mallory.
Frank would lose 48 happiness units by sitting next to Alice.
Frank would gain 91 happiness units by sitting next to Bob.
Frank would gain 8 happiness units by sitting next to Carol.
Frank would lose 66 happiness units by sitting next to David.
Frank would gain 97 happiness units by sitting next to Eric.
Frank would lose 9 happiness units by sitting next to George.
Frank would lose 92 happiness units by sitting next to Mallory.
George would lose 44 happiness units by sitting next to Alice.
George would lose 25 happiness units by sitting next to Bob.
George would gain 17 happiness units by sitting next to Carol.
George would gain 92 happiness units by sitting next to David.
George would lose 92 happiness units by sitting next to Eric.
George would gain 18 happiness units by sitting next to Frank.
George would gain 97 happiness units by sitting next to Mallory.
Mallory would gain 92 happiness units by sitting next to Alice.
Mallory would lose 96 happiness units by sitting next to Bob.
Mallory would lose 51 happiness units by sitting next to Carol.
Mallory would lose 81 happiness units by sitting next to David.
Mallory would gain 31 happiness units by sitting next to Eric.
Mallory would lose 73 happiness units by sitting next to Frank.
Mallory would lose 89 happiness units by sitting next to George.)";

int main() {
    solution(input);
}
