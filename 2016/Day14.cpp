#include <openssl/md5.h>
#include <iostream>
#include <bitset>
#include <array>
#include <charconv>
#include <cstring>

unsigned char toHexChar(unsigned char in) {
    if(in < 10) return '0'+in;
    else return 'a'+(in-10);
}

void output(unsigned char (&hash)[MD5_DIGEST_LENGTH]) {
    for(auto c : hash) {
        std::cout << toHexChar(c >> 4) << toHexChar(c & 0xf);
    }
}

auto to_md5_string(unsigned char (&hash)[MD5_DIGEST_LENGTH]) {
    std::array<unsigned char,MD5_DIGEST_LENGTH*2> ret;
    int current = 0;
    for(auto c : hash) {
        ret[current++] = toHexChar(c >> 4);
        ret[current++] = toHexChar(c & 0xf);
    }
    return ret;
}

struct HashInfo {
    std::bitset<16> has_quintuple = {0};
    int first_triplet = -1;
};

HashInfo analyze(unsigned char (&hash)[MD5_DIGEST_LENGTH]) {
    HashInfo ret;
    unsigned char current = hash[0] >> 4;
    int num = 1;
    for(int i = 1; i < MD5_DIGEST_LENGTH*2; ++i) {
        unsigned char next = hash[i/2];
        if(i % 2 == 0) next >>= 4;
        else next &= 0xf;

        if(next == current) num++;
        else {
            if(ret.first_triplet == -1 and num >= 3) ret.first_triplet = current;
            if(num >= 5) ret.has_quintuple.set(current);
            current = next;
            num = 1;
        }
    }
    if(ret.first_triplet == -1 and num >=3) ret.first_triplet = current;
    if(num >= 5) ret.has_quintuple.set(current);
    return ret;
}

HashInfo getHash(MD5_CTX ctx, int num, int extra_hashes = 0) {
    unsigned char resulting_hash[MD5_DIGEST_LENGTH];
    std::array<char,16> out;
    auto [p,ec] = std::to_chars(out.begin(),out.end(),num);

    MD5_Update(&ctx,(unsigned char*)out.begin(),p-out.begin());
    MD5_Final(resulting_hash,&ctx);

    for(int i = 0; i < extra_hashes; ++i) {
        auto s = to_md5_string(resulting_hash);
        MD5(s.data(),MD5_DIGEST_LENGTH*2,resulting_hash);
    }

    return analyze(resulting_hash);
}

auto run(MD5_CTX ctx, int extra_hashes = 0) {
    int current = -1;
    int num_found = 0;

    std::array<HashInfo,1000> infos;
    std::generate(infos.begin(),infos.end(),[&,n=-1]()mutable{return getHash(ctx,++n,extra_hashes);});

    while(num_found < 64) {
        auto& info = infos[++current % infos.size()];
        auto triplet = info.first_triplet;
        info = getHash(ctx,current+1001,extra_hashes);
        if(triplet == -1) continue;
        if(std::any_of(infos.begin(),infos.end(),[triplet](auto& i){return i.has_quintuple.test(triplet);})) {
            num_found++;
        }
    }

    return current+1;
}

int main() {
    const unsigned char base[] = "jlmsuwbz";
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx,base,sizeof(base)-1);

    std::cout << "Part 1: " << run(ctx) << '\n';
    std::cout << "Part 2: " << run(ctx,2016) << '\n';
}
