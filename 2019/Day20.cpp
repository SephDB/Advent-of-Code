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
    int level = 0;
    Coord operator+(Coord o) const {return {x+o.x,y+o.y,level};}
    Coord& operator+=(Coord o) {*this = *this + o; return *this;}
    Coord operator-(Coord o) const {return {x-o.x,y-o.y,level};}
    Coord& operator-=(Coord o) {*this = *this - o; return *this;}
    Coord operator-() const {return {-x,-y,level};}
    Coord operator*(Coord o) const {auto res = std::complex<int>{x,y}*std::complex<int>{o.x,o.y}; return {res.real(),res.imag(),level};}
    Coord& operator*=(Coord o) {*this = *this * o; return *this;}
    bool operator==(Coord o) const {return x==o.x and y==o.y and level == o.level;}
    bool operator!=(Coord o) const {return not (*this == o);}
    bool operator<(Coord o) const {return std::tie(level,x,y) < std::tie(o.level,o.x,o.y);}
};

constexpr std::array<Coord,4> directions{{{1,0},{-1,0},{0,1},{0,-1}}};

int opposite_coord(int current) {
    return current + 1 - 2*(current%2);
}

struct Tile {
    std::bitset<4> open_directions;
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

        //Discard outermost layer
        height = lines.size()-2;
        width = lines[0].size()-2;
        auto offset = Coord{1,1};

        std::map<int,std::pair<Coord,Coord>> found_portals;

        auto lookup = [&lines](Coord c) {return lines[c.y][c.x];};
        for(auto y = 1; y < lines.size()-1; ++y) {
            for(auto x = 1; x < lines[0].size()-1; ++x) {
                Coord current = {x,y};
                auto tile = lookup(current);
                std::bitset<4> open_directions;
                if(tile >= 'A' and tile <= 'Z') {
                    //Portal
                    auto dir_to_tile = std::find_if(directions.begin(),directions.end(),[&](Coord dir){return lookup(current+dir)=='.';});
                    if(dir_to_tile != directions.end()) {
                        auto tile_loc = current + *dir_to_tile;
                        char other = lookup(current - *dir_to_tile);
                        if(*dir_to_tile < Coord{0,0}) {
                            std::swap(tile,other);
                        }
                        int name = (other-'A')*26+tile-'A';
                        auto match = found_portals.find(name);
                        if(tile == 'A' and other == 'A') {
                            start = tile_loc-offset;
                        } else if(tile == 'Z' and other == 'Z') {
                            end = tile_loc-offset;
                        } else if(match == found_portals.end()) {
                            found_portals[name] = {current,tile_loc};
                        } else {
                            auto [portal,t] = match->second;
                            portals[current-offset] = t-offset;
                            portals[portal-offset] = tile_loc-offset;
                        }
                    }
                } else if(tile == '.') {
                    for(int i=0; i < 4; ++i) {
                        open_directions[i] = lookup(current+directions[i]) != '#';
                    }
                }
                tiles.push_back({open_directions});
            }
        }
        remove_dead_ends();
    }

    Coord actual(Coord c) const {
        auto p = portals.find({c.x,c.y}); //Throw away level in the search
        if(p != portals.end()) {
            bool outer = c.x == 0 or c.x == width-1 or c.y == 0 or c.y == height-1;
            auto endpoint = p->second;
            c = {endpoint.x,endpoint.y,c.level};
            if(recursive)
                c.level += outer ? -1 : 1;
        }
        return c;
    }

    Tile& get(Coord c) {
        c = actual(c);
        return tiles[c.y*width+c.x];
    }

    const Tile& get(Coord c) const {
        c = actual(c);
        return tiles[c.y*width+c.x];
    }

    const Tile& operator[](Coord c) const {
        return get(c);
    }

    void close(Coord c) {
        auto& t = get(c);
        for_each_neighbor(c,[&](Coord loc) {
            auto& neighbor = get(loc);
            neighbor.for_all_dirs([&](int d, Coord back) {
                if(actual(loc+back) == actual(c)) {
                    neighbor.open_directions.reset(d);
                }
            });
        });
        t.open_directions &= 0;
    }

    template<typename F>
    void for_each_tile(F&& f) const {
        for(int y = 1; y < height-1; ++y) {
            for(int x = 1; x < width-1; ++x) {
                f(Coord{x,y},get(Coord{x,y}));
            }
        }
    }

    template<typename F>
    void for_each_tile(F&& f) {
        for(int y = 1; y < height-1; ++y) {
            for(int x = 1; x < width-1; ++x) {
                f(Coord{x,y},get(Coord{x,y}));
            }
        }
    }

    template<typename F>
    void for_each_neighbor(Coord c, F&& f) const {
        get(c).for_all_dirs([&](int,Coord dir) {
            auto next = actual(c+dir);
            if(next.level < 0 and recursive) return;
            f(actual(c+dir));
        });
    }

    void remove_dead_ends() {
        for_each_tile([this](Coord tc, auto&&) {
            while(get(tc).open_directions.count() == 1 and tc != start and tc != end) {
                Coord next;
                for_each_neighbor(tc,[&](Coord n) {
                    next = n;
                });
                close(tc);
                tc = next;
            }
        });
    }

    void draw() const {
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                Coord current{x,y};
                if(current == start) {
                    std::cout << 'A';
                } else if(current == end) {
                    std::cout << 'Z';
                } else if(portals.contains(current)) {
                    std::cout << 'P';
                } else if(get(current).open_directions.count() == 0) {
                    std::cout << '#';
                } else {
                    std::cout << '.';
                }
            }
            std::cout << '\n';
        }
    }

    std::vector<Tile> tiles;
    std::map<Coord,Coord> portals;
    std::size_t width, height;
    Coord start, end;
    bool recursive = false;
};

