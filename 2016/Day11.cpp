#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <queue>
#include <charconv>
#include <numeric>
#include <unordered_set>
#include <algorithm>
#include <bitset>

constexpr int num_floors = 4;
constexpr int num_pairs = 5;

struct Pair {
    int micro_floor;
    int gen_floor;
    int state_rep() const {
        return micro_floor*num_floors+gen_floor;
    }
    bool operator<(const Pair& o) const {
        return state_rep() < o.state_rep();
    }
};

struct State {
    std::array<Pair,num_pairs> pairs;
    int elevator;
    bool valid() const {
        std::bitset<num_floors> has_generator;
        std::bitset<num_floors> unprotected_chip;
        for(auto [micro,gen] : pairs) {
            has_generator.set(gen);
            if(micro != gen) {
                unprotected_chip.set(micro);
            }
        }
        return (has_generator & unprotected_chip).none();
    }

    int state_rep() const {
        return std::accumulate(pairs.begin(),pairs.end(),0,[](auto acc, Pair p) {return acc*num_floors*num_floors + p.state_rep();})*num_floors + elevator;
    }

    template<typename F>
    void neighbours(F&& callback) {
        auto get = [](auto& ps, int num) -> int& {
            auto& p = ps[num/2];
            return num % 2 ? p.gen_floor : p.micro_floor;
        };
        auto next = [&get](auto ps, int num, int level) {
            get(ps,num) = level;
            std::sort(ps.begin(),ps.end());
            return ps;
        };

        for(int i = 0; i < num_pairs*2; ++i) {
            if(get(pairs,i) != elevator) continue;

            auto inner = [this,&i,&next,&get,&callback](int level) {
                auto imoved = next(pairs,i,level);
                if(State{imoved,level}.valid())
                    callback(State{imoved,level});
                for(int j = i+1; j < num_pairs*2; ++j) {
                    if(get(pairs,j) != elevator) continue;
                    auto n = next(imoved,j,level);
                    if(State{n,level}.valid())
                        callback(State{n,level});
                }
            };
            if(elevator < num_floors-1) {
                inner(elevator+1);
            }
            if(elevator > 0) {
                inner(elevator-1);
            }
        }
    }
};

std::ostream& operator<<(std::ostream& o, State s) {
    std::cout << s.elevator << ' ';
    for(auto p : s.pairs) {
        std::cout << "(m" << p.micro_floor << " g" << p.gen_floor << ") ";
    }
    return o;
}

State parse(std::string_view input) {
    //Thulium and cobalt have both on the first floor, and polonium and promethium are split between floor 1 and 2(microchip being on floor 2)
    return {{{{0,0},{0,0},{0,0},{1,0},{1,0}}},0};
    //return {{{{0,1},{0,2}}},0};
}

auto part1(State start) {
    auto is_goal = [](const State& s) {
        return std::all_of(s.pairs.begin(),s.pairs.end(),[](auto p) {
            return p.gen_floor == p.micro_floor and p.micro_floor == num_floors-1;
        });
    };
    
    std::unordered_set<int> seen;
    std::queue<std::pair<int,State>> states;
    states.push({0,start});
    seen.insert(start.state_rep());

    while(not states.empty()) {
        auto [len,current] = states.front();
        states.pop();
        if(is_goal(current)) {
            return len;
        }
        current.neighbours([&](State next) {
            if(seen.insert(next.state_rep()).second) {
                states.push({len+1,next});
            }
        });
    }

    return 0;
}

void solution(std::string_view input) {
    auto start = parse(input);
    std::cout << "Part 1: " << part1(start) << '\n';
}

std::string_view input = R"(The first floor contains a polonium generator, a thulium generator, a thulium-compatible microchip, a promethium generator, a ruthenium generator, a ruthenium-compatible microchip, a cobalt generator, and a cobalt-compatible microchip.
The second floor contains a polonium-compatible microchip and a promethium-compatible microchip.
The third floor contains nothing relevant.
The fourth floor contains nothing relevant.)";

int main() {
    solution(input);
}
