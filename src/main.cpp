#include <iostream>
#include <string>
#include <thread>

#include "concurrent_queue.hpp"
#include "hash_map.hpp"
#include "int_queue.hpp"
#include "optional.hpp"
#include "ring_buffer.hpp"
#include "shared_ptr.hpp"
#include "unique_ptr.hpp"
#include "vector.hpp"

namespace {
    void heading(const char* title) {
        std::cout << "\n=== " << title << " ===\n";
    }

    struct Point {
        int x{};
        int y{};

        void print() const {
            std::cout << "  Point{" << x << ", " << y << "}\n";
        }
    };

    void demo_vector() {
        heading("my::Vector");

        my::Vector<int> v;
        for (int i = 1; i <= 5; ++i)
            v.push_back(i * 10);

        std::cout << "size=" << v.size() << " capacity=" << v.capacity() << '\n';

        std::cout << "contents:";
        for (std::size_t i = 0; i < v.size(); ++i)
            std::cout << ' ' << v[i];
        std::cout << '\n';

        v.at(0) = 99;
        std::cout << "after v.at(0) = 99 -> v[0]=" << v[0] << '\n';

        try {
            (void)v.at(v.size());
        } catch (const std::out_of_range& e) {
            std::cout << "at() threw: " << e.what() << '\n';
        }

        v.pop_back();
        std::cout << "after pop_back, size=" << v.size() << '\n';

        my::Vector<std::string> words(4);
        std::cout << "words: size=" << words.size()
                  << " capacity=" << words.capacity()
                  << " empty=" << std::boolalpha << words.empty() << '\n';
        words.push_back("hello");
        words.push_back(std::string{"moved"});

        my::Vector<char> filled(3, 'x');
        my::Vector<char> copy = filled;
        my::Vector<char> moved = std::move(filled);
        std::cout << "filled(3,'x') copied -> size=" << copy.size()
                  << ", moved-from size=" << filled.size() << '\n';

        my::Vector<char> assigned;
        assigned = copy;
        std::cout << "assigned[2]=" << assigned[2] << '\n';
    }

    void demo_optional() {
        heading("my::Optional");

        my::Optional<int> empty;
        my::Optional<int> answer = 42;
        my::Optional<std::string> nulled = my::nullopt;

        std::cout << "empty.has_value()=" << std::boolalpha << empty.has_value()
                  << " answer.has_value()=" << answer.has_value() << '\n';

        if (answer)
            std::cout << "*answer=" << *answer << '\n';

        std::cout << "empty.value_or(-1)=" << empty.value_or(-1) << '\n';

        try {
            (void)empty.value();
        } catch (const my::bad_optional_access& e) {
            std::cout << "value() threw: " << e.what() << '\n';
        }

        my::Optional<Point> p = Point{3, 4};
        std::cout << "p->x=" << p->x << " p->y=" << p->y << '\n';

        answer.reset();
        std::cout << "after reset, answer.has_value()=" << answer.has_value() << '\n';

        nulled = std::string{"now holds a string"};
        std::cout << "nulled=" << *nulled << '\n';
    }

    void demo_unique_ptr() {
        heading("my::Unique_Ptr");

        auto p = my::make_unique<Point>(1, 2);
        p->print();
        std::cout << "p is " << (p ? "non-null" : "null") << '\n';

        auto q = std::move(p);
        std::cout << "after move, p is " << (p ? "non-null" : "null")
                  << ", q->x=" << q->x << '\n';

        q.reset(new Point{7, 8});
        (*q).print();

        Point* raw = q.release();
        std::cout << "released raw pointer, q is " << (q ? "non-null" : "null") << '\n';
        delete raw;

        // a std::size_t argument picks the T[] overload, an int picks Unique_Ptr<int>
        const std::size_t count = 4;
        auto arr = my::make_unique<int>(count);
        for (std::size_t i = 0; i < count; ++i)
            arr[i] = static_cast<int>(i * i);

        std::cout << "arr:";
        for (std::size_t i = 0; i < count; ++i)
            std::cout << ' ' << arr[i];
        std::cout << '\n';
    }

    void demo_shared_ptr() {
        heading("my::Shared_Ptr / my::Weak_Ptr");

        auto a = my::make_shared<Point>(5, 6);
        std::cout << "use_count=" << a.use_count() << '\n';

        {
            auto b = a;
            std::cout << "inside scope, use_count=" << b.use_count() << '\n';
            b->x = 50;
        }
        std::cout << "after scope, use_count=" << a.use_count()
                  << " a->x=" << a->x << '\n';

        my::Weak_Ptr<Point> weak = a;
        std::cout << "weak.expired()=" << std::boolalpha << weak.expired() << '\n';

        if (auto locked = weak.lock()) {
            std::cout << "locked ok, use_count=" << locked.use_count() << '\n';
            locked->print();
        }

        a.reset();
        std::cout << "after a.reset(), weak.expired()=" << weak.expired() << '\n';

        const auto locked = weak.lock();
        std::cout << "lock() on expired weak gives "
                  << (locked ? "a pointer" : "an empty Shared_Ptr") << '\n';

        my::Shared_Ptr<Point> owned{new Point{9, 9}};
        std::cout << "adopted raw pointer, use_count=" << owned.use_count() << '\n';
    }

