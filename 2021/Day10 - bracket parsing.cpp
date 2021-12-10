#include <iostream>
#include <string_view>
#include <array>
#include <variant>
#include <stack>
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

struct Corrupt {
    char illegal;
};

struct Incomplete {
    std::stack<char> remaining;
};

std::variant<Corrupt,Incomplete> parse(std::string_view line) {
    std::stack<char> expected;
    for(auto c : line) {
        switch(c) {
            case '(': expected.push(')'); break;
            case '[': expected.push(']'); break;
            case '{': expected.push('}'); break;
            case '<': expected.push('>'); break;
            default:
                if(expected.empty())
                    return Corrupt{c};
                if(expected.top() == c) {
                    expected.pop();
                } else {
                    return Corrupt{c};
                }
                break;
        }
    }
    return Incomplete{std::move(expected)};
}

void solution(std::string_view input) {
    int score_1 = 0;
    std::vector<int64_t> scores_2;
    split(input,'\n',[&](auto line) {
        auto s = parse(line);
        if(Corrupt* c = get_if<Corrupt>(&s); c) {
            switch(c->illegal) {
                case ')': score_1 += 3; break;
                case ']': score_1 += 57; break;
                case '}': score_1 += 1197; break;
                case '>': score_1 += 25137; break;
            }
        }
        else {
            std::stack<char>& stack = std::get<Incomplete>(s).remaining;
            int64_t total = 0;
            while(!stack.empty()) {
                total *= 5;
                switch(stack.top()) {
                    case ')': total += 1; break;
                    case ']': total += 2; break;
                    case '}': total += 3; break;
                    case '>': total += 4; break;
                }
                stack.pop();
            }
            scores_2.push_back(total);
        }
    });

    std::cout << "Part 1: " << score_1 << '\n';
    std::ranges::nth_element(scores_2,scores_2.begin()+scores_2.size()/2);
    std::cout << "Part 2: " << scores_2[scores_2.size()/2] << '\n';
}

