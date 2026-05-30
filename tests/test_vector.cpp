#include <gtest/gtest.h>
#include <utility>
#include "vector.hpp"

// ── Default Constructor ──────────────────────────────────────────────────────

TEST(VectorDefaultConstructor, SizeIsZero) {
    my::Vector v;
    EXPECT_EQ(v.size(), 0);
}

TEST(VectorDefaultConstructor, CapacityIsZero) {
    my::Vector v;
    EXPECT_EQ(v.capacity(), 0);
}

TEST(VectorDefaultConstructor, IsEmpty) {
    my::Vector v;
    EXPECT_TRUE(v.empty());
}

// ── Capacity Constructor ─────────────────────────────────────────────────────

TEST(VectorCapacityConstructor, SetsCapacity) {
    my::Vector v(8);
    EXPECT_EQ(v.capacity(), 8);
}

TEST(VectorCapacityConstructor, SizeRemainsZero) {
    my::Vector v(8);
    EXPECT_EQ(v.size(), 0);
}

TEST(VectorCapacityConstructor, IsEmpty) {
    my::Vector v(8);
    EXPECT_TRUE(v.empty());
}

TEST(VectorCapacityConstructor, ZeroCapacity) {
    my::Vector v(0);
    EXPECT_EQ(v.capacity(), 0);
    EXPECT_EQ(v.size(), 0);
}

// ── Fill Constructor ─────────────────────────────────────────────────────────

TEST(VectorFillConstructor, SetsCorrectSize) {
    my::Vector v(5, 3);
    EXPECT_EQ(v.size(), 5);
}

TEST(VectorFillConstructor, SetsCorrectCapacity) {
    my::Vector v(5, 3);
    EXPECT_EQ(v.capacity(), 5);
}

TEST(VectorFillConstructor, AllElementsMatchValue) {
    my::Vector v(4, 7);
    for (std::size_t i = 0; i < v.size(); ++i)
        EXPECT_EQ(v[i], 7);
}

TEST(VectorFillConstructor, NegativeValue) {
    my::Vector v(3, -42);
    for (std::size_t i = 0; i < v.size(); ++i)
        EXPECT_EQ(v[i], -42);
}

TEST(VectorFillConstructor, ZeroSize) {
    my::Vector v(0, 99);
    EXPECT_EQ(v.size(), 0);
    EXPECT_TRUE(v.empty());
}

TEST(VectorFillConstructor, SingleElement) {
    my::Vector v(1, 100);
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], 100);
}

// ── Copy Constructor ─────────────────────────────────────────────────────────

TEST(VectorCopyConstructor, CopyOfEmptyVector) {
    my::Vector src;
    my::Vector dst(src);
    EXPECT_EQ(dst.size(), 0);
    EXPECT_EQ(dst.capacity(), 0);
    EXPECT_TRUE(dst.empty());
}

TEST(VectorCopyConstructor, CopiesSize) {
    my::Vector src(3, 1);
    my::Vector dst(src);
    EXPECT_EQ(dst.size(), src.size());
}

TEST(VectorCopyConstructor, CopiesCapacity) {
    my::Vector src(3, 1);
    my::Vector dst(src);
    EXPECT_EQ(dst.capacity(), src.capacity());
}

TEST(VectorCopyConstructor, CopiesValues) {
    my::Vector src(3, 5);
    my::Vector dst(src);
    for (std::size_t i = 0; i < dst.size(); ++i)
        EXPECT_EQ(dst[i], 5);
}

TEST(VectorCopyConstructor, IndependenceFromSource) {
    my::Vector src(3, 1);
    my::Vector dst(src);
    dst[0] = 99;
    EXPECT_EQ(src[0], 1);
}

TEST(VectorCopyConstructor, PreservesSpareCapacity) {
    my::Vector src(4);  // capacity=4, size=0
    src.push_back(1);
    src.push_back(2);   // size=2, capacity=4 — spare capacity of 2
    my::Vector dst(src);
    EXPECT_EQ(dst.size(), 2);
    EXPECT_EQ(dst.capacity(), 4);
}

// ── Move Constructor ─────────────────────────────────────────────────────────

TEST(VectorMoveConstructor, MovesValues) {
    my::Vector src(3, 4);
    my::Vector dst(std::move(src));
    for (std::size_t i = 0; i < dst.size(); ++i)
        EXPECT_EQ(dst[i], 4);
}

TEST(VectorMoveConstructor, MovesSize) {
    my::Vector src(3, 4);
    my::Vector dst(std::move(src));
    EXPECT_EQ(dst.size(), 3);
}

TEST(VectorMoveConstructor, SourceSizeIsZero) {
    my::Vector src(3, 4);
    my::Vector dst(std::move(src));
    EXPECT_EQ(src.size(), 0);
}

TEST(VectorMoveConstructor, SourceCapacityIsZero) {
    my::Vector src(3, 4);
    my::Vector dst(std::move(src));
    EXPECT_EQ(src.capacity(), 0);
}

