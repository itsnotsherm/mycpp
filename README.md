# MyCpp

Reimplementations of parts of the C++ standard library, written from scratch to understand how they
work internally. Everything lives in namespace `my::` and is header-only. These are learning
exercises rather than replacements for the standard library — the tables below list what each
component supports and what it does not.

C++20, CMake 3.22+, header-only. Built with GCC 13 and clang 18 on Linux.

## Building

```sh
cmake -S . -B build
cmake --build build
./build/examples
```

`src/main.cpp` is a runnable tour of everything below — each component gets a short demo that
prints what it is doing, which is the quickest way to see the APIs in use.

To use the headers elsewhere, `add_subdirectory` this repository and link the `mycpp` interface
target, or copy the individual header. They depend on nothing beyond the standard library.

## Contents

| Header | Component |
|---|---|
| `vector.hpp` | `my::Vector` — growable array with manual memory management |
| `optional.hpp` | `my::Optional` — union-based, `constexpr`-friendly |
| `unique_ptr.hpp` | `my::Unique_Ptr` — move-only owner, plus a `T[]` specialization |
| `shared_ptr.hpp` | `my::Shared_Ptr` / `my::Weak_Ptr` — atomic reference counting |
| `hash_map.hpp` | `my::HashMap` — separate chaining |
| `ring_buffer.hpp` | `my::RingBuffer` — fixed-capacity circular queue |
| `int_queue.hpp` | `my::IntQueue` — first pass at a blocking queue, `int` only |
| `concurrent_queue.hpp` | `my::ConcurrentQueue` — the templated version |

---

### `my::Vector<T>`

| API | What it does |
|---|---|
| `Vector()` | Empty, allocates nothing |
| `Vector(capacity)` | Reserves room for that many elements, size stays 0 — unlike `std::vector`, which would produce that many default-constructed elements |
| `Vector(size, value)` | Fills with that many copies of `value` |
| copy / move construction | Deep copy / takes over the buffer and leaves the source empty |
| assignment | Copy-and-swap, so one operator covers both copy and move assignment |
| `v[i]` | Unchecked element access |
| `at(i)` | Bounds-checked, throws `std::out_of_range` |
| `push_back(value)` | Appends, doubling capacity when full; accepts both lvalues and rvalues, and reallocation uses `std::move_if_noexcept` so a throwing move leaves the old buffer intact |
| `pop_back()` | Destroys the last element, no-op when empty |
| `size()` / `capacity()` | Element count / allocated room |
| `empty()` | Whether there are no elements |
| `swap(a, b)` | Swaps the buffer, size and capacity |

Storage is raw aligned `operator new` plus placement new, so elements are constructed and destroyed
individually rather than the whole buffer being default-constructed up front. Growth is guarded
against overflowing `size_t`.

**Not implemented:** iterators (so no range-for and nothing from `<algorithm>`), `emplace_back`,
`reserve`, `resize`, `clear`, `shrink_to_fit`, `front`/`back`/`data`, positional `insert`/`erase`,
`initializer_list` construction, comparison operators, allocator support.

### `my::Optional<T>`

| API | What it does |
|---|---|
| `Optional()` / `Optional(nullopt)` | Empty; `T` is never constructed |
| `Optional(value)` | Holds a value, copied or moved in |
| copy / move construction | Only constructs `T` if the source has one; the move propagates `noexcept` |
| assignment from another `Optional` | Assigns into the existing value if both are engaged, constructs if not, clears if the source is empty |
| assignment from `nullopt` | Clears |
| assignment from a value | Replaces the current contents |
| `has_value()` / `if (opt)` | Whether a value is held |
| `reset()` | Destroys the value and becomes empty |
| `*opt` | Unchecked access |
| `opt->member` | Member access on the contained value |
| `value()` | Checked access, throws `my::bad_optional_access` when empty |
| `value_or(fallback)` | The value, or the fallback converted to `T`; constrained with `std::convertible_to` |
| `my::nullopt` | Tag for the empty state |
| `my::bad_optional_access` | Thrown by `value()`, provides `what()` |

