#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace my {
    template <class T>
    class RingBuffer {
        // push , pop, full, empty, size
        std::vector<T> buffer_;
        std::size_t head_{};
        std::size_t tail_{};
        std::size_t size_{};

        template <class U>
        bool push_preserved(U&& val) {
            if (full())
                return false;

            buffer_[tail_] = std::forward<U>(val);
            tail_ = (tail_ + 1 == capacity()) ? 0 : tail_ + 1;
            size_++;
            return true;
        }
    public:
        explicit RingBuffer(std::size_t capacity)
            : buffer_(capacity) {
            if (capacity == 0) throw std::invalid_argument("RingBuffer capacity must be > 0");
        }

        bool push(const T& val) {
            return push_preserved(val);
        }

        bool push(T&& val) {
            return push_preserved(std::move(val));
        }

        bool pop(T& out) {
            if (empty())
                return false;
            out = std::move(buffer_[head_]);
            head_ = (head_ + 1 == capacity()) ? 0 : head_ + 1;
            size_--;
            buffer_[head_] = T{};
            return true;
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return size_;
        }

        [[nodiscard]] std::size_t capacity() const noexcept {
            return buffer_.size();
        }

        [[nodiscard]] bool full() const noexcept {
            return size_ == capacity();
        }

        [[nodiscard]] bool empty() const noexcept {
            return size_ == 0;
        }
    };
}