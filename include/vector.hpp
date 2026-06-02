#pragma once
#include <cstddef>
#include <limits>
#include <stdexcept>

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
                data_ = new T[capacity_];
        }

        Vector(const std::size_t size, const T& value)
            : size_{size}, capacity_{size} {
            if (size_ > 0) {
               data_ = new T[capacity_];
               for (std::size_t i = 0; i < size_; ++i) {
                   data_[i] = value;
               }
            }

        }

        ~Vector() {
            delete[] data_;
        }

        Vector(const Vector& other)
            : size_{other.size_}, capacity_{other.capacity_} {
            if (capacity_ > 0) {
                data_ = new T[capacity_];
                for (std::size_t i = 0; i < size_; ++i) {
                    data_[i] = other.data_[i];
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

        Vector& operator=(const Vector& other) {
            if (this == &other)
                return *this;

            T* tmp = nullptr;
            if (other.capacity_ > 0) {
                tmp = new T[other.capacity_];
                for (std::size_t i = 0; i < other.size_; ++i) {
                    tmp[i] = other.data_[i];
                }
            }

            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = tmp;
            return *this;
        }

        Vector& operator=(Vector&& other) noexcept {
            if (this == &other)
                return *this;

            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;

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

        std::size_t size() const {
            return size_;
        }

        std::size_t capacity() const {
            return capacity_;
        }

        bool empty() const {
            return size_ == 0;
        }

        void push_back(const T& value) {
            if (size_ == capacity_)
                grow();

            data_[size_] = value;
            size_++;
        }

        void pop_back() {
            if (size_ == 0)
                return;

            size_--;
        }

    private:
        void grow() {
            if (capacity_ > (std::numeric_limits<std::size_t>::max() / 2))
                throw std::length_error("Vector can no longer increase in size");

            const std::size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            T* tmp = new T[new_capacity];
            for (std::size_t i = 0; i < size_; ++i) {
                tmp[i] = data_[i];
            }
            delete[] data_;
            capacity_ = new_capacity;
            data_ = tmp;
        }
    };
}

