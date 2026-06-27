#pragma once
#include <atomic>
#include <utility>

namespace my {
    struct ControlBlockBase {
        std::atomic<long> reference_count_{1};
        std::atomic<long> weak_count_{1};

        ControlBlockBase() = default;

        long increment() {
            return reference_count_.fetch_add(1, std::memory_order_relaxed);
        }

        long decrement() {
            return reference_count_.fetch_sub(1, std::memory_order_acq_rel);
        }

        bool try_increment() {
            long cnt = reference_count_.load(std::memory_order_relaxed);
            while (cnt != 0) {
                if (reference_count_.compare_exchange_weak(
                    cnt, cnt+1, std::memory_order_acq_rel, std::memory_order_relaxed))
                    return true;
            }
            return false;
        }

        long acquire_weak() {
            return weak_count_.fetch_add(1, std::memory_order_relaxed);
        }

        void release_weak() {
            if (weak_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
                delete this;
        }

        void release_shared() noexcept {
            if (decrement() == 1) {
                dispose();
                release_weak();
            }
        }

        long getCount() const {
            return reference_count_.load(std::memory_order_relaxed);
        }

        virtual void dispose() noexcept = 0;
        virtual ~ControlBlockBase() = default;
    };

    template <typename T>
    struct PtrBlock : ControlBlockBase {
        T* ptr_{};

        explicit PtrBlock(T* ptr)
            : ptr_{ptr} {

        }

        void dispose() noexcept override {
            delete ptr_;
        }
    };

    template <typename T>
    struct InPlaceBlock : ControlBlockBase {
        alignas(T) unsigned char storage_[sizeof(T)];

        template <typename... Args>
        explicit InPlaceBlock(Args&&... args) {
            ::new (storage_) T(std::forward<Args>(args)...);
        }

        T* get() noexcept {
            return reinterpret_cast<T*>(storage_);
        }

        void dispose() noexcept override {
            get()->~T();
        }
    };

    template <typename U> class Weak_Ptr;

    template <typename T>
    class Shared_Ptr {
    private:
        T* ptr_{};
        ControlBlockBase* blk_{};

        Shared_Ptr(T* ptr, ControlBlockBase* blk) noexcept
            : ptr_{ptr}, blk_{blk} {}

        template <typename U, typename... Args>
        friend Shared_Ptr<U> make_shared(Args&&...);

        friend void swap(Shared_Ptr& a, Shared_Ptr& b) noexcept {
            using std::swap;
            swap(a.ptr_, b. ptr_);
            swap(a.blk_, b.blk_);
        }

        template <typename U> friend class Weak_Ptr;

    public:
        Shared_Ptr() = default;

        explicit Shared_Ptr(T* ptr)
            : ptr_{ptr} {
            if (ptr) {
                try {
                    blk_ = new PtrBlock(ptr);
                } catch (...) {
                    delete ptr_;
                    throw;
                }
            }
        }

        ~Shared_Ptr() {
            if (blk_)
                blk_->release_shared();
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
            PtrBlock<T>* tmp = (ptr) ? new PtrBlock{ptr} : nullptr;
            auto old_blk = blk_;
            ptr_ = ptr;
            blk_ = tmp;

            if (old_blk)
                old_blk->release_shared();
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

    template <typename T>
    class Weak_Ptr {
    private:
        T* ptr_{};
        ControlBlockBase* blk_{};
    public:
        Weak_Ptr() = default;

        Weak_Ptr(const Shared_Ptr<T>& ptr)
            : ptr_{ptr.ptr_}, blk_ {ptr.blk_} {
            if (blk_)
                blk_->acquire_weak();
        }

        ~Weak_Ptr() {
            if (blk_)
                blk_->release_weak();
        }

        Weak_Ptr(const Weak_Ptr& other)
            : ptr_{other.ptr_}, blk_{other.blk_} {
            if (blk_)
                blk_->acquire_weak();
        }

        Weak_Ptr(Weak_Ptr&& other) noexcept
            : ptr_{std::exchange(other.ptr_, nullptr)}, blk_{std::exchange(other.blk_, nullptr)} {
        }

        Weak_Ptr& operator=(const Weak_Ptr& other) {
            if (this != &other) {
                T* old_ptr = ptr_;
                ControlBlockBase* old_blk = blk_;

                ptr_ = other.ptr_;
                blk_ = other.blk_;

                if (blk_)
                    blk_->acquire_weak();

                if (old_blk)
                    old_blk->release_weak();
            }
            return *this;
        }

        Weak_Ptr& operator=(Weak_Ptr&& other) noexcept {
            if (this != &other) {
                if (blk_)
                    blk_->release_weak();

                ptr_ = std::exchange(other.ptr_, nullptr);
                blk_ = std::exchange(other.blk_, nullptr);
            }
            return *this;
        }

        long use_count() const noexcept {
            return (blk_) ? blk_->getCount() : 0;
        }

        bool expired() const noexcept {
            return use_count() == 0;
        }

        Shared_Ptr<T> lock() const noexcept {
            if (blk_ && blk_->try_increment())
                return Shared_Ptr<T>(ptr_, blk_);
            return Shared_Ptr<T>{};
        }
    };

    template <typename T, typename... Args>
    Shared_Ptr<T> make_shared(Args&&... args) {
        auto* blk = new InPlaceBlock<T>(std::forward<Args>(args)...);
        return Shared_Ptr<T>(blk->get(), blk);
    }
}
