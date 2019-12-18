#include <vector>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <functional>
#include <deque>
#include <span>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <complex>
#include <sstream>

namespace IntCode {
    struct Storage {
        std::vector<std::int64_t> main;
        std::map<std::int64_t,std::int64_t> extra{};
        std::int64_t& operator[](std::size_t loc) {
            return loc < main.size() ? main[loc] : extra[loc];
        }
    };

    struct ins {
        std::int64_t op;
        std::array<std::int64_t,3> modes;
    };

    ins decode(std::int64_t instruction) {
        return {instruction % 100, {instruction / 100 % 10, instruction / 1000 % 10, instruction / 10000 % 10}};
    }

    class Program {
        public:
            Program(Storage s) : prog(s) {};
            struct RunResult {
                std::vector<std::int64_t> output;
                bool input_requested;
            };
            RunResult run(std::deque<std::int64_t> input) {
                std::array sizes = {0,4,4,2,2,3,3,4,4,2};
                std::vector<std::int64_t> output;
                while(prog[loc] != 99) {
                    auto [op,modes] = decode(prog[loc]);
                    auto fetch = [&,modes=modes](std::int64_t num) -> std::int64_t& {
                        auto& address = prog[loc+num];
                        switch(modes[num-1]) {
                            case 0:
                                return prog[address];
                            case 1:
                                return address;
                            case 2:
                                return prog[address+relative_base];
                        }
                        return modes[num-1] ? prog[loc+num] : prog[prog[loc + num]];
                    };
                    switch(op) {
                        case 1:
                            fetch(3) = fetch(1) + fetch(2);
                            break;
                        case 2:
                            fetch(3) = fetch(1) * fetch(2);
                            break;
                        case 3:
                            if(input.empty()) return {output,true};
                            fetch(1) = input.front();
                            input.pop_front();
                            break;
                        case 4:
                            output.push_back(fetch(1));
                            break;
                        case 5:
                            if(fetch(1)) loc = fetch(2) - 3;
                            break;
                        case 6:
                            if(not fetch(1)) loc = fetch(2) - 3;
                            break;
                        case 7:
                            fetch(3) = fetch(1) < fetch(2);
                            break;
                        case 8:
                            fetch(3) = fetch(1) == fetch(2);
                            break;
                        case 9:
                            relative_base += fetch(1);
                            break;
                    }
                    loc += sizes[op];
                }
                return {output,false};
            }
        private:
            Storage prog;
            std::size_t loc = 0;
            std::int64_t relative_base = 0;
    };

    template<typename InputFunc, typename OutputFunc>
    void run(Storage prog, InputFunc&& input, OutputFunc&& out) {
        Program p(std::move(prog));
        Program::RunResult res = p.run({});
        for(auto o : res.output) out(o);
        while(res.input_requested) {
            res = p.run({input()});
            for(auto o : res.output) out(o);
        }
    }

    template<int N, typename F>
    auto output_every(F output) {
        return [output, a = std::array<std::int64_t,N>{},current=0](std::int64_t in) mutable {
            a[current++] = in;
            if(current == N) {
                std::apply(output,a);
                current = 0;
            }
        };
    }
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
    int x,y;
    Coord operator+(Coord o) const {return {x+o.x,y+o.y};}
    Coord& operator+=(Coord o) {*this = *this + o; return *this;}
    Coord operator*(int i) const {return {x*i,y*i};}
    Coord& operator*=(int i) {*this = *this*i; return *this;}
    Coord operator*(Coord o) const {auto res = std::complex<int>{x,y}*std::complex<int>{o.x,o.y}; return {res.real(),res.imag()};}
    Coord& operator*=(Coord o) {*this = *this * o; return *this;}
    Coord operator/(Coord o) const {auto res = std::complex<int>{x,y}/std::complex<int>{o.x,o.y}; return {res.real(),res.imag()};}
    Coord& operator/=(Coord o) {*this = *this * o; return *this;}
};

struct Grid {
    std::string_view grid;
    int width, height;
    char operator[](Coord c) const {return grid[c.x+(width+1)*c.y];}
    std::pair<Coord,Coord> start() const {
        std::map<char,Coord> dir = {
            {'>',{1,0}},
            {'<',{-1,0}},
            {'^',{0,-1}},
            {'v',{0,1}}
        };
        auto pos = grid.find_first_of("><^v");
        return {{pos % (width+1), pos / (width+1)},dir[grid[pos]]};
    }
};

using path_segment = std::pair<char,int>;

std::ostream& operator<<(std::ostream& o, path_segment p) {
    o << p.first << ',' << p.second;
    return o;
}

