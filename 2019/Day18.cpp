#include <array>
#include <iostream>
#include <vector>
#include <bitset>
#include <complex>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <chrono>

template<typename F>
double time(F&& f, int num_tries=1) {
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < num_tries; ++i) {
        f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    return diff.count()/num_tries;
}

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

struct Coord {
    int x = -1;
    int y = -1;
    Coord operator+(Coord o) const {return {x+o.x,y+o.y};}
    Coord& operator+=(Coord o) {*this = *this + o; return *this;}
    Coord operator-() const {return {-x,-y};}
    bool operator==(Coord o) {return x==o.x and y==o.y;}
    Coord operator*(Coord o) const {auto res = std::complex<int>{x,y}*std::complex<int>{o.x,o.y}; return {res.real(),res.imag()};}
    Coord& operator*=(Coord o) {*this = *this * o; return *this;}
};

constexpr std::array<Coord,4> directions{{{1,0},{-1,0},{0,1},{0,-1}}};

int opposite_coord(int current) {
    return current + 1 - 2*(current%2);
}

struct Tile {
    std::bitset<4> open_directions;
    char tile;
    bool isKey() const {return tile >= 'a' and tile <= 'z';}
    bool isDoor() const {return tile >= 'A' and tile <= 'Z';}
    template<typename F>
    void for_all_dirs(F&& f) const {
        for(int i = 0; i < 4; ++i) {
            if(open_directions.test(i)) f(i,directions[i]);
        }
    }
};

struct Maze {
    Maze(std::string_view input) {
        auto lines = split(input,'\n');

        //Discard outer tiles bc they're not interesting anyway
        height = lines.size()-2;
        width = lines[0].size()-2;

        auto lookup = [&lines](Coord c) {return lines[c.y][c.x];};
        for(auto y = 1; y < lines.size()-1; ++y) {
            for(auto x = 1; x < lines[0].size()-1; ++x) {
                Coord current = {x,y};
                auto t = Tile{{},lookup(current)};
                if(t.tile == '@') {
                    start.push_back({current.x-1,current.y-1});
                } else if(t.isKey()) {
                    num_keys++;
                }
                if(t.tile != '#')
                    for(int i=0; i < 4; ++i) {
                        t.open_directions[i] = lookup(current+directions[i]) != '#';
                    }
                tiles.push_back(t);
            }
        }
    }
    Tile& get(Coord c) {
        return tiles[c.y*width+c.x];
    }

    const Tile& operator[](Coord c) const {
        return tiles[c.y*width+c.x];
    }

    void close(Coord c) {
        auto& t = get(c);
        t.for_all_dirs([&](int d,Coord add) {
            auto& neighbor = get(c+add);
            neighbor.open_directions.reset(opposite_coord(d));
        });
        t.open_directions &= 0;
    }

    void print() {
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                std::cout << (*this)[{x,y}].tile;
            }
            std::cout << '\n';
        }
    }

    template<typename F>
    void for_each_tile(F&& f) const {
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                f(Coord{x,y},(*this)[Coord{x,y}]);
            }
        }
    }

    std::vector<Tile> tiles;
    std::size_t width, height, num_keys=0;
    std::vector<Coord> start;
};

struct NextStates {
    struct Path {
        std::bitset<26> needed_keys;
        std::size_t length;
        bool possible(std::bitset<26> keys) const {
            return (needed_keys & keys) == needed_keys;
        }
    };

    struct PathGroup {
        std::vector<Path> paths;
        std::optional<Path> shortest(std::bitset<26> keys) const {
            std::optional<Path> ret{};
            for(auto& p : paths) {
                if(p.possible(keys)) {
                    if(not ret.has_value() or ret->length < p.length) {
                        ret = p;
                    }
                }
            }
            return ret;
        }
        bool update(Path p) {
            auto shortest_equivalent = shortest(p.needed_keys);
            if(shortest_equivalent.has_value() and shortest_equivalent->length <= p.length) {
                return false;
            }
            paths.erase(std::remove_if(paths.begin(),paths.end(),[&](auto&& path) {
                //The new path is possible with a subset of the other's keys
                //and is shorter, so completely subsumes it
                return p.possible(path.needed_keys) and p.length <= path.length;
            }),paths.end());
            paths.push_back(p);
            return true;
        }
    };

