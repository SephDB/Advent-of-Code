#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

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
auto split_known(std::string_view in, char delim) {
    std::array<std::string_view,N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = in.substr(0,pos);
        in.remove_prefix(pos+1);
    }
    ret[current++] = in;
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct Arg {
    int val;
    bool is_reg;
};

Arg parse_arg(std::string_view s) {
    if(s.size() == 1 and s[0] >= 'a' and s[0] <= 'd') {
        return {s[0]-'a',true};
    } else {
        return {to_int(s),false};
    }
}

struct Ins {
    enum {CPY,INC,DEC,JNZ,OUT} type;
    Arg value;
    Arg target;
};

using Program = std::vector<Ins>;

auto parse(std::string_view input) {
    auto lines = split(input,'\n');
    Program p;
    p.reserve(lines.size());
    for(auto l : lines) {
        //b is possibly uninitialized but we won't read from it if it is
        auto [ins,a,b] = split_known<3>(l,' ');
        switch(ins[0]) {
            case 'c':
            case 'j':
                p.push_back({ins[0] == 'c' ? Ins::CPY : Ins::JNZ, parse_arg(a), parse_arg(b)});
                break;
            case 'i':
            case 'd':
                p.push_back({ins[0] == 'i' ? Ins::INC : Ins::DEC, parse_arg(a), 1});
                break;
            case 'o':
                p.push_back({Ins::OUT,parse_arg(a),1});
                break;
        }
    }
    return p;
}

auto run(const Program& p, std::array<int,4> regs = {0,0,0,0}) {
    int pc = 0;
    auto get = [&](Arg a) {return a.is_reg ? regs[a.val] : a.val;};

    while(pc < p.size()) {
        auto& instr = p[pc];
        switch(instr.type) {
            case Ins::CPY:
                regs[instr.target.val] = get(instr.value);
                break;
            case Ins::INC:
                regs[instr.value.val]++;
                break;
            case Ins::DEC:
                regs[instr.value.val]--;
                break;
            case Ins::JNZ:
                if(get(instr.value) != 0) pc += get(instr.target)-1;
                break;
            case Ins::OUT:
                std::cout << get(instr.value);
        }
        ++pc;
    }
    std::cout << '\n';
    return regs[0];
}

void solution(std::string_view input) {
    auto in = parse(input);
    auto min = in[1].value.val*in[2].value.val;
    std::cout << min << '\n';
    //The output of the main loop is the bit pattern(from least to most significant bit) of min+start input.
    //Solution is to find the first number with the required bit pattern that's larger than min, and subtract min from it
    int i = 2;
    for(; i < min; i = (i << 2)+2);
    run(in,{i-min});
    std::cout << i - min << '\n';
}

//Removed last jump bc it's what makes the pattern infinite, it basically resets the main loop
std::string_view input = R"(cpy a d
cpy 14 c
cpy 182 b
inc d
dec b
jnz b -2
dec c
jnz c -5
cpy d a
jnz 0 0
cpy a b
cpy 0 a
cpy 2 c
jnz b 2
jnz 1 6
dec b
dec c
jnz c -4
inc a
jnz 1 -7
cpy 2 b
jnz c 2
jnz 1 4
dec b
dec c
jnz 1 -4
jnz 0 0
out b
jnz a -19)";

int main() {
    solution(input);
}