struct CompressedMaze {
    struct Path {
        int length;
        int level_diff;
    };
    struct Node {
        std::map<Coord,std::pair<Path,int>> connections;
        template<typename F>
        void for_all_paths(F&& f) const {
            for(auto& [coord,data] : connections) {
                auto& [path,next] = data;
                f(path,next);
            };
        }
    };
    std::vector<Node> nodes;
    int start;
    int end;
};

CompressedMaze compress(Maze m) {
    m.recursive = true;
    std::map<Coord,int> coord_to_node;
    std::vector<CompressedMaze::Node> nodes;
    m.for_each_tile([&](auto coord, auto& t) {
        if(coord == m.start or coord == m.end or t.open_directions.count() > 2) {
            coord_to_node[coord] = nodes.size();
            nodes.push_back({});
        }
    });
    
    for(auto [coord,n] : coord_to_node) {
        auto& node = nodes[n];
        if(coord == m.start or coord == m.end) {
            //Skip these bc they don't have actual portals behind them
            continue;
        }
        Coord ini = coord;
        ini.level = m.portals.size(); //Ensure we can take all portals if needed to reach a neighboring node

        m.for_each_neighbor(ini,[&,n=n](Coord ini_neighbor) {
            //If we hit this direction from the other side before
            if(node.connections[{ini_neighbor.x,ini_neighbor.y}].first.length > 0) return;

            Coord current = ini_neighbor;
            Coord last = ini;
            
            CompressedMaze::Path p = {1,current.level-last.level};
            int total = 300;

            while(not coord_to_node.contains({current.x,current.y})) {
                Coord n;
                m.for_each_neighbor(current,[&](Coord next) {
                    if(next != last) {
                        n = next;
                    }
                });
                last = current;
                current = n;
                p.length++;
                p.level_diff += (current.level - last.level);
            }
            auto& neighbor_id = coord_to_node.at({current.x,current.y});
            node.connections[{ini_neighbor.x,ini_neighbor.y}] = {p,neighbor_id};
            p.level_diff *= -1; //Level difference inverts
            nodes[neighbor_id].connections[{last.x,last.y}] = {p,n};
        });
    }
    return {nodes,coord_to_node.at(m.start),coord_to_node.at(m.end)};
}

