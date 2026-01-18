#pragma once

// Compatibility header for WebAssembly builds
// Provides fallbacks for C++23 features that may not be available

#include <stdexcept>
#include <string>

// Simple std::expected polyfill for WebAssembly compatibility
#if !defined(__cpp_lib_expected) || __cpp_lib_expected < 202211L

namespace std {
    
template<typename T, typename E>
class expected {
private:
    bool has_val;
    union {
        T value;
        E error;
    };
    
public:
    // Constructors
    expected(const T& val) : has_val(true), value(val) {}
    expected(T&& val) : has_val(true), value(std::move(val)) {}
    expected(const E& err) : has_val(false), error(err) {}
    expected(E&& err) : has_val(false), error(std::move(err)) {}
    
    // Copy/move constructors
    expected(const expected& other) : has_val(other.has_val) {
        if (has_val) {
            new(&value) T(other.value);
        } else {
            new(&error) E(other.error);
        }
    }
    
    expected(expected&& other) : has_val(other.has_val) {
        if (has_val) {
            new(&value) T(std::move(other.value));
        } else {
            new(&error) E(std::move(other.error));
        }
    }
    
    // Destructor
    ~expected() {
        if (has_val) {
            value.~T();
        } else {
            error.~E();
        }
    }
    
    // Assignment operators
    expected& operator=(const expected& other) {
        if (this != &other) {
            this->~expected();
            new(this) expected(other);
        }
        return *this;
    }
    
    expected& operator=(expected&& other) {
        if (this != &other) {
            this->~expected();
            new(this) expected(std::move(other));
        }
        return *this;
    }
    
    // Observers
    bool has_value() const { return has_val; }
    explicit operator bool() const { return has_val; }
    
    T& value() & { 
        if (!has_val) throw std::runtime_error("Expected has no value");
        return this->value; 
    }
    
    const T& value() const& { 
        if (!has_val) throw std::runtime_error("Expected has no value");
        return this->value; 
    }
    
    T&& value() && { 
        if (!has_val) throw std::runtime_error("Expected has no value");
        return std::move(this->value); 
    }
    
    const T&& value() const&& { 
        if (!has_val) throw std::runtime_error("Expected has no value");
        return std::move(this->value); 
    }
    
    E& error() & { return this->error; }
    const E& error() const& { return this->error; }
    E&& error() && { return std::move(this->error); }
    const E&& error() const&& { return std::move(this->error); }
};

template<typename E>
class unexpected {
private:
    E err;
    
public:
    explicit unexpected(const E& e) : err(e) {}
    explicit unexpected(E&& e) : err(std::move(e)) {}
    
    E& error() & { return err; }
    const E& error() const& { return err; }
    E&& error() && { return std::move(err); }
    const E&& error() const&& { return std::move(err); }
};

template<typename E>
unexpected<E> make_unexpected(E&& e) {
    return unexpected<E>(std::forward<E>(e));
}

} // namespace std

#endif // __cpp_lib_expected