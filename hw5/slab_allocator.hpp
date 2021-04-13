#pragma once

#include <sys/user.h>
#include <cstddef>
#include <vector>

namespace hse::arch_os {

    struct SlabCtrlBase {
        void *memory = nullptr;
        SlabCtrlBase *next = nullptr; // in freeList
        SlabCtrlBase *prev = nullptr; // in freeList
        SlabCtrlBase *realNext = nullptr; // in this SLAB
    };

    struct SlabPageDescriptor {
        SlabCtrlBase *slab = nullptr;
        SlabPageDescriptor *nextHeadDescriptor = nullptr;
        SlabPageDescriptor *blockHeadDescriptor = nullptr;
    };


    class SlabAllocator {
    public:
        static constexpr std::size_t BIG_SIZE = PAGE_SIZE / 8;

        explicit SlabAllocator(std::size_t objectSize);

        ~SlabAllocator();

        void *allocate();

        void deallocate(void *memory);

    private:
        static constexpr std::size_t SLAB_CTRL_SIZE = sizeof(SlabCtrlBase);
        std::size_t _objectSize;
        static constexpr std::size_t PAGE_ORDER = 6;
        SlabCtrlBase *freeList = nullptr;
        SlabPageDescriptor *blockList = nullptr;
        SlabPageDescriptor *blockListStart = nullptr;
        SlabAllocator *slabCtrlAllocator = nullptr;

        void *_allocate_small();

        void *_allocate_big();

        void _deallocate_small(void *memory);

        void _deallocate_big(void *memory);

        void _create_small_slab();

        void _create_big_slab();
    };
}