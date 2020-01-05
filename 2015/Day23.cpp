#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <span>

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

template<int N>
auto split_known_max(std::string_view in, char delim) {
    std::array<std::string_view,N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = in.substr(0,pos);
        in.remove_prefix(pos+1);
    }
    ret[current++] = in;
    return std::pair(ret,current);
}

int to_int(std::string_view s) {
    if(s[0] == '+') s.remove_prefix(1);
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct Instruction {
    enum {
        INC, TPL, HLF, JMP, JIO, JIE
    } type;
    int arg;
    bool which_reg;
};

auto parse(std::string_view input) {
    std::vector<Instruction> ret;
    for(auto l : split(input,'\n')) {
        Instruction next;
        auto [comps,total] = split_known_max<3>(l,' ');
        auto ins = comps[0];
        next.which_reg = comps[1][0] == 'b';
        if(ins[0] == 'j') {
            next.arg = to_int(comps[total-1]);
            if(ins == "jmp") next.type = Instruction::JMP;
            else if(ins == "jio") next.type = Instruction::JIO;
            else next.type = Instruction::JIE;
        } 
        else if(ins == "inc") next.type = Instruction::INC;
        else if(ins == "tpl") next.type = Instruction::TPL;
        else if(ins == "hlf") next.type = Instruction::HLF;
        else next.type = Instruction::TPL;
        ret.push_back(next);
    }
    return ret;
}

int run(const std::vector<Instruction> instructions, int a = 0) {
    int pc = 0;
    std::array<std::uint64_t,2> registers = {a,0};
    while(pc < instructions.size()) {
        auto& current = instructions[pc];
        auto& reg = registers[current.which_reg];
        switch(current.type) {
            case Instruction::INC:
                reg++;
            break;
            case Instruction::TPL:
                reg *= 3;
            break;
            case Instruction::HLF:
                reg /= 2;
            break;
            case Instruction::JMP:
                pc += (current.arg-1);
            break;
            case Instruction::JIO:
                if(reg == 1) pc += (current.arg-1);
            break;
            case Instruction::JIE:
                if(reg % 2 == 0) pc += (current.arg-1);
            break;
        }
        ++pc;
    }
    return registers[1];
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << run(in) << '\n';
    std::cout << "Part 2: " << run(in,1) << '\n';
}

std::string_view input = R"(jio a, +18
inc a
tpl a
inc a
tpl a
tpl a
tpl a
inc a
tpl a
inc a
tpl a
inc a
inc a
tpl a
tpl a
tpl a
inc a
jmp +22
tpl a
inc a
tpl a
inc a
inc a
tpl a
inc a
tpl a
inc a
inc a
tpl a
tpl a
inc a
inc a
tpl a
inc a
inc a
tpl a
inc a
inc a
tpl a
jio a, +8
inc b
jie a, +4
tpl a
inc a
jmp +2
hlf a
jmp -7)";

int main() {
    solution(input);
}
