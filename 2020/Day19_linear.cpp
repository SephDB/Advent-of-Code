#include <iostream>
#include <string_view>
#include <array>
#include <vector>
#include <map>
#include <variant>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <cassert>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

template<int N, typename F>
auto split_known(std::string_view in, std::string_view delim, F&& apply) {
    std::array<decltype(apply(in)),N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = apply(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    ret[current++] = apply(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct SingleChar {
    char c;
};

//front-decided alternatives
struct AltsFront {
    std::array<int,2> sub;
};

//back-decided alternatives
struct AltsBack {
    std::array<int,2> sub;
};

/*
15: 1 | 14 wtih 1 and 14 being the single chars
*/
struct AnySingle {
};

/*
8: 42
0: 8 11
*/
struct Fixed {
    std::array<int,2> parts = {-1,-1};
};

struct Rule {
    std::variant<SingleChar,AltsFront,AltsBack,AnySingle,Fixed> kind;
    int length = 0;

    bool fits(std::string_view v, const std::vector<Rule>& rules) const {
        return std::visit([&,this](auto c) {return fits(c,v,rules);},kind);
    }

    bool fits(SingleChar c, std::string_view v, const std::vector<Rule>& rules) const {
        return v[0] == c.c;
    }

    bool fits(AltsFront cur, std::string_view v, const std::vector<Rule>& rules) const {
        auto c = v.front()-'a';
        v.remove_prefix(1);
        return rules[cur.sub[c]].fits(v,rules);
    }

    bool fits(AltsBack cur, std::string_view v, const std::vector<Rule>& rules) const {
        auto c = v.back()-'a';
        v.remove_suffix(1);
        return rules[cur.sub[c]].fits(v,rules);
    }

    bool fits(AnySingle cur, std::string_view v, const std::vector<Rule>& rules) const {
        return true;
    }

    bool fits(Fixed cur, std::string_view v, const std::vector<Rule>& rules) const {
        bool ret = true;
        for(auto r : cur.parts) {
            if(r != -1) {
                const Rule& rule = rules[r];
                ret &= rule.fits(v.substr(0,rule.length),rules);
                v.remove_prefix(rule.length);
            }
        }
        return ret;
    }
};

struct ParseOut {
    std::vector<Rule> rules;
    std::string_view queries;
};

auto parse(std::string_view input) {
    auto [rules_s,queries] = split_known<2>(input,"\n\n",[](auto a){return a;});
    
    std::map<int,std::string_view> rules_unparsed;
    std::array<int,2> single_char{};

    split(rules_s,'\n',[&](auto r) {
        auto [num_s,rule] = split_known<2>(r,":",[](auto a){return a;});
        int num = to_int(num_s);
        if(rule[1] == '"') {
            single_char[rule[2]-'a'] = num;
        } else {
            rules_unparsed[num] = rule;
        }
    });

    std::vector<Rule> rules(rules_unparsed.rbegin()->first+1);

    rules[single_char[0]] = {SingleChar{'a'},1};
    rules[single_char[1]] = {SingleChar{'b'},1};

    auto is_single = [&single_char](auto i) {return i == single_char[0] || i == single_char[1];};

    auto parse_rule = [&](int n, auto&& rec) {
        Rule& r = rules[n];
        if(r.length != 0) return r.length;

        auto s = rules_unparsed.at(n);
        
        s.remove_prefix(1);

        if(s.find('|') != s.npos) {
            //AltsFront/Back/Single
            auto num_spaces = std::ranges::count(s,' ');
            if(num_spaces == 2) {
                auto [left,b,right] = split_known<3>(s," ",to_int);
                assert(is_single(left) && is_single(right));
                r.length = 1;
                r.kind = AnySingle{};
            } else {
                auto [l1,l2,b,r1,r2] = split_known<5>(s," ",to_int);
                if(is_single(l1) && is_single(r1) && (l1 != r1)) {
                    AltsFront f{{l2,r2}};
                    if(r1 == single_char[0]) {
                        std::swap(f.sub[0],f.sub[1]);
                    }
                    auto length1 = rec(l2,rec);
                    auto length2 = rec(r2,rec);
                    assert(length1 == length2);
                    r.kind = f;
                    r.length = length1 + 1;
                } else if(is_single(l2) && is_single(r2) && (l2 != r2)) {
                    AltsBack f{{l1,r1}};
                    if(r2 == single_char[0]) {
                        std::swap(f.sub[0],f.sub[1]);
                    }
                    auto length1 = rec(l1,rec);
                    auto length2 = rec(r1,rec);
                    assert(length1 == length2);
                    r.kind = f;
                    r.length = length1 + 1;
                } else {
                    assert(false);
                }
            }
        } else {
            //Fixed
            Fixed f;
            auto space = s.find(' ');
            f.parts[0] = to_int(s.substr(0,space));
            r.length += rec(f.parts[0],rec);
            if(space != s.npos) {
                s.remove_prefix(space+1);
                f.parts[1] = to_int(s);
                r.length += rec(f.parts[1],rec);
            }
            r.kind = f;
        }

        return r.length;
    };

    parse_rule(0,parse_rule);

    return ParseOut{rules,queries};
}

auto part1(const decltype(parse(""))& input) {
    auto& r = input.rules[0];
    int total = 0;
    split(input.queries,'\n',[&](auto q) {
        total += q.length() == r.length && r.fits(q,input.rules);
    });
    return total;
}

auto part2(const decltype(parse(""))& input) {
    //8: 42 8 | 42
    //11: 42 31 | 42 11 31
    // => 42^m 31^n with m > n >= 1
    auto& r42 = input.rules[42];
    auto& r31 = input.rules[31];
    assert(r42.length == r31.length);
    int total = 0;
    split(input.queries,'\n',[&](auto q) {
        if(q.length() < 3*r42.length) return;
        if(q.length() % r42.length != 0) return;
        //Has to at least fit 42 twice at the beginning and 31 once at the end
        if(not r42.fits(q.substr(0,r42.length),input.rules) || not r42.fits(q.substr(r42.length,r42.length),input.rules)) return;
        q.remove_prefix(2*r42.length);
        if(not r31.fits(q.substr(q.length()-r31.length),input.rules)) return;
        q.remove_suffix(r31.length);

        int n = 2;
        int m = 1;

        while(not q.empty() and r42.fits(q.substr(0,r42.length),input.rules)) {
            q.remove_prefix(r42.length);
            ++n;
        }
        while(not q.empty() and r31.fits(q.substr(0,r31.length),input.rules)) {
            q.remove_prefix(r31.length);
            ++m;
        }
        total += q.empty() && (n > m);
    });
    return total;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(2: 12 16 | 41 26
55: 92 16 | 84 26
107: 48 26 | 29 16
91: 16 86 | 26 120
56: 19 16 | 30 26
33: 69 16 | 127 26
65: 112 16 | 76 26
23: 16 16 | 44 26
102: 16 116 | 26 132
39: 16 26 | 26 26
40: 23 26 | 76 16
108: 16 53 | 26 51
22: 110 26 | 55 16
42: 1 16 | 47 26
14: 112 26 | 46 16
117: 115 26 | 76 16
120: 26 6 | 16 59
72: 26 130 | 16 66
131: 102 26 | 20 16
93: 16 16 | 26 16
58: 97 26 | 104 16
69: 26 88 | 16 46
54: 76 16 | 116 26
1: 26 64 | 16 28
48: 13 26 | 61 16
92: 85 26 | 117 16
49: 124 26 | 98 16
6: 44 44
24: 112 26
17: 112 16 | 116 26
115: 44 16 | 16 26
113: 16 128 | 26 89
106: 26 132 | 16 6
16: "b"
67: 44 16 | 26 26
104: 44 88
41: 26 132 | 16 76
38: 16 59 | 26 46
89: 16 24 | 26 62
80: 18 26 | 35 16
98: 46 26
101: 16 132 | 26 46
85: 16 59
126: 16 67 | 26 59
9: 26 49 | 16 80
10: 26 67 | 16 59
34: 26 93 | 16 23
4: 70 26 | 107 16
100: 123 26 | 63 16
109: 118 16 | 54 26
77: 16 50 | 26 99
88: 26 26 | 16 16
81: 67 26
18: 16 115 | 26 88
123: 57 16 | 103 26
60: 26 18 | 16 43
94: 26 23 | 16 59
0: 8 11
57: 46 16 | 125 26
110: 26 58 | 16 60
20: 44 76
15: 56 26 | 33 16
114: 26 132 | 16 23
7: 16 6 | 26 115
28: 16 3 | 26 25
51: 112 16 | 23 26
43: 88 16 | 116 26
111: 26 6 | 16 93
62: 26 132 | 16 112
76: 16 26 | 26 16
27: 96 26 | 45 16
50: 26 93 | 16 76
132: 16 26
35: 16 115 | 26 132
53: 16 46 | 26 116
75: 104 26 | 81 16
82: 26 9 | 16 100
78: 26 116 | 16 125
19: 16 39 | 26 132
37: 26 41 | 16 127
45: 91 16 | 108 26
59: 26 26 | 26 16
116: 26 16
84: 7 16 | 94 26
86: 16 6 | 26 88
63: 94 26 | 17 16
103: 115 26 | 59 16
130: 16 34 | 26 20
99: 39 26 | 6 16
26: "a"
64: 16 15 | 26 21
97: 67 16 | 23 26
83: 105 26 | 27 16
21: 75 16 | 52 26
30: 26 125 | 16 112
3: 16 121 | 26 74
105: 26 73 | 16 113
125: 16 16
13: 16 88 | 26 39
32: 72 26 | 122 16
122: 16 77 | 26 2
90: 4 26 | 32 16
12: 26 6 | 16 39
29: 68 26 | 101 16
79: 106 26 | 111 16
61: 132 16 | 46 26
31: 83 26 | 90 16
96: 16 87 | 26 5
118: 26 93 | 16 116
44: 26 | 16
25: 37 16 | 71 26
52: 10 26 | 7 16
124: 125 16 | 6 26
66: 26 36 | 16 14
127: 16 116 | 26 39
68: 26 46 | 16 6
70: 79 26 | 109 16
128: 126 16 | 40 26
8: 42
71: 26 114 | 16 78
73: 129 16 | 131 26
5: 16 65 | 26 118
11: 42 31
119: 16 112 | 26 6
95: 16 88 | 26 59
87: 119 16 | 14 26
121: 35 26 | 95 16
47: 22 26 | 82 16
46: 16 16 | 16 26
129: 12 16 | 17 26
112: 26 26
36: 67 26 | 125 16
74: 16 10 | 26 38

aaaaaababbaaababaaaabbbb
aabbabbabbbbbaaaababbbbbaaabbaabababbaaabababbab
baabaabaaabbaaaaaaabaabb
aabaaababababbbaabababaaaaaaabba
babbaabaaaabbabbabbaabba
aabbabbbaaaaaaabaabaabaa
aaabaabbaaabaabbaaaaababaabaabbabbabaaababaabaabbaabaabbaabaaaabbabababa
abbabaabbbaabbbbaabaababaabbaabbbabbbabbaabbabbaabaabbbbbaaabaababbabbba
abaaabaababaaabbbaaabbbb
baabbabbaaababbbabaabaab
abababbabbabbbaabaaaaaab
bbabbaababbbbbbbbaababba
abaaaaaaabbaaaaabbaabbba
abbbbaabbabbbaabbabbbbbbaaababbaabaaaababaaabbbabaaaaabbbababaabbbaababb
aabaaabbaaabbbaababbbaaaaabbaaababbbababbbabaabbabbaabbbbbabbbbbaaaababbabbabaaa
aaabbbababbbabababbaabba
babbbbabbbaabaabbbbaabba
ababbbbbbbbaaabababbaaaabbaabaaabbbabbab
bbabaaaaabababaababaabaaabababbb
bbabbabbabbaaaabbbaababbbaaaabaabbaaaaaa
baaaabaabbabbabbabbbbbbaabbbbbbbbbaababbbbabbaabaabababb
babbaaaabaababaabaababab
bbbababaababaabbbabaabbb
baaababbbbbbbababbabbabaaaaabbaa
baababaababbabbbabbababaabbaaabababbaaab
bbbaaabbbabbbaaaaaabaabaabbbbaabbaaabbba
aabbabaaaaaabaabababaabbbbbaabbaaababbbb
aaababbbaabaabbababaababbbaaabbabaaaabab
bbbbbbbbabbbaaaabbbbaaaa
abbbbaaaaabaababbbbaabba
abbbbaaaaabbbabbbbbaabbbaaabbabbabbababaababbaaa
bababbabaaabbaaaaabaabaababababa
aabaabbabbabaabaaaabbaaabbbbbbbabbbbbabbabbbbabb
abbaaabaaaaaaaaabaaabbbaabbbaaabbaaaaabaaaabbbbaaaabbaab
aabaaababbaaabbbaabbbaaabaabbbbabbbaaabaabbaababababaaba
aaaaaabaabbaaaababbaabba
aaaaaaaabaaabbaabaabaaabbbababaaabbabbab
bbabbabaaabbababaabababb
aabbbbabbaaabbbabaababaaaaabbbba
baaaaaaabbbababbabbaaabaaabababbaababaaaababaaabaababaaa
bbbaabbbabaaaabaaaabbbaa
baaaabaabbababaabaabaaaa
bbbbbaabbaaaaaaabbaaabaa
aabbbbabbbbaaabbabbbbaab
babaababbabbabbabbababbbaaabbbbb
babbbbabbbaabaaabbbaabaa
abbbbaaaabbaaaaabbabbbaaabbbaababaababbbaaaababbaaabbbbbaabaaaaa
baabbbbabbbbbaabbbaabbba
aabbabbbaaabaaaabbaabbbb
babaabaabaaaabbaabababbb
aabbabbbbabbaababbaaaaaa
bbaababbababbbbbbbbbaaab
bbbbbbabaaabbabbabaababbbbbbbbaa
baabbaaabbabaabbbbbababb
abbbbbbaaabaaabbaababbbb
bababbbabbabaabbbabbaabaaabbbabaababbababbababab
aababbababbbbbaabaababab
babbbaaaaaaaaaabbbabaaba
abbbaaabbaaaabaaababaabbaabaaaaaabbaabbaabaabababbabaaaa
aababaabaabbaaaabababaab
babababbaaaaaabaabababababbbabbbbbababaabaababba
bbbbbabbbbaababaabaabbbbbababababaabbaba
baabbbbabbabaabbbaababbb
abbbbbabbababbbbbbaaabbabababbabaabbaabbabaaabbaaaaabababaabbaababbbabbaaaabaaaa
aababababaaaabbbaaaabbbaaaaabbbb
ababbaabbbaaaabaaaabaaab
abbbbbbaaababaabbabbabbababbbaaabbaababa
bbaabaabbbbaabbbbbaaabbbabbababbbbbbaabb
abbababbabababbbaabaaaaa
abbbaaaaaaaaabaabababbab
bbaababbabababbaababaabbabbbbbabbbabbbbb
babbabbabaabbaabaababbbb
bbbbbaaaaabbabbabbaaaaab
aaababababbbabaababbabab
baababbbaaabbaababaaaabbabbaabbb
aababaabaabaaaabbbbabaabbbbbbbabaabbbbaa
aaaaaaaabbbbaabbbbbababaabbaabbbaaababba
bbbababbbbaabbabbbababbaabababababbaaabaababaaab
baababaaaabbbabbabbbabbababbababbabbaaab
baabbbbaaabbbababbaaaabb
bbababbbabbbbaaaabbabbab
babbabbbabbbabaabaabbbbb
aabaabbabbababaabbbbaaaa
aabbbabbbbababaabaaaaaab
aaabaaaabbbabaabaabbabaabbbabbbb
aababaabbbbbabaaaaaaabba
babaabbabbababbbbaabbbaabbbbbaababbbbabbbbabbbab
abaaabbbbbababbbbabbbbba
abbbbaaaaabbbabaaababbaa
ababbbaaabbbabbbbabbaabb
aaaaaaabaaaaabaababbbbabbabbbbababaabbaa
aaaabbabbaaababbbbbabababbabaabaaabbbabbbbbbbabbaaabbbbbbabbabab
bbbaaabbbbbbaabbaaabbbba
bbbaaababbabbabbbaabaabb
abbbbbbbaababbbbbbbbabbaabaabbaababbbaba
aaaaaabbbbbbbababbaababbbaabbbba
baaaabaaabbababaabaabaaaababbbbbbababaaabbbbbabb
bbabbbaabbbaabbbbabaaabbbabaaabaabbbbbbbaaaaabba
aaaaaabaabababbabbbbabba
ababbaababababbaaaaabbba
aabaababbabbbbabbbbaabbbabaaaaaaaabaaababaabbbbb
baabbabbbbaaabbabababbaaaaaababb
aaabbaabababbabaababaaab
abbaaaabaaaaaabbaaabbbaa
ababbababbabbabababbaaaababbabbabbbabaabbababaabbbabbbbaabbbbabbabbabbbb
baaaaaaabbababbababaabbabaaaabbaaabababaabaaaaabaabbbbaa
aabbaaaabbaaababbbaaabbaabbbababababaaaa
aaaaabbbaaabababbaabbaabbaaaaaba
bbababaabaaabaabbbbbabbb
aababaabaaaaabbbabaaabaaabababbb
bbbabababbaaaabbabaaabbabbabaaba
bababbaaabaaaaaababbaabb
aaaaaabbaaabaaaabaababab
baaabaabbabaababaabbbbabaabaabbbbbbabbbabbaabbaa
aaaaabaaaaabbabbbbbbbabb
babbabaaaaabaaaaabbbbabb
bbbbbaababbbbbbabbbaaabbabbbabbbbabbaabbbababbbb
aaabababbbabbabbaaaabaababbaabbbaabababb
bbaaababaaaaabaabaabbbbabbabbabbbabbbbbabaaabbba
aabaaabbababbbbbbbbbaaaa
aababbabbbaabaaabbabbaabaabbababbabbbbbb
bababaaabbbbbaabbbbabaabbaabaaba
aaaabaaaaaaaaababbaaaaabbbbababbbabbbaab
aabaaaabbaaabaabaababbaa
aabaababbbbabaabbaaabbba
bbbbababbbabaaabbabbaabbababaaabbbbbabba
aabbaaaaabbaaaababbbaabb
aaabaaaaaaaaaaaabaaababaabbbabbbaababaabbaabaabbbbaabbabbbbabbaa
abaaaaaaabbbabbbababbbaabaabaabb
abbaaaaaaabbaaaababbbaab
bbabaabbaaaaaabbabaabbba
bbabaabbabbbabaababbbbba
baabbbbaaaabaabaaabaabaa
bbabbbbbbbaabbbaaabbaabbbbaabbabbabbbabbbbabbbab
aaabababbbabbaababababababbbbabaabababaaaaaaabba
aabbbaaaaababbabbbaaabbabbaaaabaabbbbaaaabbaabba
ababaabbaabbbabaabaaaabbbbabbbbbbaaabbbb
baababbbabbbabbbaababaaa
bbbbbaaaabababababbabaab
bbbbbaabbbabbaabbaaaaaba
abaaabbbbabaababbabaaababbaababa
aabbbbabbabaaabbbbababaaabaaaabbbabaabaa
babbabbbababbbbbbaabbbbaabbabbab
abababababbaaaaaaaabbbba
aabbbabbbbabaaabbabbbbabaabaaaabbbbabaaaaaabbbba
aabbbabbbbaababbbbbbbaab
bbaaaababbbababaaaabbbbaabbbaaabbababbbb
abbabbaabaaaabababaabababaaabbaaababbbbaababbabbaaabbabaababbaab
aaabababaaaabaabaaaaaaabbbababbababbabbababbaaab
babaabbabaabbabbbbbaabaa
babbabbbbbaabaaabbbbbabaabbbabbabbaaaabbabbbabaabbbaaaaa
bbbababaaabaaabbabababaabaaababbbabaaaaaabbaabba
baabbabbababbbaaaaaabbaa
aabaabababbbbbaabaababba
ababbbaaababbbaaaaabbbaa
abbabbbabbaababbaababbbb
aababaababaabaaabbabaaababaabababbabbbab
aaaabaaaaabbbbabaabbbabbbbaababbbbbbaaab
bbbbababbbbbababaaaabbaa
abbbaaaabbbaaabaabbbaaab
bbabaaabbbbbabaababbaaaabbbaaabbbaaaaabaabbbabbbbbabbbbbbaaaaabbaaabbbabbbbaabababaaabaa
aaabbaabaabbababaaaaabba
bbabaaaaabbbaabaaabbabaaaabbabbbaaabaabbbbaabbab
bbaabaaaaabbbaaababaabaaabbabaab
bbbbababababaabbbaaababa
baabaaaaabbababaaababaabaabbbabaabbabababbababbaabaaaabaabbbbaaaabaabbbbbaabbbbb
bbababaaababababbababaaabbbabbbaaaaababa
bbaabaaaabbbbbbbaaaabbbb
aabaabbaababbbbbbaaababb
abbbaaaaaabbabbababaaabaabbabaabababaaaabababbbb
abbbbbaaaaaabbabbbabaaabbaaaaaaabbbabbab
bbabbabbbbabbabbaabbaabb
aabbabbababbaababbbabbba
abababbabaababaababbbbaababbabab
bbababbabbaabbbbbbbaaaaabababaabaaaaabba
abaaaabbbaabbaabbaabaabb
ababbaababbabbbaabaababa
abaaaabaaabbbabbbaabbaaaababaaaabbbabbbabbaaaaaa
aabbababbbabbbaaabaabbba
abbabbbaaaaaabbbaabbabaabbaaabbbababbabbabaaabba
aabbbabaaaabbbabbabbbbbabbabaabbabaababa
babaabbaabaabaaababbabbaababababaabababa
abbaaaaaaaabbbabbbaaabbbbaaaaaba
bbaaabbbbbaaabbabababbab
aabbababbbbaabbbaababbba
abaaabbaaabaabbbbaaaabab
babbaabaaabbbbabbaaabbaabbaaaabb
bbabbabbababbbaabbaababa
babbbababbabbbbabbbaabbbbbaaaabaabbbababaabbabbabbabbaaababbbbaaaabaaaabbbaabaabaaabbabb
bbabbabbbbbbbbabbabbbbbb
bbbaaabbabaaabaaabbababbbabababbbbabaaabaaaababb
babaaababbaababbababbababbbabaabbbbbbabbabbaaababbabaabaaababbaa
aabbbbabbaaaabbaaabaabbabaabbabbabbbaaabaaaababa
abbaaaaaabbababbbbbbbabb
baabbbaaabbaababbbbabbaa
ababbabababbbaabbbaabbbb
abababbabaabbbabbaababaaababaabbbbbabbaabbaabbababbabbababbbbabbaabbbaaaaabbaababaabbaba
aabaababababbbaabbaabbab
babaabbabbbbaabbaababbaa
baabbaabbbbbbababaabbaaaabababaabaabbbabbbbbabaa
abbabababbaabaabbbaababa
baababbbaabaaabaabaaabbbaabababa
bbabaabbbbbbbaabbabaabbb
babaabbaabaabbbaaabaabbbbabbabaababbabbb
babaababbbaababbbbaabaaaabbbbabaabbabbbbaaaabbba
ababaabaababbbbaaaaabbba
aabbbabbaabbbbabaabbabbbabbbaababaaaaaaaabbabbbb
aaabaaaaaabaabbaaabaaaaa
abbababbabbbaababbbaabba
abababbababbbbabbaaaaaab
aaaabaabaaabbbabbbaabbab
abbbbbbbbaababbbbabbabab
ababaabbbbbbbbababbbbaab
aaaaaaabaabbabababaaaaaaaaabababbabbbbaa
bbaabaaaaabbbabbbaaabbba
baabbaaababbabaabbbbaaba
bbbaabbbabbabbbabaaaaaab
aaaaababbbbabbaababbabababaabbab
aaabbbababbbabbaabaabbbb
abababaaabababababbaabba
ababbbaaabbaabaaabbabbbb
bbabaabbbaabbbbabaaabaaa
ababbaabaaaabaabbbbbbbab
baabaaabbbaabaaaaabaaaabbaabbbabbbbabbbbaabababb
abbaabbbabaababaabaabaaaabbbaaaababbaabbbabaabaa
aababbabbababaaaababbbba
bbabbabaabaaaaaaabaaaaaabbaababbaaaabbaababbbbaabbabbbbaaabbabaa
aaaaaaabbbaababbababbbaabbabbabaaababbaabbabbbbbaabababa
aaabaaaabaaabaababaabbab
baaaaaaabaabaaababaaaabaaabbaabb
bbaabaabaabaaaabbbbbaaab
abaabbbaaaabaaaaabbbababaaabaaaaaaaabaaaababbbabbbababbaaabbabbb
abaaabbbabbbbbabbabaaaaa
ababbababbbabaababbbbabb
babaababaaaaaaaaaaabbbabaaaababa
aabaaabbaaaaabbbaaabbaba
baaabbaaaabaaababaababaaaabaaabaaaabaaaaabbbaaba
aaaaabbbbabbaabaaaabbaabbbbbbabb
aabbbbababbbaaaabbaaaaab
bbbabaabaabaaaabbabaaaaa
abbbbaaaabbbaabaabaabbbb
abbaaaaababbaaaaabbbabbabaaaaaba
abbbaabaabbbaababbababab
bbbaaabbbaabaababaaabbab
aabaabababbbaaaabbbbaaba
baababbbbaababbbbabbbbabbbbababbabbbbaab
bbbbababaababaabbbbbbaaabbaaaabaabbbbaaabbbabbba
baaabbaabaabbbabaaaaabaaaabbaabb
aabaaababbbabababbbabbbb
aabbaababaabbbbabaaaaaba
aaaabbababbaababbabbbabb
ababababaaaabbababaaabab
aaabaabababbbbababbabbbbaabbaaabaabaaabbbbbababbabbabbabaaabbbba
aabaaabbaabaaabbbbbaaabbbbbabaaa
bbbaabbbbabaaababbbbbbabababbbbbaaabbabbabbabbaa
abbaaaabbaabaababbbaaaab
baaaabaaabaaaabbbbabbbab
bbabbababbbbbbbbbabaabbb
babaaabaaabbabbabbababab
bbabbabaabbbababbbbabbba
abaaaabababbbaaabbabbabaaabaaabb
aaaabbabaaaaaaabbabbbaba
abaaaaaabaabbaaababbbbabaabbabbabbbaaaaaaaabbbba
ababbabaabbbbbaaaabaabaa
abbbaababaaababbaabaaaaa
bbbabaabbbaabbbbbaaabaaa
bbabbabbbaaabbaabaaabbba
babbabaabbabbaababaabbba
abbababaababababaabbbbaa
baabbbabaaaaabbbbabbbbbb
aabbabaaaabbabbaabbbaaab
abbbbbaaaabaaaabbbbabbbb
bbbaabbbbbbbabaaababaaaa
ababbaabbbaababbbbaaababbaaabaabbbaababbbbbbbbaa
bbbabbababbbbbaaaabbbabbbbbaabbabbaaaaabbaabbaaaabbbbabaaababaaa
aababaaababbbaababaabbbaaababbbabbbbbbaa
babbabbbaaaabbabaaabaaab
aabbbaaaaaabaabaababaaab
babaaabaaabbbaaaabaabbba
aabaaaababbbabbaabaababa
bbbbbbaaabbabbaaabaabaabaaabbabaaaababbbabbbabbb
aababaababbaaaabbabbaabb
abbbaaaabaaabbaabbabababbaabaabb
aaaaaabbabbaaaaababababa
bbbbabaabbbaaabaaababbbb
babaabababbbabbabbbabaaa
bbbaaabbbaabbbbaaaabbbaa
abbabbbababbbaaabaabbabbababbabbababbbbaaaabbabababbbbabaabbaaba
abababbabbbaaababaabbaba
aabbababaaabbbbbbbaaaabbbbbbbbababbbabaabbabbbbb
abaabaaabbababbbbaaaabababaabbaaaaaabbbbbbbbaaaabbbbbbaabbaababa
aabbabbaababbbbbbbbbaaaa
aaaaaabbaabbababbbaaaaaa
bbbaaababbbbbbabaaabbaabbbbaabba
bbabbabbabbbabbabbabbbab
bbaabaaabbaabbbbabbbaabb
babbbaaaaababaabbbbbbabb
abaaabaaabaaaabaaabbbbbb
bbababaaabbbabbaabbbabbbbabbbbbaababaaba
abababaabbabaabbbbaaabbabaabbbababbaabbb
babbaabaabbbabbaabaabbbb
baabbbbabbababbabaababba
bbaaabbbaabbaaaabaaababbbabbabaabaabbbbb
aaaabbabbbbbbbbbaaaabbabbaaaabbb
bbbbabaaabbaababbbaabbab
baaaabbaabaaaaaababbabaabaaababa
aabaabbabbaaaababababbaababbabab
abaaaaaabaaababbabbaaabb
abbbabbabbbbabaabbbbaaba
babababbbbabbbaaaabbbabbaaaaaabbbabbabbbaabaaaabababaaaa
aaaabaababbbbbbaabaabbaa
ababbbaaababababababbaaa
ababbbaabababaaabbbbbabb
abbababbaabaaabbbbbaabab
babaababbababbbabbbbabbb
abbbbabbabbbaabbabaaaaabbaaabbbbbabbaaababaababa
baabbbbaaaabbabbbbbababb
aaaaabaaabbabbbababbbbaabaaabbaabbbabbba
baabaaabaabbbaaaabaaabba
aabaabbabbbbbababaababbb
aabbbabaabaaaaaaabaabbaa
abaabababaabaababbbbbbaabbabaabbaabaabbb
aabaaabbbbabbabaabaaaababbaababaaabbbaab
abaaaaaabababbbaabbbaabb
bbabaabbaaabaaaababbbabaabbaabba
bbbbabababbbabababaababb
babbaaabababbabaabbabababaaababbbbabbbbbbbabbabbbaaaabaaabbbbbba
abbbbbbbbbbbabababbabbaaabbaaaababbbbabaaabbaaaaaababbbbbbaababbbbaabaabababaabbababaabbbbbbbbba
bbbaaababaaaabbabaaababbababbaabbbbbbbba
babbabbaaaabbbabababbabb
ababaabbabbabbbabbabbbba
aaabbaabababbbaaababaaab
ababbbabaababbaaababbaaa
baaaabaaaabbaaaaabaababa
baaababbababababbbabbaaa
ababbabababaabababaabaabaaaabbbbbbaaaaababaabbaabbaaaabb
abbbbbaaaaaaabbbabbabaab
aaaaaaabbbaaababbbbaaababaababaaababbaaa
aaabbbabbabaabaaaabbbaab
abbbabbabbababaaaaabbbbb
abbbbabaaaaabbbaabbabaab
bbabaabbaabbbaaabaabbbbb
ababbaababbabbbabbabbaabbbaaaaabbbbbbbba
aabaaabababbbbaabaabaabb
aabbbaaabaabaaabbbbabbaa
babbbaaababbaabaabbbabbbababbaabababaaba
aababaabbabbabbababbbbabaabbbbabbbbbaaaaaabaabaa
aabaaaabbbaabbbabaabaaababaaabababbbabbbabbaaababbaabaababbabbbbabbabaaa
abbbabbbbabbbbaaaabbbabbaabaaabaabaababbbbbabaaa
babababbaababbababbabaaa
aaaaabbbabbbabbabbbaabba
bbaabaaaabaabbabababaaab
babbabaaaaaaaababaaababbbaabbbaaabaabaab
abbbbbbbabbaabaabbbbbbaa
aabaaaaabaaaaaabaaaaabbaaabababa
abababbabbabbabbaabaabbb
aabbbaaaabababbaabbaaaabbbababab
baababaababbabaaaabaaabaaabbbababbbbabbabbabbaaababbbbba
bbababaabaaaabbaababbababbabaaaaabbabbab
abbbbbbbabbbbbaaaaaabbba
bababbbabbbaaabaaabbbabbabaaabaaaabbabbbababbbbbababbbab
abababaaaaaaaababbbaabaa
babbabbbaabbabbbaababaaa
abbaabaababaaaabbbbbabbaaaabbabbbbbababaabbbbbabbaaaaabbbbaaaabaaabbaaab
aaababababbaaaabbaaaabbb
aaaaaabaaaaabaabaaaabbaa
bbaaabbbabbbbbbaaabbaaaaababbabbbaaabbba
bbbaabbbbbabbbaaabbbbbab
aabbaabaabbbababbaaabbbb
bbabbbabbbaabbaaabbbbbbababaaabaabbababbaabbaabbbaabaaaabaababbb
aaaabaaaabbbabaababbbbaabaaaaaba
abbbabbaabaaabbbabbababaaaaabbabbaabbaaaababbbab
baaabaabaabbaaaabbababbbabbaaaababbbbbab
bbbbbbababaaabaabbaaaaaa
baaabbaaaabaaabaaaabaaab
bbbaaabaaabbabbaabbabaab
babaabababbbbbbaaaaabbba
aaabbbabaaaaaaaabababbab
bbbbababaabbabbababbbbbb
aaaaabaaaaaaaababaaaaabb
aaaaaaaababbbbababbbabbaabababbb
abbbbbbaaababaababbbbaab
aaaaaabababaababaaaaabba
aabbaaaabababbabaabaababbbaabbbbabbbaaaaaabbabaaabaabbbb
ababbbbaabbabbbabbbbbaaababababbaababaaabaaaaaab
bbabbabbbabaaabaaabbbbaa
baaaabbaaabbbabaabaabaaabbabbababbbbbbbbaaabaaaababbbbbabaaabbabbabbbbbaaaababaa
abbababaaaabababbbabbaaa
babaabbabbbaabbbaaaabbaa
abbbbbaaaabaabbaaabbabbaabbabaaaabbabaaa
abababbababaaabaabbababaaaaabbababaaaaaaabababbaabaabbbabbaaaabb
bbbaabbbababaabbabaaaaab
aabaabaaabbabaabbbaaabaa
abababaabaabbaababbabbbb
babaabbaaaaaaaabaaaaabba
bbaabbbaababaaaaaaaababa
bbabaaaaabaaaaaaaaabaaaabbbabababaabbbababaaabba
abaaabaaabaaaabbbaaaabab
aabbabaaaaaaabbbababbbba
abababaabbbbaabbaabbaaabbbbbaaaabababababababaaaabbbbaaaaaaaabbbbabbbaaa
abbbabbabababbaaabaabbaa
aaabaaaaaabbaaaabbaabbba
aabaabbababbabbaaabbabbbababbaab
bbbbbaabaaabaaabaabbaabbbbaababa)";

int main() {
    solution(input);
}
