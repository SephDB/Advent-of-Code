#include <iostream>
#include <string_view>
#include <vector>
#include <set>
#include <map>
#include <numeric>
#include <algorithm>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

struct Coord {
    int x, y;
    Coord operator+(Coord other) const {return {x+other.x,y+other.y};}
    Coord& operator+=(Coord other) {*this = *this+other; return *this;}
    bool operator<(Coord other) const {
        return std::tie(x,y) < std::tie(other.x,other.y);
    }
};

auto parse(std::string_view input) {
    std::set<Coord> set;
    split(input,'\n',[&set](std::string_view line) {
        Coord current{0,0};
        while(not line.empty()) {
            int size = 1;
            Coord dir{0,0};
            switch(line[0]) {
                case 'w':
                    dir = {0,-1};
                    break;
                case 'e':
                    dir = {0,1};
                    break;
                case 'n':
                    size = 2;
                    dir = {1,-(line[1]=='w')};
                    break;
                case 's':
                    size = 2;
                    dir = {-1,(line[1]=='e')};
                    break;
            }
            line.remove_prefix(size);
            current += dir;
        }
        if(auto k = set.find(current); k != set.end()) {
            set.erase(k);
        } else {
            set.insert(current);
        }
    });
    return set;
}

auto part1(const decltype(parse(""))& input) {
    return input.size();
}

struct neighbor_state {
    int active_neighbors;
    bool active;
};

auto solve(std::vector<Coord> input, int iterations) {
    constexpr std::array<Coord,6> neighbor_d = {{{1,0},{1,-1},{0,-1},{-1,0},{-1,1},{0,1}}};
    for(int i = 0; i < iterations; ++i) {
        std::map<Coord,neighbor_state> neighbors;
        for(auto c : input) {
            neighbors[c].active = true;
            for(auto n : neighbor_d) {
                neighbors[c+n].active_neighbors++;
            }
        }
        input.clear();
        for(auto [c,s] : neighbors) {
            if(s.active_neighbors == 2 || (s.active && s.active_neighbors == 1)) {
                input.push_back(c);
            }
        }
    }
    return input.size();
}

