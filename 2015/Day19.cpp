#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <map>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <span>

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

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

auto splitChem(std::string_view chem) {
    std::vector<std::string_view> ret;
    std::size_t pos = 0;
    while(pos != chem.npos) {
        std::size_t next = chem.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ",pos+1);
        ret.push_back(chem.substr(pos,next-pos));
        pos = next;
    }
    return ret;
}

auto parse(std::string_view input) {
    std::unordered_map<std::string_view, std::vector<std::string_view>> rules;
    auto lines = split(input,'\n');
    auto chem = splitChem(lines.back());
    lines.pop_back();
    lines.pop_back();
    for(auto l : lines) {
        auto comps = split(l,'=');
        comps[0].remove_suffix(1);
        comps[1].remove_prefix(2);
        rules[comps[0]].push_back(comps[1]);
    }
    return std::pair{rules,chem};
}

template<typename C, typename F>
void pairwise(const C& c, F&& func) {
    auto current1 = std::begin(c);
    auto current2 = std::begin(c)+1;
    auto end = std::end(c);
    while(current2 != end) {
        func(*current1++,*current2++);
    }
}

auto part1(decltype(parse("")) input) {
    auto& rules = input.first;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> forward_dupes, back_dupes;
    for(auto& [lhs,trans] : rules) {
        for(auto rhs : trans) {
            if(rhs.starts_with(lhs)) {
                forward_dupes[lhs].insert(rhs.substr(lhs.size()));
            }
            if(rhs.ends_with(lhs)) {
                rhs.remove_suffix(lhs.size());
                back_dupes[lhs].insert(rhs);
            }
        }
    }
    auto num = rules[input.second[0]].size();
    pairwise(input.second,[&](std::string_view a, std::string_view b) {
        if(rules.contains(b)) {
            auto dup = std::count_if(back_dupes[b].begin(),back_dupes[b].end(),[&](std::string_view dup) {
                return forward_dupes[a].contains(dup);
            });
            num += rules[b].size() - dup;
        }
    });
    return num;
}

int part2(std::span<std::string_view> chem) {
    //Every replacement is either AB or ARnBAr|ARnBYCAr|ARnBYCYDAr
    //We assume there is a solution
    int total = 0;
    while(chem.size() > 1) {
        if(chem[1] != "Rn") {
            //Form AB, replace with a single letter
            total++;
            chem = chem.subspan(1);
        } else {
            //Find matching Ar, compress all sub-sequences along the way
            int level = 0;
            auto find = [&](std::string_view c) {
                if(c == "Rn") level++;
                else if(c == "Y" and level == 0) return true;
                else if(c == "Ar") level--;
                return level < 0;
            };
            auto current = 1;
            do {
                auto next = std::find_if(chem.begin()+current+1,chem.end(),find) - chem.begin();
                total += part2(chem.subspan(current+1,next-current-1));
                current = next;
            } while(chem[current] != "Ar");
            total++;
            chem = chem.subspan(current);
        }
    }
    return total;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in.second) << '\n';
}

std::string_view input = R"(Al => ThF
Al => ThRnFAr
B => BCa
B => TiB
B => TiRnFAr
Ca => CaCa
Ca => PB
Ca => PRnFAr
Ca => SiRnFYFAr
Ca => SiRnMgAr
Ca => SiTh
F => CaF
F => PMg
F => SiAl
H => CRnAlAr
H => CRnFYFYFAr
H => CRnFYMgAr
H => CRnMgYFAr
H => HCa
H => NRnFYFAr
H => NRnMgAr
H => NTh
H => OB
H => ORnFAr
Mg => BF
Mg => TiMg
N => CRnFAr
N => HSi
O => CRnFYFAr
O => CRnMgAr
O => HP
O => NRnFAr
O => OTi
P => CaP
P => PTi
P => SiRnFAr
Si => CaSi
Th => ThCa
Ti => BP
Ti => TiTi
e => HF
e => NAl
e => OMg

ORnPBPMgArCaCaCaSiThCaCaSiThCaCaPBSiRnFArRnFArCaCaSiThCaCaSiThCaCaCaCaCaCaSiRnFYFArSiRnMgArCaSiRnPTiTiBFYPBFArSiRnCaSiRnTiRnFArSiAlArPTiBPTiRnCaSiAlArCaPTiTiBPMgYFArPTiRnFArSiRnCaCaFArRnCaFArCaSiRnSiRnMgArFYCaSiRnMgArCaCaSiThPRnFArPBCaSiRnMgArCaCaSiThCaSiRnTiMgArFArSiThSiThCaCaSiRnMgArCaCaSiRnFArTiBPTiRnCaSiAlArCaPTiRnFArPBPBCaCaSiThCaPBSiThPRnFArSiThCaSiThCaSiThCaPTiBSiRnFYFArCaCaPRnFArPBCaCaPBSiRnTiRnFArCaPRnFArSiRnCaCaCaSiThCaRnCaFArYCaSiRnFArBCaCaCaSiThFArPBFArCaSiRnFArRnCaCaCaFArSiRnFArTiRnPMgArF)";

int main() {
    solution(input);
}