    void demo_hash_map() {
        heading("my::HashMap");

        my::HashMap<std::string, int> ages;

        ages["alice"] = 30;
        ages["bob"] = 25;
        ++ages["alice"];

        auto [entry, inserted] = ages.insert({"carol", 41});
        std::cout << "insert carol -> inserted=" << std::boolalpha << inserted
                  << " value=" << entry->second << '\n';

        auto [again, inserted_again] = ages.insert({"carol", 99});
        std::cout << "insert carol again -> inserted=" << inserted_again
                  << " value stays " << again->second << '\n';

        if (const auto* found = ages.find("alice"))
            std::cout << "find(alice) -> " << found->first << '=' << found->second << '\n';

        if (ages.find("dave") == nullptr)
            std::cout << "find(dave) -> nullptr\n";

        std::cout << "erase(bob) removed " << ages.erase("bob") << " entry\n";
        std::cout << "erase(bob) again removed " << ages.erase("bob") << " entries\n";
        std::cout << "size=" << ages.size() << " empty=" << ages.empty() << '\n';

        my::HashMap<int, std::string> codes(64);
        for (int i = 0; i < 100; ++i)
            codes[i] = "value-" + std::to_string(i);
        std::cout << "codes size after 100 inserts=" << codes.size()
                  << " codes[42]=" << codes[42] << '\n';

        my::HashMap<int, std::string> copy = codes;
        codes.erase(42);
        std::cout << "copy is independent: copy[42]=" << copy[42]
                  << " (codes size now " << codes.size() << ")\n";
    }

    void demo_ring_buffer() {
        heading("my::RingBuffer");

        my::RingBuffer<int> rb(3);
        std::cout << "capacity=" << rb.capacity()
                  << " empty=" << std::boolalpha << rb.empty() << '\n';

        for (int i = 1; i <= 4; ++i)
            std::cout << "push(" << i << ") -> " << rb.push(i) << '\n';

        std::cout << "size=" << rb.size() << " full=" << rb.full() << '\n';

        int out = 0;
        while (rb.pop(out))
            std::cout << "pop -> " << out << '\n';

        std::cout << "pop on empty -> " << rb.pop(out) << '\n';

        std::cout << "push(100) after draining -> " << rb.push(100) << '\n';
        std::cout << "push(200) after draining -> " << rb.push(200) << '\n';
        std::cout << "pop -> " << (rb.pop(out) ? out : -1) << " size=" << rb.size() << '\n';
    }

    void demo_int_queue() {
        heading("my::IntQueue");

        my::IntQueue q;

        std::thread producer{[&q] {
            for (int i = 1; i <= 5; ++i)
                q.push(i);
            q.close();
        }};

        std::thread consumer{[&q] {
            while (const auto value = q.pop())
                std::cout << "  consumed " << *value << '\n';
            std::cout << "  queue closed and drained\n";
        }};

        producer.join();
        consumer.join();

        my::IntQueue idle;
        std::cout << "try_pop on empty queue has_value="
                  << std::boolalpha << idle.try_pop().has_value() << '\n';
        idle.close();
        std::cout << "push after close -> " << idle.push(1) << '\n';
    }

    void demo_concurrent_queue() {
        heading("my::ConcurrentQueue");

        my::ConcurrentQueue<std::string> q;

        std::thread producer{[&q] {
            for (int i = 1; i <= 3; ++i)
                (void)q.push("job-" + std::to_string(i));
            (void)q.emplace(5, '!');
            q.close();
        }};

        std::thread consumer{[&q] {
            std::string job;
            while (q.pop(job))
                std::cout << "  handled " << job << '\n';
            std::cout << "  consumer exiting\n";
        }};

        producer.join();
        consumer.join();

        my::ConcurrentQueue<int> idle;
        int value = 0;
        std::cout << "try_pop on empty queue -> " << std::boolalpha
                  << idle.try_pop(value) << '\n';
        idle.close();
        std::cout << "push after close -> " << idle.push(1) << '\n';
    }
}

int main() {
    demo_vector();
    demo_optional();
    demo_unique_ptr();
    demo_shared_ptr();
    demo_hash_map();
    demo_ring_buffer();
    demo_int_queue();
    demo_concurrent_queue();
}
