#pragma once

#include <utility>

namespace my {
    template <typename T>
    class Unique_Ptr {
    private:
        T* ptr_{};
    public:
        Unique_Ptr() noexcept = default;

        explicit Unique_Ptr(T* ptr) noexcept
            : ptr_{ptr} {

        }

        ~Unique_Ptr() {
            delete ptr_;
        }

        Unique_Ptr(const Unique_Ptr&) = delete;

        Unique_Ptr(Unique_Ptr&& other) noexcept
            : ptr_{std::exchange(other.ptr_, nullptr)} {

        }

        Unique_Ptr& operator=(const Unique_Ptr&) = delete;

        Unique_Ptr& operator=(Unique_Ptr&& other) noexcept {
            if (this != &other) {
                delete ptr_;
                ptr_ = std::exchange(other.ptr_, nullptr);
            }
            return *this;
        }

        T& operator*() const noexcept {
          return *ptr_;
        }

        T* operator->() const noexcept {
            return ptr_;
        }

        T* release() noexcept {
            return std::exchange(ptr_, nullptr);
        }

        void reset(T* p = nullptr) noexcept {
            T* old = ptr_;
            ptr_ = p;
            delete old;
        }

        T* get() const noexcept {
            return ptr_;
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }
    };

    template <typename T>
    class Unique_Ptr<T[]> {
    private:
        T* ptr_{};
    public:
        Unique_Ptr() noexcept = default;

        explicit Unique_Ptr(T* ptr) noexcept
            : ptr_{ptr} {

        }

        ~Unique_Ptr() {
            delete[] ptr_;
        }

        Unique_Ptr(const Unique_Ptr&) = delete;

        Unique_Ptr(Unique_Ptr&& other) noexcept
            : ptr_{std::exchange(other.ptr_, nullptr)} {

        }

        Unique_Ptr& operator=(const Unique_Ptr&) = delete;

        Unique_Ptr& operator=(Unique_Ptr&& other) noexcept {
            if (this != &other) {
                delete[] ptr_;
                ptr_ = std::exchange(other.ptr_, nullptr);
            }
            return *this;
        }

        T& operator[](std::size_t index) noexcept {
            return ptr_[index];
        }

        const T& operator[](std::size_t index) const noexcept {
            return ptr_[index];
        }

        T* release() noexcept {
            return std::exchange(ptr_, nullptr);
        }

        void reset(T* p = nullptr) noexcept {
            T* old = ptr_;
            ptr_ = p;
            delete[] old;
        }

        T* get() const noexcept {
            return ptr_;
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }
    };

    template <typename T, typename... Args>
    Unique_Ptr<T> make_unique(Args&&... args) {
        return Unique_Ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename T>
    Unique_Ptr<T[]> make_unique(const std::size_t n) {
        return Unique_Ptr<T[]>(new T[n]());
    }
};