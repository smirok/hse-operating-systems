#include <iostream>
#include "slab_allocator.hpp"
#include "page_allocator.hpp"

hse::arch_os::SlabAllocator::SlabAllocator(std::size_t objectSize) : _objectSize(objectSize) {
    if (objectSize < BIG_SIZE) {
        _create_small_slab();
    } else {
        void *ptr = pageAllocator().allocate(0);
        slabCtrlAllocator = new(ptr) SlabAllocator(SLAB_CTRL_SIZE);
        _create_big_slab();
    }
}

hse::arch_os::SlabAllocator::~SlabAllocator() {
    while (blockListStart != nullptr) {
        pageAllocator().deallocate(blockListStart->slab->memory);
        blockListStart = blockListStart->nextHeadDescriptor;
    }

    if (slabCtrlAllocator != nullptr)
        slabCtrlAllocator->~SlabAllocator();
}

void *hse::arch_os::SlabAllocator::allocate() {
    if (_objectSize < BIG_SIZE) {
        return _allocate_small();
    } else {
        return _allocate_big();
    }
}

void hse::arch_os::SlabAllocator::deallocate(void *memory) {
    if (_objectSize < BIG_SIZE) {
        _deallocate_small(memory);
    } else {
        _deallocate_big(memory);
    }
}

void *hse::arch_os::SlabAllocator::_allocate_small() {
    if (freeList) {
        SlabCtrlBase *slabCtrl = freeList;
        freeList = slabCtrl->prev;
        if (freeList)
            freeList->next = nullptr;
        return (reinterpret_cast<char *>(slabCtrl) + SLAB_CTRL_SIZE);
    }

    _create_small_slab();

    SlabCtrlBase *slabCtrl = freeList;
    freeList = slabCtrl->prev;
    return (reinterpret_cast<char *>(slabCtrl) + SLAB_CTRL_SIZE);
}

void hse::arch_os::SlabAllocator::_create_small_slab() {
    void *ptr = pageAllocator().allocate(0);

    auto *pageDescriptor = &pageAllocator().pageDescriptor(ptr);
    pageDescriptor->slab = new(ptr) SlabCtrlBase();

    SlabCtrlBase *currentCtrl = pageDescriptor->slab;
    currentCtrl->memory = ptr;

    if (!freeList) {
        freeList = currentCtrl;
    } else {
        freeList->next = currentCtrl;
        currentCtrl->prev = freeList;
        freeList = currentCtrl;
    }

    if (!blockListStart) {
        blockListStart = blockList = pageDescriptor;
    } else {
        blockList->nextHeadDescriptor = pageDescriptor;
        blockList = pageDescriptor;
    }

    for (std::size_t i = _objectSize + SLAB_CTRL_SIZE;
         i < PAGE_SIZE - _objectSize - SLAB_CTRL_SIZE;
         i += _objectSize + SLAB_CTRL_SIZE) {
        currentCtrl->next = new(static_cast<char *>(ptr) + i) SlabCtrlBase();
        currentCtrl = currentCtrl->next;

        freeList->next = currentCtrl;
        currentCtrl->prev = freeList;
        freeList = currentCtrl;
    }
}

void hse::arch_os::SlabAllocator::_deallocate_small(void *memory) {
    auto *slabCtrl = reinterpret_cast<SlabCtrlBase *>(static_cast<char *>(memory) - SLAB_CTRL_SIZE);
    if (!freeList) {
        freeList = slabCtrl;
        slabCtrl->next = nullptr;
        slabCtrl->prev = nullptr;
    } else {
        freeList->next = slabCtrl;
        slabCtrl->prev = freeList;
        slabCtrl->next = nullptr;
        freeList = slabCtrl;
    }
}

void *hse::arch_os::SlabAllocator::_allocate_big() {
    if (freeList) {
        SlabCtrlBase *slabCtrl = freeList;
        freeList = slabCtrl->prev;
        if (freeList)
            freeList->next = nullptr;
        return slabCtrl->memory;
    }

    _create_big_slab();

    SlabCtrlBase *slabCtrl = freeList;
    freeList = slabCtrl->prev;
    return slabCtrl->memory;
}

void hse::arch_os::SlabAllocator::_deallocate_big(void *memory) {
    SlabPageDescriptor *pageDescriptor = &pageAllocator().pageDescriptor(
            reinterpret_cast<void *>((reinterpret_cast<std::uintptr_t>(memory) / PAGE_SIZE) * PAGE_SIZE)
    );
    pageDescriptor = pageDescriptor->blockHeadDescriptor;
    SlabCtrlBase *slabCtrl = pageDescriptor->slab;

    while (slabCtrl != nullptr && slabCtrl->memory != memory) {
        slabCtrl = slabCtrl->realNext;
    }

    if (!freeList) {
        freeList = slabCtrl;
        slabCtrl->next = nullptr;
        slabCtrl->prev = nullptr;
    } else {
        freeList->next = slabCtrl;
        slabCtrl->prev = freeList;
        slabCtrl->next = nullptr;
        freeList = slabCtrl;
    }
}

void hse::arch_os::SlabAllocator::_create_big_slab() {
    void *ptr = pageAllocator().allocate(PAGE_ORDER);
    SlabPageDescriptor *pageDescriptor = &pageAllocator().pageDescriptor(ptr);

    pageDescriptor->blockHeadDescriptor = pageDescriptor;

    for (std::size_t i = 1; i < (1 << PAGE_ORDER); ++i) {
        SlabPageDescriptor *blockDescriptor = &pageAllocator().pageDescriptor(static_cast<char *>(ptr) + i * PAGE_SIZE);
        blockDescriptor->blockHeadDescriptor = pageDescriptor;
    }

    pageDescriptor->slab = new(slabCtrlAllocator->allocate()) SlabCtrlBase();
    SlabCtrlBase *currentCtrl = pageDescriptor->slab;

    currentCtrl->memory = ptr;

    if (!freeList) {
        freeList = currentCtrl;
    } else {
        freeList->next = currentCtrl;
        currentCtrl->prev = freeList;
        freeList = currentCtrl;
    }

    if (!blockListStart) {
        blockListStart = blockList = pageDescriptor;
    } else {
        blockList->nextHeadDescriptor = pageDescriptor;
        blockList = pageDescriptor;
    }

    for (std::size_t i = 1; i < (((1 << PAGE_ORDER) * PAGE_SIZE) / _objectSize); ++i) {
        currentCtrl->next = new(slabCtrlAllocator->allocate()) SlabCtrlBase();
        currentCtrl->realNext = currentCtrl->next;
        currentCtrl = currentCtrl->next;

        freeList->next = currentCtrl;
        currentCtrl->prev = freeList;
        freeList = currentCtrl;

        ptr = static_cast<char *>(ptr) + _objectSize;
        currentCtrl->memory = ptr;
    }
}
