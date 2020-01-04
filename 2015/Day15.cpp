#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <span>
#include <unordered_map>
#include <charconv>
#include <numeric>
#include <algorithm>
#include <span>

std::vector<std::string_view> split(std::string_view in, char delim) {
    std::vector<std::string_view> ret;
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        ret.push_back(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    ret.push_back(in);
    return ret;
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

namespace detail {
    template<typename... Tuples>
    using tuple_type = std::remove_reference_t<decltype((std::declval<Tuples>(),...))>;

    template<typename... Tuples>
    constexpr auto common_length = std::tuple_size<tuple_type<Tuples...>>::value;

    template<std::size_t I, class F, class... Tuples>
    constexpr auto invoke_nth(F&& f, Tuples&&... ts) {
        return std::forward<F>(f)(std::get<I>(std::forward<Tuples>(ts))...);
    }

    template <class F, std::size_t... I, class... Tuples>
    constexpr auto zip_apply_impl(F&& f, std::index_sequence<I...>, Tuples&&... ts)
    {
        return std::array{invoke_nth<I>(std::forward<F>(f),std::forward<Tuples>(ts)...)...};
    }
}

template <class F, class... Tuples>
constexpr auto zip_apply(F&& f, Tuples&&... ts)
{
    return detail::zip_apply_impl(std::forward<F>(f), std::make_index_sequence<detail::common_length<Tuples...>>{},
        std::forward<Tuples>(ts)...);
}

template<int N>
struct Vec {
    Vec(std::array<int,N> v) : values(v) {}
    Vec() = default;
    std::array<int,N> values;
    Vec operator+(Vec other) const {
        return zip_apply(std::plus{},values,other.values);
    }
    Vec& operator+=(Vec other) {
        *this = *this + other;
        return *this;
    }
    Vec operator-(Vec other) const {
        return zip_apply(std::minus{},values,other.values);
    }
    Vec& operator-=(Vec other) {
        *this = *this - other;
        return *this;
    }
    Vec operator*(int i) {
        return zip_apply([&](int x) {return x * i;},values);
    }
    Vec& operator*=(int i) {
        *this = *this*i;
        return *this;
    }
    void min_clamp(int n) {
        *this = zip_apply([=](int i)->int {return std::max(i,n);},values);
    }
    int& operator[](int i) {
        return values[i];
    }
    int operator[](int i) const {
        return values[i];
    }
    Vec abs() const {
        return zip_apply([](int i){return std::abs(i);},values);
    }
    friend Vec max(Vec a, Vec b) {
        return zip_apply([](int a, int b)->int{return std::max(a,b);}, a.values, b.values);
    }
};

auto parse(std::string_view in) {
    std::vector<Vec<5>> ret;
    for(auto l : split(in,'\n')) {
        Vec<5> ing;
        auto current = 0;
        for(auto in : split(l,',')) {
            ing[current++] = to_int(split(in,' ').back());
        }
        ret.push_back(ing);
    }
    return ret;
}

bool oppositeSigns(int x, int y) 
{ 
    return (x^y) < 0; 
}

int part2 = 0;

std::size_t part1(std::span<Vec<5>> in, Vec<5> total = {}, int remaining = 100) {
    if(in.size()==1 or remaining == 0) {
        total += in.front()*remaining;
        total.min_clamp(0);
        auto res = std::accumulate(total.values.begin(),total.values.end()-1,1,std::multiplies{});
        if(total[4] == 500) {
            part2 = std::max(part2,res);
        }
        return res;
    }
    int start = 0, end = remaining;
    auto current = in.back();
    in = in.first(in.size()-1);
    auto max_v = std::accumulate(in.begin()+1,in.end(),in.front(),[](Vec<5> a, Vec<5> b) -> Vec<5> {return max(a,b);});
    for(int i = 0; i < 4; ++i) {
        if(not oppositeSigns(max_v[i],current[i])) continue;
        //whether max_v or current is < 0, boundary gives the amount of current that's allowed(or needed, respectively) to make sure total > 0
        auto boundary = (total[i]+remaining*max_v[i])/(max_v[i]-current[i]);
        if(current[i] < 0) {
            //x < (current+remaining*max)/(max-xval)
            end = std::min(end, boundary);
        } else {
            start = std::max(start,boundary);
        }
    }
    std::size_t max_result = 0;
    while(end >= start) {
        max_result = std::max(max_result,part1(in,total+current*end,remaining-end));
        end--;
    }
    return max_result;
}
void solution(std::string_view input) {
    auto ingredients = parse(input);
    std::cout << "Part 1: " << part1(ingredients) << '\n';
    std::cout << "Part 2: " << part2 << '\n';
}

std::string_view input = R"(Sprinkles: capacity 2, durability 0, flavor -2, texture 0, calories 3
Butterscotch: capacity 0, durability 5, flavor -3, texture 0, calories 3
Chocolate: capacity 0, durability 0, flavor 5, texture -1, calories 8
Candy: capacity 0, durability -1, flavor 0, texture 5, calories 8)";

int main() {
    solution(input);
}
