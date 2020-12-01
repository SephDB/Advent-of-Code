#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

enum class Type {
    snd,
    set,
    add,
    mul,
    mod,
    rcv,
    jgz
};

struct Ins {
    Type type;
    int reg;
    int64_t second = 0;
    bool is_constant = true;
};

auto parse(std::string_view input) {
    std::vector<Ins> instructions;
    split(input,'\n',[&](std::string_view line) {
        auto ins = line.substr(0,3);
        int reg = line[4] - 'a';
        if(ins == "snd") {
            instructions.push_back({Type::snd,reg});
        } else if(ins == "rcv") {
            instructions.push_back({Type::rcv,reg});
        } else {
            Type t = Type::set;
            if(ins == "add") t = Type::add;
            else if(ins == "mul") t = Type::mul;
            else if(ins == "mod") t = Type::mod;
            else if(ins == "jgz") t = Type::jgz;

            auto second = line.substr(6);
            if(second[0] >= 'a' && second[0] <= 'z') {
                instructions.push_back({t,reg,second[0]-'a',false});
            } else {
                instructions.push_back({t,reg,to_int(second),true});
            }
        }
    });
    return instructions;
}

void solution(std::string_view input) {
    auto instructions = parse(input);
    std::array<int64_t,26> regs;
    std::fill(regs.begin(),regs.end(),0);
    int sound_played = 0;
    int ins = 0;
    auto get = [&regs](Ins s) {
        if(s.is_constant) return s.second;
        return regs[s.second];
    };
    while(ins >= 0 && ins < instructions.size()) {
        auto current = instructions[ins];
        switch(current.type) {
            case Type::snd:
                sound_played = regs[current.reg];
                break;
            case Type::set:
                regs[current.reg] = get(current);
                break;
            case Type::add:
                regs[current.reg] += get(current);
                break;
            case Type::mul:
                regs[current.reg] *= get(current);
                break;
            case Type::mod:
                regs[current.reg] %= get(current);
                break;
            case Type::rcv:
                if(regs[current.reg] != 0) {
                    std::cout << "Part 1: " << sound_played << '\n';
                    return;
                }
                break;
            case Type::jgz:
                if(regs[current.reg] > 0) {
                    ins += get(current) - 1;
                }
                break;
        }
        ++ins;
    }
}

std::string_view input = R"(set i 31
set a 1
mul p 17
jgz p p
mul a 2
add i -1
jgz i -2
add a -1
set i 127
set p 735
mul p 8505
mod p a
mul p 129749
add p 12345
mod p a
set b p
mod b 10000
snd b
add i -1
jgz i -9
jgz a 3
rcv b
jgz b -1
set f 0
set i 126
rcv a
rcv b
set p a
mul p -1
add p b
jgz p 4
snd a
set a b
jgz 1 3
snd b
set f 1
add i -1
jgz i -11
snd a
jgz f -16
jgz a -19)";

int main() {
    solution(input);
}
