#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>

constexpr std::array weapons{8,10,25,40,74};
constexpr std::array damage_rings{25,50,100};
constexpr std::array armor{13,31,53,75,102};
constexpr std::array armor_rings{20,40,80};

struct RingBonus {
    int cost;
    int rings;
    int bonus;
};

constexpr auto bonuses(std::array<int,3> rings) {
    std::array<RingBonus,7> ret;
    ret[0] = {0,0,0};
    int current = 1;
    for(int i = 0; i < 3; ++i) {
        ret[current++] = {rings[i],1,i+1};
        for(int j = i+1; j < 3; ++j) {
            ret[current++] = {rings[i]+rings[j],2,i+j+2};
        }
    }
    return ret;
}

constexpr auto damage_bonus = bonuses(damage_rings);
constexpr auto armor_bonus = bonuses(armor_rings);

struct RingCost {
    int cost;
    int rings;
};

struct Lookup {
    std::map<int,std::vector<RingCost>> damage_costs;
    std::map<int,std::vector<RingCost>> armor_costs;
};

auto precompute() {
    Lookup res;

    res.armor_costs[0].push_back({0,0});
    for(int i = 0; i < armor.size(); ++i) {
        for(auto bonus : armor_bonus) {
            res.armor_costs[i+1+bonus.bonus].push_back({armor[i]+bonus.cost,bonus.rings});
        }
    }
    for(int i = 0; i < weapons.size(); ++i) {
        for(auto bonus : damage_bonus) {
            res.damage_costs[i+4+bonus.bonus].push_back({weapons[i]+bonus.cost,bonus.rings});
        }
    }
    return res;
}

struct Entity {
    int hp;
    int damage;
    int armor;
};

int min_armor_to_defeat(Entity t, int damage) {
    const int player_hp = 100;
    const int damage_per_turn = std::max(damage-t.armor,1);
    const int rounds_to_kill = t.hp/damage_per_turn + (t.hp % damage_per_turn != 0);
    const int rounds_to_survive = rounds_to_kill - 1;
    
    return t.damage - player_hp/rounds_to_survive + (player_hp % rounds_to_survive == 0);
}

auto part1(const Lookup& l, Entity boss) {
    auto cost = std::numeric_limits<int>::max();
    for(const auto& [damage,weapon_combos] : l.damage_costs) {
        int armor_needed = min_armor_to_defeat(boss,damage);
        if(not l.armor_costs.contains(armor_needed)) continue;
        for(RingCost armor : l.armor_costs.at(armor_needed)) {
            for(RingCost weapon : weapon_combos) {
                if(armor.rings + weapon.rings <= 2) {
                    cost = std::min(cost,armor.cost+weapon.cost);
                }
            }
        }
    }
    return cost;
}

auto part2(const Lookup& l, Entity boss) {
    auto cost = std::numeric_limits<int>::min();
    for(const auto& [damage,weapon_combos] : l.damage_costs) {
        int armor_needed = min_armor_to_defeat(boss,damage) - 1; //to lose
        if(not l.armor_costs.contains(armor_needed)) continue;
        for(RingCost armor : l.armor_costs.at(armor_needed)) {
            for(RingCost weapon : weapon_combos) {
                if(armor.rings + weapon.rings <= 2) {
                    cost = std::max(cost,armor.cost+weapon.cost);
                }
            }
        }
    }
    return cost;
}

int main() {
    auto lookup = precompute();
    Entity boss = {103,9,2};
    
    std::cout << "Part 1: " << part1(lookup,boss) << '\n';
    std::cout << "Part 2: " << part2(lookup,boss) << '\n';
    
}
