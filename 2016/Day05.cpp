#include <openssl/md5.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <charconv>
#include <optional>

char toHexchar(unsigned char in) {
    if(in < 10) return '0'+in;
    else return 'a'+(in-10);
}

void output(std::array<std::optional<char>,8>& c) {
    for(auto&& p : c) {
        if(p) std::cout << *p;
        else std::cout << '_';
    }
    std::cout << '\n';
}

int main() {
    const unsigned char base[] = "ugkcyxxp";
    unsigned char resulting_hash[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx,base,sizeof(base)-1);

    int current = -1;
    std::string part1;
    std::array<std::optional<char>,8> part2;
    while(not std::all_of(part2.begin(),part2.end(),[](auto&& p){return p.has_value();})) {
        std::array<char,10> out;
        auto [p,ec] = std::to_chars(out.begin(),out.end(),++current);

        auto ctx_copy = ctx;
        MD5_Update(&ctx_copy,(unsigned char*)out.begin(),p-out.begin());
        MD5_Final(resulting_hash,&ctx_copy);

        if(resulting_hash[0] == 0 and resulting_hash[1] == 0 and resulting_hash[2] <= 0xf) {
            if(part1.size() < 8)
                part1.push_back(toHexchar(resulting_hash[2]));
            if(resulting_hash[2] < 8 and not part2[resulting_hash[2]].has_value()) {
                part2[resulting_hash[2]] = toHexchar(resulting_hash[3]>>4);
                output(part2);
            }
        }
    };
    std::cout << "Part 1: " << part1 << '\n';
    std::cout << "Part 2: ";
    output(part2);
}