std::string_view input = R"(([<([{{<<(<<{[<><>)}<([][])<<>{}>>>{[{(){}}(()[])]<{()()}<()()>>}>([<<{}<>>(<><>)><[{}[]](<>())>][
({{<<(<<<<{{[({}{})<[]()>]<([]{}){[]<>}>}{([{}[]]}}}>({(((()()))<{<>()}[<>()]>)[{(<>{})(()[])}[{[]
[{{[([<({[([<{[]}({}<>)>]{(([]())[()[]]){<[]<>>[<><>]}}>{<[(()<>){[]<>}][[<>{}]<()()>]>[[[<>[]][()
{(<<{([([<(<{(<>[])<()<>>}>){[[<<>()>[<>[]]]{[{}{}]{[]<>}}]{<{{}<>}{{}()}>{[()()]<[]<>>})}>[{({[{}
<{<<<<<{{{({<<[][]>[()[]]><<(){}>([]{})>}{{<[]{}><{}[]>}{<{}{}>({}<>)}}){([(()[]){[]()}][<[]()><[]{}>])
<{({{<{{{<(({(<>)({}[])}{<()>{(){}}})[[(<><>)[[]]]<[{}[]]{<>()}>])>{<<([{}()]{[][]})({{}{}}{()})><
<([[<<[{[{({{({}<>)[(){}]}{[{}<>]{[]{}}}}({[()()]<<><>>}(<{}[]><[]{}>)))}<({<<[]<>>({}[])>
[[{<{{[{<{<[{[()[]]<()()>}][<(<>[]){<>()}>[{<><>}(<>())]]><[{{{}<>}<{}{}>}{[{}[]][[][]]}]<[([]<>){[]{}}]<(
[({[<([[{<[{{(()[])<[]{}>}<[{}<>>{[][]}>}(<<[]{}>([]<>)>[([]){()[]}])]<[[[[]()][<>{}]]<(<>{})((
<{[[{{{[([([{[<>()]{[]{}}}<{[][]}[<>()]>](<<<><>>{()()}>))<<{{<><>}[{}()]}{(<><>)[<>{}]}>>])]{[<<[<([]
[[(<<<({[(((([<>()]([]{}))((<>){[]()})})[[<[[]<>]<{}<>>>[<()()>[()<>]]][[[<>{}]{[][]}]]])<<(({{}[
{{{[[<<{{[{<[[{}()]]>[{[<>[]]([][])}<{{}()}{{}[]}>]}([[{()}{(){}}]([<>{}]<{}<>>)][{{[]<>}}[{{}{}}[{
{[{[(<{{([{<<<[][]><()<>>>[{{}()}{[]<>}]>({<{}()>[()[]]}<<()[]>([][])>)}{[([[]{}][<><>]){{{}[]}<
{[({<{[<{{[(({()<>}({}[]))([()]{<>[]}))({{[]{}}<<>[]>>)]{((<()[]>{(){}})[{[]<>}])}}}<[[<<<(){}><()
[[[[[{{[{<<[<<{}<>>>{<()()><{}[]>}]>(<([()<>}<<>[]>)(((){})<()[]>)><({{}{}}<[]()>)<{{}<>}(
{{<<[<{(<[([(<{}<>>[[][]])[(()())]]{[([][])({}<>)]<(<>{}){[][]}>})([{(<>{})<()<>>}])]>(([[[(<>{})](<[]
<<([{<(<{({<{({}<>)<()()>}{<<>[]>[{}{}]}>(<((){})>(<[]()>[<><>]))}<<(([])[()])<([][])(()<>)>>>)([{([<>{}]([]<
{[[{{[[({{[([{<>[]}[{}[]]]){(<[]{}>[()()]){[<>{}][<>]>}]}{<{<<{}()>({}())>}{[[<>[]]{<>{}}]<[{}[]]{{}{}}>}
{<[[{({<{<[<(<(){}><<>{}>)>{[[(){}]]{({}())({}[])}}]{[{{(){}}}]}>}(<{{{([]{})[{}<>]}(<<>{}>)}(
<[[<(<([[({(<{<>()}([][])><(<>[])[<>{}]>)((<<>()>[[]()])<{()<>}<()()>>)}>[[({{{}<>}[[]<>]}
<{([{({[[({([[{}()]<()()>][<()[]>[[]<>]]}([<(){}>{<><>}][<{}<>><<>{}>])}[(([<>[]][<>[]])){
<{[<(<(<[[[{[{<>{}}]((())({}()))}][(([{}<>]{{}{}}))[{[[]()]((){})}<[<>[]]([][])>]]]][[((({
({[<<([[[<[[<(<>{}){{}[]}>[((){})[<><>]]][{{<>{}}{(){}}}(<[]{}}({}<>))]]{((<{}[]>({}<>))<<<>{}>[[]{}]>)[((
{[<<[([[([{<<[{}{}]({}[])><<{}<>)(<>[])>>[([()()]{<><>}){[[]{}]{(){}}}]}{<([<>]([]())){<(){}>{()<>}}><<{
[<[[[[({{([{{(<>[])<<><>>}<{[]{}}<<>>>}(<{[]{}}<<>()>>)][{(<<><>><()[]>){[{}()]>}<[{{}{}}{<>[]}]>]
{<{[<(([{{({[<<>[]><{}[]>]<{<>{}}{()()}>}([<{}{}>{[]<>}]{<{}<>><[]>}))<([([]<>){[]{}}]<{[]<>}
[<<(({<<[{[([[<>]<()[]>]{[[]<>]{()<>}}){{<<><>>{[]<>]}[({}<>){()<>}]}](<{{()[]}}({{}[]}<{}[
({(<{<{<([([[((){})<{}<>>]<[{}[]](<>{})>])])<<[{<<<>[]>>}<[(<><>){<>{}}][(()[])(<>{})]>]<(<{<>{}}
([<{<<[([({<<<()()>(())><[[]<>]{<>[]}>>([(()())]{[[]()]<{}<>>})})[<[[(()<>){<>{}}]][<(<><>){<>[]}>]><<([{}
([[<[[[({[<<[[{}()](()<>)]<{()[]}>>{<[{}<>]<()[]>><<[][]>>}>]<(<<[{}<>){<>}>{<[]()>[[][]]}>)>}({
(<[(<[([<{<{<[(){}][<>()]><(<>())(()<>)>}{{[{}>([]<>)}[<<>{}>(()())]}>[{([<><>]<()()>)}{([()]{()<
({{{{<[{{<{<<(()[])>{[(){}]{[]{}}}>}{{([<><>][<><>])<(()<>)[[][]]>}<<(()<>)><{<>()}>>}>}}][(<{(<[{<><>}[{}[
<{[{((([({{({<()()>(())}){([()<>]((){}))<(()<>)[(){}]>}}{[{([]())((){})}[{[]<>>{<>{}}]]<<<()[]><()<>>>((()()
{([({[(<[<([{[<>[]][()<>]}{[(){}]{{}<>}}][<<{}()>>[<<>()>[()}]])<((({}{})[[]<>]))<{[[]{}]([]<>)}>>><{
([[[{<[<([{(<<<>()>({}<>)><[<>{}]>)[[<<><>><()[]>](<()<>>(<>[]))]}([({()()}([]())){<[]{}>[[]<>]}][<
<<((<({{(<((<({}[])(<>())>{({}[])({}<>)}))<<<([]<>)[()[]]>{<()<>>[[]<>]}>>)({(<[<>{}]((){})><{()()}[{}(
{({([(({{<{{{<[]()>{{}[]})[[(){}](()<>)]}(([[]{}]{()<>}))}>{(<[{()()}]{[<>[]][()()]}>{{<()()
{[(<{<[<<[{<<<<>{}>>[[[]{}]<<>()>]>((<[]{}>{{}()}))}{({[{}()](()<>)}<<[][]>[{}<>]>)({[{}[]]}<<{}[]
{{{<{<{[[{([{{[]()}}({()[]}(<>))]<((<><>)({}{}))>)}]]{[({<(<{}[]>{<>{}})((()<>)([]()))>})[<[[{<>{}}<<><>>]<{<
[[<{<<[(<<<([<{}()>[{}()]](<{}[]>[{}()]))[([[]<>][{}{}])[<[]()><[]()>]]><[[[[]<>]({}{})][{[]}{<>[]}]][<[<>
(<<<<(<<[<[(({[]()}[[]<>]))({(())(()[])}[<<>[]>{()[]}])]{<({{}{}}(<>{}))><[[<>[]]<[]()>]>}>{({{(<>()){
<<(<[(<<<{({[<()>{()()}][([]{})([][])]}{({{}}{<>{}})<(())[{}()]>})}[[{<[[][]]{()}){[{}<>]<{}(
<<{<{{[<[(({({[][]}<<><>>)}<((<><>)[{}[]])<<[]<>>>>)[{{[()[]][[]<>]}{({}())<(){}>}}])({[([[]<>](<><>))<
<<{<[(<(<([[{{()()}{<>()}}(<<>()>({}[]))]<<{{}{}}{()<>}>(({})(()()))>])[[([(<>[])(<>)](<(){
({[((({[([<{[{<><>}[()[]]][{[][]}{(){}}]>>(<{{[]{}}<()[]>}<{()<>}<[]()>>>[{({}<>)<{}()>}])])]<(<{{{<[]()
<[{<<{<<{<(({[<><>]{{}<>}})){<[{<>[]}({}{})]([{}()]{<>()})>[<{<><>}<<>{}>}([<>[]]<[]()>)]}>(
(<<(({<<(<(({[<>[]]<[][]>}({<>()}[[]()])){((<>[])){([]{}){[][]}}}){{[[[]{}][{}[]]][<{}[]>([]<>)]}[(({
<(<(<<<[<{({([<>()])[[[][]]{<>[]}]}[<[()]<{}<>>><[(){}]<()()>>])((<(()[])<{}{})>)((([][]))))}>]<[<([[<<>()
{{<[<<<{((([[([]{})][[<>[]]]])(<[[(){}]<<>[]>]>{[({}())([]<>)][<<>{}><[]()>]}))){{<[[[[][]
{([<{[<<[{({({<>{}}[[][]])[{(){}}{{}()}]}[[[{}()]<()()>]])({<[{}{}]>({{}{}}[[]{}])}{{({}())<()()>}{{<><>}<[]
[<{[{<{{<{<[{{{}{}}[[]()]}([{}[]]<{}()>)](<{()()}{<>[]}><(<>[])>)>}{{{((()<>)[<><>])[[()()]
([{[<[({([<<<([][])<{}{}>>[[()<>](<>{})]>{{({}<>)}([<><>]<()<>>)}>](<(<<[]<>>({}[])>[<<>{}>[()()]])(<<()[]>((
(<[{<({<([<({<<>()><{}()>}(<[]{}>({}<>)))[([[]()][<><>>){[<>[]]{{}<>}}]>({[{[]{}}({}[])]<{[
<<[<(<({<(<({<()><[]()>})>{[{[[]()]<{}>}<(()<>)(<>())>)(((()())[<><>])([(){}]))})([(<{<>()}([]())>{[{}{
[{<{<{<[<((({[()[]]{{}}}[[<><>]<<>{}>])<({[][]}{()[]})<([]())([]{})>>))((<([{}<>]{(){}})>[
[[[[([<{[[(<{{{}[]}[<>()]}<{[]}{<><>}>><{({}<>){()<>}}[{[]{}}{<>[]}]>)[([({}<>}([]<>)]{{{}{}
<{{[[<{[(({({<{}>[(){}]}(<[]()>))}<<<[{}{}]>(<[]{}>(<>[]))>([{{}<>}(()())]<{<><>}{<>()}>)>)<[<<
{[{{{(<<([([((()[])[[]<>])({{}{}}({}<>))]<({{}[]}{[][]})<{[][]}>>){<<({})[[]<>]>(<[][]>(<>{}))>}]<[
<[<(<{<[([[[{[()[]][<>{}]}[{(){}}{<><>}]]{<([]{}){()<>}>[({}<>)<<>[]>]}][<[({}[]){()[]}][<<>{}><()<>>]>]]<(
([<{(([[<<{([([]{})([]())]<{(){}}{[]<>}>)([<<>{}>[()()]]{[[]{}]<[]<>>})}[({{()()}<<>{}>>){[<()
([({{{[[[([{<{<><>}{()()}>(<[]())[<>])}{([<><>]<(){}>){(<>())[{}()]}}]<{[[<><>]{{}[]}]([{}<>]{[]()
[[<<<{{({[{[({{}{}}<(){}>)<[[]{}]({}<>)>]}][(<{<<>{}>}{{[]{}}{()[]}}>[{[<>{}]<[][]>}<{<><>
[([{(([{(([{[{[]()}[{}]][[[]<>]{<>{}}]}{[<{}{}>({}[])][<<><>>(()<>)]}]{<({[]{}}{<>{}}}>}){
(<<<<<[{<{({({[]{}}({}())){([][])[{}{}]}}([{<><>}{{}{}}][[{}[]]({}{})]))[[[{{}()}[<>{}]][{{}<>}{()}]
(([{<(<[(<{{{[[]<>](<>[])}{[()<>]<()()>}}}>)<{{({<{}<>>[{}{}]}({<>[]}[()()]))}([([<>[]][[]{}])(<<><>
((<{[(([{{([<[<>()]<<>[]>>{((){})([])}][<{[]()}<<>()>>(<{}()>(<>))])}[{{{{[]()}}{(<>[])[()<
<[[{{[<<({[<<{(){}}{(){}}>>(<{[]<>}<<>()>>({()<>}{{}<>}))]{[([[][]][{}<>])][((<>{})(()[]))[<()[]>{<>()}]]}
{<{<<[{[([{{{{{}()}<()<>>}<[()[]]>}<[<[]<>><()<>>](({}())<{}>)>}<<({[]()}(()()))({{}<>}{{}<>
{{[<{[<{[[{{({[]<>}<{}[]>)((<>{})({}{}))}([[[]{}]<{}[]>]<({}()){()<>}>)}[<([{}<>}<<><>>)><{[()[]][<>[]]}{
([{{{[{(<<<[<{<>[]}{()[]}>(([]())<<>{}>)]>[[(<{}()><[]<>>)][{[[][]]<(){}>}<<()()>({}[])>)]>>)<<[[(<(<>())((
<({[{{(({(<<<{[]()}(<>{})>{(()<>)[[]<>]}><[[<>()]]{([]{})[[]()]}>>)})({[{<(<[]<>>{{}[]])[<<>{}>[(){}]]><(({}(
((<(([{([{<[{<[]()>(()[])}<({})[<>[]]>]>}]{[<<{[[]()]<<>()>)<<{}[]>{<><>}>>>[[([(){}](<>[]))]{{<
[([<<([(<[<<([[]{}]<(){}>)([()[]][{}{}])>([([]<>)<(){}>]([[][]]{[]<>}))>{{[{[]<>}[{}[]]]{([]
[<{<[[<{<({<[<()<>><[]<>>][<{}<>><<>()>]><{((){})(()())}>}{[([{}<>]({}{})){(<>{})}]([[<><>](<>)])})><{<{
[(<[[<({{<<([<(){}><<><>}]<<{}()>({}<>)>){[(<>())[(){}]]<<<><>>[{}<>]>}><{[{(){}}][<{}{}>[()(
<[<{[{{<([{<[{()<>}{()<>}]]{<[{}<>](<>{})>([{}<>]{{}()})}}<[([(){}](()[]))<{{}{}}>]{{{<>}<[
(<{[[{[[{[[(<[[][]][<>{}]>(<()<>>(<>{})))[{([]{}}<{}[]>}[[[]{}]{()()}]]]{<[<<>{}>(()<>)]<{{}<>}<[
(<<<[[(<{<({{(<>())<(){}>}<(()())<()[]>>}<[({}{})[()[]]]>)>}{<{[<[()<>][<>[]]>[([]{})(()())]][({{}{}}
({({[[[(<<[{<<<>[]][{}()]>(({}())([]()))}]{<{[()[]]({}())}>({<()[]><[]{}>}[{<>()}<[]{}>])}>>)]]]}[({[[{({(((
((([[<{[(<({<{<><>}[[]]>[{(){}}>}((<()<>>{()()})<<<>()>({}())>))((([<>()]{[][]})[[(){}]<{}
<[{([(<[({[[([{}()]({}{}))([<><>]({}()))](({(){}}{()[]}><[[]()]<<><>>>)](<([(){}][{}{}])[{
{<((({<((<[[<{()[]}<{}{}>>](<<[]>{<>}><[[]{}]({})>)]<<{[{}{}]}[{<>{}}}>>><(([<<>>(()())]<(
{[[[<<(<{(<(<[{}<>]<[]()>>[{{}{}}{()<>}])[<{{}()}{{}{}}><{[]<>}({}())>]>)<<(<<()<>><<><>>>)>>}[(
[<{<[([({([[[{{}()}{{}()}]([[]()])]])}<<<[<<<>{}>[<><>)><{[]<>}(()<>)>](({{}<>}[()<>]){{<>[]}((){})})>({(
([[{{[[[<(<({(()())<{}[]>}<({}<>){[][]}>)[<{()()}[[]<>]>]>)([[({<>[]}[()()])[{[]<>}[{}{}]]][[<
({[(<<<{((<{<[()()][[]<>]>{<{}<>><()()>}}<{[()[]]([]<>)}<<{}()><{}{}>>>>[<<{(){}}[()[]]>[([]<
([({{{<[{[{{[([]{})<[]{}>][<<>[]>[{}[]]]}}[{[<<><>>{[][]}]((<>()){<>{}})}]]]][[(<({[[]()][[]{}]
(((<{{<<<([[{(<><>)<{}>}<{{}{}}[{}<>]>]{[(()())[{}<>}]}]{<(<[]()>)>([<()[]>{{}()}]<({}()){(){}}>)})<[({(<>()
[{(([[<(<<(<{<[]()>(<>{})}[{()[]}[()()]]><[<[]()>[[]<>]]>)<{<[[]<>]<()<>>>{[()[]]<{}[]>}}>>([((<[][
{<{<{[([(<<[{<()<>>(<>[])}[(<><>)[[]<>]]]<<<[]<>><(){}>>{{{}<>}<<><>>}>>(<{{<>()}[()<>]}><[)";

int main() {
    solution(input);
}