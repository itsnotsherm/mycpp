#pragma once

namespace my {
    struct nullopt_t {
        explicit constexpr nullopt_t(int) {}
    };
    inline constexpr nullopt_t nullopt{0};

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
    };
}