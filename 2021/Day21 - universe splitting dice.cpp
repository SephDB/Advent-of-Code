#include <iostream>
#include <array>
#include <optional>
#include <map>

struct Player {
    int pos;
    int score = 0;
    auto operator<=>(const Player& other) const = default;
};

auto part1(auto input) {
    auto [current,next] = input;
    current.pos--;
    next.pos--;
    int die_state = -1;
    auto roll = [&](){return (++die_state)%100+1;};
    do {
        current.pos += roll() + roll() + roll();
        current.pos %= 10;
        current.score += current.pos+1;
        std::swap(current,next);
    } while(next.score < 1000);
    return (die_state+1)*current.score;
}

constexpr auto roll_amounts() {
    std::array<int,7> rolls{};
    for(int i = 1; i <= 3; ++i) {
        for(int j = 1; j <= 3; ++j) {
            for(int k = 1; k <= 3; ++k) {
                rolls[i+j+k-3]++;
            }
        }
    }
    return rolls;
}

auto part2(auto input) {
    auto [current,next] = input;
    current.pos--;
    next.pos--;
    std::map<std::pair<Player,Player>,std::pair<int64_t,int64_t>> cache;
    auto dfs = [&cache](Player current, Player next, auto&& rec) -> std::pair<int64_t,int64_t> {
        
        if(auto f = cache.find({current,next}); f != cache.end()) return f->second;

        std::pair<int64_t,int64_t> wins{};
        constexpr auto rolls = roll_amounts();
        for(int rolled = 3; rolled <= 9; ++rolled) {
            int num_universes = rolls[rolled-3];
            Player c = current;
            c.pos += rolled;
            c.pos %= 10;
            c.score += c.pos+1;
            if(c.score >= 21) {
                wins.first += num_universes;
            }
            else {
                auto [next_wins,current_wins] = rec(next,c,rec);
                wins.first += num_universes*current_wins;
                wins.second += num_universes*next_wins;
            }
        }
        cache.insert(std::pair{std::pair{current,next},wins});
        return wins;
    };
    auto [first,second] = dfs(current,next,dfs);
    return std::max(first,second);
}

int main() {
    auto in = std::pair{Player{8},Player{2}};
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}
