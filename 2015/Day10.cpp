#include <iostream>
#include <string>
#include <algorithm>

int next_length(const std::string& s) {
    int ret = 1;
    std::adjacent_find(s.begin(),s.end(),[&](char a, char b) {ret += (a != b); return false;});
    return ret*2;
}

std::string next(const std::string& s) {
    std::string ret;
    int count = 1;
    char current = s[0];
    for(int i = 1; i < s.size(); ++i) {
        if(s[i] == current) {
            ++count;
        } else {
            ret.push_back('0'+count);
            ret.push_back(current);
            count = 1;
            current = s[i];
        }
    }
    ret.push_back('0'+count);
    ret.push_back(current);
    return ret;
}

auto apply(std::string number, int iterations) {
    for(int i = 0; i < iterations-1; i++) {
        number = next(number);
    }
    return next_length(number);
}

namespace HeapLess {
    enum class IterState {COUNT,CURRENT,END};
    template<typename I>
    struct Iter {
        I begin,end;
        IterState state;
        int count;
        char c;
        Iter& operator++() {
            switch(state) {
                case IterState::COUNT:
                    state = IterState::CURRENT;
                    break;
                case IterState::CURRENT:
                    if(begin == end) {
                        state = IterState::END;
                        break;
                    }
                    c = *begin;
                    count = 0;
                    while(begin != end and *begin == c) {++count; ++begin;}
                    if(count == 0) {
                        state = IterState::END;
                    } else {
                        state = IterState::COUNT;
                    }
                    break;
            }
            return *this;
        }
        char operator*() const {
            switch(state) {
                case IterState::COUNT:
                    return '0'+count;
                case IterState::CURRENT:
                    return c;
            }
            throw;
        }
        bool operator==(Iter o) const {return begin == o.begin and state == o.state;}
        bool operator!=(Iter o) const {return !(*this == o);}
    };
    template<typename I>
    Iter(I,I,IterState) -> Iter<I>;
    
    template<int N>
    struct Seq;

    template<>
    struct Seq<0> {
        Seq(std::string_view a) : s(a) {};
        std::string_view s;
        auto begin() const {
            return s.begin();
        }
        auto end() const {
            return s.end();
        }
    };

    template<int N>
    struct Seq {
        Seq(std::string_view a) : next(a) {};
        auto begin() const {
            return ++Iter{next.begin(),next.end(),IterState::CURRENT};
        }
        auto end() const {
            return Iter{next.end(),next.end(),IterState::END};
        }

        Seq<N-1> next;
    };

    template<int N>
    int size(std::string_view a) {
        int count = 0;
        for(auto c : Seq<N>(a)) ++count;
        return count;
    }
}

int main() {
    std::cout << "Heapless: " << HeapLess::size<15>("1113122113") << '\n'; //About the biggest that compiles in a reasonable time
    std::cout << "Part 1: " << apply("1113122113",40) << '\n';
    std::cout << "Part 2: " << apply("1113122113",50) << '\n';
}
