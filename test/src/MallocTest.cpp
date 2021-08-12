#include <Malloc.hpp>
#include <TestSuite.hpp>
#include <Assert.hpp>
#include <TestSuite.hpp>
#include <cstdlib>
#include <thread>
#include <sys/resource.h>
#include <iostream>

size_t expectedArenaAllocations(size_t blockSize) {
    return (pageSize - sizeof(Arena)) / blockSize;
}

size_t getArenaSize(size_t n) {
    for (size_t i = 0; i < 8; i++) {
        size_t arenaSize = 0x8 << i;

        if (n <= arenaSize) {
            return arenaSize;
        }
    }

    return SIZE_MAX;
}

class Barrier {
    std::atomic<size_t> m_count;

public:
    Barrier(size_t count): m_count(count) { }

    void wait() {
        m_count--;

        while(m_count.load() > 0) { }
    }
};

template <typename T> class MyAllocator {
public:
    typedef T value_type;
    MyAllocator() noexcept {} //default ctor not required by C++ Standard Library

    // A converting copy constructor:
    template<class U> MyAllocator(const MyAllocator<U>&) noexcept {}
    template<class U> bool operator==(const MyAllocator<U>&) const noexcept
    {
        return true;
    }
    template<class U> bool operator!=(const MyAllocator<U>&) const noexcept
    {
        return false;
    }

    T* allocate(size_t n, const void* hint = nullptr) {
        return static_cast<T*>(myMalloc(n * sizeof(T)));
    }

    void deallocate(T* const ptr, size_t _) const noexcept {
        myFree(ptr);
    }
};

/**
 * For the single and multi-threaded tests, sets the number of iterations
 * of malloc and free.
 */
constexpr size_t numIterations = 1'000'000;

/**
 * The maximum number of bytes allocated in a the random walk tests.
 */
constexpr size_t maxBytes = 10'000;

size_t getRandomSize() {
    switch(rand() % 10) {
        case 0: return rand() % 8 + 1;
        case 1: return rand() % 16 + 1;
        case 2: return rand() % 32 + 1;
        case 4: return rand() % 64 + 1;
        case 5: return rand() % 128 + 1;
        case 6: return rand() % 256 + 1;
        case 7: return rand() % 512 + 1;
        case 8: return rand() % 1024 + 1;
        default: return rand() % (maxBytes - 1024) + 1024;
    }
}


void canAllocateBigObject() {
    constexpr size_t len = 123456789;
    auto data = (volatile char*)BigAlloc::alloc(len);

    ASSERT_TRUE(reinterpret_cast<intptr_t>(data) % 8 == 0);
    ASSERT_EQ(MMapObject::outstandingPages(), 1);
    ASSERT_TRUE(data != nullptr);

    for (size_t i = 0; i < len; i++) {
        data[i] = static_cast<char>(i & 0xFF);
        ASSERT_EQ(data[i], static_cast<char>(i & 0xFF));
    }

    MMapObject::dealloc((void*)data);

    ASSERT_EQ(MMapObject::outstandingPages(), 0);
}

void mmapObjectHasCorrectSize() {
    auto data = BigAlloc::alloc(1234);

    ASSERT_TRUE(data != nullptr);

    auto mmapObjectPtr = reinterpret_cast<MMapObject*>(((char*)data - sizeof(BigAlloc)));

    ASSERT_EQ(mmapObjectPtr->mmapSize(), 1234 + sizeof(BigAlloc));
    ASSERT_EQ(mmapObjectPtr->arenaSize(), 0);
    ASSERT_TRUE(data != mmapObjectPtr);

    MMapObject::dealloc(mmapObjectPtr);
}

void arenaHasCorrectSize() {
    auto arena = Arena::create(16);

    // Creating an arena should alloc a pageset.
    ASSERT_EQ(MMapObject::outstandingPages(), 1);

    ASSERT_EQ(arena->mmapSize(), pageSize);
    ASSERT_EQ(arena->arenaSize(), 16);
    ASSERT_EQ(arena->next(), reinterpret_cast<char*>(arena) + sizeof(Arena))

    MMapObject::dealloc(arena);
    ASSERT_EQ(MMapObject::outstandingPages(), 0);
}

void canAllocCorrectNumberOfBlocks() {
    for (size_t arenaSize = 8; arenaSize <= 1024; arenaSize *= 2) {
        Arena* arena = Arena::create(arenaSize);
        size_t numAllocs = 0;

        while(!arena->full()) {
            void* ptr = arena->alloc();

            // ptr should not be null
            ASSERT_TRUE(ptr != nullptr);

            // Assert pointer is 64-bit aligned
            ASSERT_EQ(reinterpret_cast<intptr_t>(ptr) % 8, 0);

            numAllocs++;
        }

        MMapObject::dealloc(arena);

        size_t expectedAllocations = expectedArenaAllocations(arenaSize);

        // Assert we didn't alloc beyond a page boundary.
        ASSERT_TRUE(arenaSize * numAllocs + sizeof(Arena) <= pageSize);

        // Assert we got the corrct number of allocations.
        ASSERT_EQ(expectedAllocations, numAllocs);
    }

    ASSERT_EQ(MMapObject::outstandingPages(), 0);
}

