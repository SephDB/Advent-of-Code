#include <iostream>
#include <array>
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>

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

struct index_pair {
    int a = 0;
    int b = 0;
};

template<int N>
constexpr auto all_pairs() {
    std::array<index_pair,(N*(N-1))/2> ret;
    int current = 0;
    for(int i = 0; i < 3; ++i) {
        for(int j = i+1; j < 4; ++j) {
            ret[current++] = {i,j};
        }
    }
    return ret;
}

struct Vec3D {
    Vec3D(std::array<int,3> v) : values(v) {}
    std::array<int,3> values;
    Vec3D operator+(Vec3D other) const {
        return zip_apply(std::plus{},values,other.values);
    }
    Vec3D& operator+=(Vec3D other) {
        *this = *this + other;
        return *this;
    }
    Vec3D operator-(Vec3D other) const {
        return zip_apply(std::minus{},values,other.values);
    }
    Vec3D& operator-=(Vec3D other) {
        *this = *this - other;
        return *this;
    }
    Vec3D& operator*=(int i) {
        values = zip_apply([&](int x) {return x * i;},values);
        return *this;
    }
    Vec3D& normalize() {
        values = zip_apply([](int i) {return i < 0 ? -1 : (i > 0 ? 1 : 0);},values);
        return *this;
    }
    Vec3D abs() const {
        return zip_apply([](int i){return std::abs(i);},values);
    }
    int energy() const {
        auto absolute = abs();
        return std::accumulate(absolute.values.begin(),absolute.values.end(),0);
    }
};

std::ostream& operator<<(std::ostream& o, Vec3D v) {
    bool first = true;
    o << '<';
    for(auto i : v.values) {
        if(not first) {
            o << ',';
        }
        o << i;
        first = false;
    }
    o << '>';
    return o;
}

struct Moon {
    Moon(Vec3D pos) : position(pos) {};
    Vec3D position;
    Vec3D velocity = {{0,0,0}};
    int total_energy() const {
        return position.energy()*velocity.energy();
    }
};

int main() {
    std::array<Moon,4> moons = {
        Vec3D{{5, 13, -3}},
        Vec3D{{18, -7, 13}},
        Vec3D{{16, 3, 4}},
        Vec3D{{0, 8, 8}}
    };
    auto start = moons;
    constexpr auto pairs = all_pairs<4>();
    std::array<std::int64_t,3> equals_found{0,0,0};
    std::int64_t total = 0;
    do {
        for(auto [a,b] : pairs) {
            auto vdelta = (moons[b].position - moons[a].position).normalize();
            moons[a].velocity += vdelta;
            moons[b].velocity -= vdelta;
        }
        for(auto& moon : moons) {
            moon.position += moon.velocity;
        }

        if(++total == 1000) {
            std::cout << "Part 1: " << std::accumulate(moons.begin(),moons.end(),0,[](int total, Moon m) {return total + m.total_energy();}) << '\n';
        }

        for(int i = 0; i < 3; ++i) {
            if(equals_found[i] != 0) continue;
            auto eq = zip_apply([i](Moon a, Moon b) {
                return a.position.values[i] == b.position.values[i]
                   and a.velocity.values[i] == b.velocity.values[i];},start,moons);
            if(std::all_of(eq.begin(),eq.end(),std::identity{})) {
                equals_found[i] = total;
            }
        }

    } while(total <= 1000 or std::any_of(equals_found.begin(),equals_found.end(),[](int i) {return i == 0;}));
    std::cout << "Part 2: " << std::accumulate(equals_found.begin()+1,equals_found.end(), equals_found[0], [](auto acc, auto next) {return std::lcm(acc,next);});
}
