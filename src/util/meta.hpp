#pragma once

#include <array>
#include <tuple>

namespace meta {

template <typename T>
using StaticVector = std::tuple<const T*, size_t>;

template <typename T, size_t N>
constexpr StaticVector<T> length_erase_array(std::array<T, N> const& a) {
    return StaticVector<T> { &a[0], a.size() };
}

}