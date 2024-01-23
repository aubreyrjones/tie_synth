#pragma once

#include <functional>
#include <optional>
#include <tuple>

namespace audio {

/**
 * This class is intended to keep the controls of a synthesizer in sync across multiple different
 * input methods. The Control itself is owned by the synth module and stores the state of the 
 * control value.
*/
template <typename VALTYPE>
class Control {
public:
    using valtype = VALTYPE;

    using update_function = std::function<void(valtype const&)>;

private:
    /// @brief Current authoritative value of the control.
    valtype value;

    /// @brief A flag used to indicate that this control has changed.
    bool _dirty = false;

    update_function onUpdate = [this](valtype v){ };

public:

    /// @brief Construct a Control with no update function (i.e. you'll poll the value when appropriate).
    /// @param initialValue the initial value for the control to take on.
    Control(valtype const& initialValue) : value(initialValue) {}

    /// @brief Construct a Control with an update function called when this Control is dirty and `doUpdate()` is called on it.
    /// @param initialValue the initial value for the control to take on.
    /// @param onUpdate called with the updated value
    Control(valtype const& initialValue, update_function onUpdate) : value(initialValue), onUpdate(onUpdate) {}
    
    /// @brief Construct a Control with an update function and numerical limits.
    Control(valtype const& initialValue, std::tuple<valtype, valtype> const& limits, update_function onUpdate) : value(initialValue), onUpdate(onUpdate) {}

    bool const& dirty() const { return _dirty; }

    void wash() { _dirty = false; }


    /// @brief If dirty, run the update function with the current value and wash the control.
    void doUpdate() {
        if (!_dirty) return;
        onUpdate(value);
        wash();
    }

    /// @brief Set the value.
    /// @param v the new value
    void set(valtype const& v) {
        if (v == value) return;
        value = v;
        _dirty = true;

        // TODO move this to an update step!
        doUpdate();
    }

    /// @brief Get the current value.
    /// @return a const ref to the current value.
    valtype const& get() const {
        return value;
    }

    /// @brief Sugar for get().
    valtype const& operator*() {
        return get();
    }
};


/**
 * Responsible for tracking control state and dirtiness.
*/
class ControlBank {
};

}