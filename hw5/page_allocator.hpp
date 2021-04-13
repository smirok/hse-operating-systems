#pragma once

#include "mmap_page_allocator.hpp"
#include "slab_allocator.hpp"

namespace hse::arch_os {
    using PageAllocator = MmapPageAllocator<SlabPageDescriptor>;

    // Вам предоставлена
    inline PageAllocator &pageAllocator() {
        static PageAllocator allocator;
        return allocator;
    }
}