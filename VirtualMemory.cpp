#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}




void VMinitialize() {
    clearTable(0);
}


int VMread(uint64_t virtualAddress, word_t* value) {
    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    return 1;
}


/*TODO Tasks:
    1. Split the input address correctly to seperate parts: p1...pn,d.
    2. Write a DFS algorithm that searches for an empty frame
    3. Handle Evicting pages
    4. Recursive function for handling each p_i, where each such recursion takes heed not to
        evict frames set there by previous p's
    5. Frame Choice:
        5.1: Empty Frame: All rows are 0. No need to evict, but remove the reference to it from its parent
        5.2: Unused Frame: Since we fill frames in order in the beginning, all used frames are in the memory
                one after the other. So, if max_frame_index+1<NUM_FRAMES then frame at index max_frame_index+1


 #When traversing the tree, keep an index of the maximal frame visited
 */