int shortest_path(const CompressedMaze& m, bool recursive) {
    struct BFS_Node {
        int node;
        int level;
        bool operator<(BFS_Node o) const {return std::tie(level,node) < std::tie(o.level,o.node);}
    };
    std::map<BFS_Node, int> costs;
    auto lookup = [&](BFS_Node n) -> int& {
        if(not recursive) n.level = 0;
        auto res = costs.try_emplace(n,std::numeric_limits<int>::max());
        return res.first->second;
    };
    auto end = BFS_Node{m.end,0};

    using queue_elem = std::pair<int,BFS_Node>;
    std::priority_queue<queue_elem,std::vector<queue_elem>,std::greater<queue_elem>> q;
    q.push({0,BFS_Node{m.start,0}});
    while(not q.empty()) {
        auto [len,current] = q.top();
        q.pop();
        if(lookup(current) < len) continue;
        if(lookup(end) < len) continue;
        lookup(current) = len;
        m.nodes[current.node].for_all_paths([&,len=len,level=current.level](auto p, int next){
            auto nlevel = level+p.level_diff;
            if(recursive) {
                if(nlevel < 0) return;
                if(nlevel != 0 and (next == m.start or next == m.end)) return;
            }
            q.push({len+p.length,BFS_Node{next,nlevel}});
        });
    }
    return lookup(end);
}

int shortest_path(const Maze& m) {
    std::map<Coord,int> lengths;
    auto cost = [&](Coord c) -> int& {
        auto res = lengths.try_emplace(c,std::numeric_limits<int>::max());
        return res.first->second;
    };
    std::queue<std::pair<Coord,int>> q;
    q.emplace(m.start,0);
    while(not q.empty()) {
        auto [current,len] = q.front();
        q.pop();
        if(len >= cost(current)) continue;
        cost(current) = len;
        if(current == m.end) break;
        m.for_each_neighbor(current,[&q,len=len](Coord c) {
            q.emplace(c,len+1);
        });
    }
    return cost(m.end);
}

