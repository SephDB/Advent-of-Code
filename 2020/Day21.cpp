#include <iostream>
#include <string_view>
#include <array>
#include <vector>
#include <bitset>
#include <map>
#include <unordered_map>
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

using set = std::bitset<200>;

void solution(std::string_view input) {
    std::map<std::string_view,set> allergens; //we need to keep them sorted by name
    
    std::array<int,set{}.size()> ingredient_occurences{};
    std::ranges::fill(ingredient_occurences,0);

    std::unordered_map<std::string_view,int> ing_to_id;

    auto to_id = [&m=ing_to_id,current=0](std::string_view ingredient) mutable {
        auto [it,success] = m.try_emplace(ingredient,current);
        current += success;
        return it->second;
    };

    split(input,'\n',[&](auto line) {
        auto contains = line.substr(line.find("contains"));
        line.remove_suffix(contains.size()+2);
        contains.remove_prefix(sizeof("contains"));

        set current;
        split(line,' ',[&](auto ing) {
            current.set(to_id(ing));
            ingredient_occurences[to_id(ing)]++;
        });

        split(contains,' ',[&](auto allergen) {
            allergen.remove_suffix(1);
            auto [it,success] = allergens.try_emplace(allergen,current);
            if(not success) it->second &= current;
        });
    });

    std::vector<std::pair<std::string_view,set>> allergen_sets_sorted;
    std::ranges::copy(allergens,std::back_inserter(allergen_sets_sorted));

    //day 16 again, find unique mapping
    set used{};
    while(used.count() != allergen_sets_sorted.size()) {
        for(auto& [name,s] : allergen_sets_sorted) {
            if(s.count() == 1) used |= s;
            else s &= ~used;
            if(s.count() == 1) used |= s;
        }
    }

    std::vector<std::string_view> sorted_ing_names(allergen_sets_sorted.size());
    
    int total1 = 0;
    for(auto [ing,i] : ing_to_id) {
        auto s = std::ranges::find_if(allergen_sets_sorted,[i](auto s){return s.second.test(i);});
        if(s == allergen_sets_sorted.end()) {
            total1 += ingredient_occurences[i];
        } else {
            sorted_ing_names[s-allergen_sets_sorted.begin()] = ing;
        }
    }
    std::cout << "Part 1: " << total1 << '\n';
    std::cout << "Part 2: ";
    for(auto i : sorted_ing_names) {
        std::cout << i << ',';
    }
    std::cout << '\n';
}

