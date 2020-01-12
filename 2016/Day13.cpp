#include <iostream>
#include <array>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <complex>
#include <queue>

template<typename ScoreType, typename State, typename HFunc, typename GoalFunc, typename NextFunc>
std::pair<State,ScoreType> As(State start, HFunc&& heuristic, GoalFunc&& isgoal, NextFunc&& for_each_neighbor) {
    std::unordered_map<State,ScoreType> scores;

    using queue_type = std::pair<ScoreType,State>;
    //Reverse ordering bc smallest needs to go out first
    auto cmp = [](auto a, auto b) {return a.first > b.first;};
    std::priority_queue<queue_type,std::vector<queue_type>,decltype(cmp)> AsQueue(cmp);
    
    AsQueue.emplace(heuristic(start),start);
    scores[start] = 0;
    
    std::size_t num_states = 0;
    std::size_t states_expanded = 1;

    while(not AsQueue.empty()) {
        auto [fScore,state] = AsQueue.top();
        AsQueue.pop();
        if(fScore-heuristic(state) > scores[state]) continue;
        num_states++;
        if(isgoal(state)) {
            std::cout << num_states << ' ' << states_expanded << ' ';
            return {state,scores[state]};
        }
        auto current_len = scores[state];
        for_each_neighbor(state, [&,state=state](State next, ScoreType length) {
            if(not scores.contains(next) or current_len+length < scores[next]) {
                scores[next] = current_len+length;
                AsQueue.emplace(current_len+length+heuristic(next),next);
                states_expanded++;
            }
        });
    }
    return {start,0};
}

using Coord = std::complex<unsigned int>;

namespace std {
    template<>
    struct hash<Coord> {
        auto operator()(Coord c) const {
            return (c.real() << 16) | c.imag();
        }
    };
}

template<typename Next>
unsigned int reachable(Coord start, int total_length, Next&& for_each) {
    std::unordered_set<Coord> reached;
    std::queue<std::pair<Coord,int>> next;
    next.push({start,0});
    while(not next.empty()) {
        auto [pos,len] = next.front();
        next.pop();
        if(len == total_length+1) break;

        if(reached.insert(pos).second) {
            for_each(pos,[&next,&reached,len=len](Coord c,auto) {
                if(not reached.contains(c)) next.push({c,len+1});
            });
        }
    }
    return reached.size();
}

constexpr unsigned int fav_number = 1350;

bool open(Coord loc) {
    auto x = loc.real();
    auto y = loc.imag();
    return std::popcount(fav_number + x*x + 3*x + 2*x*y + y + y*y) % 2 == 0;
}

unsigned int abs_distance(unsigned int a, unsigned int b) {
    return std::max(a,b) - std::min(a,b);
}

int main() {
    Coord start = {1,1};
    Coord target = {31,39};
    auto heuristic = [](Coord c) {return 0;};
    auto is_goal = [&target](Coord c) {return c == target;};
    auto for_each_neighbor = [](Coord c, auto&& callback) {
        std::array<Coord,4> dirs{{{0,1},{0,-1},{1,0},{-1,0}}};
        for(auto d : dirs) {
            if(d.real() == -1 and c.real() == 0) continue;
            if(d.imag() == -1 and c.imag() == 0) continue;
            auto next = c + d;
            if(open(next)) callback(next,1);
        }
    };

    auto manhattan = [&target](Coord c) {
        return abs_distance(c.real(),target.real()) + abs_distance(c.imag(),target.imag());
    };

    std::cout << "Part 1: " << As<unsigned int>(start,heuristic,is_goal,for_each_neighbor).second << '\n';
    std::cout << "Part 1b:" << As<unsigned int>(start,manhattan,is_goal,for_each_neighbor).second << '\n';
    std::cout << "Part 2: " << reachable(start,50,for_each_neighbor) << '\n';
}