TEST(VectorMoveConstructor, MoveOfEmptyVector) {
    my::Vector src;
    my::Vector dst(std::move(src));
    EXPECT_EQ(dst.size(), 0);
    EXPECT_EQ(dst.capacity(), 0);
}

// ── Copy Assignment ──────────────────────────────────────────────────────────

TEST(VectorCopyAssignment, SelfAssignment) {
    my::Vector v(3, 2);
    v = v;
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 2);
}

TEST(VectorCopyAssignment, AssignNonEmptyToEmpty) {
    my::Vector src(3, 9);
    my::Vector dst;
    dst = src;
    EXPECT_EQ(dst.size(), 3);
    EXPECT_EQ(dst[0], 9);
}

TEST(VectorCopyAssignment, AssignEmptyToNonEmpty) {
    my::Vector src;
    my::Vector dst(3, 9);
    dst = src;
    EXPECT_EQ(dst.size(), 0);
    EXPECT_TRUE(dst.empty());
}

TEST(VectorCopyAssignment, CopiesValues) {
    my::Vector src(2, 6);
    my::Vector dst(2, 1);
    dst = src;
    for (std::size_t i = 0; i < dst.size(); ++i)
        EXPECT_EQ(dst[i], 6);
}

TEST(VectorCopyAssignment, Independence) {
    my::Vector src(2, 6);
    my::Vector dst;
    dst = src;
    dst[0] = 99;
    EXPECT_EQ(src[0], 6);
}

TEST(VectorCopyAssignment, ReturnsReference) {
    my::Vector a(1, 1), b(1, 2), c(1, 3);
    a = b = c;
    EXPECT_EQ(a[0], 3);
    EXPECT_EQ(b[0], 3);
}

TEST(VectorCopyAssignment, SourceLargerThanDestination) {
    my::Vector src(5, 9);
    my::Vector dst(2, 1);
    dst = src;
    EXPECT_EQ(dst.size(), 5);
    for (std::size_t i = 0; i < dst.size(); ++i)
        EXPECT_EQ(dst[i], 9);
}

TEST(VectorCopyAssignment, SourceSmallerThanDestination) {
    my::Vector src(2, 3);
    my::Vector dst(5, 7);
    dst = src;
    EXPECT_EQ(dst.size(), 2);
    for (std::size_t i = 0; i < dst.size(); ++i)
        EXPECT_EQ(dst[i], 3);
}

// ── Move Assignment ──────────────────────────────────────────────────────────

TEST(VectorMoveAssignment, SelfAssignment) {
    my::Vector v(3, 5);
    v = std::move(v);
    EXPECT_EQ(v.size(), 3);
}

TEST(VectorMoveAssignment, MovesValues) {
    my::Vector src(3, 7);
    my::Vector dst;
    dst = std::move(src);
    for (std::size_t i = 0; i < dst.size(); ++i)
        EXPECT_EQ(dst[i], 7);
}

TEST(VectorMoveAssignment, SourceIsCleared) {
    my::Vector src(3, 7);
    my::Vector dst;
    dst = std::move(src);
    EXPECT_EQ(src.size(), 0);
    EXPECT_EQ(src.capacity(), 0);
}

TEST(VectorMoveAssignment, MovesSize) {
    my::Vector src(4, 1);
    my::Vector dst;
    dst = std::move(src);
    EXPECT_EQ(dst.size(), 4);
}

TEST(VectorMoveAssignment, PreservesCapacity) {
    my::Vector src(4);  // capacity=4, size=0
    src.push_back(1);
    src.push_back(2);   // size=2, capacity=4
    my::Vector dst;
    dst = std::move(src);
    EXPECT_EQ(dst.size(), 2);
    EXPECT_EQ(dst.capacity(), 4);
}

// ── operator[] ───────────────────────────────────────────────────────────────

TEST(VectorSubscript, AccessFirstElement) {
    my::Vector v(3, 0);
    v[0] = 10;
    EXPECT_EQ(v[0], 10);
}

TEST(VectorSubscript, AccessLastElement) {
    my::Vector v(3, 0);
    v[2] = 99;
    EXPECT_EQ(v[2], 99);
}

TEST(VectorSubscript, MutateElement) {
    my::Vector v(3, 1);
    v[1] = 42;
    EXPECT_EQ(v[1], 42);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[2], 1);
}

TEST(VectorSubscript, ConstAccess) {
    const my::Vector v(3, 5);
    EXPECT_EQ(v[0], 5);
    EXPECT_EQ(v[2], 5);
}

// ── at() ─────────────────────────────────────────────────────────────────────

TEST(VectorAt, ValidIndexReturnsValue) {
    my::Vector v(3, 8);
    EXPECT_EQ(v.at(0), 8);
    EXPECT_EQ(v.at(2), 8);
}

TEST(VectorAt, MutateElement) {
    my::Vector v(3, 1);
    v.at(1) = 55;
    EXPECT_EQ(v.at(1), 55);
}

TEST(VectorAt, IndexEqualToSizeThrows) {
    my::Vector v(3, 1);
    EXPECT_THROW(v.at(3), std::out_of_range);
}

