#include <iostream>
#include "buddy_allocator.hpp"

using namespace std;

void *const regionStart = reinterpret_cast<void *>(0x0000'0800'0000'0000ull);

int main() {
    char *f = new char[4096 * 1000];
    hse::arch_os::BuddyAllocator buddyAllocator = hse::arch_os::BuddyAllocator(f, 4096 * 10);
    buddyAllocator.deallocate(buddyAllocator.allocate(2));
    buddyAllocator.deallocate(buddyAllocator.allocate(1));
    buddyAllocator.deallocate(buddyAllocator.allocate(0));
    cout << 1;

    cout <<
    return 0;
}