auto part2(const decltype(parse(""))& input) {
    std::array<Coord,6> neighbors = {{{1,0},{1,-1},{0,-1},{-1,0},{-1,1},{0,1}}};
    std::vector<Coord> active(input.size());
    std::ranges::copy(input,active.begin());
    return solve(active,100);
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(wwwnwsenwnwnwnwnwnewnewsewnwnwnww
nenwnwnewnweewsenwenwnwwswwnwswsw
seswswseneeeswswnwnwswseswnwswswswswsw
swenenwneswswswswsenwswswnwenenwseswsw
wneneenwseeeewneeseneeeneeenenene
wwwwenwwwwwnwnewwnwswwwwwse
neneneneneneeneswneeneenwneeneneenesw
seneseswseseswswseseswswseswsewsesesesw
enweneeeeeseeeesw
wwwsewswwwwswwenwneew
sesweenwseeneeeswneeseeeseswenwsese
seswswwswswswswseswswnwswswene
swwnwwwwwwswnwwwseswwwwwseww
wwnewwsewwwwswwneseweewwnw
wswswseneswswseswseneswseswseswswseswnesw
nenwnwneneseneeneswnewenweeswneeese
wwenwwwwnwwwnewsewwnwwwswwnw
newnwswneswswswneeeneenenee
swwsewswswswnwswswswnewewseswswswsw
eseneeenwseesweesw
wnenenenwnenenwnenwnwnenwnwsesenwnesene
eswneeeeeeeeneeeneenenewnee
enwenweeweewesesesweswswnweew
eseeseseseeswsewswww
wseeneewnwswswenweee
swwneswswwwwwswwswwwswwseswww
wesesesewsweseenwnesewneneswseswsw
nwswsesenwswnenenwenenwne
neeneseenenweneneneeeweeneeeswe
wwnwnwwwwsenwnwwnwwwnwwnwnwnw
eswnewswwnwnwwnwnwnwenwwsenewenww
wswsenwsewswseeneseseswswswneseswsw
sewswewwwnwwwnwwneewnwnwnwswnw
wsweswswwnwwsweswswswswnwswswswswswswe
swneseswswswswswsesenwsesw
neeeesewseenweeesweseeeeeee
nenenenenwnenenwneneneneeswswnwnenenwnwnw
nwwwewnwwnwwwwnwwnewnwwwwsenw
senwseeeesenwseeeseenwsweseseswnee
seseseeesenwseseeseeseseesesese
nwsesesesenwseseeseseseswewseswswswsw
wwwwnwwwwwnwnenwnwnwewnwwswww
neneneneswneswneneeeenwneneneeenene
seswnwseswsenenwnewseneswsenwsenwseenwswse
neeswwewwwnwneswnwwswswnw
eswwwswwnwwswneswwwsweswwweww
nwswwsewneeeneswneeeeeneeneneesw
eeswewwneseneneeneeeneseeeneneee
seseewseeseneweeeweesewsenewe
swseswwswneswswswsewsweswnwswseswwe
nwnwenwnewnwsenwsewwnwwnwnwwwnewnw
nenwsesesenwwnwsewenwwwwweswwwwe
nesenweseeeseeswsewsewswwseeeese
sewwswseseseneseswnwesenwseesenwenw
weseseseesesesesesenwseseseseesesese
swwnwnwnewnwnwwnwwwew
nwnwnwnwnenwswnwnwnwnwnwwnwnwenwsesesew
swsewwneswwnwswseswwneswswsewnewnew
wwwwnwwswwnwnwewwwwwwnwwnw
seseseseseeseseeseewnweseswnwesesese
nwwnenwnwwnenenwnwnwnwneneenenenwnenwse
nwwsenwnwnewnwwwnwwnwnwnwwnwnwwsenww
sesewsesenweseseseseseseswsesese
seewneswwwwnwwwseswwnwewnwnwnw
neneneneneneswneneneswswe
wwwwwwwwsenwwwwwwwwww
wnwwwnwwnwsewnwwwwwwswe
nwswnwnwewnwsenwneneneneswnwenene
senweweneseeweeeseneeeweeenw
wnwnwwwwenwwwswnwnwewswnwnewwne
eewnewswnenwswwswwsweeseswnwwnw
wsenenenwnenenwnwnenenenwnenenenenenenw
swseswwswwswnwswswseneseeesw
eeenwnweeeseeeeeeeeeswee
seseneewsesesenwseseeseswsesesesese
swswwswsweswswwswswsw
seseseseseweeeeeeseeseswneeenw
nenwnenwneseneswnenwnenwsene
neneneneneneneswenenenenene
nwnwnwswnwnwenwneneneneeswnwnwnwwnwne
wswnwneneenenwneswnwneswnenenwswsenwnwe
nwnwnwswnwnwnwsenwnwnewnwsenwnwnwenwswnw
seneneneneenenweenenenenenenenenenew
nwwswnwnwnwwnwnwnwnwnwwsenwnwsenwenw
seseseseswsenesesesesesesesesese
enwwwwenwswwswnewwswwsesww
wnwsenwnewenewswswswseewwwwswswswe
seswswnwswwwswswenwneesenwswswswnwesw
sesenwnwswsesesesesenewsesesenwsesesee
wseswnwswseseseeneeswsesesesesewsesene
swwseneswnwwwswnwwewwnewweewsww
neswseenwseneneenesenenenewnenewswnee
eeseeseweeeseeeeseesesesee
esesewseseseseneseswsenweseeeesese
wwnwnwwwwwwwnwnwnwsenewnwse
seenweseseseseeeseswnweeseseeenee
swswwswwwwwwwwwwnewwwwnenew
nenenenenenenwnesenenenewnenenenenenwne
enesweeeeenweeneeenwsww
eeneeeeeeneeeseeeeeeewswe
sweeeeeeeeeeeneeneee
wnwnwwwsenenwwwnwnwwnwnesesenwnewsw
neneeenweeseneeswneeweesenwneee
seneeeseseswnesewswnwneswswnwsewwnwnwsw
nwneeenwnenenenwnwnenenenwnwnewnewne
neswnwsweesewswswnwwneswswwswneswwsene
seeseseeeenwswseeenesesesenwsesewse
nwnwnwnwnenwnwnenwnwnwenwnenwenewwse
enwnwnwnwnwnwnwnwnwswnwnwswnwswnwenwnwnwne
nwwwneneswswneeewnwwnwswswseeseseee
nenwnwswnwnwsewneneenwnwwnw
senwweweswnwneeneseeeswnweseswse
wnwwnwnenwnenwsesenwnwwnwnwswnwswnwnwnenw
wwnwnwsewnwnwwnwesweswnewwwswne
nenenewneswneneswneeweswneenwnesenw
eswnwwwsweseswwswswwwnwswswnwswsenw
eeeswsenwseeweeeeeeseeeeee
nwwwnwsewwnwwnwwwwnewwsewwnwwne
seseseesesesesesesenwseseseseseseseswsw
nwnwnwswsenwnenenenenwnwnwnwnwnwwnwne
nwsesewesenwnwnwnwswsewenwswnwnenwwe
newneseneneseswnwnwswnwewswnwnwseesw
eeeeeeneweeeneeneswneeseene
nwnwwnwnwnwnwwwswnwnwnwnwwewnwwswe
swswswseswneseseseswswseseswswswnwsesesesw
eeseneneseweneneeeneeneenwneewne
eseeswseeeeseeneseeeenwseeeese
sesesenwsesesesewsesenesesesesesesesesesese
senwseseseseseseseseseseseseseseswsesenwsw
neeneneswnenenenenenenenweneneneneswne
swswswseswwswswswswnwswswswswwseswswnwsw
neeeeeeseneeneswwneeeeeneeee
enweesweneneswweneeewenewenee
sewwswswwseneswwwsenwwswwwnewwww
wewwwenwseswnwwwwwwewnwnwnw
nwwenwsewswwwsw
neneneeneneneseneneweseenenweenenene
nwswswneswswwnwesenwsweswswswsw
swenwnwnwnwnwnwnenwsenwnwwnenwnenw
swseseneseseseseneswwsewneseewswnwsene
wneeneneswnwneneewneneneeneenenee
eenwneeweeeeeeesweeeee
seneeswnenweweweeweneeswseee
swseseesewswswsenwswswswswswswsw
wswwswwwswwswwwwnewwwwsewnesw
nwnenwsewnenenenwnenwnwnwne
neenwneneneneneseneneneenewnenenwnenesw
enwnwwnwnwesenwswnenwnwnwenwnwnwswnw
swnenwswsweseeswswswseseswwnwswswseseswsw
nwswnesweswswsewswseswnwswswswswneseswesw
swneseneswswwsenwswswswsw
eeeeseeeeeeeweeeeeneee
wnwwnwswewwnwwwwwwwseww
neenenwswnenweneeeeseneseee
neneweneeeneneneswnenweweeseene
neeswnesenenwnwnenenwswnenenwswwnesenw
senenwnenenenenenwnene
sewnwneseeweswseeseenesesenweswwwne
swswnewseswswswswswwseseswswseneswswswsesw
nwnwswenwnwnwnwnwnwnwnwnwnwsewnwnwnwnenw
eseneswswnwswnweneeeeeeeeeeee
nenwnenwnwnenwnenwnwnwneswnwnenwe
senenenwsweswsweseenwnwwswnwswswswnwwe
nweneeneseneswwneswneneenewneeneene
seeseswnesenwnwenwnwnweneswwwnenwnwwnw
eseseeseeeseesesenesenweeseswsesese
swswsesenwswswswsenwswneswswseneneswsew
nenwnwnwseewnwwnwnwsewnew
swswseswswswneweswswswwswswswswswswswswse
wswsweswswswwsweswswwnwswswswswww
swnewwsenwwswwnewnwwsewwswnwnwne
wnwwwwnwwwsewnenwneenwswsewwnwww
nenweeeeeneeeesweee
sesesenwswswneswenwswwswwneswnesw
swswsewswwswswswenewswsww
seswseesesesesenesese
nweeseseneswwewseswseneseeeeeese
swnwswswseswwsweswwwswwwwewwnw
wnwnwnwnwneswnwnwnwnwwnwnwnenwnwsenwnwnwnw
nesenenwwneneswnenwnwsewneenwnwnwnwnw
eweeneneeseneeenweneeeneseenene
nwwnwnenwsewnesewwewwnewwnwese
eseeseseeeeseseenweseeseseeesw
wwnwwnwwewnwnwnwnwwnwwwnwsww
wnenweewsesewww
nwseseseseseseweeseeseseseseseesese
seseswwseswswsesweswseseswswswsesw
wswswsweswswwswswswwswswwsw
eeeneenweneeeeeeeewseseene
neneneenwseseswswenewsenenenwwsw
nwnwsenwnwsenwnwwnwnwnwnwnwnwnwnwnwnwnw
eeneseseneenenwewneeeeeneene
wwswswswwwwswswswwswwnwsewsw
senewseseewnwswseseseswseseswseswsesese
seswseswnwnwweseneseswsesesweseswsew
nenwnwseswseeweewnwnwneswnwswwnwsww
nwnwnenwnwnwnenenenenwnwnenesenwsenenenwnw
sewnwneswsewwswnwwseewnenenweswne
nwseswnwswsesenwnwnwwswnenewseswsewe
newnweseswsesenwwnwwnwnwnwnwnee
swwnweswwneeswnwswenwnwswswwswwse
sewwnwswenwwwwsewnwwseswneeswneswe
wwwwwwwnwwnesewwwwwnwnwww
seenwwswenwswnweeeeeeseeewsese
wswnwwwwwsewnewwswsewswwseenw
nenenenwwenwnwnenenenwnwnenesenwnwwse
nwnwnwnwnwnwwnwenwnwnwnwnwnwsenwnwnwnwnw
seseswseswnwseseseseseseseseswnwsesesesw
wnwwwwsenenwsenwwnenwwnwwwwenwnw
nwnewseseswsewnwnenwswnwseenesewnwnenw
neswsewwwwweneswwwweswwwnenw
nweswneneeneeeeneneneneswneneeneee
wswneneseeswswsewsenweseseswenwnwsw
sewswnwnwnwnenwnwnwnwswsewwwnwnenwwne
swswwswswswwswnenwswswsweswsenwweseswse
neseswwwnwnwnewnwseswnwwwenwnwnwe
swwswswswseswneswswwswswswwswnwswsesw
wnwnenwneswwnwwnwnwwsenwwnwwwwnw
wwnewsenwwnwwnwwwnwnwenwwnwww
swseswnweewwswnewswwenesenewswnw
eneeseenwsesweeneenweenwwesese
sesesewswwswswswswseeseseseseseswewe
nenwenwnwwnwnesenenwnwnenwswnwnenenwe
seenweeeseseseesenwseswseswsew
nenwnenesenwwnwnenenwnwnenwnwnenenenwnw
nwswseewswnwneswswneseseswsewseswnwse
nweneswwnwnwwwewsenwwwwwnesww
neeswswswwwwnwsewneswswswsewwwwww
wswseswneswesewneneesesenwseenwenw
wewwswewwnwwsewwwwwnwnwwwne
ewseneesesesesenwseesesweseseseee
nwenesweeeeeneneeneneneeneeee
nwswseswsesesewswseseswseseneseswse
wnwnewneneneswewneneneeneneenenenene
nwnwnwnwnwnwnwnwnwnwnwnwwnwenw
wwswswwnwwwwswswwweswwwwsww
seseseseewseseswnwneeeseseesenesese
neenwnwnesenwswnenweenwsewnwwswwnwse
swswseswnwswseswswswseseswseswnwsesweswswsw
eseeeseenenwenesweneweeenwwse
nwwwwwwwnewwwnwsewwwewwwsw
swswseswseeswswseswswswswsenenwwseswsw
nwneswseneneseeseseeswseswsenwseseswsese
nenwwwwwwwwseseweswwwwwww
wsesenwwseeseneseneneswwseseeseswsese
swswswneswswswwwwwwswswswswnwsewswse
eesewsesewneswseneeseenweeswese
swsenwsenwswneswswswseswseneseseswsesenwse
wwwswnwsweswwesewwwwwwswwwe
eswswnwwwwnwweenwwwwwwnwwnww
seweeeneeneeeeenwsweneneewswsee
nwswnwnwnwwnwnwnwsenwnwnenwnwnwnwwnwnw
sweswwseseswsesesw
nwwnwenwswwnwnewwswsewnwwwewwwne
nwnwnwnwwwnwnwnewnwnwnwnwnwwsenwnww
wswswswneswseswswnwweewswwswswwswsw
neneneeneswnenwnenenesenewneeneeene
neswseseseneseeewewneseseseseseswnw
sesweswnenenwneneeneneneneenenwnwsenene
nwwenenenwseewwseseswnwwwwwenewe
nwnenwnwnenenenenesewswnenwneenw
neeneneneneenewseneneneenene
wwwswwesenwswneswswswwwswneswswswsw
sewwwwwwwwwwwne
seseeeeeseseseseweneseseswsesewsenw
seeseeeseewseeneseseeesesesesee
wswwswwwwswweswsw
nwswnwnenwnwneswnenwnwnwnenwnwnwnenwnwne
wswnenwnwnenwnenwswnenwsenenenenenwnwenwne
enweeeeeeeeseeeswewweeee
seeeesweseeeeseweseneenesenwsese
nenwnwswnwnwwswwsewnwnwnwnewnwwwnw
sesesweneweseswseseseswnwsesesesesesese
senewwwswswwnewwnewsewnwwenwsenww
wwwsenwnwwnwnwnwnwwnwwnwwnwsewnw
swswswswswseswswswswnewnwwnwswwwswe
seeeneeseesenweseeeewseeseswnwsw
wwswnewewsewnwweswwwwswswww
nwnwwnwnenwnwswnwenwnwnwnwnenwnwnwnwnwnw
neneeneneeneeenweewneneswne
nesenewenenenenwnwnenenesenewsenesewe
seneeseeswesesesesenweeewese
sesesesesesesesenwesesesesw
wnwwnwwsewwwsenenw
neneeneeeneneeeeenwesweeeee
eeeseneneneeneeneneeneewewee
seseseseseseswsesenwsesesesesesese
wsweswswnewswwsweswwswwswswwnwswne
wswseswwwnwswswesewswneswnwwwnee
eseseseswneseseeseseseswseneswnwsewsesww
nwnwsenwnenwnwnenenwswwnwswwnwneeee
neneeeneneewwneeneneneeswseneneeee
nenwewnwwwnwnwsenwse
nwnwnwnwnwnwnwnwswwnesenwnenw
nwweseseseseseeeseseseeeenesesese
nenwnenwnwnewswnenenenwnesenenenenenesenw
seneseseseseseseeseseseseswew
neseeneenenenenenenenenewneenenwene
nwnwnwnenenenwnwnenesenenenenwnenewnwne
swseswswseswnewswwswswseneswneswenwsw
eeneeseenwesweeneweeseenwene
wswseseseseneseseseseswwseneswesesenenw
sesesesesesewseseseseseseseeseneswsesew
nweswnwnwesesweseswswnwnweseenwsene
seneseeseeeeseweseeeseseesesese
wneeneeneweneseneneneesenenenenene
seseesesesewwseseseseeesesenwsesese
neneweeseneewnwseneneeswneneeeneene
neswenwswseseswswneswnwenenwseswsenwnw
swswnwseseseseswswsweswnwsenwswnwsesesese
wnesenwwwweswwwwwwwswwwww
eswnwwswweweeeswnweswnenweseww
wwnwwweseswnwwnweseeneswswe
wnwnewwnewwwwwwnwsewwwwwsww
neswnenenwneneneneneesenewneneeeenene
wneewnewsenwsenwnewswesenwswwnewsw
nwsesesewwnenwnwe
nwnenwnwnwwnewenwsenenwneesenwnwnewse
seneseswseseseswseswsewnese
swseswswswseeswswswswnwsesesesw
newnwseeenwnwwswnenenwnwnenwnw
swwwnwswswswsewwswewwswwswwwnesw
nwnenwnwnwseseneenwnwnenenwnwnwnwsewsw
seeseseseseseseeswnwsesesesesesesenw
sewenewnwneneewnwnwnwnwnenenwnwnenwnw
neneneesenenwnwwneneneneneneenenwneswnw
wwwwswswswswewwenwnwwswwwswsew
senwwwwwwwnwwwwwwwwwweww
sesesesesesesesesenwsesesese
nwwnwnwnwnwnwnwnwwnwnwwnwsenwnwnww
nenwnwnwnwsenwnwnesewnenwnenenwnwnwnwnenw
nwneneeeeeeeeeeneeeeseswee
swseswseswswswswswswswseswnwsweswswswsw
swswswnwswswswswswseswswswswneswswswswse
seseewsesewwnwnesenenenenenenewnenesew
neswseswnwseswswswesesweneswnwseswnwsew
senesewnwswwseseseseseeeneeseeese
nwsweenwenwnwnwwnwnwwnwnwnwnwwnwswsw
seseswswswnenwseswneeswswswsweseseswnw
seseswseswswseseneseewseswswseswswsewse
swswswwsesewswswswswwswnwwwwneswswnesw
eeeeeeenweseeesw
eeeneeeneeweseeeee
nwnenewneneneenwsw
neeneswenwweeswewwseenenw
nwswnwewwnwnwnwwsenwnwnwnwwnwnwnwsenww
enewenwseneneswnwnesewsewnenenwneseswne
seneswswseseswswesenwsesw
nwnwnwwwnwnwwnwnewnwsenwwnwnwnwewnw
nwnwnwswswwnwsenwnenwenwnwsenwnwnenwnw
nwseneswnenwwnwnenenenwnenwnenene
wswswswswwwwswwswsenwwswswswsw
wwwnwnwwenwwsenwwwwewwwnwww
eseeneswseenwwseeeeswsesenwswwe
neseeswnenwwnewsesesewsweneseeswsese
seswswwseswswswsenwseseseseseeseswseswnw
swnwswwnweweewwwswwwwswesww
nenwsenwenwnwnwnwnwnwnwnwwnwnwnwnwnwnw
swswseswneswswswsenwseswswswnwneseewswsw
neneweenenenesenenene
swneswswswswswswswswswsw
nenenenenenenwnenenesewnenwnwnenenesenene
eneeseeeeeeeeeeseeewesee
seeseeseseeneseseeeseseseswese
seseswenwswnwseswswswswseseeswswseseseswse
nwwnwwnwnwwnwnwnwseewnwnwnwnwnw
seneswswswwswswseneswswseswswswseseswsese
nenenwneneneenwnwwnenwnwseneswnenenwnw
seseewsenwneeneswenwnwneswenenenese
nwnwsenenenwnwnwnwswnwnenwnwnenenwnenwne
nwnwnwwenwswnwnwnwnenwnwnwwnwwnwwwsww
swswnwseseseneswneneesewseenwnwsesew
neneenwnewneswnenwnenenenwne
wswswneseseswseswsesewswswseseseseseese
nwnwnwnenwnwnwwnwsewswnww
wwewnwwwnwwwwnwwwnwnwnwwwsw
swsesewsewnenesenesesesesesenweswsese
newnenwswnwneneweswsenenwenenwnenwese
swswswswswwswwswswswsewswswnwswswswwe
ewswneswnwwswwwwwsewwwwwswwww
wwwwwwswwnwnwwwwweewwwww
wswseneswnwneswnwnenenenwwsenwnenwnese
nwnwswneswnwnenwwnenwnwenwnwnwnwwnwese
neenenenenewneneneneneenene
weewneswseneneseneswnenww
sesesesesesweseseeeseesewseewwenw
nwnwnwnwnenwnwnwwwnwsenwnwwnweswnwnw
nwnwnenwwwsenwnwnwnwnwnwnenwnenwnwnwnesene
swseseseseswseseseseseswsesenwnwsesesesese
wwwnwwnwnwnwewwnwwwewswnwww
seswseenwseseeeweee
swswswnenwseswnwseeseswneseseseneesesww
neeneenwswesweneneswswneeneeswene
nwswswweesewwwnenwwewwwneww
neeeeeewseseseeeseseee
sesenesesesweseseeswseswseswwseswnewnww
newenwsewseweseseseseseswseseenesesee
seseeseewenwwseeseseseewneesesewse
nenenenesenwneeneswneneneswneeeneneee
sesesesewseseswseseseseneswseseseswsese
newnwnenenwnweneneeswnenwnwsenwnwnenene
swswswseswswseswswswneseswnwswseseswesw
nwneswswnwnwenwnwnwnwnwnwne
wnewswwsewwenwwwwnwwnwwwnew
swsenweseseeeenwseseeeseseneesewse
wwenwwnwnwwnwseewnwwewsewnw
nenenwnwneseswnenwnenwnenwnenenwnenenwnene
seneswswswswnwswswswswswswseswswswswswse
nwweswwwswseswwwwnewwwwwswwswsw
wwneswnwnwnwenwwsewwnwnenwnwwnwsenesw
sesewnwnwnwswsenewwnenwnwnenwnwnwnwnwnenw
swnwswswswwswswswswneswneseswswswswswsw
wswwneswswnewswswsesesesesesenweswsw
neseseswseseneswwswsesewseseseseswswsese
swewneeswswswswsenw
esenwnenwswsewswswseswseswnwseswswswswse
swewswneeeseseseeeeeweeenenwe
nwwnenenwwneseneseneneewseneneneeswne
nenewnenwenwneweswnwne
eeeeeeeeeseseewweseesenesene
swnwswnwenwewwwswnwswnwwnenenesesw
seenesweswsesweeeenewneneenewesw
seeseeweseseseseseseseseseseseewsese
swswswneseneswswswwweswseswseswswswsw
sweeeeeeeeeneeneneeswene
eswwsenwswswswseswswswswswswwswnwneswsw
sesesesweseeseneeseseseseseseseseswene
enweseseeeseeseseseneeesweeese
wwswwwwwswwewwnenewwwswwww
seewnewenwneseeeweeesweeeseene
seneseneneneenenewneenenenenewnenene
wnweseswswnwnweswnwenwswseeswswswswnw
nwneneneeneneswneneneneneneneneneneswnenene
nwnwnenwnwnwsenwnwswnenwnwnwnwenwnwnese
nenenwneneneneneeneeneeneneneeneswne
nenwneeseenenewsesweneenenenwneeene
neneneeneeseewneeneewwe
wenwsesweseeeenwesewenweesesese
nwnwwnenwnwnwsenwsenwenenwnenwnwnwnwnwnw
swesesewseswswseswswswseneneseseswnwsesw
nwnenwnwnwnwswnwnwnwsenwnenwnwswnwwnwne
eswwswseswsewenwnewwnewseswneswwnwsw
swseswsesesenwseseseseswsese
esweswnwewsenwsewse
wsesewwwwwewnenewnwwswwwneww
swseseswwswswseseswswneseswswsesewswneswsw
swswswswseswseswswswswnwnwswswswseswnesw
nenenesenenenenenenenenenenenenwneneneswne
eseswesewnwseeseneesweewseseenwese
swwswswswnwseswswswswwswsweswswswswswsw
neseeenwnweswenwsewseseneswwswnwsww
swswwswneneswswswswnewswseswswsenw
senwnenenweneneeneswneeneneeseeswnew
nenwnwnenwnwnenwswsenwnenenenenwnwwnwe
nwwnwnwwnwnwwnwnwnwnwnwnwenwwnwnw
swswswseswswswswswnwwseswenwswseswsweswe
wnwwwenwsewswwwwswnewwwnwwww
nwwwnwnewsewwwwnwnwwwnwnw
nwswweswewswswnesw
sewsweseswswswswnwsweswswswswswswswse
ewneneseeseneeeeeneeneenwnwnee
seswswswwseswswseswswswswswneswsese
eeeneeseeeeeseseswesese
neswsenwwenwnwsenwswnenenenenw
eswewswwneenenenenwsenwsewnwenewnw
wwswwswwenwswwewswswwwwwwsw
eenenenwwnesweeenenewseswenesene
seswneneneseswwenwnweneneneenenenene
wenwwnwswwwnwwwnwnenwnwwnwswwe
nenenenenesenwneweneenenenenenenesene
wwwwwwwwswwswwwnwwswswesww
wwsesenwnwnwnwwnwnwnwnwnwnwnwnwsewnw
senewnwenenenenenewneneneneneneneswneene
sesewnwneneneneneneneneewneeneswnw
swwswswswswswswnesw
swweneswnwwnwwneswseswswesewwswse
neneesenewnewneneenenewnenenenenene
nwnwswneswseswnweee
wenewwwseseswnewwwwnwwswswswwnw
swsewnenwenwsenenwse
nenenenenwneneneenenwnesenenenenenesenene
newwwwnwsewwwwwswswwnesewsww
wnwenwwnwwswnwwnwwwwswnwnwenwnw
weswswwwewsweswswwswwwwswww
seneeeseenwsesenwswsweesenwseeeewe
swnwenenenenenenwsenenwnwnenesenenenwnwne
nwnwnweswenwnwnwnwnwnwwnwswwne
swswwswswswseswswswswsweswnwswswswesw
nwnenwnwnwnwneswnwnenenenenenewsenenwnenw
nwsesenenenenenenenenenenenenenenenenwsenew
eseeeeeeeeeeeeweene
senwnwnwwenweswnenwnwneewnwnwwww
swnwsenwswneneneenweswseweneneneee
weeeewseseeeeseeeseseseseee
wnwenwnwenwwneswnwwnwneneeswnwenw
nwsenwnwnwwwnewwnwseneseswnwnwwnenwnw
nwnenenwsenwnenenenwnenenenewnwnwneew
senwnenwnenwnwnwnwnenwnenenwwsenenwnwnw
enwwwnwnenenewneneswenenesewenene
nenweneneseeenenwenwnenewneswseneswe
wneseneneneenwneneneneneswnenenenenenenene
neneneeseeneeneweneeneneenene
eenesweeneneneneneneneneeenene
esweseeneeeeneeneneeeeewnwse
sesesweswseseseswseswnwseswseseseswsese
nwnenwnwwsenwnwnwnwnw
nwenenweneneswsesweeneneewneeene
ewneseseeswseeneeeseew
swswwswswnwswswswswwswseswswswnewswewsw
sesenweseeseeeneneeseweswsesesesw
nwsenwseeswswsewwsenenwwnwenenwswwene
swswnwwseeneswwnwnew
swswswseswseswswnwseswswswseeenwswswsese
swnwneswnwwsweswseswwwnwswwswswewse
nwnwnwnenwewnwnwnwnwnwneneswnwnwnw
seseseswsesesweswseswswseneswwseswswsese
swwweswwneswswwswwwseesenwnewse
wswwnwwsewnewswwwneswnewweswswswne
eseenewneneswsesenwneneeneneswswnenewsw
seswesenwnwnwneswswnewwnenwneseneenew
wsweswswswswswswneswswwwswneswswwswwse
swneneneenewneneneenwwsewneeeneswne
nwneswnenwwswnenwsenwnwnwneenwseenenenwsw
seseswseswseseseneswswsweswswswsewsesw
ewneneseeneeneneswnw
eeseswnwseseeeeeseseswnweseeseee
wwewseswwwswswwwnenewswswwwswsw
nwewwenwswnenwseswwnwwwnewnwnwswe
seswseswsesesweswseswsesenwsesesesesese
swswswwwswwswewswswnewwewswwwsw
swnesesenwewwseseeswnewnenwwenese
swnwwseswenwsenwneseseseseseseenwsesw
nesenewsenwnweswwwwnwneswnwwsenenwne
ewnwnwnwnesenwnenwnwwswnwsenenwenwnw
esesewneseseswsenesenwwseswsesesenenw
seneswwwnwenwseswnwwwwwswswwene
eswwnewnenenwnwnwnenwnwnenwsenwneene
wneseseesenwnwseneneneswswwwwnwswenese
nesenwnwwnesenesenenwneswnwneswnwnwnenene
senesesesewseseseseseseseseseseesesese
neweeeeneenenwswenesweeeswneee
swneneenenenewnenenenwwnenwsweenenene
nwwenwwwseswswwneswwsenewenwnwnw
nenenenenwsewneseneswneswnenenwne
swswswwswsweswswswswswswsw
nenenwsenwnwnenwnwnenenenwnenenenwneswnw
neneswnwnwnwnwnenwnenwenwnwnwnesenwnenw
nwnwswswswewswswswswswswswseswswswswsw
weeeeeeenee
swnwswswsenwseneswseeseswswneseseswswnwse
swswwseseswswneswseswswneswseseneswswsw
weesweneseweeenesewesweneesee
seswswswseseseseseseseneseswsesenwesese
eweneseeeseneewnwneeeneneenese
swwnenwswwnwnwnwnwswwswenweenwnww
nwwwwwswwswwwwwswswswwswswwe
eneenwneseseeneneeenwseneswneenew
sesesewneswnwnenewneseeneswnwnenweew
eswswswsenwnwnwnewswnenwseswnenwnenee
enwwwesweeeeeeeeseeseeeeee
eseeeeeeeseeeneseswwseeeeee
nwnenenenenenwnenenenenwneswsenwse
weseseseeseseseseseseseeeenesewse
neeneeeweeesweeswenweeswnwenw
nwwwneswswwswswewewwseswswswww
nenewnenenwnwnwnwnwnenwnwnenenesenwnwnw
esesesenesewseseneseseswsese
sesweswneneneneswwnewnwnenweenenenwsw
nwneneswnwswswwnewneneneseseenwsewnese
wneseewnwnwenwnesenwwseseswwnewnwe
senwwsweeesenwse
wnewwnewsewwnwseswwwswnewwwsw
nwnwwnwnenwnwnwnenwnwwnwsenenwsenwsenw
enwwwwnwwnwnwwwnww
wwnwnwswewneenw
wnewewswwwwnwswwswswseswswwnesw
wneenweweneeweseeeeseeeseee
eeswswweeeneseeeneenwswseseenw
nenenenwnwnwnwnwnwswnwnewnwnenwnwnwenwne
nwswswsenwseswnesewnwswswswswseswneswsw
seeweeeeeseseseseeneseseseseswse
sesewneseswneseeseswseesewwswsesesw
nweneswsesenewweseswwneseneeseswswe
senwseseswsesesesenwesenwsesesewesesesw
eneneneeenwnwnwnwseseweneswse
neswneswnenenewswnesenwnenenenenwnwnesw
neswswseswswseswswseweswswseswswswswsesww
neeeeeweneesw
eneneeeeneswnene
swswswswneswswswswsewswswswnwseswneswswsw
wnwesweneneseseneweeesewswwwnwe
wnwewwwnwnwwnwwenwsewswnwswww
neeswnenwnenwsenwnenewweswsenenwnwswnw
nwswnwnwwnwnwnenwnwenw
swneswswswswwswswswswswswswwswswsw
swnwwnenwneswneenenenenenenwseeeesesw
wnwnwnwnwnwnwnwnenwnwnwnwnwswwswnwnenwnw
seswswnenwseswnwsesweseseseseswseneesww
seewnwnwswneweswwwenewwewsenwse
swswseswswswswnwswweswseswseneseswswsw
neneneeeneeseeswsweeeewneeeenw
swseswseswswswneneseswneswnwne
neeeneeneneneneneswnwenene
swsewewnwnwnenewnewnwwseewseesese
nwnwnwewnenwenwewnwnwnwnwnwswnwnwnwnw
nwwweeeeseswseseseseesenweseewe
sesesewswsweseseswseswswseseswswneenwnw
nenenenenenenwneneneseswneneneswne)";

int main() {
    solution(input);
}
