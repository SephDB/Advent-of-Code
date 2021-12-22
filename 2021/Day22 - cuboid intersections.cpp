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
auto split_known(std::string_view in, std::string_view delim, F&& apply = {}) {
    std::array<decltype(apply(in)),N> ret;
    std::size_t pos = 0;
    std::size_t current = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret[current++] = apply(in.substr(0,pos));
        in.remove_prefix(pos+delim.size());
    }
    ret[current++] = apply(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

struct Vec3 {
    std::array<int,3> coords;
    auto operator<=>(const Vec3&) const = default;

    constexpr Vec3 operator+(const Vec3& other) const {
        return apply(other,std::plus{});
    }

    constexpr Vec3 operator-(const Vec3& other) const {
        return apply(other,std::minus{});
    }

    constexpr Vec3 apply(const Vec3& other, auto f) const {
        return std::apply([&](auto... as) {
            return std::apply([&](auto... bs) {
                return Vec3{{f(as,bs)...}};
            },other.coords);
        },coords);
    }

    constexpr bool all(auto pred) const {
        return std::apply([&](auto... as) {
            return (pred(as) && ...);
        },coords);
    }

    constexpr bool comp(const Vec3& other, auto pred) const {
        return std::apply([&](auto... as) {
            return std::apply([&](auto... bs) {
                return (pred(as,bs) && ...);
            },other.coords);
        },coords);
    }

    constexpr int manhattan() const {
        return std::apply([](auto... as) {return ((as < 0 ? -as : as) + ...);},coords);
    }

    constexpr int size_sq() const {
        return std::apply([](auto... as) {return ((as*as)+...);},coords);
    }

    friend std::ostream& operator<<(std::ostream& o, const Vec3& v) {
        o << v.coords[0] << ',' << v.coords[1] << ',' << v.coords[2];
        return o;
    }
};

struct Block {
    Vec3 min,max; //max is exclusive
    bool contains(const Vec3& point) const {
        return point.comp(min,std::greater_equal{}) && point.comp(max,std::less{});
    }

    bool intersects(const Block& other) const {
        return min.comp(other.max,std::less{}) && other.min.comp(max,std::less{});
    }

    Block intersection(const Block& other) const {
        return {min.apply(other.min,[](int a, int b){return std::max({a,b});}),
                max.apply(other.max,[](int a, int b){return std::min({a,b});})};
    }

    int64_t volume() const {
        Vec3 size = max-min;
        return static_cast<int64_t>(size.coords[0])*size.coords[1]*size.coords[2];
    }

    void remove(const Block& other, auto&& foreach_new) const {
        auto current = *this;
        for(int coord = 0; coord < 3; ++coord) {
            if(current.min.coords[coord] < other.min.coords[coord]) {
                auto next = current;
                next.max.coords[coord] = current.min.coords[coord] = other.min.coords[coord];
                foreach_new(next);
            }
            if(current.max.coords[coord] > other.max.coords[coord]) {
                auto next = current;
                next.min.coords[coord] = current.max.coords[coord] = other.max.coords[coord];
                foreach_new(next);
            }
        }
    }
};

struct Instruction {
    Block area;
    bool on;
};

auto parse(std::string_view input) {
    std::vector<Instruction> instructions;
    split(input,'\n',[&](std::string_view line) {
        auto [type,coord] = split_known<2>(line," ");
        bool on = type == "on";
        auto [x,y,z] = split_known<3>(coord,",",[](std::string_view c) {
            c.remove_prefix(2);
            return split_known<2>(c,"..",to_int);
        });
        Block a = {Vec3{x[0],y[0],z[0]},Vec3{x[1],y[1],z[1]}+Vec3{1,1,1}};
        instructions.emplace_back(a,on);
    });
    std::ranges::reverse(instructions);
    return instructions;
}

void solution(std::string_view input) {
    auto in = parse(input);

    std::vector<Block> seen_so_far;
    std::vector<Block> added_this_round;
    auto step = [&added_this_round](Block b, auto&& to_check, auto&& rec) -> void {
        auto first_intersect = std::ranges::find_if(to_check,[&](Block n){return n.intersects(b);});
        if(first_intersect == to_check.end()) {
            added_this_round.push_back(b);
        }
        else {
            b.remove(*first_intersect,[&](Block next) {
                rec(next,std::ranges::subrange(first_intersect+1,to_check.end()),rec);
            });
        }
    };
    int64_t part1 = 0;
    Block part1_block = {Vec3{-50,-50,-50},Vec3{51,51,51}};
    int64_t part2 = 0;
    for(auto& instr : in) {
        step(instr.area,seen_so_far,step);
        seen_so_far.insert(seen_so_far.end(),added_this_round.begin(),added_this_round.end());
        if(instr.on) {
            part2 += std::transform_reduce(added_this_round.begin(),added_this_round.end(),int64_t{0},std::plus{},[](auto b){return b.volume();});
            std::erase_if(added_this_round,[&](auto b){return !b.intersects(part1_block);});
            part1 += std::transform_reduce(added_this_round.begin(),added_this_round.end(),int64_t{0},std::plus{},[&](auto b){return b.intersection(part1_block).volume();});
        }
        added_this_round.clear();
    }
    std::cout << "Part 1: " << part1 << '\n';
    std::cout << "Part 2: " << part2 << '\n';
}

std::string_view input = R"(on x=-4..48,y=-30..24,z=-39..15
on x=-19..33,y=-15..29,z=-4..42
on x=-1..49,y=-14..38,z=-27..20
on x=-28..16,y=-16..36,z=-21..27
on x=-23..27,y=-26..20,z=-23..22
on x=-8..43,y=-10..43,z=-21..30
on x=-45..4,y=-6..47,z=-15..33
on x=-22..22,y=-6..45,z=-29..19
on x=-30..18,y=-47..-3,z=-24..22
on x=-6..43,y=-37..8,z=-19..27
off x=18..28,y=-15..4,z=32..49
on x=-2..45,y=-17..35,z=-15..34
off x=-49..-33,y=-49..-31,z=-10..5
on x=-31..21,y=-12..34,z=-48..6
off x=-38..-27,y=-44..-25,z=-17..-4
on x=-21..32,y=-9..45,z=-18..33
off x=-14..3,y=-10..-1,z=-40..-29
on x=-30..16,y=-13..34,z=-4..45
off x=24..34,y=13..32,z=-38..-25
on x=-27..26,y=-14..38,z=-24..22
on x=35544..55136,y=51736..62748,z=-58537..-33425
on x=33690..50869,y=-78828..-40948,z=-44269..-23196
on x=65407..77846,y=-1962..21102,z=30745..35336
on x=43820..63036,y=45669..68304,z=-27802..-19946
on x=-80968..-79217,y=-15189..550,z=-20396..-2177
on x=22543..55131,y=-50184..-36252,z=46098..64106
on x=-12595..7924,y=-24867..-13344,z=-96867..-68102
on x=9874..31255,y=65277..89043,z=-936..16277
on x=-13411..5153,y=-10968..4147,z=-87107..-78849
on x=-8897..8799,y=57380..81655,z=9642..37776
on x=67474..84855,y=-6483..10922,z=41290..57415
on x=-8012..1755,y=34193..64326,z=48900..73023
on x=54961..78882,y=-7659..5406,z=36768..70357
on x=4538..32860,y=-47844..-36498,z=51393..73621
on x=16397..28684,y=16381..47358,z=-87090..-57131
on x=38468..57198,y=-14413..-5639,z=-67748..-57437
on x=-60663..-47089,y=-34032..-26482,z=49838..59849
on x=-35900..-24050,y=-62434..-43634,z=35478..45195
on x=39580..65361,y=49921..69519,z=19635..40186
on x=72891..74867,y=-42056..-22591,z=-6916..20783
on x=1951..24816,y=-74448..-64168,z=-36633..-31963
on x=24833..49754,y=20298..43211,z=51546..73514
on x=-3167..30380,y=35722..38122,z=-76109..-57000
on x=-82903..-79067,y=-8395..20541,z=-5265..4392
on x=3218..14953,y=-13800..-1624,z=-89481..-62873
on x=-53283..-38277,y=1641..4903,z=-70901..-45748
on x=-32141..-25640,y=-34847..-13896,z=69407..86876
on x=37990..61788,y=44601..58722,z=-49195..-29888
on x=46963..82000,y=-20841..-6866,z=27132..57574
on x=53439..75061,y=42180..56045,z=-29192..-7810
on x=56225..83464,y=-29936..-24016,z=9181..40276
on x=-25381..6615,y=-36495..-470,z=-92049..-72629
on x=32658..54160,y=47754..70225,z=-6772..7057
on x=1541..22570,y=64100..80080,z=-46051..-26336
on x=290..25368,y=-75499..-38271,z=-71991..-44993
on x=-90436..-74173,y=-16161..9329,z=-6785..12872
on x=-77341..-60209,y=12324..44621,z=-14282..15269
on x=43442..74799,y=38197..48084,z=9898..46142
on x=-8617..-3246,y=-18762..3821,z=-87843..-72818
on x=30362..51314,y=-8722..8473,z=54067..78087
on x=-27208..-9871,y=10214..23370,z=70814..83559
on x=-34980..-14547,y=-78208..-70283,z=-27640..-3899
on x=-42751..-6005,y=44998..73226,z=34016..57989
on x=-82302..-64007,y=13489..29455,z=-38227..-20899
on x=-11988..12700,y=-28404..-6516,z=-90902..-70602
on x=33172..47497,y=-22050..5845,z=63959..73452
on x=4520..20681,y=53723..81778,z=42603..64776
on x=9456..39703,y=48507..61312,z=-51529..-40905
on x=-40098..-6364,y=30241..51715,z=-80424..-53739
on x=12558..41048,y=-75142..-67808,z=-28847..-5450
on x=-45305..-37476,y=59373..65800,z=30613..52009
on x=-51862..-28691,y=-80526..-49298,z=7585..33680
on x=33988..56509,y=-76294..-52403,z=-24060..8376
on x=66701..87231,y=-14532..2225,z=-2979..12349
on x=6390..28035,y=24319..41006,z=66533..72374
on x=-92154..-59055,y=17719..22642,z=6955..28468
on x=33232..43716,y=-9213..10287,z=-76824..-69144
on x=-15670..8102,y=55811..91367,z=-36831..-16337
on x=-11992..-4723,y=51539..87697,z=23232..47761
on x=-5295..18741,y=-72590..-48553,z=-73622..-40811
on x=18186..41716,y=67026..78411,z=-34911..-5988
on x=-68762..-64823,y=-45789..-18913,z=9605..45048
on x=-12471..-8333,y=-49562..-33936,z=-68222..-58934
on x=-1314..15331,y=-30976..-16222,z=67572..84940
on x=-67163..-61062,y=-53863..-18643,z=-47613..-15657
on x=71390..87446,y=-28061..-15108,z=-17260..5398
on x=43837..73348,y=47081..51019,z=-27859..-13016
on x=38886..48555,y=44108..70423,z=28181..43111
on x=-77134..-58601,y=-13936..11454,z=22269..39015
on x=16000..20829,y=-96924..-65406,z=-17350..6468
on x=-63622..-40677,y=39467..48832,z=28335..33734
on x=-83957..-71425,y=18893..29929,z=-11533..-2293
on x=-92470..-69285,y=14707..19654,z=18369..26120
on x=13641..38318,y=-32184..-19122,z=-82832..-59293
on x=38368..44793,y=7451..23119,z=-79164..-55642
on x=-34078..-24088,y=37612..70266,z=-57847..-30148
on x=45511..61076,y=25218..47528,z=21820..50200
on x=-9251..14390,y=-26731..9955,z=-84053..-62781
on x=7166..29475,y=65191..78750,z=-56267..-31353
on x=-1215..16834,y=-98600..-71219,z=-235..21112
on x=66193..69134,y=28687..42487,z=-17341..-7323
on x=-32865..-5842,y=14478..43666,z=-87864..-55075
on x=40182..48489,y=57444..85093,z=-21528..-5631
on x=-37152..-5019,y=30492..43671,z=56217..78887
on x=46382..55909,y=-67994..-58067,z=-16846..-3160
on x=32700..56924,y=22430..44279,z=41830..63068
on x=26774..34296,y=-80268..-60786,z=13217..29206
on x=-5802..1019,y=34264..65124,z=50883..74910
on x=-42392..-13729,y=13094..41810,z=-79316..-51263
on x=-16021..473,y=57439..86998,z=-46753..-30286
on x=-62595..-45603,y=4608..9583,z=57915..82579
on x=-4473..1973,y=57840..73813,z=-68167..-33772
on x=-83547..-62073,y=-33126..-18871,z=-17468..2918
on x=-46545..-39844,y=-70331..-60310,z=13062..37682
on x=-31519..469,y=-711..7423,z=-94431..-62298
on x=60002..89047,y=-29111..-9661,z=12744..28690
on x=-1450..20279,y=-13723..5802,z=-98979..-77404
on x=-7347..23250,y=-90904..-68475,z=-38218..-31606
on x=-15622..-8177,y=62746..92916,z=-31184..3126
on x=-53804..-52634,y=45539..66268,z=-46218..-7476
on x=-65959..-57397,y=36117..52643,z=19481..45747
on x=43928..54629,y=8065..16553,z=-72837..-47865
on x=-77510..-62388,y=-54448..-20445,z=-16968..15103
on x=28751..32027,y=-47236..-17317,z=55145..71895
on x=-63166..-38470,y=-77573..-47295,z=-20443..6131
on x=28272..58068,y=53583..65317,z=5882..27642
on x=-87017..-47718,y=-29823..-6062,z=24910..39735
on x=9462..38606,y=66092..87363,z=14791..34975
on x=60870..83615,y=16444..35987,z=-1375..18757
on x=-61017..-51891,y=38733..57918,z=-55845..-32294
on x=17842..45753,y=58058..78326,z=16889..44761
on x=66304..81980,y=-51921..-35554,z=-1171..8798
on x=-33872..-17168,y=37853..39657,z=48783..81069
on x=17025..42143,y=-32845..-10455,z=65973..83280
on x=35256..55278,y=-72080..-40057,z=7549..32601
on x=30028..53691,y=-15660..2864,z=55672..84796
on x=-93477..-72259,y=13273..37746,z=-13865..-732
on x=8114..19004,y=61128..80348,z=40437..54937
on x=35042..46361,y=23302..44970,z=57849..81964
on x=15670..27956,y=-50201..-32885,z=43089..78261
on x=-22149..-4701,y=70832..92978,z=-18375..8955
on x=-11307..5395,y=64032..80176,z=16681..41609
on x=-43425..-28061,y=67202..74457,z=-36752..-9283
on x=-47157..-23853,y=22746..46164,z=59388..69758
on x=-66470..-41360,y=24611..42210,z=-58386..-38174
on x=24871..39254,y=52512..83471,z=-2514..17277
on x=9854..12006,y=-42027..-22328,z=-78647..-71822
on x=-63822..-45142,y=-74770..-40518,z=-12693..-1226
on x=7514..29519,y=-70540..-40815,z=50673..66670
on x=-86355..-64456,y=1739..18125,z=21907..45251
on x=23938..36472,y=-35351..-19431,z=56359..87714
on x=-61952..-33569,y=51907..81871,z=-38456..-18781
on x=54670..64202,y=19047..45756,z=-35785..-24071
on x=-62952..-32348,y=-23703..-756,z=64776..79497
on x=42131..72686,y=43348..69631,z=-12082..9850
on x=19070..30412,y=980..20591,z=-90809..-62142
on x=-9351..-4058,y=46899..70855,z=-57535..-32949
on x=58882..72704,y=21502..45491,z=26413..44449
on x=27826..59456,y=-80504..-41682,z=17382..44358
on x=-48248..-41584,y=23606..41769,z=54404..66807
on x=803..3373,y=-78943..-70599,z=14956..29213
on x=-77364..-60633,y=-36875..-17085,z=-1174..21376
on x=-71159..-63592,y=-30044..-11779,z=-61398..-36500
on x=-69639..-51727,y=-25463..-11440,z=35873..47448
on x=-14747..1355,y=-90524..-63391,z=-33627..-15447
on x=-14250..14802,y=-51215..-28495,z=61720..73235
on x=-34423..-6304,y=-44713..-24670,z=65483..74818
on x=62322..90758,y=13367..39938,z=7438..27708
on x=46367..81400,y=11677..35279,z=-47304..-24771
on x=4379..23035,y=61932..84911,z=-63724..-43631
on x=-89042..-68392,y=-5068..16120,z=-14807..4087
on x=63034..91408,y=-10916..23888,z=12401..43459
on x=-82262..-59907,y=3979..40319,z=-1175..25220
on x=16006..42587,y=-72561..-58071,z=-33042..-9704
on x=68016..92195,y=8804..14233,z=-33255..-1477
on x=-50921..-37413,y=45745..81563,z=1283..20895
on x=11428..31965,y=47684..73555,z=26199..46769
on x=-22972..1981,y=16390..37660,z=62583..75045
on x=33829..56047,y=53258..57241,z=-44084..-20261
on x=-84051..-68366,y=29094..43279,z=-17225..10812
on x=50085..75114,y=6497..31618,z=-37737..-17385
on x=17484..32905,y=-48591..-23834,z=-75180..-54610
on x=51877..82949,y=-18174..-5750,z=-53652..-42825
on x=32775..58264,y=-11346..9920,z=57936..66561
on x=-52169..-28507,y=-90305..-53670,z=-19641..13206
on x=-71218..-41412,y=-59994..-48983,z=15647..33905
on x=26805..52467,y=62380..84830,z=-4520..10963
on x=-4087..2092,y=71217..97333,z=8093..33139
on x=-80230..-57401,y=-31567..3147,z=39782..58307
on x=-80572..-67522,y=31920..50297,z=-7054..10350
on x=-71271..-49989,y=-37730..-28383,z=37368..55962
on x=-75528..-56735,y=37170..52802,z=-34426..-19488
on x=64763..85408,y=-7784..5034,z=-41782..-15632
on x=-61687..-55301,y=54758..61276,z=-15846..9067
on x=30543..63245,y=-18542..-4101,z=-79361..-45457
on x=-14932..5661,y=54028..79525,z=-61556..-32216
on x=24493..31197,y=-34578..1825,z=63560..86978
on x=-61036..-47222,y=14404..38815,z=29338..54873
on x=-38192..-8201,y=-81948..-57915,z=-17188..10726
on x=1990..15491,y=56137..78488,z=-42010..-28371
on x=5049..33267,y=-79591..-57008,z=-42567..-22184
on x=24693..34274,y=24506..45113,z=-79046..-65370
on x=68172..98565,y=-14312..8628,z=-9721..13285
on x=-18235..6826,y=-25122..10050,z=60096..85636
on x=45928..69951,y=-60287..-37529,z=-20540..7359
on x=-32041..-5059,y=51092..63261,z=-66204..-31843
on x=63653..71469,y=-34873..-31161,z=14048..39702
on x=22396..43880,y=-3824..14660,z=-90531..-56170
on x=-1717..21789,y=53858..67026,z=-58144..-47816
on x=-51019..-40959,y=-75814..-60550,z=11755..46123
on x=-57489..-39503,y=-64465..-33404,z=-48668..-30131
on x=27760..59108,y=42790..67256,z=17743..35790
on x=-43692..-29811,y=-88277..-59083,z=5968..37461
on x=-47420..-22185,y=-68408..-52385,z=23869..44928
on x=39709..61505,y=-21268..1882,z=-76594..-60038
on x=54698..76952,y=-17074..10219,z=24567..44175
on x=-95465..-62390,y=-29895..-23300,z=-11863..4898
on x=23365..40278,y=58938..80416,z=-21545..9812
on x=11923..35757,y=-66990..-41080,z=-61382..-33846
on x=67045..75089,y=-11897..4681,z=-47955..-24267
on x=-90376..-59893,y=117..26316,z=28188..35096
off x=24651..28210,y=-24590..-10312,z=66417..75292
off x=-43951..-18830,y=-69398..-52261,z=28497..50225
off x=42514..62323,y=-60372..-24865,z=37347..49320
on x=49798..77900,y=-72682..-49927,z=-10258..958
off x=24604..45145,y=36058..57891,z=-53557..-34761
on x=-65518..-39972,y=34159..54763,z=-17646..9176
off x=-76836..-49612,y=27033..40308,z=24773..37790
off x=41029..63639,y=34627..54572,z=-60195..-35060
off x=5440..40626,y=31825..56887,z=58565..74579
on x=-85410..-68908,y=-16763..12913,z=1884..24605
on x=-70712..-56718,y=-48053..-38417,z=-37942..-32604
off x=-66574..-45489,y=34467..41894,z=-46802..-24009
off x=-30928..-7167,y=43518..70824,z=33329..64950
off x=-52088..-17091,y=-28441..-10616,z=56304..83190
off x=73277..76646,y=-5922..10725,z=23075..35243
off x=-44861..-12403,y=-18397..9805,z=-90975..-64778
on x=-19159..-8086,y=-84649..-57626,z=-4777..30377
on x=-48235..-45399,y=-76542..-41786,z=-27853..-11708
off x=-41696..-17405,y=-81352..-58226,z=-51727..-15321
on x=-21612..8433,y=-88881..-57104,z=20197..38496
off x=64866..82781,y=-35287..-19222,z=2225..31593
off x=-60086..-31957,y=-66807..-52584,z=-47785..-31167
on x=-75232..-65410,y=6879..15928,z=25821..33108
on x=49721..74332,y=-32211..-5492,z=37490..67240
on x=-69008..-59258,y=-67365..-39177,z=2702..22494
on x=40598..77320,y=-44858..-24495,z=-39106..-18658
on x=-66046..-43012,y=9311..31541,z=51758..55618
on x=-74902..-54476,y=9991..38601,z=-18468..-2400
off x=934..27205,y=-52862..-47494,z=-78005..-46221
off x=36318..42994,y=63750..79912,z=-11834..6870
off x=-9448..6260,y=69407..96104,z=3742..26473
on x=34180..59194,y=56640..70053,z=-6782..2219
off x=-71868..-58289,y=-50419..-43052,z=-16738..3185
off x=-55443..-21609,y=-43083..-21999,z=-75650..-51578
off x=-3124..2680,y=26555..50324,z=-84693..-65786
off x=65726..80418,y=-21970..-4777,z=28280..55831
off x=-12755..2382,y=21228..27803,z=-80546..-62078
on x=-70824..-46210,y=-46992..-35115,z=-6484..18899
off x=19667..33161,y=59955..85149,z=19245..33893
off x=5654..11498,y=56328..89517,z=-51989..-33158
on x=-20574..-5577,y=39609..41373,z=66022..82822
on x=-16635..-2094,y=61755..97252,z=-10151..3379
off x=-61710..-29756,y=-53529..-20524,z=50061..67887
off x=12337..45140,y=60042..89297,z=9701..36968
off x=-42381..-17258,y=-40434..-12651,z=-79807..-52351
off x=-57645..-52169,y=-20403..4163,z=-62673..-54906
on x=26686..40465,y=7785..22540,z=72481..83980
on x=-23030..-18498,y=-31032..-9833,z=61054..83029
on x=-10867..9838,y=45397..66784,z=-51464..-39505
off x=56847..77259,y=-17184..2949,z=-55341..-38773
on x=15524..40419,y=-42588..-27508,z=61847..79655
on x=-91271..-71952,y=15556..34300,z=14005..28676
on x=65780..81595,y=-29919..-17998,z=28053..44908
on x=31479..58997,y=55159..82009,z=-20523..11865
on x=2149..19762,y=-86184..-56666,z=18206..36344
off x=27069..56637,y=-73675..-53032,z=-13508..15432
on x=67416..85446,y=2917..28648,z=5402..35606
on x=-44500..-23236,y=-91223..-65053,z=-3525..14263
on x=-75739..-51755,y=49292..67356,z=-23308..10605
on x=-34841..-9806,y=-19491..-1709,z=64409..96498
on x=9551..24535,y=65245..79434,z=-24616..-12986
on x=-83205..-55438,y=-39820..-19718,z=22074..38142
off x=-71953..-53073,y=-44959..-30448,z=34060..50956
off x=-48764..-35469,y=-61736..-43279,z=41195..52261
on x=15917..37063,y=-75662..-56956,z=18898..36595
on x=37396..54561,y=-29290..-16200,z=64044..76199
off x=-14179..10592,y=-80049..-51222,z=38386..68137
on x=-29902..-21453,y=-94461..-71699,z=-6448..6244
off x=66863..74181,y=9066..35219,z=-28995..-8750
on x=7191..37647,y=-74506..-52303,z=45114..65447
off x=53433..79524,y=21402..48649,z=5035..16962
off x=54924..71855,y=28434..48319,z=-27603..-14643
off x=44568..73341,y=41377..69060,z=2132..18323
off x=32192..46654,y=-86893..-59876,z=21008..38923
off x=-26751..-3856,y=21442..39353,z=-84098..-58106
off x=63303..65533,y=42998..54340,z=3302..15908
on x=4235..13687,y=-50180..-34351,z=-76521..-58485
on x=-33290..-9251,y=40469..57296,z=-66106..-48947
off x=-81172..-62578,y=6482..36668,z=-45865..-22894
off x=-81904..-65892,y=-11941..4053,z=-42478..-22147
off x=35829..70381,y=41448..69602,z=18749..38582
on x=-10433..-2099,y=-82602..-51316,z=36823..64092
on x=38080..53913,y=36937..47688,z=39322..71479
on x=-21560..-4521,y=66563..85082,z=20252..45582
off x=-85857..-68417,y=-41847..-28573,z=-7729..2847
on x=-68327..-31793,y=-21900..-289,z=56789..69398
on x=-67562..-42688,y=16743..32262,z=22000..44570
off x=48052..67704,y=-23423..4103,z=44261..78217
on x=-22576..-194,y=61895..82600,z=-40620..-5831
on x=-16931..7379,y=-15102..1748,z=-87988..-74150
on x=-26047..12491,y=55107..74563,z=21205..33348
off x=-25435..-946,y=-82352..-52608,z=33364..53843
off x=-5833..18798,y=39831..56611,z=-72213..-47388
off x=-5822..14115,y=-98135..-64976,z=-20248..9788
on x=-15154..670,y=-75227..-54455,z=33215..60140
off x=18909..48180,y=62835..69076,z=27175..44519
off x=-58838..-51811,y=-8942..14922,z=-72422..-48233
on x=47362..82935,y=-58723..-33218,z=-14356..7150
on x=-64609..-42063,y=-58790..-35608,z=-50387..-39455
on x=9832..20482,y=40076..58832,z=41495..59344
on x=-29810..-7444,y=-73368..-47276,z=-64805..-37263
on x=-66021..-35774,y=44614..81545,z=-2880..20810
on x=30417..44347,y=64937..66634,z=27916..47000
on x=14925..33804,y=-59990..-48743,z=-55335..-46081
off x=-45330..-16788,y=-2912..25446,z=-78240..-63474
off x=-75837..-58168,y=-7970..3635,z=30476..60250
off x=-12011..13258,y=-89694..-69437,z=23137..41026
on x=58141..91626,y=-3240..13201,z=23976..30297
off x=-69440..-42294,y=-3341..11023,z=-75797..-55367
off x=28775..48112,y=49087..66505,z=-47634..-19480
off x=17405..40878,y=-78515..-63319,z=20589..35237
on x=46505..79525,y=46254..52587,z=-16956..-979
off x=14778..49904,y=51599..61793,z=-68550..-48309
off x=-17127..-2237,y=-80624..-47506,z=-43373..-23646
on x=42852..55498,y=60931..70121,z=-913..10924
on x=53866..67942,y=10584..28056,z=44574..51590
on x=52164..61481,y=25073..45284,z=20579..54644
off x=-38817..-3710,y=49963..73191,z=35577..68161
off x=74571..87380,y=-26549..-8365,z=-41849..-15478
on x=-11635..-3471,y=-2190..2324,z=-84076..-75086
on x=-87133..-75239,y=-9635..11698,z=-1887..26028
off x=-65700..-58148,y=-27843..-17147,z=46837..51188
on x=-8973..19499,y=-57314..-31198,z=-82041..-63223
on x=-8234..7290,y=72201..85190,z=-41415..-12525
off x=-69515..-63879,y=-11878..12202,z=-61049..-31131
on x=-593..25155,y=71211..88034,z=-13360..13827
off x=-41118..-22761,y=-51023..-43558,z=42131..56647
on x=45695..64697,y=-70454..-47486,z=4653..27783
off x=-69391..-49194,y=-6224..21792,z=-56102..-46702
on x=12567..16989,y=63359..83739,z=-7433..18488
on x=-47607..-28894,y=-76890..-58285,z=16882..41914
on x=-30015..4988,y=14737..22104,z=62453..85467
on x=-54404..-44808,y=-50918..-32444,z=-49452..-43588
off x=12954..44089,y=-28041..-5913,z=67605..87219
on x=-82272..-53812,y=25783..42439,z=-42516..-12409
off x=-94480..-60573,y=-25004..-2471,z=-2718..11057
off x=51649..65225,y=-14724..6310,z=38450..66205
off x=69856..80286,y=21304..46173,z=-11935..15518
on x=-77660..-69829,y=-25072..-3423,z=25500..50923
off x=37047..59725,y=51835..79489,z=272..35097
on x=-67713..-31489,y=35835..38541,z=-57246..-47586
on x=-18556..-7707,y=-17678..-8114,z=-87728..-59085
off x=-78610..-61520,y=11250..38454,z=-8429..3203
off x=-44797..-21166,y=-14536..9531,z=59712..82290
off x=45719..59834,y=-18760..-4920,z=-52363..-46018
off x=-35717..-12687,y=-83577..-62558,z=-8962..21313
on x=44536..59941,y=29580..64623,z=-24759..-4916
off x=-18864..-3108,y=48226..78222,z=42732..67354
on x=12357..39639,y=-44392..-26900,z=-82899..-54317
off x=-58310..-24651,y=15307..42877,z=-77149..-40022
on x=-43945..-20285,y=70712..82950,z=-4121..3442
off x=7101..22967,y=64872..83707,z=-25824..-9526
on x=-62076..-43190,y=-21347..-3360,z=-63874..-36231
off x=6340..27337,y=-73151..-47585,z=46269..52581
off x=47397..57486,y=-63732..-40590,z=44295..49537
off x=-78613..-57237,y=-56534..-32459,z=-35285..-13648
off x=-31072..-7685,y=-38542..-7098,z=57760..83347
on x=-63099..-35927,y=-77355..-53793,z=29264..33129
on x=-3649..25126,y=4399..19136,z=-90001..-77851
off x=8510..11246,y=57072..89213,z=11594..31605
off x=-16380..3546,y=28043..58308,z=62410..70708
off x=-73518..-51371,y=8803..36830,z=27148..50003
off x=-76901..-55804,y=-20544..-14756,z=-34886..-16018
on x=42245..58590,y=-60103..-37286,z=-38862..-20729
off x=2872..20494,y=76096..89070,z=-30072..-13378
off x=62225..70240,y=18398..51759,z=-26426..-10316
on x=-8268..25701,y=-44287..-22096,z=-73605..-62816
on x=12693..28817,y=-90968..-58755,z=-38493..-10212
on x=-72781..-51700,y=-59730..-40159,z=-22471..-10811
off x=-86186..-61431,y=-40796..-16811,z=-20606..1945
on x=19756..40482,y=41370..75797,z=22120..46137
on x=-38689..-13316,y=-24252..-16211,z=66862..80344
on x=25358..46927,y=-81264..-64854,z=29512..35413
on x=-16755..18260,y=-75084..-53865,z=48791..62218
on x=37799..45278,y=38376..53913,z=31553..51525
on x=-78288..-65705,y=-20882..5515,z=-56620..-32490
off x=-14779..-3865,y=-35491..-12433,z=60969..77610
on x=1041..7426,y=-75565..-54588,z=36056..52921
on x=-41864..-34775,y=22219..51820,z=40842..71780
off x=-23755..2275,y=51063..64402,z=-67945..-44438
on x=-93770..-74301,y=-30533..2469,z=-4612..7196
on x=-59566..-38442,y=-68259..-32849,z=36360..51994
on x=43187..57027,y=-18906..-8453,z=-77163..-57489
on x=-29794..-6913,y=28460..55404,z=-79855..-58015
off x=-57236..-52204,y=-21814..-7565,z=38317..74160
off x=56146..75194,y=-39180..-18839,z=-56810..-31930
on x=-66300..-58204,y=-19301..-4228,z=-61589..-39254
off x=-73278..-68405,y=22614..43899,z=-16199..-10518
on x=57074..79996,y=-6578..17919,z=33160..54010
off x=-40719..-2100,y=65789..80371,z=34406..44030
on x=-44080..-27053,y=2572..27247,z=51202..87551
on x=-25151..-7880,y=-60771..-53496,z=-55535..-47740
off x=-14214..11970,y=-4404..16997,z=-90900..-64733
off x=16310..46047,y=55884..79489,z=-14464..20211
on x=51594..75047,y=26992..37723,z=-11584..-911
on x=-45309..-20784,y=44096..70601,z=-52394..-33571
off x=16870..35569,y=13219..31860,z=69493..87370
on x=-22983..-16966,y=59778..70013,z=26803..46876
on x=33076..50573,y=-80935..-61509,z=-30866..-2196)";

int main() {
    solution(input);
}
