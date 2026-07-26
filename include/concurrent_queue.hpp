#pragma once

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace my {
    template <class T>
    class ConcurrentQueue {
    private:
        mutable std::mutex m_;
        std::condition_variable cv_;
        std::queue<T> queue_;
        bool closed_{false};

        template <class... Args>
        bool emplace_preserved(Args&&... args) {
            {
                std::lock_guard lock{m_};
                if (closed_) return false;
                queue_.emplace(std::forward<Args>(args)...);
            }
            cv_.notify_one();
            return true;
        }

    public:
        ConcurrentQueue() = default;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue(ConcurrentQueue&&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;

        // queue must outlive every thread accessing it
        // destroying a cv with waiters or a locked mutex is UB
        ~ConcurrentQueue() = default;

        [[nodiscard]] bool push(const T& val) {
            return emplace_preserved(val);
        }

        [[nodiscard]] bool push(T&& val) {
            return emplace_preserved(std::move(val));
        }

        template <class... Args>
        [[nodiscard]] bool emplace(Args&&... args) {
            return emplace_preserved(std::forward<Args>(args)...);
        }

        [[nodiscard]] bool pop(T& out) {
            std::unique_lock lock{m_};

            cv_.wait(lock, [this]{ return !queue_.empty() || closed_; });
            if (queue_.empty()) return false;

            out = std::move(queue_.front()); queue_.pop();
            return true;
        }

        [[nodiscard]] bool try_pop(T& out) {
            std::lock_guard lock{m_};
            if (queue_.empty()) return false;

            out = std::move(queue_.front()); queue_.pop();
            return true;
        }

        void close() {
            {
                std::lock_guard lock{m_};
                closed_ = true;
            }
            cv_.notify_all();
        }
    };
}