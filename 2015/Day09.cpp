#include <iostream>
#include <string_view>
#include <charconv>
#include <bitset>
#include <queue>
#include <cmath>
#include <vector>
#include <map>

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

auto triangle(std::size_t n) {
    return n*(n-1)/2;
}

int triangle_root(int x) {
    return (static_cast<int>(std::sqrt(8*x+1))-1)/2;
}

auto adjacency_lookup(const std::vector<int>& matrix, int x, int y) {
    return matrix[triangle(std::max(x,y))+std::min(x,y)];
}

struct ParseResult {
    std::vector<int> adjacency;
    int num_places;
    auto lookup(int x, int y) const {
        return adjacency_lookup(adjacency,x,y);
    }
};

auto parse(std::string_view input) {
    ParseResult ret;

    for(auto line : split(input,'\n')) {
        auto last = line.find_last_of(' ');
        ret.adjacency.push_back(to_int(line.substr(last+1)));
    }
    std::reverse(ret.adjacency.begin(),ret.adjacency.end());
    ret.num_places = triangle_root(ret.adjacency.size())+1;
    return ret;
}

struct AsState {
    AsState(int head, int tail, std::bitset<8> checked, int l=0) : h(std::min(head,tail)), t(std::max(head,tail)), so_far(checked), len(l) {};
    bool operator<(AsState o) const {
        auto s = so_far.to_ulong();
        auto os = o.so_far.to_ulong();
        return std::tie(h,t,s) < std::tie(o.h,o.t,os);
    }

    int h,t;
    std::bitset<8> so_far;
    int len;
};

template<typename ScoreType, typename State, typename HFunc, typename GoalFunc, typename NextFunc>
std::pair<State,ScoreType> As(State start, HFunc&& heuristic, GoalFunc&& isgoal, NextFunc&& for_each_neighbor) {
    std::map<State,ScoreType> scores;

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

int part1(const ParseResult& paths) {
    auto heuristic = [](AsState s) {return 0;};
    auto isgoal = [&](AsState s) {return s.so_far.count() == paths.num_places;};
    auto for_each_neighbor = [&paths](AsState s, auto&& callback) {
        for(int i = 0; i < paths.num_places; ++i) {
            if(s.so_far.test(i)) continue;
            auto so_far = s.so_far;
            so_far.set(i);
            callback({s.h,i,so_far},paths.lookup(s.t,i));
            callback({i,s.t,so_far},paths.lookup(s.h,i));
        }
    };

    auto startState = AsState(0,0,0);
    startState.so_far.set(0);
    
    return As<int>(startState, heuristic, isgoal, for_each_neighbor).second;
}

int part2(const ParseResult& paths) {
    auto heuristic = [](AsState s) {return 0;};
    auto isgoal = [&](AsState s) {return s.so_far.count() == paths.num_places;};
    auto for_each_neighbor = [&paths](AsState s, auto&& callback) {
        for(int i = 0; i < paths.num_places; ++i) {
            if(s.so_far.test(i)) continue;
            auto so_far = s.so_far;
            so_far.set(i);
            auto tail_distance = paths.lookup(s.t,i);
            auto head_distance = paths.lookup(s.h,i);
            callback({s.h,i,so_far,s.len+tail_distance},1.0/paths.lookup(s.t,i));
            callback({i,s.t,so_far,s.len+head_distance},1.0/paths.lookup(s.h,i));
        }
    };

    auto startState = AsState(0,0,0);
    startState.so_far.set(0);

    return As<double>(startState,heuristic, isgoal, for_each_neighbor).first.len;
}

std::string_view input = R"(Tristram to AlphaCentauri = 34
Tristram to Snowdin = 100
Tristram to Tambi = 63
Tristram to Faerun = 108
Tristram to Norrath = 111
Tristram to Straylight = 89
Tristram to Arbre = 132
AlphaCentauri to Snowdin = 4
AlphaCentauri to Tambi = 79
AlphaCentauri to Faerun = 44
AlphaCentauri to Norrath = 147
AlphaCentauri to Straylight = 133
AlphaCentauri to Arbre = 74
Snowdin to Tambi = 105
Snowdin to Faerun = 95
Snowdin to Norrath = 48
Snowdin to Straylight = 88
Snowdin to Arbre = 7
Tambi to Faerun = 68
Tambi to Norrath = 134
Tambi to Straylight = 107
Tambi to Arbre = 40
Faerun to Norrath = 11
Faerun to Straylight = 66
Faerun to Arbre = 144
Norrath to Straylight = 115
Norrath to Arbre = 135
Straylight to Arbre = 127)";

int main() {
    auto parsed = parse(input);
    std::cout << "Part 1: " << part1(parsed) << '\n';
    std::cout << "Part 2: " << part2(parsed) << '\n';
}
