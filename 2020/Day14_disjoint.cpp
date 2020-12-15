#include <iostream>
#include <string_view>
#include <array>
#include <bitset>
#include <memory>
#include <cassert>
#include <ranges>
#include <unordered_map>
#include <bit>
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

template<int N, typename F>
auto split_known(std::string_view in, char delim, F&& apply) {
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

uint64_t to_int(std::string_view s) {
    uint64_t ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct Entry {
    uint64_t pos;
    uint64_t value;
    uint64_t mask_fixed;
    uint64_t mask_float;
};

auto parse(std::string_view input) {
    uint64_t mask_fixed, mask;
    std::vector<Entry> entries;
    split(input,'\n',[&](std::string_view line) {
        if(line.starts_with("mask")) {
            auto [m,eq,value] = split_known<3>(line,' ',[](auto a){return a;});
            mask_fixed = mask = 0;
            for(auto c : value) {
                mask_fixed = (mask_fixed << 1) | (c == '1');
                mask = (mask << 1) | (c == 'X');
            }
        } else {
            auto [mem,eq,val] = split_known<3>(line,' ',[](auto a){return a;});
            entries.emplace_back(to_int(mem.substr(4)),to_int(val),mask_fixed,mask);
        }
    });
    return entries;
}

auto part1(decltype(parse("")) input) {
    std::ranges::reverse(input);
    std::ranges::stable_sort(input,{},&Entry::pos);
    auto r = std::ranges::unique(input,{},&Entry::pos);
    input.erase(r.begin(),r.end());

    return std::transform_reduce(input.begin(),input.end(),uint64_t{0},std::plus<>{},[](auto e){return (e.value & e.mask_float) | e.mask_fixed;});
}

struct AddressPattern {
    uint64_t fixed, floating;
    AddressPattern(Entry e) : fixed((e.pos | e.mask_fixed) & ~e.mask_float), floating(e.mask_float) {}
    AddressPattern(uint64_t fix, uint64_t fl) : fixed(fix), floating(fl) {}
    bool contains(AddressPattern other) const {
        return (fixed & other.fixed) == fixed && (other.fixed & (fixed | floating)) == other.fixed;
    }
    bool operator==(AddressPattern other) const {
        return fixed == other.fixed && floating == other.floating;
    }
};

bool overlaps(AddressPattern a, AddressPattern b) {
    return ((a.fixed | a.floating | b.floating) ^ (b.fixed | b.floating | a.floating)) == 0;
}

AddressPattern intersect(AddressPattern a, AddressPattern b) {
    assert(overlaps(a,b));
    return {a.fixed | b.fixed, a.floating & b.floating};
}

struct DisjointPartition {
    struct Node {
        AddressPattern pattern;
        bool finalized = false;
        std::unique_ptr<std::pair<Node,Node>> children = nullptr;
    };
    Node root = {AddressPattern{0,(uint64_t{1} << 36)-1}};
    uint64_t total = 0;

    void insert(Entry e) {
        auto insert_rec = [this, value=e.value](auto&& rec, Node& n, AddressPattern pattern) -> void {
            assert(n.pattern.contains(pattern));

            if(n.finalized) return;

            if(n.pattern == pattern && not n.children) {
                total += value * (1 << std::popcount(pattern.floating));
                n.finalized = true;
                return;
            }

            if(n.children) {
                auto& [left,right] = *n.children;
                if(overlaps(left.pattern,pattern)) {
                    rec(rec,left,intersect(left.pattern,pattern));
                }
                if(overlaps(right.pattern,pattern)) {
                    rec(rec,right,intersect(right.pattern,pattern));
                }
                if(left.finalized && right.finalized) { //All patterns within this space have been used up, reclaim memory
                    n.finalized = true;
                    n.children = nullptr;
                }
            } else {
                //no children yet, but pattern is not ours, so it must have a fixed bit where we don't
                auto diff_bit = std::countr_zero(n.pattern.floating ^ pattern.floating);
                auto mask = (uint64_t{1} << diff_bit);
                
                auto new_floating = n.pattern.floating ^ mask; //remove floating bit

                Node left{AddressPattern{n.pattern.fixed | (pattern.fixed & mask), new_floating}};
                
                rec(rec,left,pattern);

                Node right{AddressPattern{left.pattern.fixed ^ mask,new_floating}}; //right node has flipped pattern
                n.children = std::make_unique<std::pair<Node,Node>>(std::move(left),std::move(right));

            }
        };
        insert_rec(insert_rec,root,e);
    }
};

auto part2(decltype(parse("")) input) {
    DisjointPartition dp;
    for(auto e : std::ranges::reverse_view(input)) {
        dp.insert(e);
    }
    return dp.total;
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';

    AddressPattern a{0b0000,0b0111};
    AddressPattern b{0b0100,0b1001};
    std::cout << a << ' ' << b << '\n';
    std::cout << intersect(a,b) << '\n';

}

std::string_view input = R"(mask = 1X000X0101XX101101X01X101X1000111X00
mem[10004] = 3787163
mem[18866] = 665403
mem[13466] = 175657346
mem[21836] = 99681152
mask = 110X1001110110X10100010X0000X010X11X
mem[14957] = 51287
mem[47879] = 1706397
mem[60036] = 139731
mask = 0100X011110X11111100000010X1XXX1X100
mem[13312] = 473519
mem[45712] = 11576
mem[65077] = 499111
mem[20582] = 970
mem[57647] = 30626762
mem[5265] = 14655010
mask = 11X010010X010011X110X00X1XX0001101X1
mem[45904] = 324397
mem[33792] = 56494291
mem[35775] = 597
mem[58036] = 267391350
mem[10448] = 77300
mem[49376] = 12095094
mask = XX000001100001100X0X10000X010X111110
mem[17484] = 382723
mem[58181] = 58
mem[51203] = 253
mem[36984] = 552006
mask = 110XXX010X011X100X00110X101010110000
mem[46734] = 272208
mem[50532] = 1071505732
mem[42057] = 18023379
mem[40723] = 203396667
mem[60376] = 24255
mask = 1110X001010010110X00XX0110X1X00000X0
mem[6373] = 509708
mem[12417] = 1586323
mem[35427] = 457
mem[14175] = 2420631
mem[28822] = 749
mem[52248] = 12324270
mask = X1X000111X0100100X00XX10X00011X1X000
mem[26803] = 500540
mem[7065] = 8843071
mem[55118] = 40141309
mem[2804] = 3136584
mask = 110X1X0001XX100101001110XX0010X100X0
mem[19256] = 7428
mem[13879] = 1353915
mem[43372] = 5882
mem[6447] = 4535
mask = 100X11X01001XX01110X001001000X010X10
mem[55173] = 21377
mem[12745] = 125083
mem[311] = 602879
mem[39306] = 2711255
mask = 1X10X001100010110100101100110X0X0111
mem[65528] = 672476373
mem[40933] = 15862
mem[20739] = 128727771
mem[30419] = 29600646
mem[26253] = 3206153
mask = 1XX00X011101X01101X0X001110X1XX10100
mem[59482] = 66620
mem[41885] = 399931609
mem[22336] = 62900
mem[8106] = 1106171
mem[5796] = 37170431
mem[17413] = 15524
mem[15447] = 28761
mask = 11000001X10X00110XX1100000011011X100
mem[43397] = 16642
mem[24122] = 173985292
mem[10901] = 6397292
mem[7385] = 7495847
mask = X1000001X00010110100X0010000111X0100
mem[38664] = 43195
mem[17708] = 56326435
mem[47879] = 3654030
mem[26738] = 6346961
mem[22210] = 118801717
mem[56703] = 13588355
mem[65252] = 262312
mask = 1X0XXX000100X0X101001100001X101X1X00
mem[62976] = 82374293
mem[64638] = 14434
mem[43964] = 13761
mask = 01000001X00110110X00XX00X10010X10X11
mem[4724] = 2903
mem[5158] = 164937
mem[35328] = 30334
mem[46966] = 2759991
mem[52761] = 25675
mask = X1101000XX1X00X10100001011X01101XXX1
mem[4392] = 118555033
mem[23892] = 4622396
mem[65088] = 1887562
mem[13476] = 226944
mask = 11X0X001X1X1001101101X0X110X000X01X0
mem[311] = 290524064
mem[54164] = 1109593
mask = 100X100001000X0101000X010XX11011110X
mem[62632] = 2370
mem[52805] = 3829934
mem[40012] = 9175340
mem[37847] = 117719078
mask = X110000X010000X10100001X1011010X0X00
mem[37404] = 3551660
mem[15104] = 527114796
mem[49376] = 968
mem[56095] = 316752
mem[6747] = 1807
mask = 11000X010101101X01000X001X011XXX10X0
mem[41840] = 2775
mem[60125] = 182425209
mask = 10011000010XX011X100101010XX1X1X0100
mem[6747] = 4044
mem[16920] = 114852
mem[4128] = 41260
mask = X10000X11001X01X010000X000XX10X0X111
mem[14175] = 16258730
mem[41484] = 2757
mem[37593] = 33406295
mem[8317] = 415217155
mem[51008] = 2636
mem[21446] = 259080582
mem[15515] = 2734
mask = 11X0X001X10X100X01000110101X101X00X0
mem[61143] = 129
mem[23860] = 247
mem[47879] = 22784315
mask = 01000011110X111XX1000XX0110100X10X0X
mem[2435] = 1973
mem[23908] = 133875113
mem[12460] = 103067
mem[6211] = 1359103
mem[7534] = 23782
mask = 111000010100X0X10100001010X1XX000XXX
mem[53221] = 4273517
mem[13255] = 3334
mem[31239] = 113778111
mem[32900] = 6938492
mask = 1110XX010110X1XX011011000111100X1101
mem[9359] = 8377654
mem[52805] = 3549344
mask = 0X1000X01100X1XXX00X0011001010000101
mem[28303] = 227915
mem[257] = 1876777
mem[63555] = 192083072
mem[56391] = 147544840
mem[63072] = 6915
mask = XX0000111X011111010X0001100000010110
mem[10311] = 16038309
mem[41484] = 30818
mem[54410] = 3229
mem[2435] = 10793289
mem[26631] = 87736025
mem[13639] = 7253
mask = 0100101XX1X01111X1XXX0001100010101X0
mem[2028] = 6656580
mem[29799] = 2772
mem[64020] = 958010
mem[5974] = 569
mem[8667] = 2023725
mem[16595] = 25313923
mem[35402] = 558
mask = 1X0X00001X01100X1X0000010100001111X1
mem[41459] = 12342565
mem[23672] = 36167
mem[10300] = 65046557
mem[56503] = 62989647
mem[37993] = 217162
mask = 1111101100000X01X1011X111111X1100X10
mem[21085] = 15851
mem[25194] = 6075854
mem[42984] = 435548
mem[8346] = 978945
mem[5661] = 4981
mask = 0X0000011X0X0010010X1XXXX01010001X01
mem[27985] = 234997
mem[14463] = 45760
mem[15023] = 17580
mask = 110000110X01X00101X01X1X10001101X000
mem[63245] = 6018482
mem[30940] = 257367431
mem[10617] = 1134348
mem[57343] = 284899785
mem[21373] = 1004097
mask = 1100100XX10X00111X101001X10100101101
mem[54836] = 45353796
mem[43493] = 1827049
mem[790] = 48270178
mem[42104] = 67747
mem[33977] = 222196
mem[56552] = 21306885
mem[63222] = 64849326
mask = XXX00101011110110100010010111000X01X
mem[33756] = 4616
mem[44216] = 6220360
mem[25526] = 811292762
mem[59209] = 6509
mem[13255] = 2140854
mask = 1110000110X0X11X0100000110100X010110
mem[7764] = 3574
mem[872] = 16596
mem[5555] = 763
mem[59969] = 25416012
mask = 0X000001100X0010010X1X1110X0X0101010
mem[64638] = 2350
mem[36949] = 3170
mem[41641] = 2041278
mask = 11101001X1X01X0X01X01000101110000101
mem[63222] = 998162428
mem[36105] = 58286575
mem[36644] = 6605534
mem[64290] = 352783
mem[52428] = 72931
mem[1450] = 460492
mask = 00X0000111X10XX00101111XX00001XX001X
mem[47375] = 1428142
mem[12574] = 262115
mem[1393] = 215199811
mem[2729] = 53399
mem[1417] = 163417802
mem[13888] = 30705
mem[52121] = 629932866
mask = 1X00010X01X01X01010X10111X1100101000
mem[63515] = 22411
mem[23091] = 909249
mem[21738] = 438843570
mem[44851] = 181
mem[10095] = 4014
mask = 010X0X11100X1X100100000101000111011X
mem[19759] = 29570
mem[2970] = 191037
mem[9604] = 4079
mem[657] = 465
mask = 1X000X01X1X10011010X1XX11100X0111100
mem[43949] = 478212819
mem[5597] = 63092
mem[58361] = 5457568
mem[41459] = 187363
mask = XX1X0011X100011X00101X11000XX101100X
mem[13888] = 638337993
mem[37934] = 4985
mem[55575] = 76330
mem[10257] = 3056
mem[9744] = 4917501
mem[32361] = 63127
mask = 11X00011X001001X010001000010011X0X0X
mem[43761] = 6764398
mem[24646] = 30592
mem[36456] = 799
mem[42057] = 1771
mem[35165] = 2871
mask = 110000010X01X00101X01XX01X1001X00100
mem[30294] = 30942244
mem[34398] = 1382102
mem[63222] = 260411654
mem[39566] = 133753617
mem[4818] = 4462832
mem[36866] = 52
mem[14603] = 244236
mask = 1100010XX101101X01XX111X0001X1111110
mem[33984] = 490757
mem[29144] = 42516
mem[12171] = 11800
mem[35282] = 19533
mem[30399] = 14796
mem[19650] = 158686001
mem[6447] = 177
mask = X100001X1001X011X10001000110111001X0
mem[62632] = 98281297
mem[57359] = 3527162
mem[12082] = 171252979
mask = 11000011110100XX0X000001X11000100000
mem[35211] = 130598799
mem[28482] = 85586
mem[55217] = 240324
mem[6740] = 92413135
mem[1291] = 79746542
mask = 1000010XX101101101X000101X0010010110
mem[45328] = 3595262
mem[3490] = 877
mem[36826] = 11235
mem[110] = 1713
mem[7313] = 13261628
mem[17524] = 263660940
mem[40227] = 14071730
mask = 1101110X00011X10000001011110X10X0100
mem[26215] = 3075603
mem[2028] = 2167
mem[36320] = 586
mem[8270] = 71526
mem[44216] = 5252007
mask = 1110100100010011111X00X0X000X0000111
mem[50149] = 58998983
mem[47879] = 1944464
mem[19922] = 3875
mem[42517] = 43875909
mem[18735] = 7318682
mem[26678] = 17877212
mask = 0100001101X1X10101001011011X0X000100
mem[56402] = 14747004
mem[29737] = 15131100
mem[15515] = 2942003
mask = 110000X10XX110X10110111110XX01000110
mem[54032] = 100009547
mem[15057] = 5910646
mem[62982] = 135495807
mem[41840] = 44565248
mem[43139] = 14110
mem[37411] = 434104731
mask = 1100000XX10110X101000XX11010XX100100
mem[29795] = 954605601
mem[13364] = 402011
mem[22468] = 778482653
mem[46818] = 25125
mem[5527] = 101
mem[9814] = 306208600
mem[43397] = 239110
mask = X1X00011X10100X10100111010X110X00X00
mem[43405] = 4234874
mem[49758] = 5465414
mem[20027] = 12031953
mem[52428] = 2873
mem[23261] = 117091570
mem[54960] = 1624
mem[1039] = 22719
mask = X1101001011X110001X011110101011X0011
mem[14463] = 92010258
mem[61143] = 11340
mem[1450] = 15947
mask = X10000X1X1010011X10000X01011X0X1010X
mem[45367] = 1060810121
mem[26527] = 23928
mem[15407] = 30081582
mem[8768] = 7626
mask = 100X1X001001000111000011X001XX0X1X1X
mem[57779] = 1222
mem[60458] = 507523
mem[9438] = 2655
mem[11306] = 32130962
mem[13120] = 212870
mem[18699] = 32470
mask = 11X0XXX0110100X101000100100000100111
mem[50532] = 6760
mem[65458] = 1921851
mem[50231] = 1055316713
mem[11611] = 82996
mem[32408] = 1879545
mem[56334] = 6421
mem[19546] = 1572
mask = 0101101101011001X0001X111X1100010X00
mem[11808] = 39158
mem[13364] = 3686
mem[58181] = 3954
mask = 11X000110X0100110100X1XXX010011001X0
mem[18996] = 10869875
mem[9359] = 11905624
mem[10037] = 675
mem[63666] = 1335625
mask = 10011000100X000111000XX000010000X110
mem[30987] = 168211
mem[2059] = 19013
mem[36287] = 21065
mem[55613] = 24671374
mask = 01110X11000X0XX101100111111101X11010
mem[27985] = 1661941
mem[33107] = 921718
mem[23617] = 16368
mem[41120] = 74
mask = 11000001100X1X11010000X001X0X1X101X0
mem[39739] = 63291061
mem[52804] = 15173
mem[14981] = 648
mem[65088] = 1644
mem[47873] = 43267217
mask = 11X000XX100X11110XX01X001X0001010000
mem[39631] = 19215077
mem[17524] = 513536515
mem[49990] = 109517
mask = X000X000100X0001110000X1010011010001
mem[13466] = 124984033
mem[12574] = 260829
mem[1403] = 115889305
mask = 0110XX1X11010X10000X00XX01X01011001X
mem[37593] = 173042
mem[49988] = 200423
mem[52823] = 7747994
mem[2391] = 611579
mask = X01XX00X11X100100X011111010011000010
mem[50986] = 277302
mem[48505] = 2276386
mem[28286] = 12419937
mem[35832] = 7585
mem[44379] = 340032079
mask = X1X0X011000111110X000001111000100100
mem[26228] = 1347384
mem[46316] = 35047
mem[50294] = 7049
mem[31571] = 5511
mem[6747] = 2881667
mem[55522] = 239714
mask = X11XX0100101X110001001100010X1X01X00
mem[63523] = 3173
mem[15717] = 1059
mem[1039] = 1756
mem[652] = 74372
mem[52561] = 1253065
mem[34744] = 12
mem[36103] = 67545913
mask = X11X001X0X0X00110100010X0101X101X100
mem[43915] = 2312924
mem[625] = 52614
mem[16137] = 3337
mem[33395] = 56449350
mem[57004] = 9401213
mem[22475] = 273016261
mask = 11X000X1100X1X110100X10000XX0X000111
mem[17020] = 461488870
mem[64339] = 76903
mem[12489] = 1867
mem[40021] = 416088
mem[45893] = 941080
mask = 010XX0110X0X1XX10X000111101000X101X0
mem[1347] = 27142438
mem[56404] = 28056
mem[15515] = 13463506
mem[34610] = 408788
mem[23768] = 4390
mask = 0X0X000X10000X100X000001X0X00X11111X
mem[34140] = 2486
mem[45229] = 219957
mask = 1X010X0X11X1100X01000001010100110000
mem[63207] = 214605819
mem[3120] = 975024
mask = X100000100011XXX00000000110010X0X010
mem[36039] = 1351
mem[24280] = 1529
mem[54240] = 438022
mem[49990] = 7079087
mem[21713] = 14792683
mem[52177] = 1961
mem[7345] = 406699254
mask = 1X00010X01X010110X001110100X00101100
mem[27536] = 12551
mem[7678] = 10576
mem[17936] = 1475
mem[9814] = 99131792
mem[41885] = 96156
mem[13120] = 35225
mask = X111X011000X00X101XX01X10111X1X11010
mem[60152] = 82433112
mem[22477] = 4090530
mem[16768] = 58352433
mem[7764] = 192408366
mem[51535] = 69751
mem[850] = 131
mem[48467] = 377832323
mask = 0100001110010010X10X1X111101001100X1
mem[23672] = 1931
mem[6708] = 469
mem[16775] = 2368
mem[12502] = 115896934
mask = 100X0000X00XX0011100X01X010111X11011
mem[24733] = 936
mem[269] = 362232
mem[60475] = 322199998
mem[14404] = 1261
mem[42303] = 27351706
mem[13699] = 34315
mask = 0111101100X10X11010X010X0101X00X10X0
mem[11893] = 62548
mem[57647] = 237349
mem[21200] = 14649
mem[27128] = 80169
mem[32969] = 725133
mem[31032] = 131322
mask = 0100XX01100X1010010000000X01X0110110
mem[18930] = 68333625
mem[6819] = 341
mem[26253] = 220435
mem[15909] = 79688
mem[21059] = 2671693
mask = X11XX00X11XX001101X00XX0110000100111
mem[34832] = 248
mem[11532] = 708205
mem[654] = 7796300
mem[46941] = 94124
mem[49206] = 109
mem[31987] = 1877
mask = 11000X01X1001X0X01X011X00000X0000101
mem[60475] = 67279
mem[14929] = 3073
mem[38463] = 6762244
mem[47778] = 173399092
mem[62317] = 57461236
mask = 110XX000110X1001X100000001000X110000
mem[15407] = 249
mem[7202] = 55714
mem[48800] = 25528109
mem[20959] = 4106091
mem[6496] = 1976005
mem[6708] = 3013607
mask = 110000011XX1X011X1001101010XXX010X10
mem[64787] = 469608465
mem[41416] = 59853
mem[10680] = 16299
mem[19394] = 56413
mem[39572] = 25343
mem[19686] = 277550
mask = 01X0001X1100001X00101110111X1001X000
mem[4228] = 590408603
mem[62269] = 394
mem[60694] = 483056
mem[25301] = 664548494
mask = 110X00001101100101000101X1X1X00100X1
mem[29136] = 3854559
mem[24118] = 486135
mem[34398] = 430609446
mem[12782] = 3721785
mask = 110001001101X0X10100110000X0010100X0
mem[5661] = 13272
mem[2012] = 38954
mem[60732] = 7014
mask = 0100X0X1X00100100100100111111001100X
mem[49716] = 284158681
mem[12224] = 10628127
mem[32900] = 7796903
mask = 0110001XX10X0X1000X0011X00X0110X00X0
mem[8844] = 26023958
mem[65175] = 81386046
mem[54410] = 4119058
mask = X10000X1XX01X01X0100000X1X0001010100
mem[62976] = 29914819
mem[7792] = 49702987
mem[61389] = 124692445
mem[10004] = 29696422
mem[61943] = 43748477
mem[61040] = 199512596
mask = 0X00XX11110X10X1010101010X0011X00X1X
mem[10880] = 91763
mem[93] = 46698
mask = X100XX111X01001101000X00101000000000
mem[30458] = 4474263
mem[2932] = 25975
mem[25986] = 106060
mem[20974] = 20857
mem[23672] = 853256
mask = 11000011010XX01101X000XX10111001X11X
mem[39572] = 24316000
mem[14981] = 41633461
mem[41885] = 3807449
mem[49990] = 31780591
mem[2414] = 5032671
mask = 111010XXX110X1010110100001111010X101
mem[26803] = 474
mem[15407] = 33070532
mem[35211] = 24880155
mem[52428] = 396521
mem[9814] = 252376513
mask = 01000011XX011011X10XX000X11010000010
mem[20974] = 101
mem[13535] = 632
mem[12489] = 58930152
mem[61506] = 681
mem[18309] = 8028011
mem[33984] = 7404765
mask = 1100X01X01X1101101100011100X01000111
mem[15864] = 413
mem[15023] = 7825
mem[60154] = 451000
mem[53862] = 240968
mem[63761] = 5500
mem[8136] = 5727
mask = 01000X1XX101001111X00000101X110X0X00
mem[884] = 1258
mem[12502] = 150248279
mem[63] = 1403008)";

int main() {
    solution(input);
}