    //Paths from this key to other keys, possibly multiple per key if different key requirements to get there
    std::array<PathGroup,26> toKey;
};

NextStates fromKey(const Maze& m, std::array<Coord,26> key_locs, Coord start, int k) {
    //modified BFS, allows multiple runs over the same coordinates if required keys aren't subsumed
    std::vector<NextStates::PathGroup> grid(m.width*m.height);
    auto lookup = [&](Coord c) -> NextStates::PathGroup& {return grid[c.y*m.width+c.x];};

    auto start_path = NextStates::Path{1<<k,0};

    //Direction to parent, next coordinate, current path stats
    using BFS_state = std::tuple<int,Coord,NextStates::Path>;
    std::queue<BFS_state> q;
    q.emplace(4,start,start_path);
    while(not q.empty()) {
        auto [dir,coord,path] = q.front();
        q.pop();
        auto& t = m[coord];
        if(t.isDoor()) {
            path.needed_keys.set(t.tile-'A');
        }
        if(not lookup(coord).update(path)) continue;
        if(t.isKey()) {
            path.needed_keys.set(t.tile-'a');
        }
        path.length++;
        m[coord].for_all_dirs([&q,pdir=dir,coord=coord,path=path](int dir, Coord add) {
            if(pdir != dir) {
                q.emplace(opposite_coord(dir),coord+add,path);
            }
        });
    }

    NextStates ret;
    std::transform(key_locs.begin(),key_locs.end(),ret.toKey.begin(),lookup);

    return ret;
}

std::array<NextStates,30> precompute_paths(const Maze& m) {
    std::array<Coord,26> key_locs;
    m.for_each_tile([&](auto coord, auto& t) {
        if(t.isKey()) {
            key_locs[t.tile-'a'] = coord;
        }
    });

    std::array<NextStates,30> key_paths;
    for(int i = 0; i < m.num_keys; ++i) {
        key_paths[i] = fromKey(m,key_locs,key_locs[i],i);
    }
    for(int i = 0; i < m.start.size(); ++i) {
        key_paths[26+i] = fromKey(m,key_locs,m.start[i],i+26);
    }
    return key_paths;
}

struct AsState {
    std::bitset<26> keys_picked_up;
    std::array<int,4> current_keys={26,27,28,29};
    bool operator==(AsState o) const {
        return keys_picked_up==o.keys_picked_up and current_keys == o.current_keys;
    }
};

namespace std {
    template<>
    struct hash<AsState> {
        auto operator()(AsState s) const {
            //State is the pickup state of the 26 keys and the locations of each explorer
            std::bitset<26+30> combined(s.keys_picked_up.to_ulong());
            combined <<= 30;
            for(int i = 0; i < 4; ++i) {
                combined.set(s.current_keys[i]);
            }
            return std::hash<decltype(combined)>{}(combined);
        }
    };
}

struct SearchResult {
    std::size_t length, states, states_expanded;
};

