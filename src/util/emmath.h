#pragma once

#include <cstdint>
#include <cmath>
#include <tuple>

using byte = uint8_t;

namespace em {
template <typename T = int>
class range {
public:
    class iterator {
        friend class range;
    public:
        T operator *() const { return i_; }
        const iterator &operator ++() { i_ += incr; return *this; }
        iterator operator ++(int) { iterator copy(*this); i_ += incr; return copy; }

        bool operator ==(const iterator &other) const { return i_ == other.i_; }
        bool operator !=(const iterator &other) const { return i_ != other.i_; }

    protected:
        int incr;
        iterator(T start, int incr = 1) : i_ (start), incr(incr) {}

    private:
        T i_;
    };

    iterator begin() const { return begin_; }
    iterator end() const { return end_; }
    range(T begin, int count) : begin_(begin, count < 0 ? -1 : 1), end_(begin + count) {}
private:
    iterator begin_;
    iterator end_;
};

template <typename T>
struct vec {
    using val_type = T;
    using r = range<T>;

    T x, y;

    bool operator==(vec const& o) const {
        return x == o.x && y == o.y;
    }

    vec operator+(vec const& o) const {
        return vec { x + o.x, y + o.y };
    }

    vec operator-(vec const& o) const {
        return vec { x - o.x, y - o.y };
    }
};

using ivec = vec<int>;


template <typename L, typename T, typename H>
T clamp(L const& low, T const& val, H const& high) {
    if (val < low) return low;
    if (val > high) return high;
    return val;
}

template <typename L, typename T, typename H, typename I>
T clamp_incr(L const& low, T const& val, H const& high, I const& increment) {
    return clamp(low, val + increment, high);
}

inline int center(int item, int space) {
    return (space / 2) - (item / 2);
}

/// @brief Convert complex number to polar representation.
inline std::tuple<float, float> to_polar(std::tuple<float, float> const& z) {
    using std::get;
    float a = get<0>(z), b = get<1>(z);

    float r = sqrtf(pow(a, 2) + pow(b, 2));
    float theta = atan2f(b, a);
    return {r, theta};
}

inline std::tuple<float, float> to_cartesean(std::tuple<float, float> const& p) {
    using std::get;
    float r = get<0>(p), th = get<1>(p);

    float real = r * cosf(th);
    float im = r * sinf(th);

    return {real, im};
}

inline float exp_incr(float curVal) {
    if (abs(curVal) < 1) return 1.f;
    return copysign(pow(curVal, 2), curVal);
}

}