std::string_view input = R"(mfp ncqls nnkrx mgvfmvp vjqr ncbrv jdfhh llkth xnmbjh dphvbt xzgcll hscht ngjt tljxbd nhdjth hcdchl nnsvvkz tlppq mpqvcfq csglnm dsghj kvxv mzpzjm klmr fjf mhnrqp vkkcf lkrf kzz vfrcj crrvj gnvrcr spzzmx dcvrf vmls xggzsks rzgsq gcpff xzzs mcg lfmfr ltvt nnpbm pbfjh pvnt zbrncpp dvkbjh zjzrkb vkxcxr (contains soy)
qbmcl vkkcf zxxhkv xzzs cfn ncbrv kxzdg lfmfr tljxbd ctjcbcls zjzrkb fxtrzm hcdchl bsmmc mhqhbm cjljfx hvcrh zvcf vpxc nnkrx bcjz sgrhdz smmnq tsptf ctqch bvvp qrxs grqvth dfzjk fkczp cr zzlvpcq rjlqxr ccxjnp ztpss jslzv hrdt kzz tjzhk llzlhc vshqdk ptlp qhjvlz nxhrrv bht vkxcxr mgvfmvp ddvbtk cmksdt mfglxb xggzsks rqddvr vpbhh kdt lvmm dvkbjh mhnrqp vjrzsrb nhdjth xzgcll crrvj jzqhqps jtll mfp (contains dairy, soy)
tsptf vpbhh kcfk hrdt dcszch tjzhk zxxhkv nhdjth zzkqgl kthcp dvkbjh vvn jbgmczs txztr sqfls kvxv dphvbt rlkrtn vkkcf mpqvcfq mfp zz gz hcdchl lkrf crrvj ngvl cgbj drg zsxbx spzzmx mfglxb vpbrd xvbvt lfmfr nnpbm qhjvlz nnrspld mgvfmvp nmvktj prds rzgsq ncbrv xjtkz vshqdk ddvbtk dcvrf qbmcl vzg fdq pbfjh qrxs vpj kgzs zzlvpcq xzzs mhnrqp mpjjc brvbdk (contains sesame, fish)
hcdchl ltvt mgvfmvp fxkz gnvrcr zxxhkv tsptf fdq trl kxzdg fgdtk fcsxn thtj mhnrqp fbnnl vkxcxr sqfls fnzgpkd snxph pbvx dcvrf ccxjnp cs gz mpjjc pszks lknqg ptlp jgqvpj vpbrd nhdjth zzlvpcq qhjvlz vmls kgst cr xvpcp bsmmc dsjkjjz smmnq nxhrrv vpxc rthfc mzpzjm vshqdk fcdgnm xzgcll dcszch grqvth mfp vkkcf jcth ghx drg cpgxmh vfrcj cvqmhtgg dvkbjh xjtkz zjzrkb ndmtbp mbcl dmxcb sgb klmr fkczp prds vpj vpbhh gcpff nnsvvkz lhv kdt pldq kgzs txztr (contains dairy, peanuts, fish)
vpj ztcm zzkqgl fnzgpkd qrxs sgb fbbxv mhnrqp dpzv bnmgsh cpgxmh dcvrf mfp gnvrcr dcszch vhdmkr kdt kthcp trl hppgd lvxvb dmxcb hcdchl nhdjth nxhrrv rlkrtn dqfdth tsptf zvcf dvkbjh pbfjh prds xggzsks fxkz thtj vpxc fkrf mpjjc bcjz kcfk fxtrzm snxph lhv jzqhqps vgmgvd fgdtk dsjkjjz mcg vjqr mbcl tchrfb llkth (contains dairy, peanuts)
lvxvb zvrqj dqfdth ctjcbcls dvkbjh gxc hppgd jgqvpj cfn vjqr bsmmc pbfjh tjzhk bcjz fdq vpbhh tljxbd mcg hscht ltvt ktvqj vshqdk nnkrx zvcf jbgmczs pszks cs crrvj fkrf qds vpj hcdchl rrjlqh tbbfh hgvst zsxbx lfmfr tsptf prds cmksdt xjtkz vkxcxr tp mgvfmvp tlppq mfp tchrfb cjljfx xzzs mhqhbm rjlqxr ndzjn fcsxn cgbj nhdjth ctqch xzgcll klmr tzgmld dcszch ptlp zmchhmf njpg nnsvvkz fkczp grqvth dcvrf zjzrkb pvnt sqfls xggzsks (contains dairy)
ztcm rrjlqh mbcl mpjjc vpj sgb jgqvpj vjqr llzlhc gxc dqfdth ndmtbp vhdmkr ltvt hgvst cs fxkz lfkp hcdchl cgbj lfmfr mgvfmvp cjljfx xdctcc dsjkjjz gz sgrhdz jtll ccxjnp kzz nkxsm dphvbt ctqch jcth xvpcp kgzs kkflzhh vfrcj fcdgnm bvjpl dcvrf dpzv bcjz lfjrzv ptlp kdt mfp rthfc fbnnl qds jslzv ctjcbcls tbbfh dvkbjh nhdjth lhv snxph mhqhbm hrdt kvxv fxtrzm txztr zjzrkb mfglxb fj vkkcf cvqmhtgg gnvrcr xnmbjh zbrncpp tmkfc ngjt zsxbx gcssxq xzgcll (contains shellfish)
jbgmczs tbbfh zvrqj kgzs tlppq bcjz fbbxv fcsxn drg mhnrqp kzz fbnnl thtj tlxs zxxhkv ztcm qds xvbvt pszks vshqdk fxtrzm bvjpl nsvbkbz vpbhh dfzjk xzzs tjzhk mpqvcfq nhdjth dcvrf fjf dvkbjh mbcl hscht cfn zsxbx hcdchl bht ndzjn fkczp tzgmld zzkqgl brvbdk hvcrh vjrzsrb mgvfmvp nnrspld pbvx ngvl ngjt fxkz rrjlqh gz cpgxmh ptlp qrxs nmvktj mzpzjm pvnt gcssxq lknqg (contains shellfish, nuts, peanuts)
smmnq sgrhdz vpbhh dpzv lhv fklh ndmtbp lfjrzv fbbxv zxxhkv tp cs vfrcj mgvfmvp zmchhmf cmksdt tjzhk ccmjg cr ngjt tsptf ptlp txztr mpjjc dvkbjh zvrqj hvcrh mfdnj qds klmr nxhrrv vvn dcszch pldq hcdchl msvjddz bht mfp xvbvt cvqmhtgg ztcm jcth ncbrv mhnrqp crrvj fnzgpkd zpvzrdc pszks kkflzhh dmxcb hgvst dsjkjjz sqfls nhdjth jdfhh bnmgsh dcvrf ccxjnp fxtrzm tlppq jgqvpj xdctcc ktvqj zsxbx jslzv zbgpfdc kzz skfc mpqvcfq mzpzjm kvxv znhgh mhqhbm gz xcv cjljfx lfmfr lfkp hrdt vjqr spzzmx mcg mbcl njpg (contains nuts, peanuts, sesame)
njpg vhdmkr fjf xzkpktr hv ztpss cmksdt cjljfx vvn pbfjh sgb ktvqj ndzjn lhv bvjpl qrxs dvkbjh vzg ctjcbcls xcv ccmjg znhgh nxhrrv nmvktj rjlqxr ddvbtk tmkfc msvjddz vpxc bcjz kxzdg mhnrqp fxkz cfn kkflzhh hcdchl xvpcp mzpzjm lfkp vpbrd hscht mfp gxc dcvrf prds nnpbm kthcp nnkrx fkrf nsvbkbz fdq mgvfmvp pldq hvcrh smmnq zzlvpcq lfmfr dcszch zbrncpp zsxbx tp lvmm sznfj jgqvpj vfrcj llzlhc tzgmld spzzmx kdt kgzs pbvx dsjkjjz skfc rlkrtn lkrf qhjvlz (contains sesame, nuts)
xzzs kcfk tlxs tjzhk nxhrrv lvxvb gz cvqmhtgg kzz txztr sgrhdz xdctcc ngjt ktvqj cjljfx xzgcll xzkpktr vpbrd zbgpfdc hcdchl ptlp gnvrcr mhnrqp bvvp fqrnx mbcl qbmcl zmchhmf ctjcbcls bcjz jslzv vkkcf skfc vjrzsrb jcth kxzdg mpqvcfq fbnnl zxxhkv sgb ddvbtk jdfhh dcvrf vvn jbgmczs vpj vmls nnpbm xcv tlppq llzlhc tp fgdtk kkflzhh fdq nhdjth grqvth mgvfmvp sqfls rjlqxr mfp khpx mzpzjm fnzgpkd (contains shellfish, nuts, sesame)
sgb jtll nnpbm bnmgsh zz mgvfmvp jslzv tbbfh fklh mcg zbrncpp xvpcp mfp cpgxmh hrdt drg dcvrf mhnrqp fbbxv dphvbt znhgh xzkpktr xzgcll dvkbjh xggzsks nxhrrv hcdchl ndzjn ptlp vshqdk rrjlqh hvcrh vmls fcdgnm ghx ctjcbcls ztcm bsmmc zzkqgl cgbj lfmfr rjlqxr cmksdt mbcl bcjz qhjvlz (contains soy)
tlxs dcvrf khpx trl cjljfx smmnq spzzmx tlppq prds ztcm tchrfb gnvrcr qrxs gcssxq kgst mgvfmvp pvnt sgb ndzjn mfp bvjpl cvqmhtgg sznfj mhnrqp tzgmld qds dmxcb dqfdth hv ccmjg vgmgvd fbnnl fxtrzm dpzv rjlqxr ngjt fcdgnm hcdchl tp fkrf mbcl ctqch csglnm zxxhkv nhdjth vpxc vmls ccxjnp pszks nxhrrv jtll bcjz (contains sesame)
nhdjth kgst vpbrd jslzv ztpss fjf dqfdth hv prds dpzv dcszch tlxs dcvrf zzkqgl dfzjk nnkrx bsmmc kthcp mgvfmvp txztr trl ngvl msvjddz zsxbx sgrhdz fkczp jdfhh hcdchl gnvrcr rlkrtn hvcrh csglnm bvvp ztcm fnzgpkd kcfk hscht rthfc kvxv lfmfr mbcl lknqg grqvth zvcf xvpcp pszks znhgh tp mhqhbm llzlhc nnsvvkz mpjjc zvrqj cfn smmnq ngjt gcssxq ndzjn fgdtk nnrspld rzgsq bcjz nmrbb nsvbkbz mfp ncqls spzzmx mhnrqp cgbj lvxvb ncbrv xdctcc rqddvr lfkp ccxjnp vjqr (contains nuts, shellfish)
cgbj mbcl jgqvpj thtj cvqmhtgg ncqls lfjrzv jbgmczs fbnnl drg zmchhmf vmls rrjlqh sznfj rjlqxr tljxbd vzg nnsvvkz fklh jzqhqps fbbxv tbbfh qhjvlz cjljfx sqfls ltvt fj hgvst hrdt mfp bsmmc vpbrd grqvth tlxs nhdjth ccxjnp pvnt tzgmld vpxc dcvrf tsptf lfkp xzgcll fdq hcdchl hppgd sgrhdz kvxv vvn nmvktj llzlhc nnrspld dvkbjh dfzjk zpvzrdc ncbrv zjzrkb ktvqj xvpcp ngjt kcfk kzz lvmm sgb bvjpl kdt fjf mhnrqp mpqvcfq bcjz crrvj (contains fish, sesame, dairy)
fkrf nsvbkbz fj vkxcxr kgzs mhnrqp ccmjg ztpss gcpff bcjz vpbrd dmxcb fcdgnm nhdjth ngjt gnvrcr snxph lkrf khpx brvbdk lfmfr jzqhqps msvjddz ccxjnp zbrncpp mfglxb cmksdt rthfc skfc vfrcj tmkfc mbcl hcdchl kgst ptlp dqfdth vpxc zvrqj sgrhdz mfp dcvrf fxtrzm kzz gcssxq dsjkjjz bnmgsh rlkrtn dvkbjh fklh znhgh dcszch ddvbtk smmnq qrxs fxkz nnsvvkz qhjvlz nkxsm mzpzjm vpbhh fbnnl jslzv vgmgvd cgbj cvqmhtgg fgdtk nmrbb tbbfh ndzjn ncbrv (contains fish)
lkrf fklh vgmgvd cmksdt kthcp kdt zbrncpp ztcm njpg crrvj kgst tp znhgh mhnrqp zzkqgl mfp gnvrcr gcpff vfrcj dpzv zbgpfdc rjlqxr vmls dmxcb lvxvb dqfdth bcjz cpgxmh rrjlqh llkth dvkbjh tljxbd dsjkjjz xnmbjh trl ddvbtk lfjrzv nhdjth mhqhbm cs xvpcp fgdtk hvcrh hcdchl vpxc lknqg sgb mpqvcfq pldq sqfls zz ghx nkxsm jzqhqps cr fxkz fnzgpkd thtj lvmm zmchhmf hgvst jtll zvrqj gz tchrfb tjzhk jgqvpj zpvzrdc dcvrf jcth (contains dairy, shellfish)
cjljfx hvcrh nnsvvkz thtj smmnq tlxs tp kgst hcdchl zz fklh csglnm lfkp lknqg llzlhc zjzrkb lfjrzv zbgpfdc mhqhbm ncbrv lvmm rthfc gnvrcr brvbdk kxzdg xzgcll nnkrx ztcm vkkcf ndmtbp lvxvb zxxhkv grqvth vfrcj ccxjnp mhnrqp vjrzsrb fxtrzm ccmjg nnrspld mfp trl fnzgpkd vpj jdfhh bvjpl cmksdt bvvp njpg znhgh dpzv qds xcv cvqmhtgg zvrqj zvcf fbbxv qrxs mbcl bcjz nhdjth tbbfh ncqls cr zpvzrdc vvn xzzs nsglr cpgxmh mgvfmvp dqfdth dcszch zmchhmf zsxbx hrdt nmvktj lfmfr kvxv vhdmkr tzgmld mfglxb hv fxkz xnmbjh rjlqxr skfc mfdnj dcvrf pbfjh ndzjn rqddvr zzlvpcq (contains dairy, sesame)
zbrncpp ccmjg mhnrqp zvcf ncqls khpx jdfhh fkrf jbgmczs hppgd dcvrf hvcrh dqfdth znhgh zxxhkv xnmbjh zvrqj kthcp lfmfr csglnm pvnt mfdnj grqvth ptlp dvkbjh mbcl ddvbtk skfc mhqhbm dfzjk mpjjc bvjpl mfp lvmm vgmgvd lfkp kdt fgdtk fqrnx fbbxv nnrspld jtll tlxs prds nhdjth kgst bcjz gnvrcr xzzs lfjrzv kgzs rrjlqh hcdchl xjtkz qrxs lvxvb (contains nuts, dairy)
pbfjh crrvj dvkbjh dmxcb dcvrf nnsvvkz fcsxn jslzv nnkrx dpzv kkflzhh fnzgpkd tchrfb xzgcll njpg nhdjth xjtkz dphvbt cvqmhtgg mhnrqp zzkqgl kdt ddvbtk nmvktj ghx mcg xzzs cr cjljfx bsmmc kgst dsjkjjz fjf rlkrtn snxph hcdchl vkxcxr xvpcp fqrnx tzgmld trl mpjjc lvmm ndzjn rthfc pbvx vzg ltvt zz vfrcj bcjz ctjcbcls nnpbm prds vvn vshqdk hv rjlqxr nkxsm fbnnl qrxs qhjvlz tsptf tmkfc lfjrzv vpbrd pvnt drg vkkcf kcfk fbbxv hvcrh zsxbx ndmtbp sznfj mfdnj mhqhbm ncqls llkth mgvfmvp jdfhh rqddvr jcth cmksdt (contains shellfish, sesame, nuts)
cr sqfls gcssxq nnkrx vkkcf tlppq lkrf hrdt ltvt ddvbtk ptlp mpjjc zpvzrdc rlkrtn nkxsm nnpbm hgvst tchrfb ctqch vjrzsrb xzzs pbvx jzqhqps dcvrf nhdjth zbrncpp xzgcll zvcf hcdchl dvkbjh msvjddz tljxbd nsvbkbz sznfj fxkz bvvp ktvqj sgb vpj kdt dcszch rjlqxr ncbrv lfjrzv snxph ctjcbcls ccxjnp prds ngvl dpzv zzkqgl bcjz mfp lhv ztcm spzzmx mhnrqp jbgmczs znhgh thtj llzlhc cfn lvxvb fjf cpgxmh dsjkjjz vpxc jdfhh fklh mfglxb cvqmhtgg kkflzhh fxtrzm gxc vzg dqfdth lfmfr vvn rqddvr qrxs hscht xjtkz cjljfx zjzrkb fgdtk gcpff fqrnx tsptf (contains soy, nuts)
tlxs mhnrqp mfp hscht trl rlkrtn xzgcll bcjz ncqls fj nmrbb zsxbx vzg mpjjc lknqg tljxbd jzqhqps jbgmczs skfc kvxv fcdgnm nsvbkbz zzlvpcq nnsvvkz nhdjth vgmgvd ccmjg ddvbtk bvvp nnpbm cfn ctqch nnkrx lfjrzv vpj kgst kgzs hgvst sgb bvjpl kkflzhh fbbxv rqddvr hcdchl mgvfmvp kdt fgdtk fxtrzm vvn fqrnx nxhrrv brvbdk bsmmc tmkfc cgbj smmnq vkxcxr ztpss kzz rthfc spzzmx mfglxb msvjddz dcvrf jcth sqfls vpxc hvcrh (contains sesame, shellfish, nuts)
nnsvvkz nsvbkbz vshqdk csglnm fcsxn zzkqgl ncqls ncbrv fklh tlppq kdt vkxcxr lvmm fdq pbfjh gz jgqvpj ndmtbp cpgxmh llkth bnmgsh ctqch zvcf nhdjth mhqhbm tzgmld xjtkz nxhrrv vhdmkr nnpbm fxtrzm hscht vpbrd zvrqj bvvp lfjrzv tchrfb ngvl qhjvlz tmkfc grqvth vmls dcvrf dvkbjh bsmmc fj bcjz hcdchl vjrzsrb cjljfx tbbfh kcfk ctjcbcls mpqvcfq mpjjc dpzv snxph dcszch mfp nkxsm mzpzjm thtj lkrf pvnt rthfc qds lhv llzlhc cmksdt zbgpfdc hgvst mhnrqp jtll ktvqj zmchhmf (contains dairy, nuts, sesame)
zzlvpcq ccxjnp kdt ncbrv ngvl brvbdk vzg dcvrf vvn dfzjk sgb skfc hscht nmvktj kxzdg mfp kthcp mgvfmvp ltvt jbgmczs pldq kzz vfrcj dvkbjh mhnrqp hgvst ndmtbp vmls fqrnx bcjz vkkcf cmksdt thtj zvrqj ngjt bht klmr rzgsq vpbrd zz khpx hcdchl zxxhkv ztpss kcfk (contains sesame, peanuts, dairy)
fdq cpgxmh tlxs xzzs dpzv zxxhkv drg nnpbm lfmfr grqvth tchrfb gxc mfglxb fj zvcf zpvzrdc vshqdk cjljfx skfc vpxc pvnt mhnrqp xnmbjh nmvktj fbbxv pszks dcvrf nhdjth hvcrh gnvrcr fnzgpkd ndzjn cs xjtkz bcjz nxhrrv tljxbd mgvfmvp zjzrkb qds fcsxn rzgsq vpj dvkbjh lkrf zsxbx ccmjg vpbrd ltvt lvxvb bnmgsh ncqls fxtrzm hscht msvjddz mfp fqrnx ndmtbp njpg (contains soy)
sgb tjzhk trl gnvrcr rlkrtn pvnt bcjz mfp bht dsjkjjz ztpss bvjpl qhjvlz ltvt rqddvr xcv lvmm mgvfmvp lhv sgrhdz rjlqxr mhqhbm ngjt nhdjth fj llkth fxtrzm lvxvb xvbvt ktvqj vfrcj ngvl grqvth thtj prds pbvx mcg kcfk gcssxq brvbdk hv dvkbjh pldq ndzjn cjljfx fcdgnm dcvrf fkczp zsxbx xjtkz kthcp xzgcll ncqls nsglr jdfhh vshqdk mhnrqp snxph vvn tchrfb (contains shellfish, soy)
vzg jcth dsghj cr nnpbm sgb snxph cs tmkfc msvjddz mbcl znhgh vshqdk cmksdt kgst mgvfmvp cvqmhtgg fj hcdchl nhdjth pldq zjzrkb ccmjg fklh ptlp tbbfh dvkbjh vpbrd fkczp trl kvxv dsjkjjz grqvth drg dfzjk gnvrcr qbmcl vjqr lfkp jzqhqps hppgd khpx lfjrzv mfdnj vpj mhnrqp qds xcv kkflzhh bcjz klmr nxhrrv tzgmld bsmmc xzkpktr pbvx dcvrf vkxcxr sgrhdz jtll vgmgvd xzgcll ndmtbp gcssxq ndzjn fxkz gcpff jbgmczs ngjt pvnt mpjjc fdq nnrspld tp nmrbb sqfls lvxvb zxxhkv ctjcbcls lkrf (contains soy, nuts, sesame)
ktvqj gxc kdt grqvth pbvx mfglxb ndzjn xzkpktr nhdjth ccxjnp njpg rzgsq ncqls hcdchl fbnnl ccmjg sqfls ltvt dsghj vpxc zbrncpp dvkbjh mfdnj xjtkz mhnrqp ngvl xvbvt qbmcl mgvfmvp fxkz dsjkjjz znhgh jbgmczs ncbrv dcvrf mhqhbm lkrf mcg hppgd tchrfb gcssxq fxtrzm bcjz vfrcj rthfc zpvzrdc tp hscht nnkrx (contains sesame, peanuts, fish)
dvkbjh gcssxq vzg bvjpl nhdjth fxtrzm kzz vmls vvn kgst dfzjk dsghj vpbhh tsptf snxph ltvt vpbrd ngvl cjljfx ndmtbp znhgh fqrnx fbnnl mfglxb xzzs hcdchl gcpff mhnrqp tp hrdt nnsvvkz ccmjg zxxhkv dcvrf klmr mgvfmvp lkrf ndzjn dphvbt hppgd jtll nsglr mfp fxkz mpjjc fcdgnm (contains peanuts)
fxkz mfp ptlp rjlqxr njpg mgvfmvp dcszch dcvrf zzlvpcq hgvst xdctcc fkczp gxc dfzjk crrvj ngjt sznfj dvkbjh fkrf ktvqj jbgmczs bsmmc cr pvnt lkrf gnvrcr ztcm mhnrqp vpbrd bcjz rthfc kdt mpjjc bht skfc xjtkz dsghj zmchhmf ndzjn tmkfc zpvzrdc nhdjth brvbdk vgmgvd jzqhqps (contains eggs, peanuts)
prds hv mhnrqp msvjddz ncqls xzkpktr vpj vgmgvd thtj ccxjnp hcdchl klmr nkxsm zmchhmf spzzmx nhdjth cjljfx nmrbb vkxcxr nnsvvkz tsptf mfp gcssxq zvcf mfglxb ctqch vpbrd qrxs kzz bcjz gz zjzrkb njpg nxhrrv hrdt sgrhdz mgvfmvp fgdtk nsvbkbz dvkbjh fkrf kgst tmkfc mfdnj ccmjg fxtrzm dqfdth mbcl jbgmczs jslzv jdfhh vvn crrvj mpqvcfq pbvx pvnt hppgd vzg fxkz tp zz xdctcc rthfc smmnq dsjkjjz vpxc rqddvr rlkrtn dcszch cvqmhtgg hgvst sgb fj lfkp (contains fish)
zzkqgl hscht vzg lfkp xnmbjh gz ptlp cs thtj jslzv hcdchl tlxs mhnrqp sqfls dphvbt mfp llzlhc kkflzhh hv kcfk dcvrf bht rthfc jzqhqps mgvfmvp cjljfx pbfjh smmnq zmchhmf jgqvpj dvkbjh ltvt cgbj cr dfzjk khpx nnkrx njpg jdfhh tjzhk nhdjth xdctcc ccmjg jcth cmksdt lknqg bsmmc kdt kxzdg xzkpktr rlkrtn msvjddz hvcrh xvbvt ddvbtk zsxbx gcssxq vmls qbmcl zzlvpcq vpxc kgst nsvbkbz skfc znhgh vjqr bnmgsh zjzrkb ktvqj dpzv nxhrrv vkxcxr pszks (contains nuts, peanuts, eggs)
rqddvr kxzdg fxkz nmvktj cs zvrqj dvkbjh qrxs vgmgvd nnrspld nhdjth bcjz fkczp hcdchl khpx cmksdt bnmgsh fcsxn xzgcll ndmtbp xzkpktr ncqls mhnrqp ltvt fjf ghx vkkcf lkrf dsghj cvqmhtgg ndzjn xcv mzpzjm mgvfmvp crrvj fnzgpkd kthcp bvvp spzzmx nxhrrv mfp drg skfc cjljfx (contains eggs)
tzgmld vkxcxr jdfhh ngvl kvxv llkth ghx skfc sqfls ndmtbp gz cvqmhtgg fnzgpkd ptlp pvnt zmchhmf zbgpfdc tmkfc xvbvt nsvbkbz ccmjg mfglxb msvjddz pszks jgqvpj fj nhdjth nnrspld zbrncpp mfp xdctcc kdt cs ctjcbcls kcfk dvkbjh fjf snxph nkxsm dfzjk dcvrf bcjz dpzv rrjlqh cmksdt nmrbb vgmgvd qbmcl xzgcll bvvp cgbj csglnm mhnrqp prds dsghj zzkqgl xggzsks fkczp hcdchl ndzjn nmvktj fkrf (contains peanuts, fish)
tmkfc nhdjth mfp hcdchl ncbrv lhv pbfjh fjf kkflzhh kvxv mhnrqp hrdt sgrhdz fklh fdq tzgmld mfdnj pvnt grqvth vvn nnkrx vmls mgvfmvp brvbdk klmr drg pszks ndmtbp zjzrkb cgbj bvjpl kcfk zxxhkv zz cmksdt qrxs khpx xcv dsghj tlppq fbbxv fxtrzm ddvbtk znhgh mfglxb tchrfb ccxjnp jdfhh dvkbjh zzkqgl spzzmx nsglr qds cr zmchhmf ktvqj zbrncpp dcszch jslzv fgdtk vkkcf rjlqxr ngjt tsptf fcsxn vjqr lfmfr nsvbkbz dphvbt xnmbjh bcjz jcth fnzgpkd hscht xvbvt mzpzjm hvcrh (contains peanuts, nuts, shellfish)
ltvt rrjlqh hscht dcvrf mfglxb qrxs mpjjc tjzhk hvcrh ptlp prds xzzs kgst gz mhqhbm skfc crrvj lvxvb mhnrqp jcth dvkbjh nxhrrv nnkrx fkrf hrdt fbnnl mfp lknqg qds zvcf hgvst tljxbd zjzrkb lfkp xvpcp jgqvpj pldq bcjz vshqdk vvn zmchhmf fjf hcdchl fklh zz mgvfmvp (contains nuts, sesame)
zvcf dvkbjh hscht qds lvmm pbfjh msvjddz kgzs sgb nsvbkbz rzgsq jtll fklh xvpcp pvnt mfp zvrqj mgvfmvp fxkz mzpzjm ktvqj zpvzrdc kgst tbbfh mfdnj zmchhmf hcdchl qrxs lfmfr vpbrd lvxvb rlkrtn rthfc nhdjth pbvx dcvrf znhgh dsjkjjz thtj spzzmx gz nmrbb tljxbd dmxcb ncbrv vvn zsxbx hv ddvbtk ptlp bsmmc lfkp sgrhdz xggzsks kkflzhh bcjz fcsxn fxtrzm lknqg (contains fish, shellfish)
vjrzsrb mfp cvqmhtgg gcpff nhdjth tchrfb lfkp ngjt pbvx vshqdk nnkrx ncqls kvxv xnmbjh ctjcbcls grqvth nnpbm vgmgvd nnrspld bcjz snxph pbfjh zzlvpcq dmxcb gnvrcr crrvj sgrhdz thtj xzzs mbcl tjzhk cmksdt zvrqj mgvfmvp tlxs dcvrf zz tp dpzv ncbrv mhnrqp vkxcxr zbrncpp ndzjn hcdchl pvnt lfjrzv bvvp (contains soy, eggs)
bsmmc hgvst bcjz nnpbm dfzjk hscht zzkqgl vshqdk mfp mhnrqp xvbvt dvkbjh kvxv jdfhh fkczp rthfc tsptf mgvfmvp hcdchl rzgsq ngjt tjzhk bvjpl nnrspld ztcm fqrnx mbcl tzgmld xggzsks crrvj zzlvpcq nnkrx dsjkjjz tchrfb zjzrkb mfdnj ztpss kkflzhh ccmjg lvmm tlxs qhjvlz lkrf vpxc vmls khpx jbgmczs vhdmkr fkrf vfrcj nsglr fbbxv ctjcbcls vpj fcsxn xnmbjh mpqvcfq ctqch mpjjc nhdjth sgrhdz mcg vpbrd smmnq tljxbd ndzjn zbrncpp zvcf lfmfr vjqr gcpff skfc (contains nuts, soy)
ngvl crrvj hppgd tlxs ztcm rqddvr njpg vkxcxr xzgcll vpbhh cs mhqhbm vkkcf nmvktj fj dvkbjh ztpss ndzjn rjlqxr mgvfmvp tljxbd prds fbbxv mpqvcfq nnkrx sgb mfp kgst brvbdk hcdchl mhnrqp gz tjzhk fnzgpkd fkrf kcfk zz tsptf bht cvqmhtgg ddvbtk nkxsm zvcf xvbvt sqfls klmr fjf msvjddz zzkqgl qds txztr dcvrf vjqr jslzv xnmbjh qbmcl thtj rlkrtn qrxs vvn jdfhh lkrf cjljfx xzkpktr nxhrrv nnrspld nnsvvkz nsvbkbz llkth pldq fxkz kdt xzzs kxzdg kzz bvvp nhdjth lknqg pbvx (contains nuts)
ncqls vgmgvd xcv hgvst mhnrqp trl spzzmx ctqch fdq mbcl grqvth vpbrd tljxbd xzkpktr lfmfr nxhrrv ctjcbcls bsmmc pbvx kdt fkrf ktvqj vpxc vmls nnsvvkz tjzhk pszks tp ngjt lfkp qhjvlz zvcf dphvbt znhgh kzz zzkqgl ngvl txztr mgvfmvp sqfls xggzsks qds cvqmhtgg bvjpl hcdchl ghx tzgmld xvpcp dcvrf mpqvcfq mfp nhdjth cs njpg lvxvb bcjz khpx lkrf lvmm (contains fish)
sgb mgvfmvp bvvp nxhrrv qbmcl cpgxmh kzz mpqvcfq zvrqj bsmmc kxzdg ncqls fcdgnm vzg cgbj gz hcdchl gnvrcr lfkp mfdnj mbcl lknqg fkczp spzzmx dcvrf jgqvpj txztr bcjz brvbdk gcssxq sznfj pbfjh qds fklh mfp msvjddz tmkfc mcg pldq njpg mhnrqp ctqch dvkbjh nnkrx dpzv hvcrh tlppq snxph zzlvpcq ndzjn (contains dairy, nuts))";

int main() {
    solution(input);
}
