// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#pragma once

#include <hilti/rt/exception.h>
#include <hilti/rt/safe-int.h>

#include <utility>

namespace hilti::rt {

/** Exception flagging access to an interator that not, or no longer, valid. */
HILTI_EXCEPTION(InvalidIterator, RuntimeError)

namespace detail::iterator {

struct ControlBlock {};

// Base class for containers providing safe iterators.
class Controllee {
public:
    ~Controllee() { _control.reset(); }
    Controllee() = default;
    Controllee(const Controllee& /*unused*/){};
    Controllee(Controllee&& /*unused*/) noexcept {};
    Controllee& operator=(const Controllee& /*unused*/) {
        _control.reset();
        return *this;
    };
    Controllee& operator=(Controllee&& /*unused*/) noexcept {
        _control.reset();
        return *this;
    };

    std::weak_ptr<ControlBlock> control() const {
        if ( ! _control )
            _control = std::make_shared<ControlBlock>();

        return _control;
    }

private:
    mutable std::shared_ptr<ControlBlock> _control;
};

template<typename C, typename I, typename SI>
class SafeIterator {
public:
    SafeIterator() = default;
    SafeIterator(const C& c, const I& i) : _i(std::move(i)), _control(c.control()) {}
    SafeIterator(std::weak_ptr<hilti::rt::detail::iterator::ControlBlock> control, const I& i)
        : _i(std::move(i)), _control(std::move(control)) {}
    SafeIterator(const SI& s) : _i(s._i), _control(s.control()) {}

    auto& operator*() const {
        ensureValid();
        return *_i;
    }
    bool operator==(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i == other._i;
    }
    bool operator!=(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i != other._i;
    }
    bool operator<(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i < other._i;
    }
    bool operator<=(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i <= other._i;
    }
    bool operator>(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i > other._i;
    }
    bool operator>=(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i >= other._i;
    }
    auto& operator++() {
        ensureValid();
        ++_i;
        return static_cast<SI&>(*this);
    }
    auto operator++(int) { // NOLINT
        ensureValid();
        auto tmp = *this;
        ++_i;
        return static_cast<SI&>(tmp);
    }
    auto& operator--() {
        ensureValid();
        return --_i;
        return static_cast<SI&>(*this);
    }
    auto operator--(int) { // NOLINT
        ensureValid();
        auto tmp = *this;
        ++_i;
        return static_cast<SI&>(tmp);
    }

    auto operator-(const SafeIterator& other) const {
        ensureValid();
        ensureSame(other);
        return _i - other._i;
    }

    template<typename T>
    auto operator+(const T& n) const {
        ensureValid();
        return SI(_control, _i + n);
    }

    template<typename T>
    auto& operator+=(const T& n) {
        ensureValid();
        _i += n;
        return *this;
    }

    explicit operator bool() const { return ! _control.expired(); }

protected:
    void ensureValid() const {
        if ( _control.expired() )
            throw InvalidIterator("bound object has expired");
    }

    void ensureSame(const SafeIterator& other) const {
        if ( _control.lock() != other._control.lock() )
            throw InvalidIterator("iterators refer to differen objects");
    }

    I& iterator() { return _i; }
    const I& iterator() const { return _i; }

private:
    I _i;
    std::weak_ptr<hilti::rt::detail::iterator::ControlBlock> _control;
};

/** Proxy class returned by `safe_range`.  */
template<typename T>
class SafeRange {
public:
    SafeRange(const T& t) : _t(t) {}
    auto begin() const { return safe_begin(_t); }
    auto end() const { return safe_end(_t); }

private:
    const T& _t;
};

} // namespace detail::iterator

/**
 * Wrapper around safe_begin/safe_end that returns an object suitable to
 * operate range-based for loop on to iterator over a sequence.
 */
template<typename T>
auto safe_range(const T& t) {
    return detail::iterator::SafeRange(t);
}

} // namespace hilti::rt
