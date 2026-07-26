#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace my {
    class IntQueue {
    private:
        mutable std::mutex m_;
        std::condition_variable cv_;
        std::queue<int> queue_;
        bool closed_{false};
    public:
        IntQueue() = default;
        IntQueue(const IntQueue&) = delete;
        IntQueue(IntQueue&&) = delete;

        bool push(int val) {
            {
                std::lock_guard lock{m_};
                if (closed_) return false;
                queue_.push(val);
            }
            cv_.notify_one();
            return true;
        }

        std::optional<int> pop() {
            std::unique_lock lock{m_};

            cv_.wait(lock, [this]{ return !queue_.empty() || closed_; });
            if (queue_.empty()) return std::nullopt;

            int popped = queue_.front(); queue_.pop();
            return popped;
        }

        std::optional<int> try_pop() {
            std::lock_guard lock{m_};
            if (queue_.empty()) return std::nullopt;
            int popped = queue_.front(); queue_.pop();
            return popped;
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