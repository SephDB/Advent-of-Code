#include <openssl/md5.h>
#include <iostream>
#include <string_view>
#include <unordered_set>
#include <queue>
#include <numeric>
#include <algorithm>

struct State {
    std::string path = "";
    int loc = 0;
};

template<typename Goal, typename Next>
std::string search(State start, Goal&& g, Next&& for_each) {
    std::queue<std::pair<State,int>> next;
    next.push({start,0});
    while(not next.empty()) {
        auto [pos,len] = std::move(next.front());
        if(g(pos)) return pos.path;
        next.pop();
        //No reached check necessary bc all generated states are unique
        for_each(pos,[&next,len=len](State&& c) {
            next.push({std::move(c),len+1});
        });
    }
    return "";
}

template<typename Goal, typename Next>
int longest(State start, Goal&& g, Next&& for_each) {
    int max = 0;
    std::queue<std::pair<State,int>> next;
    next.push({start,0});
    while(not next.empty()) {
        auto [pos,len] = std::move(next.front());
        next.pop();
        if(g(pos)) {
            max = len;
            continue;
        }
        //No reached check necessary bc all generated states are unique
        for_each(pos,[&next,len=len](State&& c) {
            next.push({std::move(c),len+1});
        });
    }
    return max;
}

int main() {
    const unsigned char base[] = "rrrbmfta";
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx,base,sizeof(base)-1);

    auto for_each = [&ctx](const State& c,auto&& callback) {
        unsigned char result[MD5_DIGEST_LENGTH];
        auto ctx_copy = ctx;
        MD5_Update(&ctx_copy,(unsigned char*)c.path.data(),c.path.size());
        MD5_Final(result,&ctx_copy);
        std::array<int,2> coords = {c.loc%4,c.loc/4};

        auto next = [&](int coord, unsigned char hash_byte, char sub, char add) {
            if(coords[coord] > 0 and (hash_byte >> 4) > 0xa) {
                auto next_coords = coords;
                next_coords[coord]--;
                State n = c;
                n.path.push_back(sub);
                n.loc = next_coords[0]+next_coords[1]*4;
                callback(std::move(n));
            }
            if(coords[coord] < 3 and (hash_byte & 0xf) > 0xa) {
                auto next_coords = coords;
                next_coords[coord]++;
                State n = c;
                n.path.push_back(add);
                n.loc = next_coords[0]+next_coords[1]*4;
                callback(std::move(n));
            }
        };
        next(0,result[0],'U','D');
        next(1,result[1],'L','R');
    };

    auto is_goal = [](const State& c) {
        return c.loc == 15;
    };

    std::cout << "Part 1: " << search({},is_goal,for_each) << '\n';
    std::cout << "Part 2: " << longest({},is_goal,for_each) << '\n';
}
