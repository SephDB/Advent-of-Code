#include <iostream>
#include <string_view>
#include <vector>
#include <ranges>
#include <numeric>
#include <algorithm>
#include <ranges>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

struct Num {
    int value;
    int depth;
};

struct SnailFish {
    std::vector<Num> number;
    SnailFish() = default;
    SnailFish(std::string_view s) {
        int current_depth = 0;
        for(auto c : s) {
            switch(c) {
                case '[':
                    current_depth++;
                    break;
                case ']':
                    current_depth--;
                    break;
                case ',':
                    break;
                default:
                    number.emplace_back(c-'0',current_depth);
                    break;
            }
        }
    }

    SnailFish operator+(const SnailFish& other) const {
        if(number.empty()) {
            //Just here to make {} + n work
            return other;
        }

        //Pass 1: Resolve every exploding pair
        SnailFish pass1_result{};
        auto pass1 = [&v=pass1_result.number](const auto& values, int add = 0) {
            auto current = values.begin();
            while(current != values.end()) {
                Num c = *current++;
                c.value += add;
                if(c.depth == 5) {
                    if(!v.empty())
                        v.back().value += c.value;
                    v.push_back({0,4});
                    auto right = *current++;
                    add = right.value;
                } else {
                    v.push_back(c);
                    add = 0;
                }
            }
            return add;
        };

        int add = pass1(number | std::views::transform([](auto n) {return Num{n.value,n.depth+1};}));
        pass1(other.number | std::views::transform([](auto n) {return Num{n.value,n.depth+1};}),add);


        SnailFish result{};
        int extra = 0;
        for(auto n : pass1_result.number) {
            n.value += extra;
            extra = result.pass2_insert(n);
        }

        return result;
    }

    private:
    /*
        Prerequisites:
            - all exploding pairs in the input have already been resolved in pass 1
        Recursion guarantees the following:
            - all(number, n.depth <= 4 && n.value < 10)
        => any split number inserted is the first split from the left
            if it causes an explosion, the number pushed to the left is the leftmost possible split number(with no explosions remaining)
        
        on insertion of new number
            - n < 10 => regular insertion, done
            - n >= 10 => split
                - depth < 4 => two insertions, done
                - depth == 4 =>
                    - pop vector, insert left+value
                    - insert return of previous insert(0+inserted possible explosion)
                    - return right
    */
    int pass2_insert(Num n) {
        if(n.value < 10) {
            number.push_back(n);
            return 0;
        } else {
            int left = n.value/2;
            int right = n.value - left;
            if(n.depth < 4) {
                right += pass2_insert({left,n.depth+1});
                return pass2_insert({right,n.depth+1});
            }
            else {
                Num insert{0,4};
                if(!number.empty()) {
                    Num back = number.back();
                    number.pop_back();
                    back.value += left;
                    insert.value += pass2_insert(back);
                }
                right += pass2_insert(insert);
                return right;
            }
        }
    }
public:
    friend std::ostream& operator<<(std::ostream& o, const SnailFish& s) {
        auto current = s.number.begin();
        auto p = [&](int depth,auto&& rec) -> void {
            auto read_val = [&]() {
                if(current->depth == depth) {
                    o << (current++)->value;
                }
                else {
                    rec(depth+1,rec);
                }
            };
            o << '[';
            read_val();
            o << ',';
            read_val();
            o << ']';
        };
        p(1,p);
        return o;
    }

    int magnitude() const {
        auto current = number.begin();
        auto p = [&](int depth,auto&& rec) -> int {
            int total = 0;
            auto read_val = [&]() {
                if(current->depth == depth) {
                    return (current++)->value;
                }
                else {
                    return rec(depth+1,rec);
                }
            };
            total += 3*read_val();
            total += 2*read_val();
            
            return total;
        };
        return p(1,p);
    }
};


auto parse(std::string_view input) {
    std::vector<SnailFish> fish;
    split(input,'\n',[&](std::string_view line) {
        fish.emplace_back(line);
    });
    return fish;
}

auto part1(const auto& input) {
    SnailFish total{};
    for(auto& fish : input) {
        total = total + fish;
    }
    return total.magnitude();
}

