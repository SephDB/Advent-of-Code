#include <iostream>
#include <string_view>
#include <array>
#include <vector>
#include <stack>
#include <charconv>

template<int N, typename F>
auto split_known(std::string_view in, std::string_view delim, F&& apply) {
    std::array<decltype(apply(in)),N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        if(pos != 0) ret.at(current++) = apply(in.substr(0,pos));
        in.remove_prefix(pos+delim.size());
    }
    ret.at(current++) = apply(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

/**
    inp w
    mul x 0
    add x z
    mod x 26
    div z (1|26) => Pop == 26
    add x A
    eql x w
    eql x 0
    mul y 0
    add y 25
    mul y x
    add y 1
    mul z y
    mul y 0
    add y w
    add y B
    mul y x
    add z y

    Algorithm for each block:
    x = z%26
    if(Pop) z /= 26

    x = x+A != Input
    if(x) {
        z *= 26
        z += Input+B
    }

    Assumption 1: if !Pop => A is too big to have z%26+A be equal to Input, thus it will always push a new number
    Assumption 2: B <= 16 (bc otherwise we wouldn't have pop/push mechanics with mod 26)
    in effect:
    if(Pop) {
        current = back();
        pop();
        if(current+A != Input) {
            push(Input+B);
        }
    }
    else {
        push(Input+B);
    }
    Assumption 3: there are 7 push-only and 7 pop blocks => a pop block must never push, and thus has a fixed input number based on the corresponding push's input:
    push_input + push.B + pop.A == pop_input

    Helpful realization: push block is the determining factor of "largest" or "smallest" number since it's at an earlier digit
*/

struct Block {
    bool push;
    int value; //A for pop, B for push
};

auto parse(std::string_view input) {
    auto get_num = [](auto line) {
        auto components = split_known<3>(line," ",[](auto a){return a;});
        return to_int(components[2]);
    };
    int num_push = 0, num_pop = 0;
    auto blocks = split_known<14>(input,"inp w\n",[&](std::string_view blocktext) {
        blocktext.remove_suffix(1);
        auto lines = split_known<17>(blocktext,"\n",[](auto a){return a;});
        bool push = get_num(lines[3]) == 1;
        int A = get_num(lines[4]);
        int B = get_num(lines[14]);
        if(B > 16) std::cout << "Assumption 2 broken\n";
        if(push) {
            if(A < 10) std::cout << "Assumption 1 broken\n";
            num_push++;
            return Block{true,B};
        }
        else {
            num_pop++;
            return Block{false,A};
        }
    });
    if(num_push != 7 || num_pop != 7) {
        std::cout << "Assumption 3 broken!\n";
    }
    return blocks;
}

struct PushInfo {
    int digit_loc;
    int B;
};

void solution(std::string_view input) {
    auto in = parse(input);
    std::string max_number(14,'0');
    std::string min_number(14,'0');
    std::stack<PushInfo> push_blocks;
    for(int i = 0; i < 14; ++i) {
        auto [push,value] = in[i];
        if(push) {
            push_blocks.push({i,value});
        }
        else {
            auto [push_loc,B] = push_blocks.top();
            push_blocks.pop();
            int diff = B+value;

            int max_push = std::min(9,9-diff);
            int max_pop = max_push + diff;
            max_number[push_loc] = max_push+'0';
            max_number[i] = max_pop+'0';
            
            int min_push = std::max(1,1-diff);
            int min_pop = min_push+diff;
            min_number[push_loc] = min_push+'0';
            min_number[i] = min_pop+'0';
        }
    }
    std::cout << "Part 1: " << max_number << '\n';
    std::cout << "Part 2: " << min_number << '\n';
}

std::string_view input = R"(inp w
mul x 0
add x z
mod x 26
div z 1
add x 13
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 0
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 1
add x 11
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 3
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 1
add x 14
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 8
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x -5
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 5
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 1
add x 14
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 13
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 1
add x 10
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 9
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 1
add x 12
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 6
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x -14
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 1
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x -8
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 1
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 1
add x 13
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 2
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x 0
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 7
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x -5
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 5
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x -9
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 8
mul y x
add z y
inp w
mul x 0
add x z
mod x 26
div z 26
add x -1
eql x w
eql x 0
mul y 0
add y 25
mul y x
add y 1
mul z y
mul y 0
add y w
add y 15
mul y x
add z y)";

int main() {
    solution(input);
}