SearchResult search(const Maze& m, const std::array<NextStates,30>& paths) {
    //Utility
    auto for_each_path = [&](AsState s,auto&& f) {
        for(auto current_bot = 0; current_bot < m.start.size(); ++current_bot) {
            auto& paths_for_bot = paths[s.current_keys[current_bot]];
            for(int key = 0; key < m.num_keys; ++key) {
                if(not s.keys_picked_up[key]) {
                    auto shortest = paths_for_bot.toKey[key].shortest(s.keys_picked_up);
                    if(shortest.has_value()) {
                        f(current_bot,key,*shortest);
                    }
                }
            }
        }
    };

    //Heuristic is max path length of state to any other non-picked-up key
    auto heuristic = [&,memo=std::unordered_map<AsState,std::size_t>{}](AsState s) mutable {
        if(memo.contains(s)) return memo[s];
        std::size_t max = 0;
        for_each_path(s,[&](int,int,auto p) {
            max = std::max(max,p.length);
        });
        memo[s] = max;
        return max;
    };
    std::bitset<26> goal(std::string(m.num_keys,'1'));

    std::unordered_map<AsState,std::size_t> scores;

    using queue_type = std::pair<std::size_t,AsState>;
    //Reverse ordering bc smallest needs to go out first
    auto cmp = [](auto a, auto b) {return a.first > b.first;};
    std::priority_queue<queue_type,std::vector<queue_type>,decltype(cmp)> AsQueue(cmp);
    
    auto startState = AsState{0};
    AsQueue.emplace(heuristic(startState),startState);
    scores[startState] = 0;
    
    std::size_t num_states = 0;
    std::size_t states_expanded = 1;

    while(not AsQueue.empty()) {
        auto [fScore,state] = AsQueue.top();
        AsQueue.pop();
        if(fScore-heuristic(state) > scores[state]) continue;
        num_states++;
        if(state.keys_picked_up == goal) {
            return {scores[state],num_states,states_expanded};
        }
        auto current_len = scores[state];
        for_each_path(state, [&,state=state](int bot, int key, auto path) {
            auto neighbor = AsState{state.keys_picked_up,state.current_keys};
            neighbor.keys_picked_up.set(key);
            neighbor.current_keys[bot] = key;
            if(not scores.contains(neighbor) or current_len+path.length < scores[neighbor]) {
                scores[neighbor] = current_len+path.length;
                AsQueue.emplace(current_len+path.length+heuristic(neighbor),neighbor);
                states_expanded++;
            }
        });
    }
    return {};
}

void solve(const Maze& m) {
    auto paths = precompute_paths(m);
    
    auto [length,explored,expanded] = search(m,paths);
    std::cout << length << '\n';
    std::cout << "States explored: " << explored << '\n';
    std::cout << "States expanded: " << expanded << '\n';
}