void canFreeCorrectNumberOfBlocks() {
    for (size_t arenaSize = 8; arenaSize <= 1024; arenaSize *= 2) {
        Arena* arena = Arena::create(arenaSize);
        size_t numAllocs = 0;

        size_t expectedAllocations = expectedArenaAllocations(arenaSize);

        for (size_t i = 0; i < expectedAllocations; i++) {
            void* ptr = arena->alloc();
        }

        ASSERT_TRUE(arena->full());

        for (size_t i = 0; i < expectedAllocations - 1; i++) {
            ASSERT_TRUE(!arena->free());
        }

        ASSERT_TRUE(arena->free());

        MMapObject::dealloc(arena);
    }

    ASSERT_EQ(MMapObject::outstandingPages(), 0);
}

void canMallocAndFreeABunchOfStuff() {
    // Scope the vectors so they'll destruct and clear their underlying data.
    {
        std::vector<volatile char*, MyAllocator<volatile char*>> addresses;
        addresses.reserve(numIterations);

        // The above vectors should have 2 pages.
        ASSERT_EQ(MMapObject::outstandingPages(), 1);

        for (size_t i = 0; i < numIterations; i++) {
            size_t size = getRandomSize();

            auto ptr = (volatile char*)myMalloc(size);

            ASSERT_TRUE(ptr != nullptr);

            for (size_t j = 0; j < size; j++) {
                ptr[j] = 0xDE;
            }

            addresses.push_back(ptr);
        }

        for (auto ptr : addresses) {
            ASSERT_TRUE(ptr != nullptr);

            myFree((void*)ptr);
        }
    }

    // Number of outstanding pages should be less than 8 (no more than one per arena)
    ASSERT_TRUE(MMapObject::outstandingPages() <= 8);
}

void canMallocAndFreeABunchOfStuffThreaded() {
    // Scope the vectors so they'll destruct and clear their underlying data.
    // This ensures our page counts are accurate at the end of the test.
    {
        std::vector<volatile char*, MyAllocator<volatile char*>> addresses;
        addresses.reserve(numIterations);

        for (size_t i = 0; i < numIterations; i++) {
            size_t size = getRandomSize();

            auto ptr = (volatile char*)myMalloc(size);

            ASSERT_TRUE(ptr != nullptr);

            for (size_t j = 0; j < size; j++) {
                ptr[j] = 0xDE;
            }

            addresses.push_back(ptr);
        }

        constexpr size_t nThreads = 8;

        Barrier barrier0(nThreads + 1);
        std::atomic<size_t> doneThreads = 0;

        // Create a bunch of threads to free all the addresses in parallel while
        // Main thread continues allocations.
        for (size_t threads = 0; threads < nThreads; threads++) {
            std::thread([&](size_t tid) {
                barrier0.wait();

                for (size_t i = tid; i < addresses.size(); i += nThreads) {
                    volatile char* ptr = addresses[i];

                    myFree((void*)ptr);
                }

                doneThreads++;
            }, threads).detach();
        }

        std::vector<void*> moreAddresses;
        moreAddresses.reserve(numIterations);

        barrier0.wait();

        for (size_t i = 0; i < numIterations; i++) {
            void* ptr = myMalloc(getRandomSize());

            ASSERT_TRUE(ptr != nullptr);

            moreAddresses.push_back(ptr);
        }

        for (auto ptr : moreAddresses) {
            ASSERT_TRUE(ptr != nullptr);

            myFree((void*)ptr);
        }

        while (doneThreads.load() < nThreads) { }
    }

    // Number of outstanding pages should be less than 8 (no more than one per arena)
    ASSERT_TRUE(MMapObject::outstandingPages() <= 8);
}

int runMallocTests() {
    TestSuite suite;

    TEST(suite, canAllocateBigObject);
    TEST(suite, mmapObjectHasCorrectSize);
    TEST(suite, arenaHasCorrectSize);
    TEST(suite, canAllocCorrectNumberOfBlocks);
    TEST(suite, canFreeCorrectNumberOfBlocks);
    TEST(suite, canMallocAndFreeABunchOfStuff);
    TEST(suite, canMallocAndFreeABunchOfStuffThreaded);

    rusage resourseUsage;

    getrusage(RUSAGE_SELF, &resourseUsage);

    int passed = suite.run();

    std::cout << resourseUsage.ru_maxrss << " kb after all tests." << std::endl;
    return passed;
}