#include <iostream>
#include <string_view>
#include <array>
#include <bitset>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <utility>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

template<int N, typename F = decltype([](std::string_view a){return a;})>
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

using Segment = std::bitset<7>;

using Line = std::pair<std::array<Segment,10>,std::array<Segment,4>>;

auto parse(std::string_view input) {
    std::vector<Line> lines;

    auto to_segment = [](std::string_view s) -> Segment {
        Segment segment;
        for(auto a : s) {
            segment.set(a-'a');
        }
        return segment;
    };

    split(input,'\n',[&](std::string_view line) {
        auto [segments,output] = split_known<2>(line," | ");
        lines.emplace_back(split_known<10>(segments," ",to_segment),split_known<4>(output," ",to_segment));
    });
    return lines;
}

auto part1(decltype(parse("")) input) {
    return std::transform_reduce(input.begin(),input.end(),0,std::plus<>{},[](auto& line) {
        auto& [segments,output] = line;
        return std::ranges::count_if(output,[](int i) {return i == 2 || i == 4 || i == 3 || i == 7;},[](Segment s) {return s.count();});
    });
}

int deduce(Line l) {
    std::array<Segment,10> numbers{};
    auto [input,output] = l;
    auto find_segment = [&](auto f) {
        return std::exchange(*std::ranges::find_if(input,f),Segment{});
    };
    numbers[1] = find_segment([](auto s) {return s.count() == 2;});
    numbers[4] = find_segment([](auto s) {return s.count() == 4;});
    numbers[7] = find_segment([](auto s) {return s.count() == 3;});
    numbers[8] = find_segment([](auto s) {return s.all();});
    Segment a = numbers[7] ^ numbers[1];
    Segment eg = ~(numbers[1] | numbers[4] | numbers[7]);
    numbers[0] = find_segment([&](auto s) {return s.count() == 6 && (s & (eg | numbers[1])).count() == 4;});
    numbers[6] = find_segment([&](auto s) {return s.count() == 6 && (s & eg).count() == 2;});
    Segment c = numbers[6] ^ numbers[8];
    Segment f = numbers[1] ^ c;
    numbers[2] = find_segment([&](auto s) {return s.count() == 5 && (s & (a | c | eg)).count() == 4;});
    Segment d = numbers[2] ^ (a | c | eg);
    numbers[9] = find_segment([](auto s) {return s.count() == 6;});

    numbers[3] = find_segment([&](auto s) {return (s & c) == c;});
    numbers[5] = find_segment([](auto s) {return s.count() == 5;});

    return std::accumulate(output.begin(),output.end(),0,[&](int acc, Segment s) {
        return acc*10 + (std::ranges::find(numbers,s)-numbers.begin());
    });
}

auto part2(decltype(parse("")) input) {
    return std::transform_reduce(input.begin(),input.end(),0,std::plus<>{},deduce);
}

void solution(std::string_view input) {
    auto in = parse(input);
    std::cout << "Part 1: " << part1(in) << '\n';
    std::cout << "Part 2: " << part2(in) << '\n';
}