auto part2(const auto& input) {
    int total = 0;
    for(size_t i = 0; i < input.size(); ++i) {
        for(size_t j = 0; j < input.size(); ++j) {
            if(j != i) {
                total = std::max(total,(input[i]+input[j]).magnitude());
            }
        }
    }
    return total;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"([[[6,6],3],0]
[[[0,[9,9]],[[7,7],[4,6]]],[[5,6],[5,9]]]
[[[1,[6,5]],7],[[6,[3,6]],5]]
[[0,[9,[0,2]]],[[[1,9],3],[[0,1],[5,1]]]]
[[[3,[7,6]],[1,0]],[[4,[7,9]],[5,4]]]
[[0,9],[[[0,3],[2,9]],3]]
[[[[3,2],0],[7,6]],[9,[[4,6],0]]]
[[4,[[2,5],[4,0]]],[2,[8,[3,0]]]]
[[[[3,5],6],[[8,6],2]],[[6,[5,1]],[[1,0],[3,2]]]]
[[1,[[2,8],[2,9]]],[[7,[8,8]],[[2,1],[9,5]]]]
[[[4,[4,6]],[[5,7],4]],[8,0]]
[[[3,[5,9]],[1,[6,5]]],[1,9]]
[[[5,7],[4,1]],4]
[3,[[6,0],[[9,0],[8,2]]]]
[[7,[[5,4],[7,3]]],[6,[[9,0],[1,9]]]]
[[[9,3],[0,[1,4]]],[[8,0],9]]
[[7,3],[[4,2],0]]
[[[[1,6],8],1],[[[3,6],[9,9]],9]]
[[[[2,7],[1,6]],[2,6]],0]
[[[2,[0,8]],8],[[9,7],5]]
[4,[[6,3],[9,5]]]
[[[4,[0,8]],3],[[2,9],[[5,3],[2,5]]]]
[[1,0],[[6,[9,0]],[[6,3],9]]]
[[[[0,6],[1,1]],[[2,9],2]],[[2,0],3]]
[[[6,8],[[1,1],9]],[6,5]]
[1,[5,9]]
[[[[6,8],[4,6]],[[8,2],[9,2]]],[[3,[6,8]],[4,3]]]
[8,[[[2,4],[1,1]],[[7,1],[3,4]]]]
[[[[1,9],5],[[8,6],9]],[[[3,7],[9,6]],[2,0]]]
[[[7,[2,7]],[[2,9],7]],[4,[5,[9,5]]]]
[9,[[[1,9],7],[[8,7],[2,8]]]]
[9,2]
[[7,[1,[2,8]]],[9,5]]
[[[0,[5,4]],6],[8,1]]
[[[0,[9,8]],0],5]
[[9,[2,9]],[1,[8,[3,8]]]]
[3,[[5,5],[2,[2,5]]]]
[[6,7],[[[7,3],3],8]]
[[[[6,7],[2,6]],7],[0,[6,[3,5]]]]
[2,[[9,2],[[8,5],[1,2]]]]
[0,[[[0,8],[9,7]],[[5,1],7]]]
[[[2,1],6],[[9,[0,9]],[2,6]]]
[4,[9,[[5,2],[6,3]]]]
[[[9,1],4],[[6,[5,6]],[5,8]]]
[[4,[[1,5],[5,4]]],[3,[[7,2],7]]]
[[[4,5],6],[9,[9,1]]]
[3,[7,[5,2]]]
[[[0,[6,6]],[[7,8],[0,8]]],[2,[[0,5],8]]]
[[[[2,3],[0,6]],[[6,0],[9,4]]],[[[1,6],1],[[5,6],9]]]
[[[1,[2,2]],[9,[8,2]]],[[[2,9],0],[5,[2,7]]]]
[[[2,[4,9]],[2,[0,0]]],[[2,[9,7]],[[3,4],[0,7]]]]
[[1,[7,[3,5]]],[[7,[5,8]],1]]
[[[[2,3],1],[5,3]],[[0,[1,8]],[1,2]]]
[[6,9],[0,[[8,8],4]]]
[[[8,[9,1]],[[0,1],6]],[[8,8],[2,4]]]
[[0,[[5,1],[5,8]]],[[5,[5,1]],2]]
[[[8,[8,4]],8],[[2,[6,0]],[[5,8],6]]]
[5,[[1,[3,6]],[[5,8],[5,0]]]]
[8,[[5,[7,8]],[[9,9],[8,4]]]]
[2,[[[7,6],7],[[9,4],[8,9]]]]
[[5,9],[[8,[2,9]],[0,[2,8]]]]
[[[2,[7,4]],5],3]
[[[7,[1,1]],5],[[4,[7,2]],[[5,8],[2,8]]]]
[[[1,[2,8]],6],7]
[[[[2,7],[0,0]],[[5,3],5]],[[[1,1],[4,6]],7]]
[[[4,6],[1,[5,0]]],8]
[[[8,[5,3]],2],[[[0,2],[3,6]],[[7,9],[8,0]]]]
[[[2,[2,0]],5],[[[5,8],[0,1]],[8,[8,5]]]]
[[[9,[0,7]],[[5,0],[9,6]]],0]
[[[[1,6],4],[1,4]],[[[5,8],4],[[9,9],[9,7]]]]
[1,1]
[[4,4],[[6,3],9]]
[[[[4,0],[6,8]],[[6,0],0]],[8,[[7,9],7]]]
[[[[1,1],6],[[4,1],[6,4]]],4]
[[[[9,5],[5,8]],2],[[3,[8,8]],[[7,0],8]]]
[[[[9,3],4],[3,[9,2]]],[[2,7],[7,[2,3]]]]
[[[[6,8],[7,4]],[[1,6],1]],[3,4]]
[[1,[2,[8,6]]],5]
[[[[7,5],[2,5]],[[5,3],[0,3]]],[4,9]]
[[8,7],[[2,[1,6]],[[4,8],1]]]
[[9,[[0,7],[7,2]]],4]
[[4,[8,[9,6]]],[[[8,1],[3,5]],8]]
[[[[9,1],[2,2]],[[7,9],0]],[[8,[0,6]],[0,[7,3]]]]
[[[2,[8,7]],[[9,0],1]],[8,[9,0]]]
[1,[3,2]]
[[[[6,3],7],[[5,3],[3,1]]],[4,[[9,3],5]]]
[3,[1,9]]
[3,[[[6,4],[0,2]],[[3,8],[5,3]]]]
[[[[3,4],3],[[4,6],4]],[[[5,7],7],3]]
[[[5,1],[[1,0],4]],[[2,3],[0,2]]]
[[[[2,4],[8,0]],5],[[1,2],[6,[2,3]]]]
[[[2,9],4],4]
[[8,[[6,2],[1,3]]],[8,[[6,3],[4,9]]]]
[[[[2,7],9],[[5,6],[3,4]]],[5,9]]
[[[[5,5],9],1],2]
[[[[9,7],[6,9]],[[6,8],[3,9]]],[6,[3,8]]]
[[9,3],[[6,0],5]]
[[[[5,1],4],[[2,8],7]],[[[9,8],6],[[1,5],[4,0]]]]
[[[4,5],3],[[3,[5,9]],[7,[9,2]]]]
[[[[1,7],5],[0,[2,2]]],[[3,6],[9,6]]])";

int main() {
    solution(input);
}
