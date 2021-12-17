#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

struct Area {
    int xmin,xmax,ymin,ymax;
};

auto part1(Area input) {
    return input.ymin * (input.ymin+1) / 2;
}

struct DirInfo {
    int start_steps; //minimum steps to hit target
    int max_steps; //max steps to exit target
};

std::vector<DirInfo> GetX(Area a) {
    if(a.xmin * 2 < a.xmax) {
        std::cout << "Can't work with this\n";
        return {};
    }
    int steps = 0;
    int current_total = 0;
    while(current_total < a.xmin) {
        current_total += ++steps;
    }
    std::vector<DirInfo> res;
    
    int removable_steps = 0;
    int remove_total = 0;
    //Find the ones that stop
    while(current_total <= a.xmax) {
        while(current_total - remove_total - removable_steps - 1 >= a.xmin) {
            remove_total += ++removable_steps;
        }
        res.push_back({steps-removable_steps,std::numeric_limits<int>::max()});
        current_total += ++steps;
    }
    int min_remove = 0;
    int min_remove_total = 0;
    while(steps + steps-1 <= a.xmax) {
        while(current_total - min_remove_total > a.xmax) {
            min_remove_total += ++min_remove;
        }
        while(current_total - remove_total - removable_steps - 1 >= a.xmin) {
            remove_total += ++removable_steps;
        }
        res.push_back({steps-removable_steps,steps-min_remove});
        current_total += ++steps;
    }

    return res;
};

auto part2(Area input) {
    auto xdirs = GetX(input);
    auto ydircount = [&](int y) -> int {
        int current = 0;
        int steps = 0;
        while(current > input.ymax) {
            steps++;
            current += y--;
        }
        int min_steps = steps;
        while(current >= input.ymin) {
            steps++;
            current += y--;
        }
        int max_steps = steps-1;
        if(min_steps > max_steps) {
            //Overshoot
            return 0;
        }
        return std::ranges::count_if(xdirs,[&](auto d) {return min_steps <= d.max_steps && max_steps >= d.start_steps;});
    };
    int total = 0;
    for(int y = (input.ymin-1)/2; y < -input.ymin; ++y) {
        total += ydircount(y);
    }
    return total + (input.xmax - input.xmin + 1) *(input.ymax - input.ymin +1);
}

void solution(Area input) {
    std::cout << "Part 1: " << part1(input) << '\n';
    std::cout << "Part 2: " << part2(input) << '\n';
}

int main() {
    solution(Area{20,30,-10,-5});
    solution(Area{85,145,-163,-108});
}