std::string_view input = R"(ceb bgfdea febgc ec eadcgfb eagbcd fcdebg dcef gafbc egdbf | fdbgec fedbg gdabefc gefbd
af cegdabf cfdge ecdbfg dcfga edafgc cfa cabedf gdbac afge | cgdab bcagd badecgf fa
bgacfe cbedgf degcabf dfgcba gdcfb dfeca bgda cadbf bca ba | cab fbdeagc dcfbg cfgdb
dg fadgceb dacbef agfeb gcdbef edcbf gdf ecgd cgbadf defbg | bedcf bgdfac cbfedg abfeg
fedga gebfc cade gacef afc agdbfc degbcaf gdacfe gdefab ac | dacbgf gacfe faegcd decfag
gecbd fbcdg ebgdcf dcfbag dec ebdgfac cebga de efdb acdefg | afbgdc edbf de ed
cdegfb bgdafec gabfec cef afbe eagfc gecab agcdf gbadce fe | efc egacbf gecbfd cfage
dac aedb fadcge gfebac cfeba abecfd bdafc dbcgf edgabfc ad | acgefd afbce bcfeag cad
agbcfd acedf adcgeb fegb egdcfab agbec eagfc gaf fg bfeagc | acefbdg fag afbcgd fga
ceagdb eabg cfadge dgfceb dgb aedcg gb gfeadbc bcdag dcfba | cfedag bg ebcagd ebcagd
gacdbf cdfbg cb gfacd gbfde dagefc febcdag gbc eacbgd bcaf | egbdf cefadg adgcfb cbgdfa
deacbg bfdgc efcbda fcgbed afbdg gcef cebgadf begdc cfb fc | cgfe gfec cfb bfc
fe egbda fbaeg egf gbdcfae efbd agbdfe bgafc gdbcae gefcda | feagb gdbae debf acfdeg
dfbace abd dagfbec abfcg cadegf begd degca bd dcabg gcabed | gbadc fgeacd gcabfed cfdeba
bg fgb fcadgb cfdeabg ebcdgf gfdce gcfbe fcabe dgeacf dgbe | bg fcbeg cfgebd fcbaegd
gefc cfgbade fg eagfd aecgd efdcag ebfad geacbd bagcdf fga | gf gefadc dgface bfaed
cfabg dfcgea fgc cdbf dcabg fc cfdabg cbdgae fdcgeba gefba | dgaecf gfc acbdg fgc
ad adc adcbge afbcd bcfae gdfcb cbdgfae fcedab cafbeg defa | adc bgfcd fabdc dcfbea
fcaegdb agb efcbag bdcg bacfde afbcgd acfdb bgafd gafed bg | cbedgfa ebdfgca gfead cdbg
gb egb gecfbd dgbc cbfge efbgadc efdcb ebfdac gdeafb fgcea | edfgcab bdcg gebfcd dcefb
dca febdc bfaegc cgfdae agefcdb ebcdag ad adgf eagcf feadc | egcaf bgaefcd fcgade cdebf
cegfd ecdb egafc fde bfdceg bgcfd dgeafcb cbadfg edfgab de | aegfc bcagdf eafbgd fbcdg
gfdb gdafeb dceabg bd becfa dbfea degaf cfgeda gbeacdf abd | agdefb ceadgb fgaecd dfbage
ba afcegb degac gaebc gedfbca cafb fgecdb bae feadgb gfbce | fbac bafdge ebdcgf aecgd
cbefda gcbf efg bdfce dbefg bcfegad cefdga gbdea fbgedc gf | bfdcge ecadbf cdefb gf
acbfd adefgcb adefbc dag bgfda gdcf egbaf dg acbdeg abcdfg | decbga cdafgb cedgafb dag
fedabg cfgabe cedf adceb eca beadcf agbcefd afdeb adgcb ec | cegafb gecdabf ebcad dbgcfea
cadefg dgb bd adbgef fdbcgea gfcdba fagde gbcef dabe degfb | agcdfe baed bgecf db
gcdf efdcga afecg cgaefb bfagedc efgda def df cfaebd baegd | cdfaeg gfaecb cdaebfg fecag
edfcg fgcea faed dge gdbaec bfgaec de fecgda dgecbfa bcdfg | defa gafdec acefg ged
cgabe cedaf faecdb gcefda cfb bf fbcae gdeafbc gdfcab debf | gfdaec efbd efgcbda fb
cda bgacedf bafdc bfdaec ecbd bfcae cfgdae fcgbea bfgda cd | adc becfa fagdbce egabdcf
dageb agefd cdbga eb fgecabd baegcf bge dgbcae dcabgf cbde | edbga ebdc cabfegd bgecaf
abfgce fbdagce egfc gc agfedb bdfca bagcf bcaged cgb faebg | gfcba ebcfagd dbefcga bcg
agfec gdeacb fbgdca bfgacde faeb gaecb af edfcg egfcab afg | faeb gaf agdecb afg
cfd cgdae egafdbc daefc debcga gacf cf fgdbec aefdb efagcd | dacebg gfac daceg gcdbae
fbead ae cead fea gbafdec cgebfa bfecd eafcdb ecdgfb dgfab | ebacdf fea facebdg fbedca
cbgfa gdc gd ceadb adfcgb cbaegf fagd ebgfdc acdgb aebfcdg | gcd dg bdagc badgcf
cdbg gc bdfcea cfbedga ebfcg fgc agbef cbfged dcafge ecbfd | gbfedac edfcb cbdfea gefba
badecfg egfabd fcdeb eadbf geafcd aedgcb abgf bdaeg afd fa | af beafd adf agdbe
fgbedc aedbf bdfacge decbg acd ac ecag gcfbda bgadec dbeac | ecag ecga gdaecb ecdab
bfagc fcgea fce acefbd fgedacb ce aegdf eafcbg cgbe acbfdg | cafbg fce fgade fgeda
ebadc bcgad efdbag dbgcfa fcgb dgcfea dgacf bg gba bgaecfd | edagfb gb dcefabg gb
dcf cdbgea gdfec dcafeg eacgd cf fbadegc gdfeb efdcab fcga | dacge acfgde cedgf egbdf
afdcb febdag adfbe fedg dcgabe befdgac dbe de eabcgf baefg | bgdfeac dcbfaeg fcbad ed
ecbd ebg agcebf be cfdbag gcbad eabgd ceabgdf gafde gbcdae | cbde bdcag gdaecb dbgca
badgfce eabfgd cd dcfbea cda dceg fgcba gadbec adgbe adgbc | gdec bcagd dfabce bdacge
adf afceb bdafe dgfbae ad dcfage bgad gfebdc fdbagce edbgf | bgaedf acfeb da becgfd
bdg dgbcaf fdgeab bgdef bd fecbgad ecbafg dfegc gafeb abde | bfage baed bcfeag afdgecb
bcdeag bagec cadfgb cgdab bge aegbfd eb bcde gbcadfe fecga | eb fdbcag bfdecag debagc
bfegcd bgce fbdcga bc cfdeg fbc dbefc fcgade aefbgcd dbefa | dfgbca edgfac fcgadb gbafcd
bfcgaed abfgc dfbgac ecfgd acbgde db feagcb dcb dfgbc adfb | bgfdc fcebga dabf fgacbe
cebfg fgcbad fca ca faceg fadceg dbfaeg cade bcedagf dagef | cbfeg ac cebfg aegcf
ega ebdgaf cadef acdefb agbcfed cgfa ecgda afedgc ag gcdeb | eadfbc dfeabg ega fgedab
cdbeag fa adbec egbfda fabcd fda eadcfb aecf fbcgd gbadefc | af bacdgfe fa fda
fegba cgafeb fcgdbae acfg bfeac gbcead edbgf afbdce agb ga | gaecdb gdfeb ebcfag gaebf
cfega adceb bgc gdba bgadce fdaecbg cdebaf gb efcbdg cebga | gaefc edcbaf fcage eafdbgc
adcb edgfc aedbgf daecf dbecaf dfa acfeb cgadefb aecgbf ad | dabc cfeab bdacfge caefbg
bgdfeca dbacg gadefc bcegd cfbadg ca agdbf bcaf dca fadbge | eagfdb gfabd begcd ac
bfdea bg aegcdb dgabf bag bcgf dbfcga gcdefa fagdc agbdcef | bfgc defacg efabd cdafg
cgdfae dea geabcd cebad geba edcfabg ae adbcfg cabgd bcfed | gcfadb eda acdgb gfcaed
ba fdeba fbca ebdgca bea deafcgb aedfc cadbef dceafg dgbfe | defac beacdf adbfce ecdbfga
eabfg bgdeac bcfeda gdfabce befgda afgd af afb fbcge baged | cfabde fageb fegcbda afegb
gdafcb gafed cfdbgea gcea edbcf cg fagdbe gdcef gcd gcefda | begcfad dfgaec cg gdfae
gefad gecdaf gfd gaecdb cfed adcge df fbeag bfcaged bfadcg | fgbdaec ecgda fgbae dfg
cdg ceda agbfc fdbega fbedcga gbdcef facgd cd faedgc fgead | facgdeb dcfag gdfceb gcd
gcaed fdaeb bg gabedc dbcg bag agcefd edgab cgbfae fcdbega | afbegc fgabec gba gbdae
ecbagd db ebcfg agedf dgfeacb efgdbc abecgf ebfgd bed bfdc | eagdbcf db bd bde
dfecbga faebdg fgcd gd adg ebcad acefg acgbef ecafdg cedag | gd efbdga eafcg gfaebd
bfgcea fegcbda cfgea adcfge dcea dgbecf gfdac dgbaf cgd cd | fecga cgd eadc fbdga
gefbacd fgac abfce ebdacf bgfaed bceafg fg egbcf bgf gdbec | gbfec aegfcb faegbd gbdecaf
dfbage adcbe be afdce ebfc adgbc edb abegcdf cfagde dfebca | cbagd edbafg adgcb ecfda
edabc bedgafc fd adf begfad fcgd cdabf efcgab gcbfa cfdagb | df df df fecbag
cfdg adfec daecb afc fc cadgfe fcedgab cagfeb fbagde dgeaf | acdeb cf afc dfgc
efbg acgefd eb cfdgeba badef gedfa bcgeda deb dfcab fedgba | afbedg afdgbe abdfc afedcg
gaedf ed gafce afegcd egbcfa deca bdfceg dfabecg fed gdabf | ed efd afdgb edf
dcbaef gfdb dafgc efbcga bcfadg cgf bdcaf gf aedgc faecgbd | cfdeab cfg bgecaf cdgaf
defbga aegb aefdb aebdfcg febdca ag gedaf abdgfc afg dfegc | ga acdfbge cadgfeb bdgafe
dafeg cgdae efdgb dgbecf gfa bdfacg afbged bafe af efbgacd | bfcedg dagebf aefb acegd
cbga cb fegac baedf faedcg facgdeb efacbg fcb bcfegd eabfc | bfade gcfbeda bc bcf
dbfaecg aebdfg agbcef ca bfgdac bgace cag bgedc gebaf acef | agcfbe bgdce abcegf bgcea
adfge bafe gcfaedb cabgdf gabedf gaf dageb efcgd cedgab af | eadgf baef ebgad dgfcaeb
abfdcg fdebagc bdgae gadfec ed gfeadb afbgd gebca debf edg | fedb bgaedf ed aegbd
aecbd ebcga abgfc eg gbefdca caedbf degb eabdcg cge fdcage | cfdegab gebd bgdcae ecg
adebg fgcd cadef afg fdcegba caefdb gf feadg cfeagd gfcaeb | fga dfaec eafbcg fg
eb fdcgab bfade ecfgadb aecdf gfdab bfeg dabegc begfda edb | ebd abcfgd bcedfga bgfe
eac adfec gadecf faebcgd dacbf bgdcea gefdc ae egfa bcedfg | abgcdfe gbeacd cae gfea
ba feabdc adeb dcfba geabcf gbcdeaf cdfbg edacf fgaedc bac | ba afdec bdae abcdgef
bgad cgdaf bd bfceg agfbdc cadebf cdb fecdga bdcfg dcegbfa | bdfagc fdcbag cgfead agedcf
dfgcba fgec cafegb afecb efbcdga gdeba bafcde ecbga gc cag | fcgabde adgbe debfac ecfg
bcfea dabe defbac dbcfg cad efbagc ad adbgefc edacfg abdfc | aecbf dac da ad
cbaf fbe agebf geacbf cgabde gbaec bcegfd fb fdeag cfbaegd | agcdefb gfeda ebgaf cagedb
cd cebadg dbfc gcd cafbdg adfgb ecafg fcagd aegfbd efdcbga | fgdcba bfcd agcef dc
afeb ba bgfade gacdf agdbf gba egdbf gbdecaf cbgdfe cbdage | dbfga fbedg gba agbdefc
bgdafe gfced cbfagd fa bdecag cgdfa fcegadb abcf gbcad gfa | bgcad abecgd edgabc gecbfad
cgfeb gbaef dfcg bgc cgaebd gc becafd dbcfe gefacbd cefgbd | gc gcb cbfde bcegad
bg bega cfdeg cebda fdeabc dgebc cbdage dgfcab bdg fedgabc | edbgc ebcadg fgdce abcedf
feadgc bgf febgcd gbcdfa faceb gbcfe gb dcfge gedb bacfdeg | bgf gbf dfegac fbg
gbacf ab agcefd decabg bcdagfe begfac cba fdbgc agfec ebaf | agbfc ba cdbgf fcbdg
adfe fa afb cdgeba agdebf fcgebad begad ceafbg bfgdc fdagb | baegfd adfgb dbega fba
fdceba dbega ecb cb abcf gacfed cgfdeab fedca cedba gfdbce | bdeacf acdfe cb fecda
dcegfa faebcg bafgd gfb bdfca gb fgabecd agdef begd beadgf | bfdga efcagd dgabcfe ebdg
aecdgf beadgc geb dcefbg aegdb dbafgce gb dabfe cagb dceag | dgcae ecgbad deacg edcagf
edga fbgedca gef becfd fbgda dfgcba ge dbegf abfdge cagefb | eg bgadfe eacgfb bdafgc
afcbg acg dgafeb gefc afdcb cadebg befag defacbg cg abfgec | acg cg gbfcea dbfac
fdbgcea cfgeb agdefb ecbfad abfde cd cadb debcf dcf cfegad | cgfeda cdfegba ecfgb edacfb
gdaecb ecbdg dacg fgedbc caegbdf gfeab bfdeac dageb ad bad | dab gdaecb da gcfedb
gec gcaeb ecdab fegb cfgaedb eacfbg bgdcaf fgaecd eg bfcga | bcegadf gaecb acbgdef gcfab
df fbgdec fdg dgacb fabge afdbcge bdgace dbfga fcda bgdafc | cgbdfe bdfga fd df
ebdaf gabfced abecf efgad db ebfcdg deb bacd fbgeca fedcba | abcdef acbd fcbadeg fdbae
adfeg be dgbca cefgbd bedga deb dbgcfae adcgfb dacbge cabe | debfgca acbe dgafe ebd
cdgbf gcbfda gd dbga eafcbd cdafb cegfb deagcf abcegfd cgd | aegfcdb gadb fcbad cbgfd
deb bafcgd dafce cabed agdbec febdag be gebc dgbca fdacbge | gecb abdgc bed eb
afc ac cebdaf becgdf agdfe cabegf bacd fdcbe dcefa cdfagbe | ca cfa bdca cbda
aegfc edfac fabcde cfeabdg dagcfe fadg efcbg ga cbagde acg | bgecafd acg begcf dfcea
fdgac fedabc cdge ebagf cgdfae ec eac gacfe bfcdga cfbdage | fbcdeag gabfe bdfagc caegf
fagdbce fcbed fegca agdc cadfge dg fgdce egd aefdbg gebfca | cfegab gbefac deg gde
eb bcegda efgdc ecfbda dbe fbae dfebc aegcfbd dagcfb dcfba | fdbgca dcagfb afbe be
begacf fceagd fec ce defab cbge agcbdfe becfa acfgb bgadcf | ce afgbc cegb ce
fbadg cbeafg fb eacfdg efbd decfgab fdaeg bdefga acgdb baf | fadeg bdfe bfdgae ecfbag
ebfcgad ag afdecg gbafec egdbc dgcae aefdcb dcefa dfag aeg | acdfeb cafdbeg fcgdae cdgea
da ead gbeda gbaec gcebdf gbfcdea ebfdg dgebfa adefcg badf | da fgcaed dfba afdcebg
fgdaec fbegad cde ec efagcdb cgfbd dcbaef gadef dfecg agec | dce ced ecga fdaeg
edfg de dagebc caefgb dfeca gfeca ead gdaecf fgeabdc dbfca | de befcga fceagb dcfae
gda cdaef gbeacf gbafcd dfcaegb cdbg efadgb dfcag dg gafbc | afcbg gda fbegda fagbdc
gfdcbe cadgf defag bfadce ac dfcagb fdbacge cbga afc bgcdf | begdcf dcfbg aedcbf degbcf
ce cgbfed degbfac cde cagbdf cgfad debfa deacf cgfeda cega | ec bfadgc dfbcage gebcfd
dfbceg dgeac acbf aebcg ebfga bcfgae adgfecb cb gbdeaf cbe | cbe cfba bc ebcfadg
fbgdc acd cbeaf ad bcdfa fdacbeg efad agcdbe bcafeg fbcdae | cad afcbe da gdbaec
bcgde cbadefg efgdab bfc aebfg fc bgfcda febcg gafbec eacf | bfc bcf bgcdfa dgaefb
gc cdg gcfade dfcea abfdce gace cgadf aebdcgf gbafd cebgfd | cgfad ecga cg cg
eadgbc ce dec afdeb ebdfc cfeabd cegbfda afec fbgdc baefgd | afec gfbead defbc bgfdc
agcde acfegbd dbcgf dbcfge ecf fbaecg fe gdefc cfbagd bedf | cafbdg dagec cgefab gfcedba
fbacg beadfc gcd gd cdfag fdeac ecbfgd ecfadg gdfcbae dgea | fdcbaeg gbecdf daefc fdgac
gf degf bfdeag gbaef ebgfadc gaf caegb dfcaeb dbagcf fabed | afg ecdgafb cabdef adbfce
adbgc dacbeg gecabf fcda cgefbda cf dbgfe cfb dgbfc dfgcab | afdc fcb cf dacf
fdgace ba gdaeb egcbd fegbcda afdbge abg bfad efgda gcafbe | bag gab efdga fedagbc
bgadc ba bga gdace bagdfce agcdeb degbaf ecab fgcade bfcgd | begdcfa ba ba ecgad
bcegfa dafbc fbg fg dbgea adecbg bacgfed abfdeg gdabf fdge | fdcab gedf cbdfa gadebc
gabdefc gdface edbag facgd gef acfe gdefcb efgad cbgfad ef | dafecg aecf gcdaef begad
bdfcag adgfb ag efdcbga dbeacf bga egabcf cbdaf bdefg acdg | bga ceagdfb cegbaf egbcfa
aegfdc dbace bcaefd geabcd ag debfg ega eagbd dfbgeac gcba | agebdc deabcg dbfecga bceafd
ebad gfacbde ecfagb ea dcafg dfbge edfgbc fgdae eaf agdfbe | gebafd ae afbdeg dgfea
gfc gedfa fdac cgbae ebafdg edfcag cfbdeg gcadbfe fc gcfea | adgfe eadfg egdfa caegdf
fbdea eacbfg aegfb dgbf dcebgfa edgabf acfed edb gdaecb bd | deb bd dcfbeag dbgf
cgedbfa dbgf gdfcba bgdeca acefd afcebg fcg agcdb fg gfdac | dgcba fbdg gf fg
acbgdf gcedaf efgdc ecbdfga fb bdega bfce gdcfeb febgd fgb | eafcbdg defcg gfb gcadbf
ebcgaf dgcaeb acebg cdfbea gbcef fagb gfcde fb efdbagc bef | bfe gdaebc fbaegc gcabedf
fbgdace efcba bg gbc gcfbed gbad cbagfd fcagd gaefdc bcfga | gbad gb gcdfbe fabgc
afecb dcbaf aedgcfb ce aedgbf ceb cefg fbega eacdbg acgfbe | agecbf cabgfed abecfg ebc
fcdge ceabgf aebg aecfg cfbea edabcf ag befgadc acgbfd gca | bega acg cdbgfea agc
gfcde db cbdge acgbe gbfecad bed ecfagb dgceab dfabec dbag | agdb bde bgceaf efacbd
dace ae ecgfab degafc eadfg ecfgadb ebfdg aeg dgcaf bcgfad | gea age ea agcbfd
eab cdabe bgcad bcef adcegfb bgafed be ebcafd acfgde dfcea | fbcead acbed be acedf
cbgde cadeb ba fedabg ebcdagf abcf acefdb dacef dab degafc | fcagde dba ebdgaf eacdf
ebd de caefgb bdeca bgdeaf afbcd gecab cegd bgcfdae ecbagd | adegbc bcfage agfbed eagfbd
ecfdb acgdbfe gdb ebfacd bfcg gfdceb efgdb bg aegcbd gaedf | adegf bg ecfgbad bdfaec
ecgabd fb bedca beacfd gfcbead fgaed dbf cfeb bedaf cbgadf | abedc dabfe bcfead cedfgba
gfcae fdgab bafedg cdab dc bdgcaf gbcfead cgfda cdg gcbfde | bdefgc adfgb gdc bgecfd
cdegf adegcf gdfebca fdg dagc ebcdf agcef dg gdfeba cabegf | gebafc gfd dg gdac
faedcb gbecfa bf febg dgbcaef cdgab cgfae fgdeca bfcag fab | dfgacbe gbef fdbaec bfadce
gcbeda cdegf dcbfe becdgfa fbca edabcf ebgafd efb acedb fb | gdbcae abcf fbac efb
cdgfba defagb gadebc cd cdbfaeg edagb fbegc ceda cgd bdgce | dgbafc agbfde facdbg gedba
cedgb egc afcgbd ec cbea gaedfc cbgda afegdbc ebdgac ebgfd | gfcbad gec aegcdf afecgbd
gbc acdgeb egbfa egbcf cdefg bc efgdbca baefcg afcb fdaegb | ebdcga gecfd ecfgd gebdac
gdeacb aecfg ebgca defbcag adebc beg gdabef bg acfbed dgcb | dbefga geb fbacde acbge
ebgafd bc gafce cfb cdfagb dbce begfd efbcg gdfecb dacegbf | acegf fcgea cb edcbfag
eagdb deafg fgdb gbdeaf dcgabe ecdgbfa afcge df fda acbfed | gdfb fd gbfeda fda
faebg gfaecbd cfegb age gcadbf ebad fdbag bagdfe cgfeda ea | cbfge bafgcd fgabd gbdaf
dgafecb ag fdebg fdgae fgba fbdage ecgfdb aebcdg fcaed eag | gafde ag egcbfd fbegd
bdcgf dfceb afgdbc gd ebcgfa fdg gdecfab dbga afbgc adcegf | dg cfbagde bfcdg adcegbf
cdbaf aefbcd abdefg bdgcaef dgcfb ca aebdf eacd acf ecagbf | dgcbf afc eacgfb egfcba
bface dacbg dgface bgecaf efbd fbacde dabec cde ebadgcf ed | dce debac ced dagcef
ebfdc efg bfgde dbegcf afdecb cebfag gcde dfacbeg eg afbdg | cdeg edcg gfadecb egcd
efdgca agebf fcgbdae cdbfg bade dgfab edagfb dga da bafceg | dga adeb cafebg dag
gbdec cb abcfged dcfeg fdacgb cbgdae agdeb acbe egbfda cbd | dcefg baec gdbcafe dbaeg
gcdab cabfg ecgfa gfb efab dbegfc cabegdf cefagd fb gaefbc | egafdc baegfc feab aefb
dacfeb dfc dgbac fd becdfag afcbge aebfc dcafb efad gebfcd | cdf bgacef efcgbd dabcfe
dfcage bfdgec gcbfd gcfab agedbf edcfg fbd db gdceabf debc | acbfg dbf fdb bdce
dbcge egf debagc gcedf cadfe bgcdfe bdfg gf cebgadf gbafec | gbdec dcegb gefdc gedbca
ebdfcga bfecd feacd daeg cfgad ea dfagce cfgbae afbcgd aef | dcbgaf adbgefc cedbf dgfac
ebfac dcgbf ecg degf cegbf cdgefb dabcge facdgb bfcdaeg ge | ge eg bfcae fegd
bd fbacge edbc gfabed cegbf fcdgbea febgdc dcbgf dgfca dgb | gbefc efcgba db abefgc
dacfgb dbacge bcf abcdfe bf fdabgec dcagb bagf gcbfd dcgef | agbfdc deacgb bdcag fb
ecgfa bgdfeca fgdbec ebafdc gbdcfa bfacd dgf gd bgad gcdfa | fcagbd bgda fadgc gbfdec
decab gcfbe faegdc gafb bgfeca ebcadgf ag ecgdbf gea eabcg | ga egdfac bcfeg ga
ebfgd dbcgfe fgabd egad fecgab ga fagdeb cdbfa afg dgcbefa | gfa ag ag gfbad
fgcade agbf fgcdabe ceafb gefca eagbfc bca fcedb ab edbgac | bgdeac ebcdf fagb bca
eacbfg eabfcgd edfac ebfdc cgdeba bd dbc bfgedc bfdg ecbfg | gbdf bcd fdace bdfg
bfdace cbfagd cbadf gedcabf adfbe faebg beagcd de dae cefd | fcde fabdgc aegfb aed
deacb gfbeda dcbfa acbdge decfbg cagbedf ae eda bgedc gcea | dbefgc edgbfc ae ea
ecfag bcafed bgdca ebagc cebagd abegcfd dagfcb be bdeg eab | gbeca acbdeg bagfdc bae
cagef bdea dag egdaf agedbf egfbd bcfgad cgebadf ad egcbfd | dgeaf gad becdgf begcafd
efcba egbfadc fgebd gbfaec bafedc efgcad ebfad da dabc ead | ebcagdf ead fecbag bcgadfe
efa cadfb fgdcbe cegafd fcbeg abecgf cebaf aecdbfg beag ae | bdcfa bcagfe cbefg ea
ceab aedbcf fdbce cb daefb dgacbfe gcdafb bafdeg dcegf cdb | bc abce afegbd edgfba
afdegc acdfbg debcfag facebg bdgc dac dc acdbf gbcaf efdab | cd gadefbc cbfga abcgf
ecf fc gdefc feabdg dcabef edagc bfdge fgdbec fcbaedg fbcg | efc bgfcde caedg gdfce
gfeab aefdgc cdefgb bcfgd ad fad adbc acbfgd fgdba bgfedac | fgcdeba da gfdab cegdaf
bea gebfd edfbacg egcdfa ab dagec dgeba dcab abcdge cgeabf | acdbge ba gecafd dfbge)";

int main() {
    solution(input);
}