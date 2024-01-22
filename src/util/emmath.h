#pragma once

#include <cstdint>

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
T& clamp_incr(L const& low, T & val, H const& high, I const& increment) {
    val = clamp(low, val + increment, high);
    return val;
}

inline int center(int item, int space) {
    return (space / 2) - (item / 2);
}

}