std::vector<path_segment> path(const Grid& grid) {
    std::vector<path_segment> ret;
    auto inbounds = [&](Coord c) {
        return c.x >= 0 and c.x < grid.width and c.y >= 0 and c.y < grid.height;
    };
    auto is_scaffold = [&](Coord c) {
        return inbounds(c) and grid[c] != '.';
    };
    const Coord left{0,-1}, right{0,1};
    auto nextDir = [&](Coord c, Coord dir) {
        auto next = c+dir*left;
        if(is_scaffold(next)) {
            return std::pair{'L',dir*left};
        }
        next = c+dir*right;
        if(is_scaffold(next))
            return std::pair{'R',dir*right};
        return std::pair{' ',dir};
    };
    auto [start,startdir] = grid.start();
    while(true) {
        auto [c,dir] = nextDir(start,startdir);
        if(c == ' ') break;
        int current = 0;
        while(is_scaffold(start+dir*(++current)));
        ret.emplace_back(c,--current);
        startdir = dir;
        start += dir*current;
    }
    return ret;
}

struct CompressionResult {
    std::vector<char> M;
    std::span<path_segment> A,B,C;
};

struct SplitResult {
    std::vector<std::span<path_segment>> split_result;
    std::vector<int> remaining_indexes;
};

SplitResult split_with_delimiter(std::span<path_segment> in, std::span<path_segment> delim) {
    std::vector<std::span<path_segment>> splits;
    std::vector<int> positions;
    std::default_searcher s(delim.begin(),delim.end());
    for(auto start = in.begin(); start != in.end();) {
        auto next = std::search(start,in.end(),s);
        if(next != start) {
            positions.push_back(splits.size());
            splits.emplace_back(start,next);
        }
        start = next;
        if(next != in.end()) {
            splits.push_back(delim);
            start += delim.size();
        }
    }
    return {splits,positions};
}

CompressionResult compress(std::span<path_segment> input, std::size_t max_size) {
    CompressionResult ret;
    //Set A to be greedy first and go down in size later
    for(ret.A = {input.begin(),input.begin()+max_size}; ret.A.size() >= 1; ret.A = ret.A.first(ret.A.size()-1)) {
        auto [split_in,remaining_positions] = split_with_delimiter(input,ret.A);
        ret.B = split_in[remaining_positions[0]];
        for(ret.B = ret.B.first(std::min(ret.B.size(),max_size)); ret.B.size() >= 1; ret.B = ret.B.first(ret.B.size()-1)) {
            std::vector<std::span<path_segment>> end_result;
            std::vector<int> C_locs;
            auto it = remaining_positions.begin();
            for(auto i = 0; i < split_in.size(); ++i) {
                if(it == remaining_positions.end() or i != *it) end_result.push_back(split_in[i]);
                else {
                    if(split_in[i].size() == ret.B.size() and std::equal(ret.B.begin(),ret.B.end(),split_in[i].begin())) {
                        end_result.push_back(ret.B);
                    } else {
                        auto [splitB,Cpos] = split_with_delimiter(split_in[i],ret.B);
                        for(auto& i : Cpos) i += end_result.size();
                        end_result.insert(end_result.end(),splitB.begin(),splitB.end());
                        C_locs.insert(C_locs.end(),Cpos.begin(),Cpos.end());
                    }
                    ++it;
                }
            }
            if(end_result[C_locs[0]].size() > max_size) continue;
            if(std::mismatch(C_locs.begin(),C_locs.end()-1,C_locs.begin()+1,[&](auto a, auto b) {
                return std::equal(end_result[a].begin(),end_result[a].end(),end_result[b].begin(),end_result[b].end());
            }).second == C_locs.end()) {
                ret.C = end_result[C_locs[0]];
                for(auto i : C_locs) end_result[i] = ret.C;
                ret.M = {};
                for(auto el : end_result) {
                    if(el.begin() == ret.A.begin()) ret.M.push_back('A');
                    else if(el.begin() == ret.B.begin()) ret.M.push_back('B');
                    else ret.M.push_back('C');
                }
                return ret;
            }
        }
    }
    return ret;
}

template<typename Collection>
void to_stream(std::ostream& o, const Collection& c) {
    bool first = true;
    for(auto&& i : c) {
        if(not first) o << ',';
        o << i;
        first = false;
    }
}

