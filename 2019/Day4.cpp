#include <iostream>
#include <stack>
#include <array>
#include <string>
#include <algorithm>
#include <vector>

constexpr int fact(int n) {
    int total = 1;
    while(n)
        total *= n--;
    return total;
}

constexpr int factup(int x, int m) {
    int total = 1;
    while(m--) {
        total *= (x+m);
    }
    return total;
}

constexpr auto coeffs = []() {
    std::array<int,7*10> ret{};
    for(int n = 0; n < 7; ++n)
        for(int k = 0; k < 10; ++k)
            ret[n*10+k] = factup(n,k)/fact(k);
    return ret;
}();


int multiset_coeff(int n, int k) {
    return coeffs[n*10+k];
}

int remaining(int digits_left, int start) {
    /*
    Calculates combinations of valid(non-decrementing) strings of numbers.
    
    How it works:
    We have `9-start` +1's to divide up amongst our remaining digits.
    Which is the same as taking digits_left cuts into a list of that length
    Which turns out to be the same as the multiset coefficient below
    */
    if(start > 9) {
        return digits_left == 0;
    }
    return multiset_coeff(digits_left+1,9-start);
}

int no_doubles_yet(int start, int digits_left, bool exact_double, int max=9) {
    if(digits_left <= 0) {
        return 0;
    }
    int total = 0;
    for(;start <= max; ++start) {
        //Number of combinations when this starts the first double
        total += remaining(digits_left-1,start+exact_double);
        //Next digit increments, find double starting at that point
        total += no_doubles_yet(start+1,digits_left-1,exact_double);
        if(exact_double) {
            //We have three or more same digits in a row
            for(int digits = 3; digits < digits_left; ++digits) {
                total += no_doubles_yet(start+1,digits_left-digits,exact_double);
            }
        }
    }
    return total;
}

std::vector<int> toRelevantDigits(int x) {
    std::vector<int> ret;
    for(int i = 0; i < 6; ++i) {
        ret.push_back(x % 10);
        x /= 10;
    }
    std::reverse(ret.begin(),ret.end());
    ret.erase(std::is_sorted_until(ret.begin(),ret.end()),ret.end());
    return ret;
}

int double_detect(std::vector<int> digits, bool exact_double) {
    auto it = digits.begin();
    while(it < digits.end()) {
        it = std::adjacent_find(it,digits.end());
        if(not exact_double or it == digits.end()-2 or (it < digits.end()-2 and *it != *(it+2)))
            return it - digits.begin();
        it = std::find_if(it,digits.end(),[&](int i) {return i > *it;});
    }
    return it - digits.begin();
}

void print(std::vector<int> digits) {
    bool first = true;
    for(auto i : digits) {
        if(not first) {
            std::cout << ", ";
        }
        std::cout << i;
        first = false;
    }
    std::cout << '\n';
}

int solution(int start, int end, bool exact_double=false) {
    //Current assumptions:
    // start nor end have doubles in them
    // end only has one relevant digit
    int total = 0;
    auto d1 = toRelevantDigits(start);
    auto d2 = toRelevantDigits(end);
    // We don't care about digits at the start where the two inputs don't differ
    auto startPosition = std::mismatch(d1.begin(),d1.end(),d2.begin(),d2.end()).first - d1.begin();
    
    //startPosition < d1.size --> startPosition < d2.size() bc start < end
    if(startPosition < d1.size()) {
        for(int current = d1.size()-1; current > startPosition; --current) {
            //Get total for this digit until the previous digit increments
            auto digits_left = 6 - current;
            if(d1[current] == d1[current - 1]) {
                //We are at the end of a repeated digit sequence, pop this and run no_doubles_yet on previous digit
                d1.pop_back();
                continue;
            } else if(double_detect(d1,exact_double) != d1.size()) {
                //There is a double before this, so all non-decrementing numbers from this up to 999... are valid passwords
                total += remaining(digits_left,d1[current]);
            } else {
                total += no_doubles_yet(d1[current],digits_left - 1,exact_double);
            }
            d1[current-1]++;
            d1.pop_back();
        }
        //Get total for first differing digit between start and end, ending at (d2[startPosition]*10^(6-startPosition-1) - 1)
        total += no_doubles_yet(d1[startPosition],6-startPosition-1,exact_double,d2[startPosition]-1);
    }
    if(d2.size() > startPosition+1) {
        //Go up x00000 to end starting at the first digit after startPosition
        std::vector<int> d2build{d2.begin(),d2.begin()+startPosition};
        for(int current = startPosition; current < d2.size(); ++current) {
            int start = d2[current-1];
            int goal = d2[current];
            int digits_left = 6 - current;
            d2build.push_back(start);
            auto double_loc = double_detect(d2build,exact_double);
            if(double_loc < current - 1) {
                total += remaining(digits_left-1,9-goal);
            } else if(double_loc == current - 1) {
                total += remaining(6-current-1,start+exact_double);
                if(exact_double) {
                    //Account for triples+ as with implementation of no_doubles_yet
                    for(int digits = 3; digits < digits_left; ++digits) {
                        total += no_doubles_yet(start+1,digits_left-digits,exact_double);
                    }
                }
                d2build.back()++;
            }
            if(double_loc >= current-1) {
                total += no_doubles_yet(d2build.back(),6-current-1,exact_double,goal-1);
            }
            d2build.back() = goal;
        }
    }
    return total;
}

int main() {
    auto test = [](int start, int end) {
        std::cout << "Part 1: " << solution(start,end,false) << '\n';
        std::cout << "Part 2: " << solution(start,end,true) << '\n';
    };
    test(246515,700000);
}
