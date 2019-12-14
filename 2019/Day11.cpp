#include <vector>
#include <tuple>
#include <iostream>
#include <deque>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <complex>

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

struct Robot {
    IntCode::Storage program;
    using pos = std::complex<std::int64_t>;
    struct Cmp {
        bool operator()(pos a, pos b) const {
            if(a.real() == b.real()) return a.imag() < b.imag();
            return a.real() < b.real();
        }
    };
    pos position{0,0};
    pos orientation{0,1};
    std::map<pos,int,Cmp> painted{};
    auto find(pos p) {
        if(painted.contains(p)) return painted[p];
        return 0;
    }
    void run() {
        IntCode::run(program,
            [this](){return find(position);},
            IntCode::output_every<2>([this](auto color, auto turn) {
                painted[position] = color;
                orientation *= (turn == 0 ? pos(0,1) : pos(0,-1));
                position += orientation;
            }));
    }
    void draw() {
        auto minmax = [this](auto&& key) {
            auto [min,max] = std::minmax_element(painted.begin(),painted.end(),[&key](auto a, auto b) {return key(a.first) < key(b.first);});
            return std::pair{key(min->first),key(max->first)};
        };
        auto [min_x,max_x] = minmax([](auto c){return c.real();});
        auto [min_y, max_y] = minmax([](auto c){return c.imag();});
        for(int y = max_y; y >= min_y; --y) {
            for(int x = min_x; x <= max_x; ++x) {
                std::cout << (find({x,y}) ? 'X' : ' ');
            }
            std::cout << '\n';
        }
    }
};

int main() {
    IntCode::Storage program = {{3,8,1005,8,318,1106,0,11,0,0,0,104,1,104,0,3,8,1002,8,-1,10,1001,10,1,10,4,10,108,1,8,10,4,10,1002,8,1,28,1,107,14,10,1,107,18,10,3,8,102,-1,8,10,101,1,10,10,4,10,108,1,8,10,4,10,102,1,8,58,1006,0,90,2,1006,20,10,3,8,1002,8,-1,10,101,1,10,10,4,10,1008,8,1,10,4,10,1001,8,0,88,2,103,2,10,2,4,7,10,3,8,1002,8,-1,10,101,1,10,10,4,10,1008,8,1,10,4,10,1001,8,0,118,1,1009,14,10,1,1103,9,10,3,8,1002,8,-1,10,1001,10,1,10,4,10,108,0,8,10,4,10,1002,8,1,147,1006,0,59,1,104,4,10,2,106,18,10,3,8,102,-1,8,10,1001,10,1,10,4,10,1008,8,0,10,4,10,101,0,8,181,2,4,17,10,1006,0,36,1,107,7,10,2,1008,0,10,3,8,1002,8,-1,10,1001,10,1,10,4,10,108,0,8,10,4,10,101,0,8,217,3,8,102,-1,8,10,1001,10,1,10,4,10,1008,8,0,10,4,10,101,0,8,240,1006,0,64,3,8,102,-1,8,10,1001,10,1,10,4,10,108,0,8,10,4,10,1002,8,1,264,3,8,1002,8,-1,10,1001,10,1,10,4,10,1008,8,1,10,4,10,1001,8,0,287,1,1104,15,10,1,102,8,10,1006,0,2,101,1,9,9,1007,9,940,10,1005,10,15,99,109,640,104,0,104,1,21102,932700857236,1,1,21101,335,0,0,1106,0,439,21101,0,387511792424,1,21101,346,0,0,1106,0,439,3,10,104,0,104,1,3,10,104,0,104,0,3,10,104,0,104,1,3,10,104,0,104,1,3,10,104,0,104,0,3,10,104,0,104,1,21101,46372252675,0,1,21102,393,1,0,1106,0,439,21101,97806162983,0,1,21102,404,1,0,1105,1,439,3,10,104,0,104,0,3,10,104,0,104,0,21102,1,825452438376,1,21101,0,427,0,1106,0,439,21102,709475586836,1,1,21101,0,438,0,1106,0,439,99,109,2,22101,0,-1,1,21101,40,0,2,21102,1,470,3,21102,1,460,0,1106,0,503,109,-2,2106,0,0,0,1,0,0,1,109,2,3,10,204,-1,1001,465,466,481,4,0,1001,465,1,465,108,4,465,10,1006,10,497,1101,0,0,465,109,-2,2105,1,0,0,109,4,2102,1,-1,502,1207,-3,0,10,1006,10,520,21102,1,0,-3,21202,-3,1,1,21202,-2,1,2,21101,0,1,3,21101,0,539,0,1106,0,544,109,-4,2105,1,0,109,5,1207,-3,1,10,1006,10,567,2207,-4,-2,10,1006,10,567,22101,0,-4,-4,1106,0,635,21202,-4,1,1,21201,-3,-1,2,21202,-2,2,3,21102,586,1,0,1105,1,544,22101,0,1,-4,21102,1,1,-1,2207,-4,-2,10,1006,10,605,21102,0,1,-1,22202,-2,-1,-2,2107,0,-3,10,1006,10,627,22101,0,-1,1,21102,1,627,0,106,0,502,21202,-2,-1,-2,22201,-4,-2,-4,109,-5,2105,1,0}};
    {
        Robot painter{program};
        painter.run();
        std::cout << "Part 1: " << painter.painted.size() << '\n';
    }
    {
        Robot painter{program};
        painter.painted[{0,0}] = 1;
        painter.run();
        std::cout << "Part 2:\n";
        painter.draw();
    }
}
