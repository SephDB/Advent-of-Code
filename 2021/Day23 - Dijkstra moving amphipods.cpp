#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <optional>
#include <queue>
#include <charconv>
#include <numeric>
#include <algorithm>

template<int N, typename F>
auto split_known(std::string_view in, char delim, F&& apply) {
    std::array<decltype(apply(in)),N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = apply(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    ret[current++] = apply(in);
    return ret;
}

template<int RoomSize>
struct GridState {
    std::array<std::array<int,RoomSize>,4> rooms;
    std::array<int,5> hallway{-1,-1,-1,-1,-1};
    int left_edge = -1;
    int right_edge = -1;
    auto operator<=>(const GridState& other) const = default;
    bool room_needs_emptying(int room) const {
        return std::ranges::any_of(rooms[room],[&](auto s) {return s != -1 && s != room;});
    }
    int room_top(int room) const {
        return std::ranges::distance(rooms[room].begin(),std::ranges::find_if(rooms[room],[](auto s){return s != -1;}));
    }
    bool room_available(int room) const {
        int top = room_top(room);
        return top == RoomSize || std::ranges::all_of(rooms[room].begin()+top,rooms[room].end(),[room](auto s) {return s == room;});
    }
    int hallway_left(int room) const {
        return room;
    }
    int hallway_right(int room) const {
        return room+1;
    }

    friend std::ostream& operator<<(std::ostream& o, const GridState& grid) {
        auto to_char = [](int n) -> char {
            return (n == -1) ? '.' : ('A'+n);
        };
        o << "#############\n";
        o << '#' << to_char(grid.left_edge);
        for(auto& h : grid.hallway) {
            if(&h != &*grid.hallway.begin()) {
                o << '.';
            }
            o << to_char(h);
        }
        o << to_char(grid.right_edge) << "#\n";
        for(int n = 0; n < grid.rooms[0].size(); n++) {
            o << "###";
            for(auto r : grid.rooms) {
                o << to_char(r[n]) << '#';
            }
            o << "##\n";
        }
        o << "#############\n";
        return o;
    }
};

auto parse(std::string_view input) {
    GridState<2> state{};
    auto [top,hallway,room_top,room_bottom,bottom] = split_known<5>(input,'\n',[](auto a) {return a;});
    for(int i = 0; i < 4; ++i) {
        state.rooms[i] = {room_top[i*2+3]-'A',room_bottom[i*2+3]-'A'};
    }
    return state;
}

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
            std::cout << "\nExpansion stats: " << num_states << ' ' << states_expanded << "\nSolution: ";
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


auto solve(auto input) {
    constexpr std::array type_cost{1,10,100,1000};

    auto is_goal = [](const auto& state) {
        bool check = true;
        for(int i = 0; i < 4; ++i) {
            check = check && std::ranges::all_of(state.rooms[i],[i](auto n){return n == i;});
        }
        return check;
    };

    auto heur = [](auto& s) {return 0;}; //Dijkstra for now

    auto next = [&](auto state, auto&& next_f) {
        auto n = [&](auto& a, auto& b, int cost) {
            std::swap(a,b);
            next_f(state,cost*type_cost[a]);
            std::swap(a,b);
        };

        //Check moving stuff out of rooms
        for(int r = 0; r < 4; ++r) {
            if(!state.room_needs_emptying(r)) continue;

            auto& room = state.rooms[r];

            int cost_to_room_exit = state.room_top(r);
            auto& current = room[cost_to_room_exit];
            {
                //go left
                int hallway_loc = state.hallway_left(r);
                for(;hallway_loc >= 0 && state.hallway[hallway_loc] == -1; --hallway_loc) {
                    n(state.hallway[hallway_loc],current,cost_to_room_exit+2*(r-hallway_loc+1));
                }
                if(hallway_loc == -1 && state.left_edge == -1) {
                    n(state.left_edge,current,cost_to_room_exit+2*(r+1)+1);
                }
            }
            {
                //go right
                int hallway_loc = state.hallway_right(r);
                for(;hallway_loc < state.hallway.size() && state.hallway[hallway_loc] == -1; ++hallway_loc) {
                    n(state.hallway[hallway_loc],current,cost_to_room_exit+2*(hallway_loc-r));
                }
                if(hallway_loc == state.hallway.size() && state.right_edge == -1) {
                    n(state.right_edge,current,cost_to_room_exit+2*(state.hallway.size()-r-1)+1);
                }
            }
        }

        //Check moving stuff into rooms
        auto reachable_room = [&](int pos, int room) {
            int left,right,distance;
            if(pos > room) {
                right = pos-1;
                left = state.hallway_right(room);
                distance = pos - left;
                if(pos == left) return 0;
            }
            else {
                right = state.hallway_left(room);
                left = pos+1;
                distance = right-pos;
                if(pos == right) return 0;
            }
            if(std::ranges::all_of(std::ranges::subrange(
                                                            state.hallway.begin()+left,
                                                            state.hallway.begin()+right+1),
                                    [](int a){return a == -1;})) return distance*2;
            return -1;
        };
        auto room_entry_dist = [&](int room) {
            return 1 + state.room_top(room);
        };
        if(state.left_edge != -1 && state.hallway[0] == -1 && state.room_available(state.left_edge)) {
            auto dist = reachable_room(0,state.left_edge);
            if(dist != -1) {
                dist++;
                auto room_dist = room_entry_dist(state.left_edge);
                n(state.rooms[state.left_edge][room_dist-2],state.left_edge,dist+room_dist);
            }
        }
        if(state.right_edge != -1 && state.hallway.back() == -1 && state.room_available(state.right_edge)) {
            auto dist = reachable_room(state.hallway.size()-1,state.right_edge);
            if(dist != -1) {
                dist++;
                auto room_dist = room_entry_dist(state.right_edge);
                n(state.rooms[state.right_edge][room_dist-2],state.right_edge,dist+room_dist);
            }
        }
        for(int i = 0; i < state.hallway.size(); ++i) {
            if(state.hallway[i] == -1) continue;
            if(state.room_available(state.hallway[i])) {
                auto dist = reachable_room(i,state.hallway[i]);
                if(dist != -1) {
                    auto room_dist = room_entry_dist(state.hallway[i]);
                    n(state.rooms[state.hallway[i]][room_dist-2],state.hallway[i],dist+room_dist);
                }
            }
        }
    };
    std::cout << '\n' << input << '\n';

    auto [f,score] = As<int>(input,heur,is_goal,next);

    std::cout << '\n' << f << '\n';

    return score;
}

std::string_view input = R"(#############
#...........#
###C#D#D#A###
  #B#A#B#C#
  #########)";

int main() {
    auto in = parse(input);
    std::cout << "Part 1: " << solve(in) << '\n';
    GridState<4> part2_grid{};
    part2_grid.rooms[0] = {in.rooms[0][0],3,3,in.rooms[0][1]};
    part2_grid.rooms[1] = {in.rooms[1][0],2,1,in.rooms[1][1]};
    part2_grid.rooms[2] = {in.rooms[2][0],1,0,in.rooms[2][1]};
    part2_grid.rooms[3] = {in.rooms[3][0],0,2,in.rooms[3][1]};
    std::cout << "Part 2: " << solve(part2_grid) << '\n';
}