std::string_view input = 
R"(                                         J       L   P           P   V T       X                                           
                                         G       V   M           I   O R       C                                           
  #######################################.#######.###.###########.###.#.#######.#########################################  
  #...#.........#.#...#.#...#.........#.....#.#.....#.......#...#...#.....#.#...........#...........#.......#...#.......#  
  ###.#########.#.#.###.#.#######.###.###.###.#####.#.###.###.#####.###.###.#####.#.#######.#####.###.#.#####.#####.#####  
  #.#.....#...........#...#...#.#.#.....#.....#.#...#.#.....#.#.#.....#.....#.....#.#.#.#.......#.#...#.....#.#.#.....#.#  
  #.#####.#####.###.#####.###.#.#######.#.#.###.#.#####.###.#.#.#.#########.#.#.#####.#.###.#######.#.#######.#.###.###.#  
  #...#...#.....#...#.#...#.#.#...#.#.....#.#.......#...#.#.#...#.#.#.......#.#.#...............#.#.#.........#.#.#.....#  
  ###.#.#.#########.#.###.#.#.#.###.#.#.#########.#####.#.#.#.#.#.#.###.#####.###.#.#.#.###.#.###.###.#######.#.#.#.#####  
  #.....#...#.....#.........#...#.#...#.........#.....#.#...#.#.....#.....#.#.#...#.#.#.#...#...........#...#.........#.#  
  #########.#####.#.#.#.#.#.###.#.#######.#####.###.#####.###.###.###.#####.#.#.###.#.#.###.#.#####.#.###.#########.###.#  
  #.#.........#.....#.#.#.#.#.......#.......#.#.#.......#.#...#.#...#...#...#.....#.#.#...#.#.....#.#.............#.#...#  
  #.###.###.#####.#########.###.#####.###.#.#.#####.#####.###.#.###.#.###.###.#.###.#######.###.#.#.#.#######.#####.###.#  
  #...#...#.....#.#.#...#.........#...#...#.#.....#.#.#.....#...#...#.......#.#.#.........#...#.#.#.#.......#.#...#.#.#.#  
  #.#########.###.#.#.#####.#.###.###.#######.###.#.#.#.###.###.###.#.#.#.###.#####.#.###.#.#.###.#.#.#.#######.#.###.#.#  
  #...#.......#.#.#.....#.#.#.#...........#.....#.....#.#...#.....#.#.#.#.#.......#.#.#...#.#.#.#.#.#.#...#...#.#.#.#...#  
  #.#########.#.###.###.#.###########.#########.###########.#.###.###.###.#########.#########.#.#####.#.###.###.###.###.#  
  #...#.#.#.......#.#.....#.....#.....#.#.......#.#...#.#...#...#.#.#.#.#.#...#...........#.#.....#...#.....#...#.......#  
  ###.#.#.#####.#####.###.#####.#####.#.#####.###.###.#.###.#.#####.#.#.###.#######.#######.#.#######.#.###.###.#####.###  
  #.....#.#.#.#.....#.#.#.........#.#.....#.#...........#...#.....#.........#...#...#...#...#.#.#...#.#.#.#...#.#.#.....#  
  #.#.#.#.#.#.#.#######.#########.#.###.###.###.#####.#####.#.#######.#########.###.###.###.###.#.#####.#.#####.#.#.#.###  
  #.#.#.......#.....#.#.#.#.#.#.#.#.....#...#...#.#.#.#...#.#.#.#.#.....#...#...#...#...#.....#.....#.#.#.#...#...#.#.#.#  
  #####.#####.#.#####.#.#.#.#.#.#.###.#####.#.###.#.###.###.#.#.#.#.#.###.###.#.#.#####.#.#.#######.#.###.#.#####.###.#.#  
  #.#.#.#.....#.....#.......................#...#...#.......#.....#.#.#.......#.#.........#...#.#.#.#...#.....#.#.#.....#  
  #.#.#######.#.#######.###.#.#####.#######.#.#####.#.#.###.###.#####.#######.#.###.#######.#.#.#.#.#.#####.###.#.###.###  
  #...#...........#.#.#.#...#.#...#.#...#.#.#.....#...#...#.#...#...........#.#...#.....#.#.#...........#...#...#.#...#.#  
  ###.#.#.###.#.###.#.#.#.#####.#####.###.#.#.#######.#.#.#.#.###########.###.###.#.#.#.#.#.#.#.#.###.#.###.###.#.#.#.#.#  
  #.....#.#.#.#.#.....#.#...#...#.#.#.....#.#.....#...#.#.#.#...#.#.#...#.#...#.#.#.#.#.#.#.#.#.#.#.#.#...#.......#.#...#  
  ###.#.#.#.#.#####.###########.#.#.#.###.#.#.#####.#.#######.###.#.#.#.#.#.#.#.#.#.#####.#.#######.#######.#.###.#.#####  
  #.#.#.#.#...#.#.......#.......#.....#.#.#.#...#.#.#.......#.......#.#...#.#.#.#...#.#...#.#...#...#...#...#.#...#.....#  
  #.#########.#.###.#.#####.#.#.#####.#.#.#.###.#.###.###.#########.#.#######.#.#####.###.###.###.#####.#########.###.###  
  #.....#.#.#.#.....#...#...#.#.#.......#...#.....#.....#...#.......#.....#.................#...#...#.........#.#.#.#...#  
  #.#####.#.#.#######.#######.#########.#########.#########.#.#########.#.#######.#########.#.###.#####.#######.#.#.#.###  
  #.....#.#.#.#...#.#.#.#.#.#...#      F         O         O X         N U       W        #.#.....#.#.#...#.#.#.........#  
  #.#####.#.#.#.###.#.#.#.#.#.###      Z         G         D L         B Y       W        ###.#####.#.###.#.#.###.###.###  
  #...#.#.#.#.#.#...#.......#...#                                                         #.......#.....#...#...#.#.#...#  
  #.###.#.#.#.#.###.#.#.###.#.#.#                                                         #.#.#######.###.#.###.#.#.#.###  
