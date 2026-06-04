#pragma once
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace my {
    template <typename T>
    class Vector {
    private:
        T* data_{};
        std::size_t size_{};
        std::size_t capacity_{};

    public:
        Vector() = default;

        explicit Vector(const std::size_t capacity)
            : capacity_{capacity} {
            if (capacity_ != 0)
                data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
        }

        Vector(const std::size_t size, const T& value)
            : capacity_{size} {
            if (size > 0) {
                try {
                    data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
                    for (std::size_t i = 0; i < size; ++i) {
                        ::new (data_ + i) T(value);
                        size_++;
                    }
                } catch (...) {
                    destroy_and_free();
                    throw;
                }
            }

        }

        ~Vector() {
            destroy_and_free();
        }

        Vector(const Vector& other)
            : capacity_{other.capacity_} {
            if (capacity_ > 0) {
                try {
                    data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
                    for (std::size_t i = 0; i < other.size_; ++i) {
                        ::new (data_ + i) T(other.data_[i]);
                        size_++;
                    }
                } catch (...) {
                    destroy_and_free();
                    throw;
                }
            }
        }

        Vector(Vector&& other) noexcept
            : size_{other.size_}, capacity_{other.capacity_} {
            data_ = other.data_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        Vector& operator=(Vector other) {
            swap(*this, other);
            return *this;
        }

        T& operator[](const std::size_t index) {
            return data_[index];
        }

        const T& operator[](const std::size_t index) const {
            return data_[index];
        }

        T& at(const std::size_t index) {
            if (index >= size_)
                throw std::out_of_range("Index out of bounds");

            return data_[index];
        }

        const T& at(const std::size_t index) const {
            if (index >= size_)
                throw std::out_of_range("Index out of bounds");
            return data_[index];
        }

        std::size_t size() const noexcept {
            return size_;
        }

        std::size_t capacity() const noexcept {
            return capacity_;
        }

        bool empty() const noexcept {
            return size_ == 0;
        }

        void push_back(const T& value) {
            if (size_ == capacity_)
                grow();

            ::new (data_ + size_) T(value); // construct T at address (data_ + size_ offset)
            size_++;
        }

        void push_back(T&& value) {
            if (size_ == capacity_)
                grow();

            ::new (data_ + size_) T(std::move(value));
            size_++;
        }

        void pop_back() {
            if (size_ == 0)
                return;

            size_--;
            data_[size_].~T();
        }

        friend void swap(Vector& first, Vector& second) noexcept {
            using std::swap;
            swap(first.data_, second.data_);
            swap(first.size_, second.size_);
            swap(first.capacity_, second.capacity_);
        }

    private:
        void grow() {
            if (capacity_ > (std::numeric_limits<std::size_t>::max() / 2))
                throw std::length_error("Vector can no longer increase in size");

            const std::size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            T* tmp = static_cast<T*>(::operator new(new_capacity * sizeof(T)));
            std::size_t n = 0;
            try {
                for (; n < size_; ++n)
                    ::new (tmp + n) T(data_[n]);
            } catch (...) {
                for (std::size_t i = 0; i < n; ++i)
                    tmp[i].~T();
                ::operator delete(tmp);
                throw;
            }

            destroy_and_free();
            capacity_ = new_capacity;
            data_ = tmp;
        }

        void destroy_and_free() noexcept {
            for (std::size_t i = 0; i < size_; ++i)
                data_[i].~T();

            ::operator delete(data_);
        }
    };
}

