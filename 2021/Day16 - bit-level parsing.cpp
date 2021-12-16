#include <iostream>
#include <string_view>
#include <array>
#include <vector>
#include <bitset>
#include <charconv>
#include <numeric>
#include <algorithm>


unsigned int hex2int(char a) {
    if(a == '\0') {
        return 0;
    }
    if(a >= '0' && a <= '9')
        return a-'0';
    return a-'A'+10;
}

std::string_view input = R"(00537390040124EB240B3EDD36B68014D4C9ECCCE7BDA54E62522A300525813003560004223BC3F834200CC108710E98031C94C8B4BFFF42398309DDD30EEE00BCE63F03499D665AE57B698F9802F800824DB0CE1CC23100323610069D8010ECD4A5CE5B326098419C319AA2FCC44C0004B79DADB1EB48CE5EB7B2F4A42D9DF0AA74E66468C0139341F005A7BBEA5CA65F3976200D4BC01091A7E155991A7E155B9B4830056C01593829CC1FCD16C5C2011A340129496A7EFB3CA4B53F7D92675A947AB8A016CD631BE15CD5A17CB3CEF236DBAC93C4F4A735385E401804AA86802D291ED19A523DA310006832F07C97F57BC4C9BBD0764EE88800A54D5FB3E60267B8ED1C26AB4AAC0009D8400854138450C4C018855056109803D11E224112004DE4DB616C493005E461BBDC8A80350000432204248EA200F4148FD06C804EE1006618419896200FC1884F0A00010A8B315A129009256009CFE61DBE48A7F30EDF24F31FCE677A9FB018F6005E500163E600508012404A72801A4040688010A00418012002D51009FAA0051801CC01959801AC00F520027A20074EC1CE6400802A9A004A67C3E5EA0D3D5FAD3801118E75C0C00A97663004F0017B9BD8CCA4E2A7030C0179C6799555005E5CEA55BC8025F8352A4B2EC92ADF244128C44014649F52BC01793499EA4CBD402697BEBD18D713D35C9344E92CB67D7DFF05A60086001610E21A4DD67EED60A8402415802400087C108DB068001088670CA0DCC2E10056B282D6009CFC719DB0CD3980026F3EEF07A29900957801AB8803310A0943200042E3646789F37E33700BE7C527EECD13266505C95A50F0C017B004272DCE573FBB9CE5B9CAE7F77097EC830401382B105C0189C1D92E9CCE7F758B91802560084D06CC7DD679BC8048AF00400010884F18209080310FE0D47C94AA00)";
size_t current = 0;

unsigned int get(size_t start, size_t end) {
    size_t byte_start = start/4;
    size_t byte_end = end/4;
    auto n = hex2int(input[byte_start]);
    if(start%4 != 0) {
        n &= (1 << (4-(start%4)))-1;
    }

    while(byte_start++ != byte_end) {
        n <<= 4;
        n |= hex2int(input[byte_start]);
    }

    n >>= 4-(end%4);
    return n;
}

unsigned int get_next(size_t size) {
    auto val = get(current,current+size);
    current += size;
    return val;
}

std::pair<unsigned int,uint64_t> parse_packet() {
    auto version = get_next(3);
    auto type = get_next(3);
    uint64_t value = 0;
    if(type == 4) {
        unsigned int continue_parsing = 0;
        do {
            continue_parsing = get_next(1);
            auto next = get_next(4);
            value = (value << 4) | next;
        } while(continue_parsing);
    }
    else {
        auto is_number_of_packets = get_next(1);
        auto count = get_next(is_number_of_packets ? 11 : 15);
        if(type < 4) {
            auto acc = [&](auto func, uint64_t initial) {
                if(is_number_of_packets) {
                    for(unsigned int i = 0; i < count; ++i) {
                        auto [v,val] = parse_packet();
                        version += v;
                        initial = func(initial,val);
                    }
                }
                else {
                    auto max = current+count;
                    while(current < max) {
                        auto [v,val] = parse_packet();
                        version += v;
                        initial = func(initial,val);
                    }
                }
                return initial;
            };
            switch(type) {
                case 0:
                    value = acc(std::plus<>{},0);
                    break;
                case 1:
                    value = acc(std::multiplies<>{},1);
                    break;
                case 2:
                    value = acc([](uint64_t a, uint64_t b) -> uint64_t {return std::min(a,b);},(uint64_t)-1);
                    break;
                case 3:
                    value = acc([](uint64_t a, uint64_t b) -> uint64_t {return std::max(a,b);},0);
                    break;
            }
        }
        else {
            auto [va,val_a] = parse_packet();
            auto [vb,val_b] = parse_packet();
            version += va+vb;
            switch(type) {
                case 5:
                    value = val_a > val_b;
                    break;
                case 6:
                    value = val_a < val_b;
                    break;
                case 7:
                    value = val_a == val_b;
                    break;
            }
        }
    }
    return {version,value};
}

int main() {
    auto [version, value] = parse_packet();
    std::cout << "Part 1: " << version << '\n';
    std::cout << "Part 2: " << value << '\n';
}
