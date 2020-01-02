#include <iostream>
#include <array>
#include <map>
#include <queue>
#include <numeric>
#include <variant>
#include <algorithm>

template<typename ScoreType, typename State, typename HFunc, typename GoalFunc, typename NextFunc>
std::pair<State,ScoreType> As(State start, HFunc&& heuristic, GoalFunc&& isgoal, NextFunc&& for_each_neighbor) {
    std::map<State,ScoreType> scores;

    using queue_type = std::pair<ScoreType,State>;
    //Reverse ordering bc smallest needs to go out first
    auto cmp = [](auto a, auto b) {return a.first > b.first;};
    std::priority_queue<queue_type,std::vector<queue_type>,decltype(cmp)> AsQueue(cmp);
    
    AsQueue.emplace(heuristic(start),start);
    scores[start] = 0;
    
    std::size_t num_states = 0;
    std::size_t states_expanded = 1;

    while(not AsQueue.empty()) {
        auto [fScore,state] = AsQueue.top();
        AsQueue.pop();
        if(fScore-heuristic(state) > scores[state]) continue;
        num_states++;
        if(isgoal(state)) {
            std::cout << "\nExpansion stats: " << num_states << ' ' << states_expanded << "\nSolution: ";
            return {state,scores[state]};
        }
        auto current_len = scores[state];
        for_each_neighbor(state, [&,state=state](State next, ScoreType length) {
            if(not scores.contains(next) or current_len+length < scores[next]) {
                scores[next] = current_len+length;
                AsQueue.emplace(current_len+length+heuristic(next),next);
                states_expanded++;
            }
        });
    }
    return {start,0};
}

//These are all stats that differ between states
struct GameState {
    int player_hp;
    int player_mana;
    int boss_hp;
    int shield_time = 0;
    int recharge_time = 0;
    int poison_time = 0;
    bool operator<(const GameState& o) const {
        return std::tie(player_hp,player_mana,boss_hp,shield_time,recharge_time,poison_time) < std::tie(o.player_hp,o.player_mana,o.boss_hp,o.shield_time,o.recharge_time,o.poison_time);
    }
};

struct Spell {
    int mana;
};

struct DirectSpell : Spell {
    int player_hp_diff;
    int boss_hp_diff;
    bool cast(GameState& s) const {
        if(mana > s.player_mana) return false;
        s.player_mana -= mana;
        s.player_hp += player_hp_diff;
        s.boss_hp += boss_hp_diff;
        return true;
    }
};

struct TimerSpell : Spell {
    int time;
    int GameState::* timer;
    bool cast(GameState& s) const {
        if(mana > s.player_mana) return false;
        if(s.*timer != 0) return false;
        s.player_mana -= mana;
        s.*timer = time;
        return true;
    }
};

constexpr std::array<std::variant<DirectSpell,TimerSpell>,5> spells{
    DirectSpell{53,0,-4},
    DirectSpell{73,2,-2},
    TimerSpell{113,6,&GameState::shield_time},
    TimerSpell{173,6,&GameState::poison_time},
    TimerSpell{229,5,&GameState::recharge_time}
};

struct Info {
    int player_hp;
    int player_mana;
    int boss_hp;
    int boss_damage;
};

int play(Info game) {
    GameState start{game.player_hp,game.player_mana,game.boss_hp};
    auto heuristic = [](GameState s) {return 0;};
    auto goal = [](GameState s) {return s.boss_hp <= 0;};
    auto apply_effects = [](GameState& s) {
        if(s.recharge_time) {
            s.player_mana += 101;
            s.recharge_time--;
        }
        if(s.shield_time) s.shield_time--;
        if(s.poison_time) {
            s.boss_hp -= 3;
            s.poison_time--;
        }
    };
    auto after_player = [&](GameState& s) {
        apply_effects(s);
        if(s.boss_hp <= 0) return;
        s.player_hp -= (game.boss_damage - 7*(s.shield_time > 0));
        if(s.player_hp <= 0) return;
        apply_effects(s);
    };
    auto next_states = [&](GameState s, auto&& callback) {
        for(auto spell : spells) {
            auto next = s;
            if(std::visit([&next](auto casted) {return casted.cast(next);},spell)) {
                after_player(next);
                if(next.player_hp > 0) {
                    callback(next,std::visit(&Spell::mana,spell));
                }
            }
        }
    };
    return As<int>(start,heuristic,goal,next_states).second;
}

int main() {
    Info game{50,500,71,10};
    std::cout << "Part 1: " << play(game) << '\n';
    game.player_hp--;
    game.boss_damage++;
    std::cout << "Part 2: " << play(game) << '\n';
}
