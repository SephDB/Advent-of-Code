#include <vector>
#include <iostream>
#include <deque>
#include <charconv>
#include <map>
#include <string_view>
int64_t moduloMultiplication(int64_t a,
    int64_t b,
    int64_t mod)
{
    int64_t res = 0; // Initialize result 

    // Update a if it is more than 
    // or equal to mod 
    a %= mod;

    while (b)
    {
        // If b is odd, add a with result 
        if (b & 1)
            res = (res + a) % mod;

        // Here we assume that doing 2*a 
        // doesn't cause overflow 
        a = (2 * a) % mod;

        b >>= 1; // b = b / 2 
    }

    return res;
}

int64_t modular(int64_t base, int64_t exp, int64_t mod)
{
    int64_t x = 1;
    int64_t i;
    int64_t power = base % mod;

    for (i = 0; i < sizeof(int64_t) * 8; i++) {
        int64_t least_sig_bit = 0x00000001 & (exp >> i);
        if (least_sig_bit)
            x = moduloMultiplication(x, power, mod);
        power = moduloMultiplication(power, power, mod);
    }

    return x;
}

int64_t modInverse(int64_t input, int64_t power, int64_t modulo)
{
    int64_t original = modulo;

    int64_t y = 0;
    int64_t x = 1;

    if (original == 1)
        return 0;

    while (input > 1)
    {
        int64_t q = input / modulo;
        int64_t t = modulo;
        modulo = input % modulo;
        input = t;
        t = y;
        y = x - q * y;
        x = t;
    }

    if (x < 0)
        x += original;

    return x;
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

enum class Deal {
    Reverse,
    Cut,
    Every
};

struct Ins {
    Deal type;
    std::int64_t arg;
};

//blatantly stolen from reddit bc I can't figure out the math beyond the basic idea.
//compress the inverse sequence to a single transformation, exponentiate the resulting equation/matrix(?) to the iterations' power
int64_t part2(int64_t count, int64_t iterations, std::vector<Ins> manipulations, int64_t targetSlot)
{
    std::reverse(manipulations.begin(),manipulations.end());
    int64_t a = 1;
    int64_t b = 0;
    
    for (auto [ins,arg] : manipulations)
    {
        switch(ins) {
            case Deal::Reverse:
                b++;
                b *= -1;
                a *= -1;
                break;
            case Deal::Cut:
                arg += (arg<0)*count;
                b += arg;
                break;
            case Deal::Every:
                auto inv = modInverse(arg,iterations,count);
                a = moduloMultiplication(a,inv,count);
                b = moduloMultiplication(b,inv,count);
                break;
        }
        a %= count;
        b %= count;
        b += (b < 0)*count;
        a += (a < 0)*count;
    }

    int64_t first = moduloMultiplication(modular(a, iterations, count), targetSlot, count);
    int64_t second = (modular(a, iterations, count) + count - 1) % count;
    int64_t third = moduloMultiplication(b, second, count);
    int64_t fourth = modular(a - 1, count - 2, count);
    
    int64_t result = (first + moduloMultiplication(third, fourth, count)) % count;

    return result;
}

std::int64_t to_int(std::string_view s) {
    std::int64_t res;
    std::from_chars(s.begin(),s.end(),res);
    return res;
}

Ins parse_ins(std::string_view s) {
    if(s.substr(0,3) == "cut") {
        return {Deal::Cut,to_int(s.substr(sizeof("cut")))};
    } else if(s.substr(5,4) == "with") {
        return {Deal::Every,to_int(s.substr(sizeof("deal with increment")))};
    } else {
        return {Deal::Reverse,0};
    }
}

std::vector<Ins> parse(std::string_view input) {
    auto lines = split(input,'\n');
    std::vector<Ins> instructions(lines.size());
    std::transform(lines.begin(),lines.end(),instructions.begin(),parse_ins);
    return instructions;
}

template<std::int64_t DeckSize>
std::int64_t part1(const std::vector<Ins>& ins, std::int64_t pos) {
    for(auto [type, arg] : ins) {
        switch(type) {
            case Deal::Reverse:
                pos = DeckSize - pos - 1;
                break;
            case Deal::Cut:
                pos = (pos - arg + DeckSize) % DeckSize;
                break;
            case Deal::Every:
                pos = (pos * arg) % DeckSize;
                break;
        }
    }
    return pos;
}

std::string_view input = R"(deal with increment 33
cut 3627
deal with increment 29
cut 1908
deal with increment 32
deal into new stack
cut 8923
deal with increment 19
cut 8560
deal with increment 73
deal into new stack
deal with increment 30
cut 8832
deal with increment 70
deal into new stack
deal with increment 11
cut -4208
deal with increment 47
deal into new stack
deal with increment 65
cut -5055
deal with increment 66
cut 12
deal with increment 24
cut 3069
deal into new stack
cut -1271
deal with increment 50
cut -7214
deal with increment 72
deal into new stack
cut 67
deal with increment 60
cut -7515
deal with increment 68
deal into new stack
cut -4640
deal with increment 68
cut -9047
deal with increment 53
cut 3616
deal with increment 39
deal into new stack
deal with increment 54
cut -6224
deal with increment 42
deal into new stack
deal with increment 35
deal into new stack
cut -4189
deal with increment 68
deal into new stack
cut 425
deal with increment 28
cut -9932
deal with increment 18
deal into new stack
cut 6404
deal with increment 64
cut -724
deal with increment 33
deal into new stack
cut -8328
deal into new stack
cut 4667
deal with increment 37
cut 3303
deal with increment 13
deal into new stack
deal with increment 56
cut 2288
deal with increment 13
cut -266
deal with increment 65
cut 445
deal with increment 33
cut 2652
deal with increment 57
cut -9924
deal with increment 56
cut 9807
deal into new stack
cut -1485
deal with increment 35
cut -4846
deal with increment 5
cut 7747
deal with increment 44
cut -7428
deal with increment 71
deal into new stack
cut -7677
deal with increment 3
cut -5335
deal with increment 31
cut 7778
deal with increment 5
cut 11
deal into new stack
deal with increment 32)";

int main() {
    auto instructions = parse(input);
    constexpr std::int64_t decksize = 10007;
    std::cout << "Part 1: " << part1<decksize>(instructions,2019) << '\n';
    constexpr std::int64_t part2_deck = 119315717514047;
    std::int64_t current = 2020;
    std::cout << "Part 2: " << part2(part2_deck,101741582076661,instructions,2020);
}

