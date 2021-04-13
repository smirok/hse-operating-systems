#include "buddy_allocator.hpp"

namespace hse::arch_os {

    BuddyAllocator::BuddyAllocator(void *memoryStart, std::size_t memoryLength) : _memory_start(memoryStart) {
        _pages = (memoryLength - PAGE_DESCRIPTOR_SIZE * ORDERS) / (PAGESIZE + PAGE_DESCRIPTOR_SIZE);
        _descriptors_start = reinterpret_cast<PageDescriptor *>(static_cast<char *>(memoryStart) + _pages * PAGESIZE);
        _free_list = FreeList(reinterpret_cast<PageDescriptor **>(_descriptors_start + _pages));
        for (int i = 0; i < ORDERS; ++i) {
            _free_list[i] = new(_free_list.ptr + i) PageDescriptor();
        }

        for (std::size_t i = 0; i < _pages; i++) {
            new(_descriptors_start + i)
                    PageDescriptor(-1, _descriptors_start + i);
        }

        int page_descriptors_padding = 0;
        for (int maxsz = MAX_ORDER; maxsz >= 0; maxsz--) {
            if (_pages >= (1 << maxsz)) {
                _free_list[maxsz] = _descriptors_start + page_descriptors_padding;
                _pages -= (1 << maxsz);
                _free_list[maxsz]->order = maxsz;
                _free_list[maxsz]->is_free = true;
                page_descriptors_padding += (1 << maxsz);
            } else {
                _free_list[maxsz] = nullptr;
            }
        }
        _pages = (memoryLength - PAGE_DESCRIPTOR_SIZE * ORDERS) / (PAGESIZE + PAGE_DESCRIPTOR_SIZE);
    }

    void *BuddyAllocator::allocate(std::size_t order) {
        int cur_order = order;
        for (; cur_order <= MAX_ORDER; ++cur_order) {
            if (_free_list[cur_order] != nullptr) {
                PageDescriptor *descriptor = _free_list[cur_order];
                _free_list[cur_order] = descriptor->next;

                if (_free_list[cur_order])
                    _free_list[cur_order]->prev = nullptr;

                for (; cur_order > order; cur_order--) {
                    PageDescriptor *buddy = (descriptor->ptr +
                                             (1 << (cur_order - 1)));
                    buddy->is_free = true;
                    buddy->order = cur_order - 1;
                    if (_free_list[cur_order - 1])
                        _free_list[cur_order - 1]->prev = buddy;
                    buddy->next = _free_list[cur_order - 1];
                    buddy->prev = nullptr;
                    _free_list[cur_order - 1] = buddy;
                }

                descriptor->is_free = false;
                descriptor->next = nullptr;
                descriptor->prev = nullptr;
                descriptor->order = order;
                return static_cast<char *>(_memory_start) +
                       (descriptor->ptr - _descriptors_start) *
                       PAGESIZE;
            }
        }
        return nullptr;
    }

    void BuddyAllocator::deallocate(void *memory) {
        PageDescriptor *pageDescriptor = _descriptors_start + ((static_cast<char *>(memory) -
                                                                static_cast<char *> (_memory_start)) / PAGESIZE);

        pageDescriptor->is_free = true;
        for (; pageDescriptor->order <= MAX_ORDER; pageDescriptor->order++) {
            unsigned long pos = pageDescriptor->ptr - _descriptors_start;
            unsigned long buddy_pos = pos ^(1 << (pageDescriptor->order));
            PageDescriptor *buddy = _descriptors_start + buddy_pos;

            if (buddy_pos + (1 << (pageDescriptor->order)) >= _pages || !buddy->is_free ||
                buddy->order != pageDescriptor->order) {
                if (_free_list[pageDescriptor->order])
                    _free_list[pageDescriptor->order]->prev = pageDescriptor;
                pageDescriptor->next = _free_list[pageDescriptor->order];
                pageDescriptor->prev = nullptr;
                _free_list[pageDescriptor->order] = pageDescriptor;
                pageDescriptor->is_free = true;
                return;
            }

            if (buddy_pos < pos)
                std::swap(pageDescriptor, buddy);

            _free_list.remove(buddy);
            _free_list.remove(pageDescriptor);
        }
    }

    BuddyAllocator::PageDescriptor::PageDescriptor(int order, void *ptr) {
        this->order = order;
        this->ptr = static_cast<PageDescriptor *>(ptr);
    }

    void BuddyAllocator::FreeList::remove(BuddyAllocator::PageDescriptor *pageDescriptor) {
        if (pageDescriptor->next && pageDescriptor->prev) {
            pageDescriptor->next->prev = pageDescriptor->prev;
            pageDescriptor->prev->next = pageDescriptor->next;
        } else if (pageDescriptor->next) {
            pageDescriptor->next->prev = nullptr;
        } else if (pageDescriptor->prev) {
            pageDescriptor->prev->next = nullptr;
        }

        if (pageDescriptor == ptr[pageDescriptor->order]) {
            ptr[pageDescriptor->order] = pageDescriptor->next ? pageDescriptor->next : nullptr;
        }

        pageDescriptor->next = nullptr;
        pageDescriptor->prev = nullptr;
    }
}