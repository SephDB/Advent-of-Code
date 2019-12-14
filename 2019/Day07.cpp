#include <array>
#include <tuple>
#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>

std::array storage = {3,8,1001,8,10,8,105,1,0,0,21,42,55,64,85,98,179,260,341,422,99999,3,9,101,2,9,9,102,5,9,9,1001,9,2,9,1002,9,5,9,4,9,99,3,9,1001,9,5,9,1002,9,4,9,4,9,99,3,9,101,3,9,9,4,9,99,3,9,1002,9,4,9,101,3,9,9,102,5,9,9,101,4,9,9,4,9,99,3,9,1002,9,3,9,1001,9,3,9,4,9,99,3,9,1002,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,101,1,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,101,1,9,9,4,9,3,9,101,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1002,9,2,9,4,9,3,9,1001,9,2,9,4,9,99,3,9,1002,9,2,9,4,9,3,9,101,2,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,101,2,9,9,4,9,3,9,102,2,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,101,2,9,9,4,9,99,3,9,1002,9,2,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,101,2,9,9,4,9,3,9,101,2,9,9,4,9,3,9,102,2,9,9,4,9,3,9,102,2,9,9,4,9,99,3,9,102,2,9,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,1001,9,1,9,4,9,3,9,1001,9,1,9,4,9,3,9,101,1,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,2,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,1002,9,2,9,4,9,99,3,9,1001,9,1,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1002,9,2,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1002,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,102,2,9,9,4,9,99};

struct ins {
    int op;
    std::array<bool,3> modes;
};

ins decode(int instruction) {
    return {instruction % 100, {instruction / 100 % 2 == 1, instruction / 1'000 % 2 == 1, instruction / 10'000 % 2 == 1}};
}

struct RunResult {
    int result;
    bool input_requested;
};

class Amplifier {
    public:
        Amplifier(int phase) : inputs{phase} {};
        RunResult run(int input) {
            inputs.push_back(input);
            std::array sizes = {0,4,4,2,2,3,3,4,4};
            int out = 0;
            while(loc < prog.size() and prog[loc] != 99) {
                auto [op,modes] = decode(prog[loc]);
                auto fetch = [&](int num) -> int& {
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
                        if(inputs.empty()) {
                            return {out,true};
                        }
                        fetch(1) = inputs.front();
                        inputs.pop_front();
                        break;
                    case 4:
                        out = fetch(1);
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
                }
                loc += sizes[op];
            }
            return {out,false};
        }
    private:
        decltype(storage) prog = storage;
        int loc = 0;
        std::deque<int> inputs;
};

int run(std::array<int,5> phase_settings) {
    int res = 0;
    do {
        RunResult r = {0,true};
        std::vector<Amplifier> amps(phase_settings.begin(),phase_settings.end());
        while(r.input_requested) {
            for(auto& amp : amps) {
                r = amp.run(r.result);
            }
        }
        res = std::max(r.result,res);
    } while(std::next_permutation(phase_settings.begin(),phase_settings.end()));
    return res;
}

int main() {
    std::cout << "Part 1: " << run({0,1,2,3,4}) << '\n';
    std::cout << "Part 2: " << run({5,6,7,8,9}) << '\n';
}