PL....#.#.#...#...#...#...#...#.#                                                         #.#.#.#.#...#.#.#.....#.#.#.#.#  
  #.###.#.###.#.###.#.#.#.###.#.#                                                         #.###.#.#.###.###.#######.#.#.#  
  #.......#.#.......#.#.#.#...#..EL                                                     PM..#.........#.#.....#.#.......#  
  #.#.#.###.###.#.#.###########.#                                                         #.###.###.###.#.#.#.#.###.###.#  
  #.#.#.........#.#.#.....#...#.#                                                         #.#.#...#.......#.#.......#....BR
  #############.#.#####.###.#####                                                         #.#.###.#########.#.#.###.###.#  
YP..#.........#.#...#.#.#.#.....#                                                         #.#...#.....#.#.#.#.#...#...#.#  
  #.#.###.#.#####.###.#.#.###.#.#                                                         #.#.#.###.###.#.###########.#.#  
  #...#.#.#.#.#.#.#...#.....#.#.#                                                         #...#.....#.#.#.#...#.#...#.#.#  
  #.###.###.#.#.###.#.#.#.###.###                                                         ###########.#.#.#.###.###.###.#  
  #...#.#...........#...#........TH                                                       #.......#...............#...#.#  
  ###.#.#####################.###                                                         #.#####.#.###.###.#####.#.#.###  
EV..#.#...#.................#...#                                                       LV..#.#.....#...#...#.#.#...#.#..UN
  #.###.#.#####.#.###.#.#.###.###                                                         #.#.#.###.###.#.#.#.#.#####.#.#  
  #.#...#...#...#.#...#.#.#.#...#                                                         #.#...#.#.#.#.#.#.#.#.......#.#  
  #.#.#####.#.#.#.###.###.#.#####                                                         #######.###.#######.#.#######.#  
  #.#.#.....#.#.#...#.#.#.......#                                                         #.#.......#.....#.#.#.........#  
  #.#.#.#######.#######.###.#.#.#                                                         #.#####.#.###.###.#.###########  
  #...#...........#...#...#.#.#..XC                                                       #...#.#.#.........#...........#  
  #######.#########.###.###.#####                                                         #.#.#.###.###.###.###.#.#.#.###  
WV..#.#.#.#...............#.#.#.#                                                         #.#...#.#.#.#.#.......#.#.#....OD
  #.#.#.#.#.#.#.###.#####.###.#.#                                                         #.###.#.#.#.#########.#########  
  #.....#.#.#.#...#.#.....#.#.#..PI                                                       #.#.....#...#.....#.#.....#...#  
  #####.###.#####.#.#####.#.#.#.#                                                         #.#.#.#.###.###.###.#########.#  
AA........#.....#.#.#...#.......#                                                       UN..#.#.#.....#...#...#.#.#.#...#  
  #.#######.###########.###.###.#                                                         ###.###########.###.#.#.#.###.#  
  #...........#.#.#.#.....#.#.#.#                                                         #...#...#.......#..............OA
  #.#.#.#.#.#.#.#.#.###.#####.###                                                         #####.#.#.#.###.#.#.###.###.#.#  
  #.#.#.#.#.#.....#.............#                                                         #.#...#...#.#.....#.#...#...#.#  
  #################.#########.###                                                         #.#.###.#.#########.###########  
  #.....#.#.#.#.......#.....#...#                                                         #.#...#.#.#.......#.#.#.....#.#  
  #.#.###.#.#.#######.#.###.###.#                                                         #.#.###.#.#.#########.#.#####.#  
  #.#.#...#...#.#...#.....#.#.#.#                                                       SL....#.#.#.#.#.#.....#...#.#...#  
  #.#.#.#.#.###.#.#.#######.#.#.#                                                         #####.#####.#.#.#.#####.#.#.###  
EL..#...#.........#.........#.#..WV                                                       #...............#.#.#..........XP
  #.###########.###.#########.###                                                         #.#########.###.###.###.#####.#  
  #.#.......#...#...#...........#                                                         #.#.....#...#...#...#.......#.#  
  ###.#####.###.#########.#.###.#                                                         #.#.###.#.#.#####.#.#####.#.#.#  
