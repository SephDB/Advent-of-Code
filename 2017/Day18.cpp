#include <iostream>
#include <string_view>
#include <array>
#include <deque>
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
    int64_t arg = 0;
    bool reg_constant = false;
    bool arg_constant = true;
};

auto parse(std::string_view input) {
    std::vector<Ins> instructions;
    split(input,'\n',[&](std::string_view line) {
        Ins i{};
        auto ins = line.substr(0,3);
        int reg = line[4];
        if(reg >= 'a' && reg <= 'z') {
            i.reg = reg - 'a';
            i.reg_constant = false;
        } else {
            i.reg = reg - '0';
            i.reg_constant = true;
        }
        if(ins == "snd") {
            i.type = Type::snd;
        } else if(ins == "rcv") {
            i.type = Type::rcv;
        } else {
            Type t = Type::set;
            if(ins == "add") t = Type::add;
            else if(ins == "mul") t = Type::mul;
            else if(ins == "mod") t = Type::mod;
            else if(ins == "jgz") t = Type::jgz;
            
            i.type = t;

            auto arg = line.substr(6);
            if(arg[0] >= 'a' && arg[0] <= 'z') {
                i.arg = arg[0]-'a';
                i.arg_constant = false;
            } else {
                i.arg = to_int(arg);
                i.arg_constant = true;
            }
        }
        instructions.push_back(i);
    });
    return instructions;
}

struct Prog {
    std::array<int64_t,26> regs = {0};
    int pc = 0;
    std::deque<int64_t> input = {};
    int total_sent = 0;

    int64_t get(Ins s) const {
        if(s.arg_constant) return s.arg;
        return regs[s.arg];
    }
    bool run(const std::vector<Ins>& instructions, Prog& other) {
        while(pc >= 0 && pc < instructions.size()) {
            auto current = instructions[pc];
            switch(current.type) {
                case Type::snd:
                    total_sent++;
                    other.input.push_back(current.reg_constant ? current.reg : regs[current.reg]);
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
                    if(input.empty()) return false;
                    regs[current.reg] = input.front();
                    input.pop_front();
                    break;
                case Type::jgz:
                    if((current.reg_constant ? current.reg : regs[current.reg]) > 0) {
                        pc += get(current) - 1;
                    }
                    break;
            }
            ++pc;
        }
        return true;
    }
};

void solution(std::string_view input) {
    auto instructions = parse(input);
    Prog p0,p1;
    p0.regs['p'-'a'] = 0;
    p1.regs['p'-'a'] = 1;
    bool terminated0 = false;
    bool terminated1 = false;
    p0.run(instructions,p1);
    std::cout << "Part 1: " << p1.input.back() << '\n';
    while(p0.input.size() || p1.input.size()) {
        terminated1 = p1.run(instructions,p0);
        terminated0 = p0.run(instructions,p1);
        if(terminated0 && terminated1) break;
    }
    std::cout << "Part 2: " << p1.total_sent << '\n';
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
