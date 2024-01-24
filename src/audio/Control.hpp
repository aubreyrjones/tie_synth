#pragma once

#include <functional>
#include <optional>
#include <tuple>
#include <util/emmath.h>

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

    using limits_t = std::tuple<valtype, valtype>;

private:
    /// @brief Human-readable name for this control.
    const char* _name = "?";

    /// @brief Current authoritative value of the control.
    valtype value;

    /// @brief A flag used to indicate that this control has changed.
    bool _dirty = false;

    /// @brief Update function.
    std::optional<update_function> onUpdate;

    /// @brief Limits for the value.
    std::optional<limits_t> limits;

public:

    /// @brief Construct a Control with no update function (i.e. you'll poll the value when appropriate).
    /// @param initialValue the initial value for the control to take on.
    Control(const char* name, valtype const& initialValue) : _name(name), value(initialValue) {}

    /// @brief Construct a Control with an update function called when this Control is dirty and `doUpdate()` is called on it.
    /// @param initialValue the initial value for the control to take on.
    /// @param onUpdate called with the updated value
    Control(const char* name, valtype const& initialValue, update_function onUpdate) : _name(name), value(initialValue), onUpdate(onUpdate) {}
    
    /// @brief Construct a Control with an update function and numerical limits.
    Control(const char* name, valtype const& initialValue, std::tuple<valtype, valtype> const& limits, update_function onUpdate) : _name(name), value(initialValue), onUpdate(onUpdate), limits(limits) {}

    const char* const& name() { return _name; }

    /// @brief has this control been changed since the last call to doUpdate()?
    bool const& dirty() const { return _dirty; }

    /// @brief Removes the dirty flag.
    void wash() { _dirty = false; }

    /// @brief Add or subtract the given amount from the value, checking for under/overflow before calling set().
    /// @param positive true if the delta should be added, false if the delta should be subtracted
    /// @param delta the amount to change the value
    void adjust(bool positive, valtype delta) {
        valtype newVal = value;
        if (positive) {
            newVal += delta;
        }
        else {
            newVal -= delta;
        }
        
        if (positive && newVal < value) { // overflow
            newVal = std::numeric_limits<valtype>::max();
        }
        else if (!positive && newVal > value) { // underflow
            newVal = std::numeric_limits<valtype>::lowest();
        }

        set(newVal);
    }

    /// @brief If dirty, run the update function with the current value and wash the control.
    void doUpdate() {
        if (!_dirty) return;
        if (onUpdate) 
            onUpdate.value()(value);
        wash();
    }

    /// @brief Set the value.
    /// @param v the new value
    void set(valtype const& v) {
        using std::get;

        if (v == value) return; // same value, so don't dirty ourselves just bail.

        if (limits) { // we're limited, so limit it.
            value = em::clamp(get<0>(limits.value()), v, get<1>(limits.value()));
        }
        else {
            value = v;
        }

        _dirty = true;

        // TODO move this to a separate update step!
        doUpdate();
    }

    /// @brief Get the current value.
    /// @return a const ref to the current value.
    inline valtype const& get() const {
        return value;
    }

    /// @brief Sugar for get().
    inline valtype const& operator*() const {
        return get();
    }
};


/**
 * Responsible for tracking control state and dirtiness.
*/
class ControlBank {
};

}