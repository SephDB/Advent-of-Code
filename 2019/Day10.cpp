#include <algorithm>
#include <numeric>
#include <cmath>
#include <vector>
#include <deque>
#include <iostream>
#include <string_view>

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

struct Direction {
    int xdiff,ydiff;
};

struct Point {
    int x,y;
    Direction to(Point b) {
        auto [x2,y2] = b;
        auto div = std::gcd(x2-x,y2-y);
        return {(x2-x)/div,(y2-y)/div};
    }
    Point operator+(Direction d) {
        return {x+d.xdiff,y+d.ydiff};
    }
    Point& operator+=(Direction d) {
        *this = *this+d;
        return *this;
    }
    bool operator==(Point b) {
        return x == b.x and y == b.y;
    }
    bool operator!=(Point b) {
        return not (*this==b);
    }
};

void solution(std::string_view in) {
    auto field = split(in,'\n');
    auto width = field[0].size();
    std::vector<std::vector<Direction>> totals(field.size()*width);
    auto toPoint = [&](int l) {return Point{l%width,l/width};};
    auto isAsteroid = [&](Point p) {return field[p.y][p.x] == '#';};
    auto openPath = [&](Point a, Point b) {
        auto dir = a.to(b);
        for(auto current = a+dir; current != b; current += dir) {
            if(isAsteroid(current)) return false;
        }
        return true;
    };

    for(int loc = 0; loc < totals.size(); ++loc) {
        auto start = toPoint(loc);
        if(isAsteroid(start)) {
            for(int loc2 = loc+1; loc2 < totals.size(); ++loc2) {
                auto p2 = toPoint(loc2);
                if(isAsteroid(p2) and openPath(start,p2)) {
                    totals[loc].push_back(start.to(p2));
                    totals[loc2].push_back(p2.to(start));
                }
            }
        }
    }

    auto max = std::max_element(totals.begin(),totals.end(),
                    [](auto&& a, auto&& b){return a.size() < b.size();}
                );
    std::cout << "Part 1: " << max->size() << '\n';

    Point observatory = toPoint(max-totals.begin());
    std::cout << "Observatory location: " << observatory.x << ' ' << observatory.y << '\n';

    auto& dirs = *max;
    auto zto2pi_atan2 = [](Direction d) {
        //up(0,-1) == -M_PI/2.0, rotate so it is 0 and add M_PI*2 to the quadrant that's left < 0
        auto angle = std::atan2(d.ydiff,d.xdiff) + M_PI/2.0;
        if(angle < 0.0) {
            angle += M_PI*2;
        }
        return angle;
    };
    std::sort(dirs.begin(),dirs.end(),[&](auto a, auto b){return zto2pi_atan2(a) < zto2pi_atan2(b);});

    auto asteroidsOnPath = [&](Direction dir) {
        auto in_bounds = [&](Point a) {
            return a.x >= 0 and a.x < width and a.y >= 0 and a.y < field.size();
        };
        std::deque<Point> ret;
        for(auto current = observatory + dir; in_bounds(current); current += dir) {
            if(isAsteroid(current)) ret.push_back(current);
        }
        return ret;
    };
    std::vector<std::deque<Point>> asteroids;
    std::transform(dirs.begin(),dirs.end(),std::back_inserter(asteroids),asteroidsOnPath);
    int current = 0;
    while(not asteroids.empty() and current < 200) {
        for(auto& line : asteroids) {
            if(++current == 200) {
                std::cout << "Part 2: " << line.front().x*100+line.front().y << '\n';
                break;
            }
            line.pop_front();
        }
        asteroids.erase(std::remove_if(asteroids.begin(),asteroids.end(),[](auto&& d){return d.empty();}),asteroids.end());
    }
}

std::string_view input = R"(#..#....#...#.#..#.......##.#.####
#......#..#.#..####.....#..#...##.
.##.......#..#.#....#.#..#.#....#.
###..#.....###.#....##.....#...#..
...#.##..#.###.......#....#....###
.####...##...........##..#..#.##..
..#...#.#.#.###....#.#...##.....#.
......#.....#..#...##.#..##.#..###
...###.#....#..##.#.#.#....#...###
..#.###.####..###.#.##..#.##.###..
...##...#.#..##.#............##.##
....#.##.##.##..#......##.........
.#..#.#..#.##......##...#.#.#...##
.##.....#.#.##...#.#.#...#..###...
#.#.#..##......#...#...#.......#..
#.......#..#####.###.#..#..#.#.#..
.#......##......##...#..#..#..###.
#.#...#..#....##.#....#.##.#....#.
....#..#....##..#...##..#..#.#.##.
#.#.#.#.##.#.#..###.......#....###
...#.#..##....###.####.#..#.#..#..
#....##..#...##.#.#.........##.#..
.#....#.#...#.#.........#..#......
...#..###...#...#.#.#...#.#..##.##
.####.##.#..#.#.#.#...#.##......#.
.##....##..#.#.#.......#.....####.
#.##.##....#...#..#.#..###..#.###.
...###.#..#.....#.#.#.#....#....#.
......#...#.........##....#....##.
.....#.....#..#.##.#.###.#..##....
.#.....#.#.....#####.....##..#....
.####.##...#.......####..#....##..
.#.#.......#......#.##..##.#.#..##
......##.....##...##.##...##......)";

int main() {
    solution(input);
}
