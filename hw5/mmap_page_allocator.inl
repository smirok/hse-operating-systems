#include "mmap_page_allocator.hpp"

#include <sys/mman.h>
#include <unistd.h>

namespace hse::arch_os {
    template<typename PageDescriptor>
    void * MmapPageAllocator<PageDescriptor>::allocate(std::size_t order) {
        std::size_t pageSize = sysconf(_SC_PAGESIZE);

        void * p = mmap(
                nullptr, pageSize << order,
                PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE,
                -1, 0
        );
        if (p == MAP_FAILED)
            return nullptr;
        for (std::size_t i = 0; i != 1ull << order; ++i)
            descriptors.emplace(reinterpret_cast<void *>(reinterpret_cast<char *>(p) + pageSize * i), Descriptor{});
        orders.emplace(p, order);
        return p;
    }

    template<typename PageDescriptor>
    PageDescriptor & MmapPageAllocator<PageDescriptor>::pageDescriptor(void * address) {
        return descriptors[address].userDescriptor;
    }

    template<typename PageDescriptor>
    void MmapPageAllocator<PageDescriptor>::deallocate(void * address) {
        std::size_t pageSize = sysconf(_SC_PAGESIZE);

        Descriptor &pd = descriptors[address];
        munmap(address, pd.length);
        std::size_t order = orders.at(address);
        orders.erase(address);
        for (std::size_t i = 0; i != 1ull << order; ++i)
            descriptors.erase(reinterpret_cast<void *>(reinterpret_cast<char *>(address) + pageSize * i));
    }
}