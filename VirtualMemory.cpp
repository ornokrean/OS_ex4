#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include "string.h"
#include <math.h>
#include <stdio.h>
#include <iostream>

using namespace std;

void clearTable(uint64_t frameIndex)
{
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

/*
 * Fills an int array with the p addresses and the offset.
 * */
void getAddressAt(uint64_t vAddress, uint64_t *addresses)
{
    int bitAnd = (1 << OFFSET_WIDTH) - 1; //Creates 1^OFFSET_WIDTH
    // For doing this with an array
    int i = TABLES_DEPTH;
    while (i >= 0)
    {
        addresses[i] = (vAddress & bitAnd);
        //Shift right by offset width to get the next spot:
        vAddress = (vAddress >> OFFSET_WIDTH);
        i--;
    }

}

/*
 * Returns the page number from the Virtual address
 * */
uint64_t getPageNum(uint64_t vAddress)
{
    int bitAnd = (1 << OFFSET_WIDTH) - 1; //Creates 1^OFFSET_WIDTH
    // For doing this with an array
    int i = 0;
    uint64_t page_num = 0;
    vAddress = (vAddress >> OFFSET_WIDTH);

//    while (i < TABLES_DEPTH)
//    {
//        page_num += vAddress & bitAnd;
//        //Shift right by offset width to get the next spot:
//        vAddress = (vAddress >> OFFSET_WIDTH);
//        i++;
//    }
    return page_num;
}


uint64_t translateVaddress(uint64_t addr)
{
    return 0;
}


/*Returns true if the frame is empty*/
bool isClear(uint64_t frameIndex)
{
    int w = 0;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMread(frameIndex * PAGE_SIZE + i, &w);
        if (w != 0)
        {
            return false;
        }
    }
    return true;
}


void findCyclicDistance(uint64_t page_num, int depth, uint64_t frame_index, uint64_t fixed_page, uint64_t *maxFrame)
{
    //If reached actual page
    if (depth == TABLES_DEPTH)
    {
        //Calculate the minimal distance between the current page and the page to insert:
        auto cycDist = min(fixed_page - page_num, NUM_PAGES - (fixed_page - page_num));

        //Update the max distance frame if the new distance is larger:
        if (cycDist > min(fixed_page - *maxFrame, NUM_PAGES - (fixed_page - *maxFrame)))
        {
            *maxFrame = page_num;
            return;
        }
    }
    page_num <<= OFFSET_WIDTH;
    int word = 0;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        //Get the next frame
        PMread(frame_index * PAGE_SIZE + i, &word);
        if (word != 0)
        {
            findCyclicDistance(page_num + i, depth + 1, uint64_t(word), fixed_page, maxFrame);
        }
    }
}

void findEmptyFrame(uint64_t frame, uint64_t protectedFrame, uint64_t *clearFrame)
{
    // Empty frame found
    if (*clearFrame != -1)
    {
        return;
    }
    //If the frame is empty and it is not the frame we don't want to use:
    if (isClear(frame) && frame != protectedFrame)
    {
        *clearFrame = frame;
        return;
    }
    int word = 0;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMread(frame * PAGE_SIZE + i, &word);
        if (word != 0)
        {
            findEmptyFrame(uint64_t(word), protectedFrame, clearFrame);
        }
    }
}


/*
 * Traverses the tree in DFS, and saves the max index of frames visited
 * */
void findMax(uint64_t frameIndex, uint64_t *maxFrame)
{
    int word = 0;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMread(frameIndex * PAGE_SIZE + i, &word);
        if (word != 0)
        {
            if (word > *maxFrame)
            {
                *maxFrame = uint64_t(word);
            }
            findMax(uint64_t(word), maxFrame);
        }
    }

}

