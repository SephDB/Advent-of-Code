#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

template<typename F>
void split(std::string_view in, std::string_view delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+delim.size());
    }
    f(in);
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct CircQueue {
    std::array<int,64> content;
    int start = 0, size = 0;
    int& operator[](size_t s) {
        return content[(start + s)%64];
    }
    int operator[](size_t s) const {
        return content[(start + s)%64];
    }
    int pop() {
        int current = content[start];
        start = (start + 1)%64;
        --size;
        return current;
    }
    void push(int n) {
        (*this)[size] = n;
        ++size;
    }
    bool operator==(const CircQueue& o) const {
        if(size != o.size) return false;
        for(int i = 0; i < size; ++i) {
            if((*this)[i] != o[i]) return false;
        }
        return true;
    }
};

std::ostream& operator<<(std::ostream& o, CircQueue& p) {
    for(int i = 0; i < p.size; ++i) {
        o << p[i] << ' ';
    }
    return o;
}

using State = std::array<CircQueue,2>;

auto parse(std::string_view input) {
    State players;
    int n = 0;
    split(input,"\n\n",[&](std::string_view player) {
        player.remove_prefix(sizeof("Player 1:"));
        split(player,'\n',[&](std::string_view in) {
            players[n].push(to_int(in));
        });
        ++n;
    });
    return players;
}

auto part1(State players) {
    while(players[0].size && players[1].size) {
        int p1 = players[0].pop();
        int p2 = players[1].pop();
        if(p1 > p2) {
            players[0].push(p1);
            players[0].push(p2);
        } else {
            players[1].push(p2);
            players[1].push(p1);
        }
    }
    auto winner = players[0].size == 0 ? players[1] : players[0];
    int total = 0;
    for(int i = 0; i < winner.size; ++i) {
        total += winner[i]*(winner.size-i);
    }
    return total;
}

//Returns true if player 1 wins
bool part2_impl(State& players) {
    std::array<std::vector<State>,50> state_lookup; //linear lookup for each of the possible sizes of p1's deck
    auto play = [&](int p1, int p2) -> bool {
        if(p1 <= players[0].size && p2 <= players[1].size) {
            State next = players;
            next[0].size = p1;
            next[1].size = p2;
            return part2_impl(next);
        }
        return p1 > p2;
    };

    while(players[0].size && players[1].size) {
        auto& lookup = state_lookup[players[0].size];
        if(std::ranges::find(lookup,players) != lookup.end()) return true;
        lookup.push_back(players);

        int p1 = players[0].pop();
        int p2 = players[1].pop();
        if(play(p1,p2)) {
            players[0].push(p1);
            players[0].push(p2);
        } else {
            players[1].push(p2);
            players[1].push(p1);
        }
    }
    return players[1].size == 0;
}

auto part2(State players) {
    part2_impl(players);
    auto winner = players[0].size == 0 ? players[1] : players[0];
    int total = 0;
    for(int i = 0; i < winner.size; ++i) {
        total += winner[i]*(winner.size-i);
    }
    return total;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(Player 1:
18
50
9
4
25
37
39
40
29
6
41
28
3
11
31
8
1
38
33
30
42
15
26
36
43

Player 2:
32
44
19
47
12
48
14
2
13
10
35
45
34
7
5
17
46
21
24
49
16
22
20
27
23)";

int main() {
    solution(input);
}
