#include <iostream>
#include <string>
#include <vector>
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

struct Loc {
    int val;
};

struct Instr {
    enum {SWAP,REV,ROT,MOV} type;
    int a,b;
    bool indirect = false;
};

auto parse(std::string_view input) {
    std::vector<Instr> instructions;
    for(auto l : split(input,'\n')) {
        auto s = split(l,' ');
        if(s[0] == "swap") {
            if(s[1] == "position") {
                instructions.push_back({Instr::SWAP,{s[2][0]-'0'},{s.back()[0]-'0'}});
            } else {
                instructions.push_back({Instr::SWAP,s[2][0],s.back()[0],true});
            }
        } else if(s[0] == "reverse") {
            instructions.push_back({Instr::REV,s[2][0]-'0',s.back()[0]-'0'});
        } else if(s[0] == "rotate") {
            if(s.size() == 4) {
                instructions.push_back({Instr::ROT,s[2][0]-'0',s[1][0] == 'r' ? 1 : -1});
            } else {
                instructions.push_back({Instr::ROT,s.back()[0],1,true});
            }
        } else if(s[0] == "move") {
            instructions.push_back({Instr::MOV,s[2][0]-'0',s.back()[0]-'0'});
        }
    }
    return instructions;
}

auto rot = [](int num, auto begin, auto end) {
    auto rot_point = begin;
    if(num > 0) rot_point = end - num;
    else rot_point = begin - num;
    std::rotate(begin,rot_point,end);
};

auto part1(decltype(parse("")) input, std::string current) {
    auto lookup = [&current](int c, bool indirect) -> int {
        if(indirect) return current.find(c);
        return c;
    };
    for(auto ins : input) {
        switch(ins.type) {
            case Instr::SWAP:
                std::swap(current[lookup(ins.a,ins.indirect)],current[lookup(ins.b,ins.indirect)]);
                break;
            case Instr::REV:
                std::reverse(current.begin()+ins.a,current.begin()+ins.b+1);
                break;
            case Instr::ROT:
                if(ins.indirect) {
                    auto pos = current.find(ins.a);
                    if(pos >= 4) {
                        pos++;
                    }
                    rot((pos+1)%current.size(),current.begin(),current.end());
                } else
                    rot(ins.a*ins.b,current.begin(),current.end());
                break;
            case Instr::MOV:
                rot(ins.a > ins.b ? 1 : -1, current.begin()+std::min(ins.a,ins.b), current.begin()+std::max(ins.a,ins.b)+1);
                break;
        }
    }
    return current;
}

auto part2(decltype(parse("")) input, std::string current) {
    auto lookup = [&current](int c, bool indirect) -> int {
        if(indirect) return current.find(c);
        return c;
    };

    //Lookup table for indirect rotation
    std::vector<int> num_rots(current.size(),0);
    for(int i = 0; i < current.size(); ++i) {
        auto rots = (1+i+(i >= 4)) % current.size();
        auto pos = (i+rots)%current.size();
        num_rots[pos] = rots;
    }

    std::reverse(input.begin(),input.end());
    for(auto ins : input) {
        switch(ins.type) {
            case Instr::SWAP:
                std::swap(current[lookup(ins.a,ins.indirect)],current[lookup(ins.b,ins.indirect)]);
                break;
            case Instr::REV:
                std::reverse(current.begin()+ins.a,current.begin()+ins.b+1);
                break;
            case Instr::ROT:
                if(ins.indirect) {
                    auto pos = current.find(ins.a);
                    rot(-num_rots[pos],current.begin(),current.end());
                } else
                    rot(-ins.a*ins.b,current.begin(),current.end());
                break;
            case Instr::MOV:
                rot(ins.a > ins.b ? -1 : 1, current.begin()+std::min(ins.a,ins.b), current.begin()+std::max(ins.a,ins.b)+1);
                break;
        }
    }
    return current;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in,"abcdefgh") << '\n';
    std::cout << "Part 2: " << part2(in,"fbgdceah") << '\n';
}

std::string_view input = R"(rotate right 4 steps
swap letter b with letter e
swap position 1 with position 3
reverse positions 0 through 4
rotate left 5 steps
swap position 6 with position 5
move position 3 to position 2
move position 6 to position 5
reverse positions 1 through 4
rotate based on position of letter e
reverse positions 3 through 7
reverse positions 4 through 7
rotate left 1 step
reverse positions 2 through 6
swap position 7 with position 5
swap letter e with letter c
swap letter f with letter d
swap letter a with letter e
swap position 2 with position 7
swap position 1 with position 7
swap position 6 with position 3
swap letter g with letter h
reverse positions 2 through 5
rotate based on position of letter f
rotate left 1 step
rotate right 2 steps
reverse positions 2 through 7
reverse positions 5 through 6
rotate left 6 steps
move position 2 to position 6
rotate based on position of letter a
rotate based on position of letter a
swap letter f with letter a
rotate right 5 steps
reverse positions 0 through 4
swap letter d with letter c
swap position 4 with position 7
swap letter f with letter h
swap letter h with letter a
rotate left 0 steps
rotate based on position of letter e
swap position 5 with position 4
swap letter e with letter h
swap letter h with letter d
rotate right 2 steps
rotate right 3 steps
swap position 1 with position 7
swap letter b with letter e
swap letter b with letter e
rotate based on position of letter e
rotate based on position of letter h
swap letter a with letter h
move position 7 to position 2
rotate left 2 steps
move position 3 to position 2
swap position 4 with position 6
rotate right 7 steps
reverse positions 1 through 4
move position 7 to position 0
move position 2 to position 0
reverse positions 4 through 6
rotate left 3 steps
rotate left 7 steps
move position 2 to position 3
rotate left 6 steps
swap letter a with letter h
rotate based on position of letter f
swap letter f with letter c
swap position 3 with position 0
reverse positions 1 through 3
swap letter h with letter a
swap letter b with letter a
reverse positions 2 through 3
rotate left 5 steps
swap position 7 with position 5
rotate based on position of letter g
rotate based on position of letter h
rotate right 6 steps
swap letter a with letter e
swap letter b with letter g
move position 4 to position 6
move position 6 to position 5
rotate based on position of letter e
reverse positions 2 through 6
swap letter c with letter f
swap letter h with letter g
move position 7 to position 2
reverse positions 1 through 7
reverse positions 1 through 2
rotate right 0 steps
move position 5 to position 6
swap letter f with letter a
move position 3 to position 1
move position 2 to position 4
reverse positions 1 through 2
swap letter g with letter c
rotate based on position of letter f
rotate left 7 steps
rotate based on position of letter e
swap position 6 with position 1)";

int main() {
    solution(input);
}