int main() {
    IntCode::Storage input{{1,330,331,332,109,3788,1102,1,1182,15,1102,1485,1,24,1002,0,1,570,1006,570,36,1001,571,0,0,1001,570,-1,570,1001,24,1,24,1105,1,18,1008,571,0,571,1001,15,1,15,1008,15,1485,570,1006,570,14,21101,58,0,0,1105,1,786,1006,332,62,99,21101,0,333,1,21102,73,1,0,1106,0,579,1102,1,0,572,1101,0,0,573,3,574,101,1,573,573,1007,574,65,570,1005,570,151,107,67,574,570,1005,570,151,1001,574,-64,574,1002,574,-1,574,1001,572,1,572,1007,572,11,570,1006,570,165,101,1182,572,127,102,1,574,0,3,574,101,1,573,573,1008,574,10,570,1005,570,189,1008,574,44,570,1006,570,158,1106,0,81,21102,1,340,1,1106,0,177,21102,1,477,1,1106,0,177,21101,0,514,1,21101,176,0,0,1106,0,579,99,21101,184,0,0,1105,1,579,4,574,104,10,99,1007,573,22,570,1006,570,165,1001,572,0,1182,21102,1,375,1,21101,0,211,0,1105,1,579,21101,1182,11,1,21102,1,222,0,1105,1,979,21102,388,1,1,21101,0,233,0,1105,1,579,21101,1182,22,1,21102,244,1,0,1105,1,979,21102,401,1,1,21101,0,255,0,1106,0,579,21101,1182,33,1,21101,0,266,0,1106,0,979,21102,1,414,1,21102,277,1,0,1106,0,579,3,575,1008,575,89,570,1008,575,121,575,1,575,570,575,3,574,1008,574,10,570,1006,570,291,104,10,21102,1182,1,1,21102,1,313,0,1105,1,622,1005,575,327,1101,1,0,575,21101,0,327,0,1106,0,786,4,438,99,0,1,1,6,77,97,105,110,58,10,33,10,69,120,112,101,99,116,101,100,32,102,117,110,99,116,105,111,110,32,110,97,109,101,32,98,117,116,32,103,111,116,58,32,0,12,70,117,110,99,116,105,111,110,32,65,58,10,12,70,117,110,99,116,105,111,110,32,66,58,10,12,70,117,110,99,116,105,111,110,32,67,58,10,23,67,111,110,116,105,110,117,111,117,115,32,118,105,100,101,111,32,102,101,101,100,63,10,0,37,10,69,120,112,101,99,116,101,100,32,82,44,32,76,44,32,111,114,32,100,105,115,116,97,110,99,101,32,98,117,116,32,103,111,116,58,32,36,10,69,120,112,101,99,116,101,100,32,99,111,109,109,97,32,111,114,32,110,101,119,108,105,110,101,32,98,117,116,32,103,111,116,58,32,43,10,68,101,102,105,110,105,116,105,111,110,115,32,109,97,121,32,98,101,32,97,116,32,109,111,115,116,32,50,48,32,99,104,97,114,97,99,116,101,114,115,33,10,94,62,118,60,0,1,0,-1,-1,0,1,0,0,0,0,0,0,1,0,12,0,109,4,1202,-3,1,587,20101,0,0,-1,22101,1,-3,-3,21101,0,0,-2,2208,-2,-1,570,1005,570,617,2201,-3,-2,609,4,0,21201,-2,1,-2,1106,0,597,109,-4,2106,0,0,109,5,2101,0,-4,630,20102,1,0,-2,22101,1,-4,-4,21102,0,1,-3,2208,-3,-2,570,1005,570,781,2201,-4,-3,653,20101,0,0,-1,1208,-1,-4,570,1005,570,709,1208,-1,-5,570,1005,570,734,1207,-1,0,570,1005,570,759,1206,-1,774,1001,578,562,684,1,0,576,576,1001,578,566,692,1,0,577,577,21102,1,702,0,1105,1,786,21201,-1,-1,-1,1106,0,676,1001,578,1,578,1008,578,4,570,1006,570,724,1001,578,-4,578,21102,731,1,0,1105,1,786,1106,0,774,1001,578,-1,578,1008,578,-1,570,1006,570,749,1001,578,4,578,21102,756,1,0,1105,1,786,1105,1,774,21202,-1,-11,1,22101,1182,1,1,21101,774,0,0,1105,1,622,21201,-3,1,-3,1105,1,640,109,-5,2106,0,0,109,7,1005,575,802,21002,576,1,-6,20101,0,577,-5,1106,0,814,21101,0,0,-1,21101,0,0,-5,21102,1,0,-6,20208,-6,576,-2,208,-5,577,570,22002,570,-2,-2,21202,-5,49,-3,22201,-6,-3,-3,22101,1485,-3,-3,1202,-3,1,843,1005,0,863,21202,-2,42,-4,22101,46,-4,-4,1206,-2,924,21101,0,1,-1,1105,1,924,1205,-2,873,21101,0,35,-4,1106,0,924,2101,0,-3,878,1008,0,1,570,1006,570,916,1001,374,1,374,2102,1,-3,895,1102,1,2,0,1201,-3,0,902,1001,438,0,438,2202,-6,-5,570,1,570,374,570,1,570,438,438,1001,578,558,921,21001,0,0,-4,1006,575,959,204,-4,22101,1,-6,-6,1208,-6,49,570,1006,570,814,104,10,22101,1,-5,-5,1208,-5,47,570,1006,570,810,104,10,1206,-1,974,99,1206,-1,974,1101,0,1,575,21101,0,973,0,1105,1,786,99,109,-7,2105,1,0,109,6,21102,0,1,-4,21102,0,1,-3,203,-2,22101,1,-3,-3,21208,-2,82,-1,1205,-1,1030,21208,-2,76,-1,1205,-1,1037,21207,-2,48,-1,1205,-1,1124,22107,57,-2,-1,1205,-1,1124,21201,-2,-48,-2,1106,0,1041,21101,0,-4,-2,1105,1,1041,21102,1,-5,-2,21201,-4,1,-4,21207,-4,11,-1,1206,-1,1138,2201,-5,-4,1059,2101,0,-2,0,203,-2,22101,1,-3,-3,21207,-2,48,-1,1205,-1,1107,22107,57,-2,-1,1205,-1,1107,21201,-2,-48,-2,2201,-5,-4,1090,20102,10,0,-1,22201,-2,-1,-2,2201,-5,-4,1103,1202,-2,1,0,1106,0,1060,21208,-2,10,-1,1205,-1,1162,21208,-2,44,-1,1206,-1,1131,1106,0,989,21102,439,1,1,1106,0,1150,21101,477,0,1,1105,1,1150,21101,514,0,1,21101,0,1149,0,1106,0,579,99,21101,0,1157,0,1105,1,579,204,-2,104,10,99,21207,-3,22,-1,1206,-1,1138,1201,-5,0,1176,2101,0,-4,0,109,-6,2105,1,0,26,11,38,1,9,1,18,11,9,1,9,1,18,1,9,1,9,1,9,1,18,1,9,1,9,1,3,11,14,1,9,1,9,1,3,1,5,1,3,1,14,1,9,1,9,1,3,1,5,11,8,1,9,1,9,1,3,1,9,1,5,1,8,1,9,1,1,11,1,1,9,1,5,1,8,1,9,1,1,1,7,1,1,1,1,1,9,1,5,1,8,1,1,13,5,1,1,1,1,1,9,1,5,1,8,1,1,1,7,1,1,1,1,1,5,1,1,1,1,1,9,1,5,1,2,7,1,1,7,11,1,1,1,1,9,1,5,1,10,1,9,1,1,1,7,1,1,1,9,1,5,1,10,1,9,1,1,1,7,13,5,1,10,1,9,1,1,1,9,1,15,1,10,1,9,1,1,11,5,11,10,1,9,1,17,1,20,1,9,11,7,1,20,1,19,1,7,1,20,1,19,1,7,1,40,1,7,1,40,1,7,1,40,1,7,1,40,11,46,1,1,1,46,13,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,32,7,3,7,32,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,1,9,1,38,11,6}};
    std::string out;
    IntCode::run(input, [](){return 0;}, [&](auto x) {out.push_back(x);});
    out.pop_back();
    std::cout << out;
    auto lines = split(out,'\n');
    while(lines.back().size() == 0) lines.pop_back();
    int total = 0;
    for(int y = 1; y < lines.size()-1; ++y) {
        for(int x = 1; x < lines[0].size()-1; ++x) {
            if(lines[y][x] == '#' and lines[y-1][x] == '#' and lines[y+1][x] == '#'
                and lines[y][x-1] == '#' and lines[y][x+1] == '#') {
                    total += y*x;
            }
        }
    }
    std::cout << "Part 1: " << total << '\n';
    Grid g{out,lines[0].size(),lines.size()};
    auto p = path(g);
    std::cout << "Path: ";
    for(auto [c,count] : p) {
        std::cout << c << ',' << count << ',';
    }
    std::cout << '\n';

    auto compressed = compress(p,4);

    std::ostringstream prog_input;
    to_stream(prog_input, compressed.M);
    prog_input << '\n';
    to_stream(prog_input, compressed.A);
    prog_input << '\n';
    to_stream(prog_input, compressed.B);
    prog_input << '\n';
    to_stream(prog_input, compressed.C);
    prog_input << "\nn\n";
    std::string prog_in = prog_input.str();

    input[0] = 2;
    std::int64_t output;
    IntCode::run(input, [s=std::string_view{prog_in}]()mutable{auto next = s[0]; s.remove_prefix(1); return next;},
        [&](auto in) { output = in;});
    std::cout << "Part 2: " << output << '\n';
}