Storage is an anonymous union with a `char` dummy member — no allocation, and no default-constructed
`T` when empty. The whole type is `constexpr`.

**Not implemented:** `emplace`, `in_place_t` construction, converting constructors from
`Optional<U>`, comparison operators, `std::hash`, `make_optional`, the monadic interface
(`and_then` / `transform` / `or_else`), and `bad_optional_access` deriving from `std::exception`.

### `my::Unique_Ptr<T>`

| API | What it does |
|---|---|
| `Unique_Ptr()` | Null |
| `Unique_Ptr(raw)` | Takes ownership of a raw pointer |
| destruction | `delete`s the object, or `delete[]` in the array version |
| copy | Deleted; ownership is exclusive |
| move construction / assignment | Transfers ownership, leaving the source null |
| `*p` / `p->member` | Access the object |
| `get()` | The raw pointer, ownership unchanged |
| `release()` | Returns the raw pointer and becomes null — the caller takes over ownership |
| `reset(raw)` | Deletes the current object and adopts the new pointer |
| `if (p)` | Whether it owns anything |
| `Unique_Ptr<T[]>` and `p[i]` | Partial specialization that uses `delete[]` and adds indexing |
| `make_unique<T>(args...)` | Constructs a `T` and wraps it |
| `make_unique<T>(count)` | Value-initialized array as a `Unique_Ptr<T[]>` — the count must be a `std::size_t`, otherwise the single-object overload wins and `make_unique<int>(4)` yields a `Unique_Ptr<int>` holding 4 |

**Not implemented:** custom deleters (the reason the standard type has a second template
parameter), converting moves from `Unique_Ptr<Derived>` to `Unique_Ptr<Base>`, comparison
operators, `swap`, assignment from `nullptr`.

### `my::Shared_Ptr<T>` / `my::Weak_Ptr<T>`

| API | What it does |
|---|---|
| `Shared_Ptr()` | Null, no control block |
| `Shared_Ptr(raw)` | Adopts a raw pointer, allocating a separate control block; deletes the pointer if that allocation throws |
| copy construction | Shares ownership, incrementing the strong count |
| move construction | Takes over the pointer and block without touching the counts |
| assignment | Copy-and-swap |
| `*p` / `p->member` / `get()` | Access the object |
| `reset(raw)` | Drops the current reference and optionally adopts a new pointer |
| `use_count()` | Current strong count |
| `if (p)` | Whether it points at anything |
| `make_shared<T>(args...)` | Builds the object inside the control block — one allocation instead of two |
| `Weak_Ptr(shared)` | Observes without owning, incrementing only the weak count |
| copy / move construction and assignment | Weak count maintained on both sides |
| `expired()` | Whether the object has already been destroyed |
| `use_count()` | Strong count, or 0 once expired |
| `lock()` | Promotes to a `Shared_Ptr` through a CAS loop that will not resurrect a zeroed count; returns an empty `Shared_Ptr` if expired |

The control block holds atomic strong and weak counts, split so that the block outlives the object
it manages. Increments are `relaxed` and decrements are `acq_rel`, which is the minimum ordering
required for correctness. `PtrBlock` handles adopted raw pointers and `InPlaceBlock` handles
`make_shared`.

**Not implemented:** custom deleters and allocators, the aliasing constructor, converting
construction from `Shared_Ptr<Derived>`, `enable_shared_from_this`, construction from a
`Unique_Ptr`, array support, comparison operators.

### `my::HashMap<Key, T, Hash, KeyEqual>`

| API | What it does |
|---|---|
| `HashMap()` | No buckets allocated until the first insert |
| `HashMap(bucket_count)` | Preallocates, rounded up to a power of two (minimum 8), throwing `std::length_error` if too large |
| copy / move construction and assignment | Full rule of five, deep copying every node |
| `insert({key, value})` | Inserts unless the key exists; returns a pointer to the live entry and a flag indicating which happened. Existing values are never overwritten |
| `find(key)` | Pointer to the `{key, value}` pair, or `nullptr` |
| `map[key]` | Reference to the value, default-constructing an entry if the key is new |
| `erase(key)` | Removes the key, reporting 1 or 0 |
| `size()` / `empty()` | Element count / whether it is empty |

