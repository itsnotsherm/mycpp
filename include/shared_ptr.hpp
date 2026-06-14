#pragma once
#include <atomic>
#include <utility>

namespace my {
    template <typename T>
    class Shared_Ptr {
    private:
        struct ControlBlock {
            std::atomic<long> reference_count_{1};

            ControlBlock() = default;

            long increment() {
                return reference_count_.fetch_add(1, std::memory_order_relaxed);
            }

            long decrement() {
                return reference_count_.fetch_sub(1, std::memory_order_acq_rel);
            }

            long getCount() const {
                return reference_count_.load(std::memory_order_relaxed);
            }
        };

        T* ptr_{};
        ControlBlock* blk_{};

        friend void swap(Shared_Ptr& a, Shared_Ptr& b) noexcept {
            using std::swap;
            swap(a.ptr_, b. ptr_);
            swap(a.blk_, b.blk_);
        }

    public:
        Shared_Ptr() = default;

        explicit Shared_Ptr(T* ptr)
            : ptr_{ptr}, blk_{new ControlBlock{}} {

        }

        ~Shared_Ptr() {
            if (blk_ && blk_->decrement() == 1) {
                delete ptr_;
                delete blk_;
            }
        }

        Shared_Ptr(const Shared_Ptr& other)
            : ptr_{other.ptr_}, blk_{other.blk_} {
            if (blk_)
                blk_->increment();
        }

        Shared_Ptr(Shared_Ptr&& other) noexcept
            : ptr_{std::exchange(other.ptr_, nullptr)}, blk_{std::exchange(other.blk_, nullptr)} {

        }

        Shared_Ptr& operator=(Shared_Ptr other) noexcept {
            swap(*this, other);
            return *this;
        }

        T& operator*() const noexcept {
            return *ptr_;
        }

        T* operator->() const noexcept {
            return ptr_;
        }

        void reset(T* ptr = nullptr) {
            ControlBlock* tmp = (ptr) ? new ControlBlock{} : nullptr;
            auto old_ptr = ptr_;
            auto old_blk = blk_;
            ptr_ = ptr;
            blk_ = tmp;
            if (old_blk && old_blk->decrement() == 1) {
                delete old_ptr;
                delete old_blk;
            }
        }

        T* get() const noexcept {
            return ptr_;
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }

        long use_count() const noexcept {
            return (blk_) ? blk_->getCount() : 0;
        }
    };

    template <typename T, typename... Args>
    Shared_Ptr<T> make_shared(Args&&... args) {
        return Shared_Ptr<T>(new T(std::forward<Args>(args)...));
    }
}
