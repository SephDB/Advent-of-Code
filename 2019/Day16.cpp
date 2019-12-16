#include <array>
#include <iostream>
#include <iterator>
#include <vector>
#include <cassert>
#include <numeric>
#include <string_view>

template<typename It>
It safe_next(It begin, int size, It end) {
    if(std::distance(begin,end) < size) {
        return end;
    }
    return std::next(begin,size);
}

template<typename It>
int fft(int size, It begin, It end) {
    int total = 0;
    int current_multiplier = 1;
    while(begin != end) {
        //Add up all (-)1 digits
        It chunk = safe_next(begin,size,end);
        total += current_multiplier * std::accumulate(begin,chunk,0);
        //Skip zero digits
        begin = safe_next(chunk,size,end);
        current_multiplier *= -1;
    }

    return std::abs(total) % 10;
}

template<typename It>
void fft_latter_half(It rbegin, It rend) {
    std::partial_sum(rbegin,rend,rbegin,[](auto acc, auto next) {return (acc + next) % 10;});
}

template<typename Arr>
Arr fft(Arr a) {
    for(int phase = 0; phase < 100; ++phase) {
        for(int current_digit = 0; current_digit < a.size()/2; ++current_digit) {
            //fft can start at current digit since all before are 0, hence we can do this in-place
            a[current_digit] = fft(current_digit+1,a.begin()+current_digit, a.end());
        }
        //Second half is a reverted partial_sum
        fft_latter_half(a.rbegin(),a.rbegin()+a.size()/2);
    }
    return a;
}

template<typename It>
auto to_num(It begin, int num_digits) {
    return std::accumulate(begin,begin+num_digits,0,[](auto acc,auto next){return acc*10+next;});
}

int main() {
    constexpr std::string_view input{"59775675999083203307460316227239534744196788252810996056267313158415747954523514450220630777434694464147859581700598049220155996171361500188470573584309935232530483361639265796594588423475377664322506657596419440442622029687655170723364080344399753761821561397734310612361082481766777063437812858875338922334089288117184890884363091417446200960308625363997089394409607215164553325263177638484872071167142885096660905078567883997320316971939560903959842723210017598426984179521683810628956529638813221927079630736290924180307474765551066444888559156901159193212333302170502387548724998221103376187508278234838899434485116047387731626309521488967864391"};
    std::array<int,input.size()> ds;
    std::transform(input.begin(),input.end(),ds.begin(),[](char c) {return c - '0';});

    auto fft_ds = fft(ds);
    std::cout << "Part 1: " << to_num(fft_ds.begin(),8) << '\n';

    std::vector<int> next(input.size()*10000);
    for(int i = 0; i < next.size(); ++i) {
        next[i] = ds[i%ds.size()];
    }

    auto loc = to_num(next.begin(),7);
    assert(loc >= next.size()/2);
    auto rend = next.rbegin() + (next.size() - loc);
    for(int phase = 0; phase < 100; ++phase) {
        fft_latter_half(next.rbegin(),rend);
    }
    std::cout << "Part 2: " << to_num(next.begin()+loc,8) << '\n';
}
