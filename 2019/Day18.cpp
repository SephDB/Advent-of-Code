#include <array>
#include <iostream>
#include <vector>
#include <bitset>
#include <complex>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <map>
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
    bool operator<(Coord o) const {return std::tie(x,y) < std::tie(o.x,o.y);}
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
        remove_dead_ends();
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
        t.tile = '#';
        t.open_directions &= 0;
    }

    template<typename F>
    void for_each_tile(F&& f) const {
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                f(Coord{x,y},(*this)[Coord{x,y}]);
            }
        }
    }

    template<typename F>
    void for_each_tile(F&& f) {
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                f(Coord{x,y},get(Coord{x,y}));
            }
        }
    }

    void remove_dead_ends() {
        for_each_tile([this](Coord tc, auto&&) {
            while(get(tc).open_directions.count() == 1 and not (get(tc).isKey() or get(tc).tile == '@')) {
                Coord next;
                get(tc).for_all_dirs([&](int,Coord dir) {
                    next = tc+dir;
                });
                close(tc);
                tc = next;
            }
        });
    }

    std::vector<Tile> tiles;
    std::size_t width, height, num_keys=0;
    std::vector<Coord> start;
};

struct Path {
    std::bitset<26> needed_keys;
    std::size_t length;
    bool possible(std::bitset<26> keys) const {
        return (needed_keys & keys) == needed_keys;
    }
    Path operator+(Path o) {
        return {needed_keys | o.needed_keys, length+o.length};
    }
};

struct CompressedMaze {
    struct Node {
        Tile dirs;
        std::array<std::pair<Path,int>,4> connections;
        template<typename F>
        void for_all_paths(F&& f) const {
            dirs.for_all_dirs([&](int dir,auto&&) {
                auto& [path,next] = connections[dir];
                f(path,next);
            });
        }
    };
    std::vector<Node> nodes;
    std::vector<int> start;
    int num_keys;
};

CompressedMaze compress(const Maze& m) {
    std::map<Coord,int> coord_to_node;
    std::vector<CompressedMaze::Node> nodes;
    m.for_each_tile([&](auto coord, auto& t) {
        if(t.tile == '@' or t.isKey() or t.open_directions.count() > 2 or t.open_directions.count() == 1) {
            coord_to_node[coord] = nodes.size();
            nodes.push_back({t});
        }
    });
    for(auto [coord,n] : coord_to_node) {
        auto& node = nodes[n];
        node.dirs.for_all_dirs([&,coord=coord,n=n](const int dir, Coord dirc) {
            //If we did this direction before
            if(node.connections[dir].first.length > 0) return;

            Coord current = coord+dirc;
            int lastdir = dir;
            Path p = {0,1};

            while(not coord_to_node.contains(current)) {
                if(m[current].isDoor()) {
                    p.needed_keys.set(m[current].tile-'A');
                }
                int newdir = 0;
                m[current].for_all_dirs([&](int d, Coord ndirc) {
                    if(d != opposite_coord(lastdir)) {
                        newdir = d;
                        current += ndirc;
                    }
                });
                p.length++;
                lastdir = newdir;
            }
            if(m[current].isDoor()) {
                p.needed_keys.set(m[current].tile-'A');
            }

            auto& neighbor_id = coord_to_node.at(current);
            node.connections[dir] = {p,neighbor_id};
            nodes[neighbor_id].connections[opposite_coord(lastdir)] = {p,n};
        });
    }
    std::vector<int> start(m.start.size());
    std::transform(m.start.begin(),m.start.end(),start.begin(),[&](Coord c) {
        return coord_to_node.at(c);
    });
    return {nodes,start,m.num_keys};
}

struct NextStates {

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

NextStates fromKey(const CompressedMaze& m, std::array<int,26> key_locs, int start, int k) {
    std::vector<NextStates::PathGroup> node_paths(m.nodes.size());

    struct State {
        int prev;
        int current;
        Path p;
    };
    std::queue<State> states;
    states.push({-1,start,Path{1<<k,0}});

    while(not states.empty()) {
        auto s = states.front();
        states.pop();
        if(not node_paths[s.current].update(s.p)) continue;
        auto& node = m.nodes[s.current];
        if(node.dirs.isKey())
            s.p.needed_keys.set(node.dirs.tile-'a');
        node.for_all_paths([&](Path p, int next) {
            if(next != s.prev) {
                states.push({s.current,next,s.p+p});
            }
        });
    }

    NextStates ret;
    std::transform(key_locs.begin(),key_locs.end(),ret.toKey.begin(),[&](int loc){return node_paths[loc];});

    return ret;
}

auto get_key_locs(const CompressedMaze& m) {
    std::array<int,26> key_locs;
    for(int i = 0; i < m.nodes.size(); ++i) {
        if(m.nodes[i].dirs.isKey()) {
            key_locs[m.nodes[i].dirs.tile-'a'] = i;
        }
    }
    return key_locs;
}

std::array<NextStates,30> precompute_paths(const CompressedMaze& m) {
    auto key_locs = get_key_locs(m);

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

SearchResult find_all_keys(const CompressedMaze& m, const std::array<NextStates,30>& paths) {
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

auto solve(const CompressedMaze& m) {
    auto paths = precompute_paths(m);
    
    return find_all_keys(m,paths);
}

void printResult(SearchResult r) {
    auto [length,explored,expanded] = r;
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
    auto t = time([]{
        Maze m(input);
        std::cout << "Part 1: "; printResult(solve(compress(m)));
        auto part1_start = m.start[0];
        m.start = {};
        for(int x = part1_start.x-1; x <= part1_start.x+1; ++x) {
            for(int y = part1_start.y-1; y <= part1_start.y+1; ++y) {
                if(x == part1_start.x or y == part1_start.y) m.close({x,y});
                else {
                    m.start.push_back({x,y});
                    m.get({x,y}).tile = '@';
                }
            }
        }
        std::cout << "Part 2: ";printResult(solve(compress(m)));
    });
    std::cout << "Time: " << t << "s\n";
}
