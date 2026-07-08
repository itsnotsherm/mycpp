#pragma once

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

namespace my {
    struct nullopt_t {
        explicit constexpr nullopt_t(int) {}
    };
    inline constexpr nullopt_t nullopt{0};

    struct bad_optional_access {
        const char* what() const noexcept { return "bad optional access"; }
    };

    template <typename T>
    class Optional {
    public:
        using value_type = T;
    private:
        union {
            char dummy_;
            T value_;
        };

        bool has_value_{false};
    public:
        constexpr Optional() noexcept
            : dummy_{} {
        }

        constexpr ~Optional() {
            if (has_value_)
                value_.~T();
        }

        constexpr Optional(const Optional& other) {
            if (other.has_value_) {
                std::construct_at(&value_, other.value_);
                has_value_ = true;
            }
        }

        constexpr Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            if (other.has_value_) {
                std::construct_at(&value_, std::move(other.value_));
                has_value_ = true;
            }
        }

        constexpr Optional& operator=(const Optional& other) {
            if (other.has_value_) {
                if (has_value_)
                    value_ = other.value_;
                else {
                    std::construct_at(&value_, other.value_);
                    has_value_ = true;
                }
            } else {
                reset();
            }
            return *this;
        }

        constexpr Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>
                                                                && std::is_nothrow_move_constructible_v<T>) {
            if (other.has_value_) {
                if (has_value_)
                    value_ = std::move(other.value_);
                else {
                    std::construct_at(&value_, std::move(other.value_));
                    has_value_ = true;
                }
            } else {
                reset();
            }
            return *this;
        }

        constexpr Optional(nullopt_t) noexcept
            : dummy_{} {

        }

        constexpr Optional(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
            : value_{value}, has_value_{true} {

        }

        constexpr Optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
            : value_{std::move(value)}, has_value_{true} {

        }

        constexpr Optional& operator=(nullopt_t) noexcept {
            reset();
            return *this;
        }

        constexpr Optional& operator=(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            reset();
            std::construct_at(&value_, value);
            has_value_ = true;
            return *this;
        }

        constexpr Optional& operator=(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
            reset();
            std::construct_at(&value_, std::move(value));
            has_value_ = true;
            return *this;
        }

        constexpr bool has_value() const noexcept {
            return has_value_;
        }

        constexpr explicit operator bool() const noexcept {
            return has_value_;
        }

        constexpr void reset() noexcept {
            if (has_value_) {
                value_.~T();
                has_value_ = false;
            }
        }

        constexpr T& operator*() & noexcept {
            return value_;
        }

        constexpr const T& operator*() const& noexcept {
            return value_;
        }

        constexpr T* operator->() noexcept {
            return &value_;
        }

        constexpr const T* operator->() const noexcept {
            return &value_;
        }

        constexpr T& value() & {
            if (!has_value_)
                throw bad_optional_access{};
            return value_;
        }

        constexpr const T& value() const& {
            if (!has_value_)
                throw bad_optional_access{};
            return value_;
        }

        template <std::convertible_to<T> U>
        constexpr T value_or(U&& or_value) const& {
            if (!has_value_)
                return static_cast<T>(std::forward<U>(or_value));
            return value_;
        }

        template <std::convertible_to<T> U>
        constexpr T value_or(U&& or_value) && {
            if (!has_value_)
                return static_cast<T>(std::forward<U>(or_value));
            return std::move(value_);
        }
    };
}