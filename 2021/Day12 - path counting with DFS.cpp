#include <iostream>
#include <string_view>
#include <array>
#include <map>
#include <charconv>
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

template<int N, typename F = decltype([](std::string_view a) {return a;})>
auto split_known(std::string_view in, char delim, F&& apply = {}) {
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

struct Node {
    bool revisitable = false;
    std::vector<size_t> connections{};
};

struct Graph {
    std::vector<Node> nodes;
    size_t start,end;
};

auto parse(std::string_view input) {
    std::unordered_map<std::string_view,size_t> ids;
    std::vector<Node> nodes;
    auto get_id = [&ids](std::string_view name) {
        return (ids.try_emplace(name,ids.size())).first->second;
    };
    auto get_node_id = [&](std::string_view name) {
        auto id = get_id(name);
        if(id == nodes.size()) {
            nodes.emplace_back(std::isupper(name[0]));
        }
        return id;
    };
    split(input,'\n',[&](std::string_view line) {
        auto [a,b] = split_known<2>(line,'-',get_node_id);
        nodes[a].connections.push_back(b);
        nodes[b].connections.push_back(a);
    });
    return Graph{std::move(nodes),ids["start"],ids["end"]};
}

auto part1(const decltype(parse(""))& input) {
    std::vector<bool> visited(input.nodes.size());
    int paths = 0;
    auto dfs = [&](size_t n, auto&& rec) -> void {
        if(n == input.end) {
            paths++;
            return;
        }
        if(visited[n] && !input.nodes[n].revisitable) {
            return;
        }
        visited[n] = true;
        for(auto next : input.nodes[n].connections) {
            rec(next,rec);
        }
        visited[n] = false;
    };
    dfs(input.start,dfs);
    return paths;
}

auto part2(const decltype(parse(""))& input) {
    std::vector<bool> visited(input.nodes.size());
    bool taken_second = false;
    int paths = 0;
    auto dfs = [&](size_t n, auto&& rec) -> void {
        if(n == input.end) {
            paths++;
            return;
        }
        bool second_visit = false;
        if(visited[n] && !input.nodes[n].revisitable) {
            if(n != input.start && !taken_second) {
                second_visit = taken_second = true;
            }
            else {
                return;
            }
        }
        visited[n] = true;
        for(auto next : input.nodes[n].connections) {
            rec(next,rec);
        }
        if(second_visit)
            taken_second = false;
        else
            visited[n] = false;
    };
    dfs(input.start,dfs);
    return paths;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(vp-BY
ui-oo
kk-IY
ij-vp
oo-start
SP-ij
kg-uj
ij-UH
SP-end
oo-IY
SP-kk
SP-vp
ui-ij
UH-ui
ij-IY
start-ui
IY-ui
uj-ui
kk-oo
IY-start
end-vp
uj-UH
ij-kk
UH-end
UH-kk)";

int main() {
    solution(input);
}
