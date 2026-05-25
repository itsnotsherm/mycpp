#pragma once
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace my {
    class Vector {
    private:
        int* data_{};
        std::size_t size_{};
        std::size_t capacity_{};

    public:
        explicit Vector() = default;

        explicit Vector(const std::size_t capacity)
            : capacity_{capacity} {
            if (capacity_ != 0) data_ = new int[capacity_];
        }

        Vector(const std::size_t size, const int value)
            : size_{size}, capacity_{size} {
            if (size_ > 0) {
               data_ = new int[capacity_];
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
            data_ = new int[capacity_];
            for (std::size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
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
            if (this != &other) {
                int* tmp = new int[other.capacity_];
                size_ = other.size_;
                capacity_ = other.capacity_;
                delete[] data_;
                data_ = tmp;
                for (std::size_t i = 0; i < size_; ++i) {
                    data_[i] = other.data_[i];
                }
            }
            return *this;
        }

        Vector& operator=(Vector&& other) noexcept {
            if (this != &other) {
                delete[] data_;
                size_ = other.size_;
                capacity_ = other.capacity_;
                data_ = other.data_;

                other.data_ = nullptr;
                other.size_ = 0;
                other.capacity_ = 0;
            }

            return *this;
        }

        int& operator[](const std::size_t index) {
            return data_[index];
        }

        const int& operator[](const std::size_t index) const {
            return data_[index];
        }

        int& at(const std::size_t index) {
            if (index >= size_) throw std::out_of_range("Index out of bounds");
            return data_[index];
        }

        const int& at(const std::size_t index) const {
            if (index >= size_) throw std::out_of_range("Index out of bounds");
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

        void push_back(const int value) {
            if (size_ == capacity_) resize();
            data_[size_] = value;
            size_++;
        }

        void pop_back() {
            if (size_ == 0) return;
            size_--;
        }

    private:
        void resize() {
            if (capacity_ > (std::numeric_limits<std::size_t>::max() / 2)) throw std::length_error("Vector can no longer increase in size");
            const std::size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            int* tmp = new int[new_capacity];
            for (std::size_t i = 0; i < size_; ++i) {
                tmp[i] = data_[i];
            }
            delete[] data_;
            capacity_ = new_capacity;
            data_ = tmp;
        }
    };
}