Separate chaining, the same strategy `std::unordered_map` is required to use. The bucket count is a
power of two so indexing masks instead of dividing, a murmur-style `mix()` finalizer keeps a weak
`std::hash` from clustering, and each node caches its hash so rehashing never recomputes it. Growth
doubles the bucket array once size passes the load factor.

Verified with ASan/UBSan and a 40k-operation differential fuzz against `std::unordered_map`, which
caught six silent bugs that ordinary functional tests missed — mostly variations on a rehash
invalidating a bucket index computed a few lines earlier. Details in
[`docs/hashmap_design.md`](docs/hashmap_design.md).

**Not implemented:** iterators (so the map cannot currently be traversed), `at`, `contains`,
`count`, `clear`, `emplace` / `try_emplace`, `insert_or_assign`, public `reserve` / `rehash` /
`load_factor` / `bucket_count`, heterogeneous lookup, and a copy-and-swap refactor of the
assignment operators.

### `my::RingBuffer<T>`

| API | What it does |
|---|---|
| `RingBuffer(capacity)` | Fixed capacity, never reallocates; throws `std::invalid_argument` on 0 |
| `push(value)` | Appends at the tail, reporting `false` when full rather than overwriting |
| `pop(out)` | Moves the head element into `out`, reporting `false` when empty |
| `size()` / `capacity()` | Live element count / fixed capacity |
| `full()` / `empty()` | Whether it is at capacity / holds nothing |

Head and tail indices with a separate size counter, rather than the sacrificial-empty-slot
approach. Wraparound is a branch rather than a modulo. `T` must be default-constructible and
assignable, since the buffer is preallocated and popped slots are cleared. Single-threaded.

**Not implemented:** `emplace`, `front`/peek, `clear`, an overwrite-oldest mode, power-of-two
capacity with masking, raw storage to lift the default-constructible requirement, and a lock-free
SPSC variant.

### `my::IntQueue`

| API | What it does |
|---|---|
| `IntQueue()` | Copy and move are deleted |
| `push(value)` | Enqueues and wakes one waiter; reports `false` if the queue is closed |
| `pop()` | Blocks until an item is available or the queue closes, returning an optional that is empty once closed and drained |
| `try_pop()` | Never blocks, empty optional if nothing is available |
| `close()` | Marks the queue closed and wakes every waiter, so blocked consumers return instead of deadlocking |

Mutex plus condition variable, `int` only. The wait predicate handles spurious wakeups, and
notification happens after the lock is released.

**Not implemented:** nothing planned. This is retained as the reference point for the templated
rewrite below.

### `my::ConcurrentQueue<T>`

| API | What it does |
|---|---|
| `ConcurrentQueue()` | Copy and move are deleted; the queue must outlive every thread using it |
| `push(value)` | Enqueues an lvalue or an rvalue, reporting `false` if closed |
| `emplace(args...)` | Constructs the element in place, with no temporary |
| `pop(out)` | Blocks until an item arrives and moves it into `out`; reports `false` once closed and drained |
| `try_pop(out)` | Never blocks, reports `false` if empty |
| `close()` | Closes the queue and wakes all waiters |

`IntQueue` generalized, and the version intended for actual use. Everything that reports success
through a return value is `[[nodiscard]]`, so an item cannot be silently dropped on a closed queue,
and notifications happen outside the lock so a woken thread does not immediately block on a mutex
the notifier still holds.

**Not implemented:** bounded capacity with blocking pushes for backpressure — the queue currently
grows without limit — along with `pop` with a timeout, `size` / `empty` / `closed` accessors, and
batch draining.

## Planned
- Iterators for `Vector`, `HashMap`, `RingBuffer`
- Lock-free SPSC ring buffer
- `my::List` — doubly linked list
- `my::Deque` — chunked double-ended queue
- `my::String` — with SBO
- `my::Function` — type-erased callable with SBO
