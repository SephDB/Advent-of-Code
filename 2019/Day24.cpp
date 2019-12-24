#include <iostream>
#include <string_view>
#include <bitset>
#include <deque>
#include <unordered_set>
#include <algorithm>
#include <numeric>

auto next(std::bitset<25> field) {
    auto next = field;
    int current = 0;
    for(int y = 0; y < 5; ++y) {
        for(int x = 0; x < 5; ++x) {
            int num = (x > 0 and field.test(current-1)) + (x < 4 and field.test(current+1))
                    + (y > 0 and field.test(current-5)) + (y < 4 and field.test(current+5));
            if(field.test(current) and num != 1) next.reset(current);
            else if(not field.test(current) and (num == 1 or num == 2)) next.set(current);
            ++current;
        }
    }
    return next;
}

auto next(std::deque<std::bitset<25>> field) {
    field.push_front({});
    field.push_back({});
    auto next = field;
    auto neighbors = [&](int level, int x, int y) -> int {
        int num = 0;
        if(level < field.size() - 1) {
            if(x == 0) {
                num += field[level+1].test(2*5+1);
            } else if(x == 4) {
                num += field[level+1].test(2*5+3);
            }
            if(y == 0) {
                num += field[level+1].test(1*5+2);
            } else if(y == 4) {
                num += field[level+1].test(3*5+2);
            }
        }
        if(level > 0) {
            if(x == 2 and y == 1) {
                for(int x1 = 0; x1 < 5; ++x1) {
                    num += field[level-1].test(x1);
                }
            } else if(x == 2 and y == 3) {
                for(int x1 = 0; x1 < 5; ++x1) {
                    num += field[level-1].test(x1+4*5);
                }
            } else if(x == 1 and y == 2) {
                for(int y1 = 0; y1 < 5; ++y1) {
                    num += field[level-1].test(y1*5+0);
                }
            } else if(x == 3 and y == 2) {
                for(int y1 = 0; y1 < 5; ++y1) {
                    num += field[level-1].test(y1*5+4);
                }
            }
        }
        auto& f = field[level];
        auto current = x+y*5;
        num += (x > 0 and f.test(current-1)) + (x < 4 and f.test(current+1))
             + (y > 0 and f.test(current-5)) + (y < 4 and f.test(current+5));
        return num;
    };
    for(int level = 0; level < field.size(); ++level) {
        for(int current = 0; current < 25; ++current) {
            if(current == 2*5+2) continue;
            auto num = neighbors(level,current%5,current/5);
            if(field[level].test(current) and num != 1) next[level].reset(current);
            else if(not field[level].test(current) and (num == 1 or num == 2)) next[level].set(current);
        }
    }
    while(next.front().count() == 0) next.pop_front();
    while(next.back().count() == 0) next.pop_back();
    return next;
}

void print(std::deque<std::bitset<25>> in) {
    for(int y = 0; y < 5; y++) {
        for(auto& level : in) {
            for(int x = 0; x < 5; ++x) {
                if(x == 2 and y == 2) std::cout << '?';
                else std::cout << (level.test(y*5+x) ? '#' : '.');
            }
            std::cout << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

std::string input = 
R"(
###..
.##..
#....
##..#
.###.)";

int main() {
    std::erase(input,'\n');
    std::reverse(input.begin(),input.end());
    std::bitset<25> field(input,0,25,'.','#');
    auto field_copy = field;
    std::unordered_set<decltype(field.to_ulong())> seen;
    while(seen.insert(field.to_ulong()).second) {
        field = next(field);
    }
    std::cout << "Part 1: " << field.to_ulong() << '\n';
    std::deque fields{field_copy};
    for(int i = 0; i < 200; ++i) {
        fields = next(fields);
    }
    std::cout << "Part 2: " << std::accumulate(fields.begin(),fields.end(),0,[](int a, auto&& b) {return a+b.count();}) << '\n';
}
