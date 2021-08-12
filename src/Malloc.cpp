#include <Malloc.hpp>
#include <sys/mman.h>

/**
 * Your special drop-in replacement for malloc(). Should behave the same way.
 */
ArenaStore myArenaStore;
void* myMalloc(size_t n) {
    // TODO: implement myMalloc
    return myArenaStore.alloc(n);
}

/**
 * Your special drop-in replacement for free(). Should behave the same way.
 */
void myFree(void* addr) {
    // TODO: implement myFree
    myArenaStore.free(addr);
}

std::atomic<size_t> MMapObject::s_outstandingPages = 0;