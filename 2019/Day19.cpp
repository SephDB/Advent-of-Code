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

int main() {
    IntCode::Storage program = {{109,424,203,1,21102,11,1,0,1106,0,282,21102,18,1,0,1106,0,259,2102,1,1,221,203,1,21102,1,31,0,1106,0,282,21101,0,38,0,1106,0,259,21001,23,0,2,22101,0,1,3,21101,1,0,1,21102,57,1,0,1106,0,303,2101,0,1,222,21002,221,1,3,20102,1,221,2,21101,259,0,1,21101,0,80,0,1105,1,225,21102,1,83,2,21101,0,91,0,1105,1,303,1202,1,1,223,20102,1,222,4,21101,259,0,3,21101,225,0,2,21102,1,225,1,21101,118,0,0,1106,0,225,21002,222,1,3,21101,179,0,2,21102,1,133,0,1105,1,303,21202,1,-1,1,22001,223,1,1,21101,0,148,0,1105,1,259,1202,1,1,223,21001,221,0,4,20101,0,222,3,21102,1,19,2,1001,132,-2,224,1002,224,2,224,1001,224,3,224,1002,132,-1,132,1,224,132,224,21001,224,1,1,21102,1,195,0,105,1,109,20207,1,223,2,21002,23,1,1,21102,-1,1,3,21102,214,1,0,1106,0,303,22101,1,1,1,204,1,99,0,0,0,0,109,5,2101,0,-4,249,21201,-3,0,1,21202,-2,1,2,22101,0,-1,3,21101,0,250,0,1106,0,225,22101,0,1,-4,109,-5,2106,0,0,109,3,22107,0,-2,-1,21202,-1,2,-1,21201,-1,-1,-1,22202,-1,-2,-2,109,-3,2106,0,0,109,3,21207,-2,0,-1,1206,-1,294,104,0,99,21202,-2,1,-2,109,-3,2105,1,0,109,5,22207,-3,-4,-1,1206,-1,346,22201,-4,-3,-4,21202,-3,-1,-1,22201,-4,-1,2,21202,2,-1,-1,22201,-4,-1,1,21201,-2,0,3,21101,343,0,0,1105,1,303,1106,0,415,22207,-2,-3,-1,1206,-1,387,22201,-3,-2,-3,21202,-2,-1,-1,22201,-3,-1,3,21202,3,-1,-1,22201,-3,-1,2,21201,-4,0,1,21101,384,0,0,1106,0,303,1105,1,415,21202,-4,-1,-4,22201,-4,-3,-4,22202,-3,-2,-2,22202,-2,-4,-4,22202,-3,-2,-3,21202,-4,-1,-2,22201,-3,-2,1,21202,1,1,-4,109,-5,2105,1,0}};
    auto get = [&](int x, int y) {
        IntCode::Program p{program};
        auto result = p.run({x,y});
        return result.output[0];
    };
    int total = 0;
    std::array<std::array<char,50>,50> grid;
    for(int x = 0; x < 50; ++x) {
        for(int y = 0; y < 50; ++y) {
            total += get(x,y);
            grid[y][x] = get(x,y) ? '#' : '.';
        }
    }
    std::cout << "Part 1: " << total << '\n';

    //somewhat arbitrary starting points chosen, but skips the weirdness at the start
    int x = 50;
    int y = 200;
    auto size = 100;
    do {
        ++y;
        while(not get(x,y)) ++x;
    } while(not get(x+size-1,y-(size-1)));
    std::cout << "Part 2: " << x*10000 + y-(size-1);
}