UY..#...#.....#.#.........#.#....XP                                                     PL..#...#...#.#.#.#.#...#...#.#.#  
  #.#.###.#.###########.#.#.#####                                                         #.###.#######.#.###.#.#.#####.#  
  #.#.#...#.#.#.#.#.#...#.#.....#                                                         #...#...............#.....#.#.#  
  #.#.#.#####.#.#.#.###.###.#####                                                         #.#.###############.#.#.###.#.#  
  #...#...................#.....#                                                         #.#.#...........#...#.#.#...#.#  
  ###############.###############                                                         #.#.###.#######.###########.###  
  #...........#.#.#.............#                                                         #.#.#.....#...........#...#.#.#  
  #####.#.#.###.###.#######.###.#                                                         #########.#.#.###########.#.#.#  
  #.....#.#.#.#.#.........#.#...#                                                         #.#.#.....#.#.#...#...#...#....TH
  #.#####.###.#.#######.#####.###                                                         #.#.###.#.#.#########.###.#.#.#  
ZZ....#...#...#.#...........#.#.#                                                       JG........#.#.................#.#  
  ###.#.###.#.#.###.#.#.#####.#.#                                                         #.###.#.###.#.#.#####.###.#.###  
JH....#.....#.......#.#...#......EV                                                       #.#...#...#.#.#...#.....#.#...#  
  #.#.#######.#.#####.#.#####.###                                                         #.###.#########.###.#.#####.#.#  
  #.#.#.......#.#.#.#.#.....#.#.#                                                         #.#...#...........#.#.#.#...#.#  
  #.#.###.#.#.###.#.#.#######.#.#    V     J           Y       T     C     B       O      #.#####.###.#.###.#.###.###.#.#  
  #.#.#...#.#...#.....#.#.......#    O     H           P       R     K     R       A      #.#.....#...#...#.#.#.#.....#.#  
  #.#.#.#.###.#.#.#.#.#.#.###.#.#####.#####.###########.#######.#####.#####.#######.###############.#.#.#.###.#.#.#.#.###  
  #.#.#.#.#...#.#.#.#...#...#.#.#.......#.......#...#.....#.....#.......#...#...#.........#.#.#.....#.#.#.#.#.#...#.#.#.#  
  ###.#####.#.#######.###.###.#######.#####.#####.###.#######.###.#######.###.#.#.#.#.###.#.#.###.#####.###.#########.#.#  
  #.....#.#.#.....#...#...#.#.#...........#.........#.....#...#.#.....#.#.....#.#.#.#.#.....#...#.#.#.........#.........#  
  #.#####.#.###.#####.#####.#.#.###.###.#.###.#.###.#####.#.#.#.#.#####.###.###.#.#####.#.###.#####.###.###.###.#.#.###.#  
  #...#...#...#.#.........#...#.#...#.#.#.#.#.#...#.#.....#.#...#.......#.#.#...#.#...#.#.#.....#.#.....#.....#.#.#.#...#  
  #######.#####.#####.#######.#####.#.#.###.#####.#####.###.###########.#.#####.#.#.#########.###.###.#.###.#######.###.#  
  #.......#.#.....#...#.........#...#.#.#...#.........#...#.#.#.......#...#.#.#.#...#...#...#.#.....#.#.#.#.#.........#.#  
  #.#.#.#.#.#.#####.#####.###.#####.#.#.#.#####.#.#.#.###.#.#.#####.#.#.###.#.#.#.###.###.###.###.###.###.###.#.###.#####  
  #.#.#.#.......#...#...#.#...#.......#.#...#...#.#.#.#.#.#.....#...#.#.....#...#...#.#.#.........#.#...#...#.#.#.......#  
  #######.###.###.#.#.###############.###.#####.###.###.#.#.#.#.#.#.#.#.###.###.###.#.#.#.###.#.###.###.#.#.#.#####.#.#.#  
  #...#.#.#.....#.#.#.#.........#.#.........#.....#.#...#.#.#.#.#.#.#.....#.#...#...#...#...#.#.....#.#.#.#.......#.#.#.#  
  #.###.###.#.#####.#.#########.#.#######.#.###.#####.###.#.#.#####.###.#######.#.###.###.#######.#.#.#####.#.#.#.###.###  
  #.....#...#...#.#.....#...............#.#.#.....#.......#.#.#.....#.#...#.....#.....#.....#.....#.....#.#.#.#.#.#.....#  
  #####.#.#.###.#.#.#########.#####.###.#.###.###.#######.#.#####.###.#.#.#####.#.###.#.#.#######.#.#.###.#.#######.#.#.#  
  #.......#.#.#.#.....#.......#.#...#.#.....#.#...#.#.#...#...#.......#.#.#.....#.#.....#...#...#.#.#.#.......#.#.#.#.#.#  
  #.###.#.#.#.###.#######.#####.#.#.#####.#.###.###.#.#.#.#.#####.#####.#####.###.#.###.###.#.###.#.###########.#.#.#.#.#  
  #.#.#.#.#...#...#...#.#.#.#.#.#.#.....#.#...#...#.....#.#...#.#.....#.#.......#.#.#...#.#.....#.#.#.........#...#.#.#.#  
  #.#.#.#.#.###.#####.#.###.#.#.###.#######.#####.#.###.###.#.#.#.#############.#######.#.#.###.#####.#.###.###.#####.###  
  #.#.#.#.#...#...#.#.....................#.#.....#...#.#...#...#.#...#.#.......#...#.#...#.#.#.#.....#.#.........#.#...#  
  #.#.#.#.#.#######.#####.#####.#####.#########.#.#.#########.#.#.###.#.#####.###.###.#.#.###.#####.#########.#####.#####  
  #.#...#.#.#...#.........#.#.#.#.......#...#.#.#.#.........#.#.#.#.....#...........#...#.#.......#.#.#.#.#...........#.#  
  #.#######.###.###.#######.#.#####.#####.###.#.#######.#####.###.###.#.###.###.#######.#######.###.#.#.#.###.###.###.#.#  
  #.....#.#.#.#.#.#.#.#.....#.........#.........#.#...#...#.#...#.....#.#.#...#.#.#.#.#.....#...............#...#.#.....#  
  #.#####.###.#.#.###.#####.#######.#####.#.###.#.#.#.#.###.#.#########.#.#.###.#.#.#.#.#########.#####.#.#.#######.#.#.#  
  #.#.#.......................#.#.#.#...#.#.#.....#.#...#.#...#.#.#.......#.#.....#.#...#...#.........#.#.#.......#.#.#.#  
  #.#.#.###.#####.#########.#.#.#.#.#.###########.#.###.#.#.###.#.###.###.###.#####.###.###.#.#.#.#.###.#####.#.###.#####  
  #.#...#.#.#.#...#.........#.........#...#.#.....#...#.#.......#...#...#.#.....#...#.........#.#.#...#...#...#.#.#...#.#  
  #######.###.###.#####.#####.#####.#.#.###.###.###.#######.#.#.#.#####.#######.#.#.#.#####.#####.#.#.#.#####.###.#####.#  
  #...............#.....#.......#...#.....#.......#.....#...#.#.#.......#.......#.#.......#.....#.#.#.#.....#...........#  
  #####################################.#####.#######.#######.###.#########.#####.#######################################  
                                       O     C       N       X   F         S     W                                         
                                       G     K       B       L   Z         L     W                                         )";



int main() {
    Maze m(input);
    auto c = compress(m);
    //m.draw();
    std::cout << "Part 1: " << shortest_path(m) << '\n';
    std::cout << "Compressed 1: " << shortest_path(c,false) << '\n';
    auto t1 = time([&]{
        shortest_path(m);
    },100);
    auto t1_c = time([&]{
        shortest_path(compress(m),false);
    },1000);
    m.recursive = true;
    std::cout << "Part 2: " << shortest_path(m) << '\n';
    std::cout << "Compressed 2: " << shortest_path(c,true) << '\n';
    auto t2 = time([&]{
        shortest_path(m);
    },10);
    auto t2_c = time([&]{
        shortest_path(compress(m),true);
    },1000);
    std::cout << "\tUncompressed\tCompressed\n";
    std::cout << "1\t" << t1*1000 << "ms\t" << t1_c*1000 << "ms\n";
    std::cout << "2\t" << t2*1000 << "ms\t" << t2_c*1000 << "ms\n";
}