std::string_view input = R"(#################################################################################
#...#z..#.....#...#...#.........#...#...#.............................#...#.....#
#.#.#.#.#.#.#.#.#.#.#.#####.###.###.#.#.#####.#######################H#.#.###.#.#
#.#.#.#...#.#...#...#.....#.#.#...#f..#.#...#.#.......#...#.........#...#.....#.#
#.#.#.#####.#############.#.#.###.###.#.#.#.#.#.#####.#.#.###.#####.###########.#
#.#...#...#.#.........#.....#...#...#.#.#.#...#.....#...#...#.#...#...#...#.....#
#.#####.#.#.#.#######.#######.#####.#.#.#.###.#####.#######I#.#.#.#####X#.#.#####
#.#...#.#.#...#.....#...#...#.....#.#.#.#...#...#.#.#.....#.#.#.#.......#.#.#...#
#.#.#.###.#######.#.###.###.#.#.###.#.#.#.#.###.#.#.#.###.#.#.#.#########.#.#.#.#
#.#.#...#.#.......#...#.#...#.#...#.#.#.#.#.#...#w#b..#.#.#.#.....#...#...#.#.#.#
#.#.###.#.#.#.#######.#.#.###.###.#.###.###.#.###.#####.#.#.###.###Y#.#.#.#.#.###
#.#.#...#...#...#.....#.#.#...#.#...#...#...#.......#.....#...#.#...#.#.#.#.#...#
#.#.#.#########.#.#####.#.#.###.#####.#.#.###########.#####.#.#.#.###.#.###.###.#
#.#.#...#.......#.#.....#.....#.......#.#............j#...#.#.#.#...#.#.....#s..#
#.#.###.#.###.#####.#.#######.#####.###.###############.#.###.###.###.#.#####.#G#
#.C.#.#...#...#.....#.#.....#.....#...#.#.............#.#.....#...#...#.#.....#.#
#####.#####.###.#######.###.#####.###.#.#.###.#####.###.#.#####.###.###.#.#####.#
#...#...#...#...#.....#.#.#.#.......#.#.#.#.#.#.....#...#.#.....#.#...#...#...#.#
#.#.###.#.###.#.#.###.#.#.#.#######.#.#.#.#.#.#.#####.#####.#####.###.#######.#.#
#.#...#...#.#.#.#...#...#.#.#.....#...#.#.#.#.#.#.....#...........T.#.#...U.#.#.#
#.###.#.###.#.#.#.#####.#.#.#.###.#####.#.#.#.###.#.###.#############.#.###.#.#.#
#...#.#.....#.#.#.#...#...#.....#.....#.#...#.#...#.#...#.P...#.E...#...#...#...#
#.#.#.#####.#.###.#.###############.#.###.###.#.###.#.###.#####.###.#####.###.###
#.#.#.....#.#.#...#...............#.#...#.#...#...#.#.#.#...#...#.#.#.N.#...#...#
#.#.#####.###.#.#################.#.###.###.#.###R#.#.#.###.###.#.#.#.#.###.###.#
#.#.....#.#...#...#.........#...#.#...#.#...#.#...#.#.#...#p#...#.#e#.#.....#...#
#.#####.#.#.#####.###.#.#####.#.#.#####.#.#####.###.#V###.#.#.###.#.#.#######.###
#.#...#.#...#...#...#.#.#.....#.#.....#.#.....#.#...#...#.#...#...#...#...#...#.#
#.#.#.#.###.#.#.###.#.#.#.#####.#####.#.#.###.#.#######.#.#######.#######.#.###.#
#.#.#.#...#.#.#.....#.#.......#...#...#.#.#...#.........#...#.........#...#.....#
#.#.#.###.###.#######.#########.#.#.###.###.#.###########.#.#.#.#####.#.#.#####.#
#m#.#...#.....#...#.....#...#...#.#.....#...#...#...#.....#.#.#.#.......#...#...#
#.###O#.#######.#.#####.#.#.#.###.#######.#####.#.###.#.###.#.#.###########.#.###
#...#.#.#.......#.....#.#.#.#...#.....#.#.#...#...#..k#...#.#.#.#.....#...#.#.#.#
###.#.#.###.#########.###.#.###.#####.#.#.#.#.###.#.#####.###L#.#.###.#.#.###.#.#
#.#.#.#.....#l#.....#a..#.#.....#...#.#.#.#.#.#...#...#.#.#...#.#.#.#...#...#.#.#
#.#.#.#######.#.###.###.#.#######.###.#.#.#.#.#######.#.#.#.###.#.#.#######.#.#.#
#...#.#.#.K...#...#...#...#...........#.#...#.......#.#.#.#.#...#...#o....#.#...#
#Q###.#.#.#.#####.#.#######.###########.#.#######.###.#.#.#.#.#####.#.###.#.###.#
#.....#...#.......#............d................#.......#...#.........#...#.....#
#######################################.@.#######################################
#...........#.....#.....#...#.....#.......#.....#...D.......#...#.........#.....#
#.#.#######.#.###.#.###.#.#.###.#.###.#.#.#.#.###.###.#.#####.#.#.###.###.#.###.#
#g#.....#...#.#.#.#.#.....#.....#.#...#.#...#.....#.#.#.#.....#.....#.#.....#...#
#.#####.#.###.#.#.#.#############.#.###.#.#########.#.###.#########.#.#########.#
#.....#.#.#...#.....#...#...#...#.#...#.#.......#...#.....#...#...#.#.#.......#.#
#######.#.###.#######.#.#.#.#.#.#.#.#.#########.#.#########.#.#.#.###.#.#####.#.#
#.......#.....#.......#...#.#.#...#.#...#.....#.#.#.........#...#.....#...#.#.#.#
#.###########.#.###########.#.#########.#.#.###.#.#.#####################.#.#M#.#
#.#.........#.#.#.....#.....#...#.......#.#.#...#.#.....#.......#...........#.#.#
#.#.#######.#.#.#####.#.#######.###.###.#.#.#.###.#####.#.#####.#.###########.#.#
#.#.......#.#.#.....#.#.#.#...#...#.#.#.#.#.#.........#...#.#...#.....#.....#.#.#
#.#######.#.#######.#.#.#.#.#.###.#.#.#.#.#.#########.#####.#.###.#####.###.#.#.#
#.#.....S.#.......#.#.#.#.#.#.....#.#...#.#.........#...#...#.....#...#.#...#.#.#
#.#.#########.#.###.#.#.#.#.#######.#.###########.#.###.###.#######.#.#.#.###.#.#
#...#.......#.#.#.....#.#.#...#.....#...#.......#.#.......#...#.....#...#...#.#.#
#######.###.#.###.#####.#.###.#.#######.#.#####.#.#######.###.#.###########.#.#.#
#.....#.#...#.....#.....#...#...#...#...#.#.#...#.#.....#.....#.#.....#.....#.#.#
#.###.#.#.#########.#####.#########.#.###.#.#.#####.###.#.#####.#.###.#.#####.#A#
#.#.....#.....#...#...#...#.........#...#...#.....#.#...#.#...#.#...#.#.#.....#.#
#.#.###########.#.###.#.#.#.###.#.#####.###F#####.#.#.#####.#.#.###.#.#.###.###.#
#.#...#.........#...#.#.#...#...#.#.....#.#...#.#v..#....t#.#n#...#.#.#.#...#...#
#.#.###.###########.###.#####.#####.#####.###.#.#########.#.###.#.#.###.#.###.###
#.#.#.....#.........#.#.#...#...#...#...#...#.#.......#...#...#.#.#.#.....#.#.#.#
#.#.#.#####.#######.#.#.#.#####.#.#####.#.###.###.###.#.#.###.###.#.#.#####.#.#.#
#.#.#...#...#.....#...#.#.#...#.#.#...#.#...#.....#...#.#...#...#.#...#...#...#.#
#.#####.#.#.#.###.###.#.#.#.#.#.#.#.#.#.###.#####.#####.#######.#.###.#.#.###.#.#
#.#.B.#.#.#.#...#...#.#.#...#.#.#...#.#.#.....#...#..y#.#..u....#...#.#.#...#.#.#
#.#.#.#.#.#.###.###.###.#####.#.#####.#.#.###.#.###.#.#.#.#######.#.###.###.#.#.#
#.#.#.#.#.#.#.....#...#.......#.....#...#.#.#.#.#.#.#...#.#.......#.......#.#...#
#.#.#.#.#.###.#######.#######.#####.###.#.#.#.#.#.#.#####.#.#############.#.###.#
#...#...#.....#.....#.......#.#.#...#.#.#.#...#...#.......#.#.........#...#c#.#.#
#.#############.###.#######.#.#.#.#.#.#.#.#.#####.#########.#.#######.#.###.#.#.#
#.#...........#...#...#...#.#.#.#.#...#.#.#.#...#.#.#.....Z.#.......#.#.#...#...#
#.#########.#####.###.#.#.#.#.#.#.#.###.#.#.#.#.#.#.#.#############.#.###.###.###
#i#.......#.#...#x#..h..#...#.#.#.#.#...#.#...#r#.#...#.....#.J.....#...#...#...#
#.#.#####.#.#.#.#.#########.#.#.#.###.###.###.###.#####.###.#.#########.###.###.#
#.#...#...#.#.#...#...#...#.#.#.#.#...#.#...#.#...#...#.#.#...#.......#...#.#q..#
#.###.#.###.#.#####.#.#.#.###.#.#.#.###.#.#.###.###.#.#.#.#####.###.#####.#.#.###
#.....#.....#.......#...#.......#.......#.#.........#...#...W.....#.........#...#
#################################################################################)";

int main() {
    Maze m(input);
    m.print();
    auto t = time([&]{
        std::cout << "Part 1: ";
        solve(m);
        auto part1_start = m.start[0];
        m.start = {};
        for(int x = part1_start.x-1; x <= part1_start.x+1; ++x) {
            for(int y = part1_start.y-1; y <= part1_start.y+1; ++y) {
                if(x == part1_start.x or y == part1_start.y) m.close({x,y});
                else m.start.push_back({x,y});
            }
        }
        std::cout << "Part 2: ";
        solve(m);
    });
    std::cout << "Time: " << t << "s\n";
}
