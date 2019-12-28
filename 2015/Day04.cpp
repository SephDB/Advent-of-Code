#include <openssl/md5.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <charconv>

int main() {
    const unsigned char base[] = "yzbqklnj";
    unsigned char resulting_hash[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx,base,sizeof(base)-1);

    bool part1 = false;
    int current = 0;
    while(true) {
        std::array<char,10> out;
        auto [p,ec] = std::to_chars(out.begin(),out.end(),++current);

        auto ctx_copy = ctx;
        MD5_Update(&ctx_copy,(unsigned char*)out.begin(),p-out.begin());
        MD5_Final(resulting_hash,&ctx_copy);

        if(resulting_hash[0] == 0 and resulting_hash[1] == 0 and resulting_hash[2] == 0) {
            std::cout << "Part 2: " << current << '\n';
            break;
        } else if(resulting_hash[0] == 0 and resulting_hash[1] == 0 and resulting_hash[2] <= 0xf and not part1) {
            std::cout << "Part 1: " << current << '\n';
            part1 = true;
        }
    };
}
