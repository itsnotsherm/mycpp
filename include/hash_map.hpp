#pragma once
#include <bit>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace my {
    template <typename Key, typename T,
                typename Hash = std::hash<Key>,
                typename KeyEqual = std::equal_to<Key>>
    class HashMap {
    public:
        using value_type = std::pair<const Key, T>;
        using size_type = std::size_t;
    private:
        struct Node {
            value_type kv;
            size_type hash;
            Node* next;
        };

        static constexpr size_type MIN_BUCKETS = 8;
        static constexpr size_type MAX_BUCKETS = size_type{1} << (std::numeric_limits<size_type>::digits - 1);

        float max_load_factor_ = 1.0f;
        std::vector<Node*> buckets_;
        size_type size_{};
        Hash hash_;
        KeyEqual key_equal_;

        static constexpr size_type mix(size_type x) noexcept {
            x ^= x >> 33;
            x *= 0xff51afd7ed558ccdULL;
            x ^= x >> 33;
            x *= 0xc4ceb9fe1a85ec53ULL;
            x ^= x >> 33;
            return x;
        }

        // rounds up to the next power of two >= n
        static constexpr size_type round_up(const size_type n) {
            if (n < MIN_BUCKETS)
                return MIN_BUCKETS;
            if (n > MAX_BUCKETS)
                throw std::length_error("bucket count too large");

            return std::bit_ceil(n);
        }

        void free() noexcept {
            for (Node* head : buckets_) {
                while (head) {
                    Node* tmp = head->next;
                    delete head;
                    head = tmp;
                }
            }

            buckets_.clear();
            size_ = 0;
        }

        Node* find_node(const Key& key, size_type idx) const {
            for (Node* node = buckets_[idx]; node != nullptr; node = node->next) {
                if (key_equal_(node->kv.first, key))
                    return node;
            }

            return nullptr;
        }

        void rehash() {
            std::vector<Node*> new_buckets;
            size_type new_bucket_count = 2 * buckets_.size();
            new_buckets.assign(new_bucket_count, nullptr);

            for (size_type i = 0; i < buckets_.size(); ++i) {
                while (buckets_[i]) {
                    Node* node = buckets_[i];
                    buckets_[i] = node->next;

                    size_type idx = node->hash & (new_bucket_count - 1);
                    node->next = new_buckets[idx];
                    new_buckets[idx] = node;
                }
            }

            buckets_ = new_buckets;
        }

    public:
        HashMap() = default;

        explicit HashMap(const size_type bucket_count) {
            if (bucket_count != 0)
                buckets_.assign(round_up(bucket_count), nullptr);
        }

        HashMap(const HashMap& other)
            : max_load_factor_{other.max_load_factor_}, size_{other.size_} {
            size_type bucket_count = other.buckets_.size();
            buckets_.assign(bucket_count, nullptr);
            for (size_type i = 0; i < bucket_count; ++i) {
                for (Node* node = other.buckets_[i]; node != nullptr; node = node->next) {
                    Node* new_node = new Node{node->kv, node->hash, buckets_[i]};
                    buckets_[i] = new_node;
                }
            }
        }

        HashMap(HashMap&& other) noexcept
            :   max_load_factor_{std::exchange(other.max_load_factor_, 1.0f)},
                buckets_{std::exchange(other.buckets_, std::vector<Node*>())},
                size_{std::exchange(other.size_, 0)}
        {

        }

        HashMap& operator=(const HashMap& other) {
            if (this != &other) {
                size_type bucket_count = other.buckets_.size();
                std::vector<Node*> new_buckets(bucket_count, nullptr);

                for (size_type i = 0; i < bucket_count; ++i) {
                    for (Node* node = other.buckets_[i]; node != nullptr; node = node->next) {
                        Node* new_node = new Node{node->kv, node->hash, new_buckets[i]};
                        new_buckets[i] = new_node;
                    }
                }

                free();
                buckets_ = std::move(new_buckets);
                size_ = other.size_;
                max_load_factor_ = other.max_load_factor_;
            }
            return *this;
        }

        HashMap& operator=(HashMap&& other) noexcept {
            if (this != &other) {
                free();
                buckets_ = std::exchange(other.buckets_, std::vector<Node*>());
                size_ = std::exchange(other.size_, 0);
                max_load_factor_ = std::exchange(other.max_load_factor_, 1.0f);
            }
            return *this;
        }

        ~HashMap() {
            free();
        }

        std::pair<value_type*, bool> insert(const value_type& kv) {
            if (buckets_.empty()) {
                buckets_.assign(MIN_BUCKETS, nullptr);
            }

            size_t h = hash_(kv.first);
            size_t m = mix(h);
            size_t idx = m & (buckets_.size() - 1);

            if (Node* node = find_node(kv.first, idx))
                return {&node->kv, false};

            if (size_ > max_load_factor_ * buckets_.size()) {
                rehash();
                idx = m & (buckets_.size() - 1);
            }

            Node* node = new Node{kv, m, nullptr};
            node->next = buckets_[idx];
            buckets_[idx] = node;
            size_++;
            return {&node->kv, true};
        }

        value_type* find(const Key& key) {
            if (buckets_.empty())
                return nullptr;

            size_type h = hash_(key);
            size_type m = mix(h);
            size_type idx = m & (buckets_.size() - 1);

            Node* found = find_node(key, idx);
            return found ? &found->kv : nullptr;
        }

        const value_type* find(const Key& key) const {
            if (buckets_.empty())
                return nullptr;

            size_type h = hash_(key);
            size_type m = mix(h);
            size_type idx = m & (buckets_.size() - 1);

            Node* found = find_node(key, idx);
            return found ? &found->kv : nullptr;
        }

        T& operator[](const Key& key) {
            if (buckets_.empty()) {
                buckets_.assign(MIN_BUCKETS, nullptr);
            }

            size_t h = hash_(key);
            size_t m = mix(h);
            size_t idx = m & (buckets_.size() - 1);

            Node* found = find_node(key, idx);
            if (!found) {
                if (size_ > max_load_factor_ * buckets_.size()) {
                    rehash();
                    idx = m & (buckets_.size() - 1);
                }

                Node* node = new Node{{key, {}}, m, nullptr};
                node->next = buckets_[idx];
                buckets_[idx] = node;
                found = node;
                size_++;
            }
            return found->kv.second;
        }

        size_type erase(const Key& key) {
            if (buckets_.empty())
                return 0;

            size_t h = hash_(key);
            size_t m = mix(h);
            size_t idx = m & (buckets_.size() - 1);

            Node* prev = nullptr;
            for (Node* node = buckets_[idx]; node != nullptr; node = node->next) {
                if (key_equal_(node->kv.first, key)) {
                    (prev ? prev->next : buckets_[idx]) = node->next;
                    delete node;
                    size_--;
                    return 1;
                }
                prev = node;
            }
            return 0;
        }

        size_type size() const {
            return size_;
        }

        bool empty() const {
            return size_ == 0;
        }
    };
}