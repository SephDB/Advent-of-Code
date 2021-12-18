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
        SnailFish next{};
        auto add_to_next = [&,&v=next.number](const auto& values, int add = 0) {
            auto current = values.begin();
            while(current != values.end()) {
                Num c = *current++;
                c.value += add;
                add = 0;
                if(c.depth == 5) {
                    if(!v.empty())
                        v.back().value += c.value;
                    v.push_back({0,4});
                    auto right = *current++;
                    add += right.value;
                } else {
                    next.number.push_back(c);
                }
            }
            return add;
        };


        if(number.empty()) {
            return other;
        }
        else {
            int add = add_to_next(number | std::views::transform([](auto n) {return Num{n.value,n.depth+1};}));
            add_to_next(other.number | std::views::transform([](auto n) {return Num{n.value,n.depth+1};}),add);
            bool need_extra_round = false;
            SnailFish current{};
            std::swap(current,next);
            do {
                add = 0;
                next.number.clear();
                need_extra_round = false;
                auto c = current.number.begin();
                while(c != current.number.end()) {
                    int val = c->value + add;
                    add = 0;
                    if(val < 10) {
                        next.number.push_back({val,c->depth});
                    }
                    else {
                        need_extra_round = true;
                        int left = val/2;
                        int right = val - left;
                        if(c->depth < 4) {
                            next.number.push_back({left,c->depth+1});
                            next.number.push_back({right,c->depth+1});
                        }
                        else {
                            next.add_explode(left);
                            add = right;
                        }
                        ++c;
                        break;
                    }
                    ++c;
                }
                while(c != current.number.end()) {
                    int val = c->value+add;
                    add = 0;
                    next.number.push_back({val,c->depth});
                    ++c;
                }
                std::swap(current,next);
            } while(need_extra_round);
            return current;
        }
    }

    private:
    int add_explode(int left) {
        if(!number.empty()) {
            number.back().value += left;
        }
        number.push_back({0,4});
        return 0;
    }
public:
    friend std::ostream& operator<<(std::ostream& o, const SnailFish& s) {
        auto current = s.number.begin();
        auto p = [&](int depth,auto&& rec) -> void {
            o << '[';
            if(current->depth == depth) {
                o << current->value;
                current++;
            }
            else {
                rec(depth+1,rec);
            }
            o << ',';
            if(current->depth == depth) {
                o << current->value;
                current++;
            }
            else {
                rec(depth+1,rec);
            }
            o << ']';
        };
        p(1,p);
        return o;
    }

    int64_t magnitude() const {
        auto current = number.begin();
        auto p = [&](int depth,auto&& rec) -> int64_t {
            int64_t total = 0;
            if(current->depth == depth) {
                total += 3*current->value;
                current++;
            }
            else {
                total = 3*rec(depth+1,rec);
            }
            
            if(current->depth == depth) {
                total += 2*current->value;
                current++;
            }
            else {
                total += 2*rec(depth+1,rec);
            }
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
        if(total.number.empty()) {
            total = fish;
        }
        else {
            total = total + fish;
        }
    }
    return total.magnitude();
}

auto part2(const auto& input) {
    int64_t total = 0;
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