TEST(VectorAt, IndexBeyondSizeThrows) {
    my::Vector v(3, 1);
    EXPECT_THROW(v.at(10), std::out_of_range);
}

TEST(VectorAt, EmptyVectorThrows) {
    my::Vector v;
    EXPECT_THROW(v.at(0), std::out_of_range);
}

TEST(VectorAt, ConstValidIndex) {
    const my::Vector v(2, 3);
    EXPECT_EQ(v.at(0), 3);
    EXPECT_EQ(v.at(1), 3);
}

TEST(VectorAt, ConstOutOfRangeThrows) {
    const my::Vector v(2, 3);
    EXPECT_THROW(v.at(2), std::out_of_range);
}

// ── size() ───────────────────────────────────────────────────────────────────

TEST(VectorSize, DefaultConstructorSizeIsZero) {
    my::Vector v;
    EXPECT_EQ(v.size(), 0);
}

TEST(VectorSize, FillConstructorSetsSize) {
    my::Vector v(6, 1);
    EXPECT_EQ(v.size(), 6);
}

TEST(VectorSize, CapacityConstructorSizeIsZero) {
    my::Vector v(10);
    EXPECT_EQ(v.size(), 0);
}

// ── capacity() ───────────────────────────────────────────────────────────────

TEST(VectorCapacity, DefaultConstructorCapacityIsZero) {
    my::Vector v;
    EXPECT_EQ(v.capacity(), 0);
}

TEST(VectorCapacity, CapacityConstructorSetsCapacity) {
    my::Vector v(12);
    EXPECT_EQ(v.capacity(), 12);
}

TEST(VectorCapacity, FillConstructorCapacityEqualsSize) {
    my::Vector v(5, 1);
    EXPECT_EQ(v.capacity(), v.size());
}

// ── empty() ──────────────────────────────────────────────────────────────────

TEST(VectorEmpty, DefaultConstructorIsEmpty) {
    my::Vector v;
    EXPECT_TRUE(v.empty());
}

TEST(VectorEmpty, FillConstructorNotEmpty) {
    my::Vector v(3, 1);
    EXPECT_FALSE(v.empty());
}

TEST(VectorEmpty, CapacityConstructorIsEmpty) {
    my::Vector v(5);
    EXPECT_TRUE(v.empty());
}

// ── push_back() ──────────────────────────────────────────────────────────────

TEST(VectorPushBack, PushToEmptyIncrementsSize) {
    my::Vector v;
    v.push_back(1);
    EXPECT_EQ(v.size(), 1);
}

TEST(VectorPushBack, PushToEmptySetsCapacityToOne) {
    my::Vector v;
    v.push_back(1);
    EXPECT_EQ(v.capacity(), 1);
}

TEST(VectorPushBack, ValueIsStored) {
    my::Vector v;
    v.push_back(42);
    EXPECT_EQ(v[0], 42);
}

TEST(VectorPushBack, SizeIncrementsEachPush) {
    my::Vector v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    EXPECT_EQ(v.size(), 3);
}

TEST(VectorPushBack, GrowDoublesCapacity) {
    my::Vector v(1);
    v.push_back(1);
    v.push_back(2);  // triggers grow: capacity 1 → 2
    EXPECT_EQ(v.capacity(), 2);
}

TEST(VectorPushBack, PushWithinCapacityDoesNotGrow) {
    my::Vector v(4);
    v.push_back(1);
    v.push_back(2);
    EXPECT_EQ(v.capacity(), 4);
}

TEST(VectorPushBack, OrderIsPreserved) {
    my::Vector v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
}

TEST(VectorPushBack, RepeatedGrowthPreservesAllValues) {
    my::Vector v;
    for (int i = 0; i < 16; ++i)
        v.push_back(i);
    EXPECT_EQ(v.size(), 16);
    for (int i = 0; i < 16; ++i)
        EXPECT_EQ(v[i], i);
}

// ── pop_back() ───────────────────────────────────────────────────────────────

TEST(VectorPopBack, PopFromEmptyIsNoOp) {
    my::Vector v;
    EXPECT_NO_THROW(v.pop_back());
    EXPECT_EQ(v.size(), 0);
}

TEST(VectorPopBack, SizeDecrementsOnPop) {
    my::Vector v(3, 1);
    v.pop_back();
    EXPECT_EQ(v.size(), 2);
}

TEST(VectorPopBack, PopLastElementMakesEmpty) {
    my::Vector v(1, 5);
    v.pop_back();
    EXPECT_TRUE(v.empty());
}

TEST(VectorPopBack, CapacityUnchangedAfterPop) {
    my::Vector v(3, 1);
    v.pop_back();
    EXPECT_EQ(v.capacity(), 3);
}

TEST(VectorPopBack, RemainingElementsUnchanged) {
    my::Vector v(3, 7);
    v.pop_back();
    EXPECT_EQ(v[0], 7);
    EXPECT_EQ(v[1], 7);
}

