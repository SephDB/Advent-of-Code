#include <algorithm>
#include <numeric>
#include <cmath>
#include <vector>
#include <stack>
#include <iterator>
#include <charconv>
#include <unordered_map>
#include <unordered_set>
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

struct Reaction {
    std::int64_t num_output;
    std::vector<std::pair<std::string_view,std::int64_t>> ingredients;
};

using ReactionMap = std::unordered_map<std::string_view, Reaction>;

std::pair<std::string_view,std::int64_t> parse_element(std::string_view in) {
    auto s = split(in,' ');
    s.erase(std::remove_if(s.begin(),s.end(),[](auto t){return t == "";}),s.end());
    std::int64_t num = 0;
    std::from_chars(s[0].data(),s[0].data()+s[0].size(),num);
    return {s[1],num};
}

ReactionMap parse(std::string_view in) {
    ReactionMap ret;
    ret["ORE"] = Reaction{1,{}};

    for(auto reaction : split(in,'\n')) {
        auto eq_pos = reaction.find('=');
        auto [result,num_output] = parse_element(reaction.substr(eq_pos+2));
        auto ingredients = split(reaction.substr(0,eq_pos),',');
        Reaction r{num_output,{}};
        std::transform(ingredients.begin(),ingredients.end(),std::back_inserter(r.ingredients),parse_element);
        ret[result] = std::move(r);
    }

    return ret;
}

std::vector<std::string_view> topo_sort(ReactionMap& reactions) {
    std::unordered_set<std::string_view> visited;
    std::vector<std::string_view> ret;
    auto visit = [&](auto&& rec, std::string_view node) {
        if(visited.contains(node)) return;
        for(auto [next, num] : reactions[node].ingredients) {
            rec(rec,next);
        }
        visited.insert(node);
        ret.push_back(node);
    };
    visit(visit,"FUEL");
    std::reverse(ret.begin(),ret.end());
    return ret;
}

void solution(ReactionMap reactions) {
    auto sorted = topo_sort(reactions);

    auto ore_needed = [&](std::int64_t fuel) {
        std::unordered_map<std::string_view,std::int64_t> total;
        total["FUEL"] = fuel;
        for(auto chem : sorted) {
            auto needed = total[chem];
            auto& [num_per_reaction, ingredients] = reactions[chem];
            auto num_reactions = needed / num_per_reaction + ((needed % num_per_reaction) > 0);
            for(auto [in, num] : ingredients) {
                total[in] += num*num_reactions;
            }
        }
        return total["ORE"];
    };

    auto one_fuel = ore_needed(1);
    std::cout << "Part 1: " << one_fuel << '\n';
    
    auto max_ore = 1'000'000'000'000;
    auto min = max_ore / one_fuel; //simple lower bound
    auto max = min*2;
    //Exponential increase if not enough
    while(ore_needed(max) < max_ore) {
        min = max;
        max *= 2;
    }
    while(min < max - 1) {
        auto next = min + (max - min)/2;
        if(ore_needed(next) > max_ore) {
            max = next;
        } else {
            min = next;
        }
    }
    std::cout << "Part 2: " << min << '\n';
}

std::string_view input = R"(3 NPNGZ, 3 TBFQ, 1 RZBF => 2 LQNR
1 GWZRW => 9 CHNX
1 DBVD, 10 VCHK, 12 WNHV, 1 FMKT, 1 DKFT, 1 BTLR, 12 VHKXD => 4 ZMWC
2 RZBF, 13 JSBVZ => 3 JLVRS
15 BZNB, 1 JSBVZ => 5 ZTCM
28 GMSF, 18 LTGJ => 9 BTLR
1 RZDM, 3 BNJD, 1 FLXL => 1 FDBX
2 BZNB, 1 JLVRS => 2 GMSF
1 FDBX, 2 ZSQR, 1 XMBS, 2 FMKT, 1 BNJD, 12 TRXVN => 7 CRNMW
1 PCHWB, 6 LXJPK, 2 ZSQR => 5 DBVD
5 LMTM, 9 RZBF => 7 TBFQ
2 KVWJG => 2 RZBF
1 LBFXF, 17 NRBGS => 6 JSBVZ
1 VQTFW => 5 LMTM
20 DBVD => 2 SRFQ
3 XSVZ, 7 JSBVZ, 5 NPNGZ => 2 VWMQZ
1 ZQDN, 1 RZBF, 1 NDNKB => 9 FMKT
7 BZVGH, 9 NDNKB => 6 LTGJ
3 VWMQZ, 1 XSVZ, 4 GKDGX => 1 TRXVN
3 VXFJM, 14 FMKT => 6 NFTJ
21 WXWHD => 1 VCHK
1 TJZVQ => 6 NDNKB
6 NFTJ => 3 RZDM
1 VHKXD, 1 TBPWN, 1 FDBX, 2 XMBS, 2 WJTRC, 20 BTLR => 8 VSBV
19 XSVZ, 7 LJQG, 10 ZTCM => 1 GKDGX
3 NPNGZ, 2 RZBF, 8 GWZRW => 1 QVDFQ
1 TBFQ => 7 VHKXD
3 LTGJ, 1 ZXWB, 2 MNPBV => 2 BNJD
9 LQNR, 2 QVDFQ, 10 GMSF => 4 XSVZ
13 VHKXD, 17 CHNX, 1 NDNKB => 8 VXFJM
122 ORE => 4 LBFXF
18 NPNGZ => 5 LXJPK
1 TJZVQ, 1 FXGH => 5 GWZRW
9 BZNB, 4 JLVRS => 3 KDCG
1 SNLNK => 8 WNHV
4 VHKXD => 4 DGFN
1 RZDM => 6 SNLNK
3 CHNX, 8 LTGJ => 4 TBPWN
2 DGFN, 1 NFTJ => 2 RNSXD
1 FXGH, 1 BDCLW => 2 LJQG
3 DGCMV, 2 BZVGH, 7 RZBF => 8 MNPBV
14 WXWHD => 2 XMBS
1 BZVGH => 8 FLXL
8 VXFJM, 1 NFTJ => 2 WXWHD
2 LXJPK => 9 ZSQR
5 NRBGS, 1 LBFXF => 9 FXGH
4 NRBGS, 27 CHNX => 9 PCHWB
3 LBFXF => 4 TJZVQ
185 ORE => 2 VQTFW
1 RTVFM, 1 TBPWN => 6 DGCMV
10 NRBGS => 3 ZQDN
5 JSBVZ, 8 FXGH, 1 TJZVQ => 7 NPNGZ
1 PCHWB, 6 LXJPK, 5 LTGJ => 2 ZXWB
1 NPNGZ, 24 FMKT => 6 WJTRC
4 KDCG, 33 BZNB => 3 KGNH
2 KGNH, 4 ZTCM, 15 CHNX => 6 BDCLW
10 LXJPK, 18 TJZVQ, 1 FXGH => 7 BZVGH
27 DBVD, 2 CRNMW, 8 ZTCM, 8 RNSXD, 14 VSBV, 6 ZMWC, 37 TBPWN, 53 SRFQ => 1 FUEL
19 FXGH, 4 TJZVQ => 3 BZNB
17 QVDFQ, 26 KDCG, 7 CHNX => 3 RTVFM
111 ORE => 6 KVWJG
3 ZTCM => 6 DKFT
124 ORE => 6 NRBGS)";

int main() {
    solution(parse(input));
}
