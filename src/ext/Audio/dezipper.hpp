#pragma once

#include <optional>

namespace dezip {

template <typename T>
class Value {
    T *value;
    std::optional<T> target;

    Value(T & v) : value(&v), target() {}

    
};

}