void combinedFind(uint64_t frameIndex, uint64_t *emptyFrame, uint64_t *maxFrame, uint64_t *cyclicFrame,
        uint64_t protectedFrame)
{

    int word = 0;
    //If the frame is empty and it is not the frame we don't want to use:
    if (isClear(frameIndex) && frameIndex != protectedFrame)
    {
        *emptyFrame = frameIndex;
    }
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMread(frameIndex * PAGE_SIZE + i, &word);
        if (word != 0)
        {
            if (word > *maxFrame)
            {
                *maxFrame = uint64_t(word);
            }
            findMax(uint64_t(word), maxFrame);
            findEmptyFrame(uint64_t(word), protectedFrame, emptyFrame);
        }
    }
}

uint64_t getFrame(uint64_t protectedIndex)
{
    //First Priority: Empty Frame
    auto emptyFrame = uint64_t(-1);
    findEmptyFrame(0, protectedIndex, &emptyFrame);
    if (emptyFrame != -1)
    {
        return emptyFrame;
    }
    //Second Priority: Unused Frame:
    uint64_t maxFrame = 0;
    findMax(0, &maxFrame);
    //Case: RAM not full yet
    if (maxFrame + 1 < NUM_FRAMES)
    {
        return maxFrame + 1;
    }
    //Third Priority: Evict a page:

}

uint64_t translate(uint64_t paddr, uint64_t frame, int depth)
{
    int addr = 0;
    PMread(frame * PAGE_SIZE + paddr, &addr);
    //Case: Page Fault- handle importing frame
    if (addr == 0)
    {
        /*Find an unused frame or evict a page from some frame*/
        uint64_t f = getFrame(frame);

        //Case: Actual Page table and not a page of page tables
        if (depth == TABLES_DEPTH)
        {
            /*Restore from disk*/
            /*PMrestore(f, addr);*/
        } else
        {
            /*Write 0's to all rows*/
            clearTable(uint64_t(f));
        }
        addr = int(f);
        //Update the "parent" with the relevant frame index
        PMwrite(frame * PAGE_SIZE + paddr, addr);
    }


    return 0;
}


void VMinitialize()
{
    clearTable(0);
}


int VMread(uint64_t virtualAddress, word_t *value)
{
    uint64_t addr = translateVaddress(virtualAddress);
    //Case: Virtual address cannot be mapped
    if (addr < 0)
    {
        return 0;
    }
    PMread(addr * PAGE_SIZE /*+ getAddressAt(virtualAddress, 0)*/, value);
    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value)
{
    uint64_t addr = translateVaddress(virtualAddress);
    //Case: Virtual address cannot be mapped
    if (addr < 0)
    {
        return 0;
    }
    PMwrite(addr * PAGE_SIZE /*+ getAddressAt(virtualAddress, 0)*/, value);

    return 1;
}


/*TODO Tasks:
    1. Split the input address correctly to separate parts: p1...pn,d.
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











void printSubTree(uint64_t root, int depth, bool isEmptyMode)
{
    if (depth == TABLES_DEPTH)
    {
        return;
    }
    word_t currValue = 0;

    if ((isEmptyMode || root == 0) && depth != 0)
    {
        isEmptyMode = true;
    }

    //right son
    PMread(root * PAGE_SIZE + 1, &currValue);
    printSubTree(static_cast<uint64_t>(currValue), depth + 1, isEmptyMode);

    //father
    for (int _ = 0; _ < depth; _++)
    {
        cout << '\t';
    }
    if (isEmptyMode)
    {
        cout << '_' << '\n';
    } else
    {
        if (depth == TABLES_DEPTH - 1)
        {
            word_t a, b;
            PMread(root * PAGE_SIZE + 0, &a);
            PMread(root * PAGE_SIZE + 1, &b);
            cout << root << " -> (" << a << ',' << b << ")\n";
        } else
        {
            cout << root << '\n';
        }
    }

//left son
    PMread(root
           * PAGE_SIZE + 0, &currValue);
    printSubTree(static_cast
                         <uint64_t>(currValue), depth
                                                + 1, isEmptyMode);

}

void printTree()
{
    cout << "---------------------" << '\n';
    cout << "Virtual Memory:" << '\n';
    printSubTree(0, 0, false);
    cout << "---------------------" << '\n';
}