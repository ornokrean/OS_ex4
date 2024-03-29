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
void getAddressParts(uint64_t vAddress, uint64_t *addresses)
{
    int bitAnd = (1 << OFFSET_WIDTH) - 1; //Creates 1^OFFSET_WIDTH
    // For doing this with an array

    for (int i = TABLES_DEPTH; i >= 0; --i)
    {
        addresses[i] = (vAddress & bitAnd);
        //Shift right by offset width to get the next spot:
//        cout << vAddress << endl;
        vAddress = (vAddress >> OFFSET_WIDTH);

    }
//    cout << "ARRAY" << endl;
//    for (int j = 0; j <= TABLES_DEPTH; ++j)
//    {
//
//        cout << addresses[j] << endl;
//    }
}

/*
 * Returns offset
 * */
uint64_t getOFFSET(uint64_t vAddress)
{
    int bitAnd = (1 << OFFSET_WIDTH) - 1; //Creates 1^OFFSET_WIDTH
    return (vAddress & bitAnd);
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

uint64_t calcCyclicDistance(uint64_t from, uint64_t to)
{
    if (to - from < NUM_PAGES - (to - from))
    {
        return to - from;
    }
    return NUM_PAGES - (to - from);
}
//
//void findCyclicDistance(uint64_t page_num, int depth, uint64_t frame_index, uint64_t fixed_page,
//                        uint64_t *currentMaxDistPage)
//{
//    //If reached actual page
//    if (depth == TABLES_DEPTH)
//    {
//        //Calculate the minimal distance between the current page and the page to insert:
//        //Update the max distance frame if the new distance is larger:
//        if (calcCyclicDistance(page_num, fixed_page) > calcCyclicDistance(*currentMaxDistPage, fixed_page))
//        {
//            *currentMaxDistPage = page_num;
//        }
//        return;
//    }
//    // Update the page number - each round, we shift it OFFSET_WIDTH bits to the left to make room for the next "part"
//    page_num <<= OFFSET_WIDTH;
//    int word = 0;
//
//    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
//    {
//        //Get the next frame
//        PMread(frame_index * PAGE_SIZE + i, &word);
//        if (word != 0)
//        {
//            findCyclicDistance(page_num + i, depth + 1, uint64_t(word), fixed_page, currentMaxDistPage);
//        }
//    }
//}
//
///*
// * Finds an empty frame (a frame with 0 in all its rows) which is not the protected frame
// * */
//void findEmptyFrame(uint64_t frame, uint64_t protectedFrame, uint64_t *clearFrame)
//{
//    // Empty frame found - exit recursion
//    if (*clearFrame != -1)
//    {
//        return;
//    }
//    //If the frame is empty and it is not the frame we don't want to use:
//    if (isClear(frame) && frame != protectedFrame)
//    {
//        *clearFrame = frame;
//        return;
//    }
//    int word = 0;
//    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
//    {
//        PMread(frame * PAGE_SIZE + i, &word);
//        if (word != 0)
//        {
//            findEmptyFrame(uint64_t(word), protectedFrame, clearFrame);
//        }
//    }
//}
//
///*
// * Traverses the tree in DFS, and saves the max index of frames visited
// * */
//void findMax(uint64_t frameIndex, uint64_t *maxFrame)
//{
//    int word = 0;
//    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
//    {
//        PMread(frameIndex * PAGE_SIZE + i, &word);
//        if (word != 0)
//        {
//            if (word > *maxFrame)
//            {
//                *maxFrame = uint64_t(word);
//            }
//            findMax(uint64_t(word), maxFrame);
//        }
//    }
//
//}

void
combinedFind(uint64_t frameIndex, uint64_t *emptyFrame, uint64_t *maxFrame, uint64_t *cyclicFrame,
             uint64_t protectedFrame, uint64_t constructedPageNum, int depth, uint64_t pageToInsert,
             uint64_t parent, uint64_t *cycPageNum, uint64_t *stepParent)
{
    //Found empty frame already, no need to continue search
    if (*emptyFrame > 0)
    {
        return;
    }
    //
    if (depth == TABLES_DEPTH)
    {
        //Calculate the minimal distance between the current page and the page to insert:
        //Update the max distance frame if the new distance is larger:
        if (*cyclicFrame == pageToInsert ||
            calcCyclicDistance(constructedPageNum, pageToInsert) <
            calcCyclicDistance(*cyclicFrame, pageToInsert))
        {
            *cyclicFrame = frameIndex;
            *cycPageNum = constructedPageNum;
            *stepParent = parent;
        }
        return;
    }
    //If the frame is empty and it is not the frame we don't want to use:
    if (isClear(frameIndex) && frameIndex != protectedFrame && frameIndex != 0)
    {
        PMwrite(parent * PAGE_SIZE + getOFFSET(constructedPageNum), 0);
        *emptyFrame = frameIndex;
        return;
    }

    int word = 0;
    constructedPageNum <<= OFFSET_WIDTH;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMread(frameIndex * PAGE_SIZE + i, &word);
        if (word != 0)
        {
            if (word > *maxFrame)
            {
                *maxFrame = uint64_t(word);
            }
            combinedFind(uint64_t(word), emptyFrame, maxFrame, cyclicFrame, protectedFrame,
                         constructedPageNum + i,
                         depth + 1, pageToInsert, frameIndex, cycPageNum, stepParent);
        }
    }
}

/*
 * Finds an available frame and returns its index
 * Priority:
 * Empty>Unused>Used(Needs evicting)
 * */
uint64_t getFrame(uint64_t protectedFrame, uint64_t page_num)
{
    //First Priority: Empty Frame
    uint64_t emptyFrame = 0;
    uint64_t maxFrame = 0;
    uint64_t cyclicFrame = page_num;
    uint64_t parent = 0;
    uint64_t cyclicPageNum = 0;
    combinedFind(0, &emptyFrame, &maxFrame, &cyclicFrame, protectedFrame, 0, 0, page_num,
                 0, &cyclicPageNum, &parent);

    //First Priority: Empty Frame
    if (emptyFrame > 0)
    {
        return emptyFrame;
    }
    //Second Priority: Unused Frame:
    //Case: RAM not full yet
    if (maxFrame + 1 < NUM_FRAMES)
    {
        return maxFrame + 1;
    }
    //Third Priority: Evict a page:
    if (cyclicFrame > 0)
    {
        PMevict(cyclicFrame, cyclicPageNum);
        PMwrite(parent * PAGE_SIZE + getOFFSET(cyclicPageNum), 0);
        return cyclicFrame;
    }
    return 0;

}


//TODO: Important!!!!!! When moving an empty frame or evicting a page
/*
 * Returns the frame index of the given page_num
 * */
uint64_t translateVaddress(const uint64_t page_num, const uint64_t *addresses)
{
    int depth = 0;
    int pageWord = 0;
    uint64_t currentFrame = 0;
    uint64_t pageIndexInFrame = 0;
    //Run on each p in addresses- not including the offset d which is in the last spot
    while (depth < TABLES_DEPTH)
    {
        pageIndexInFrame = addresses[depth];
        //cout << pageIndexInFrame << endl;
        PMread(currentFrame * PAGE_SIZE + pageIndexInFrame, &pageWord);
        //Case: Page Fault- handle importing frame
        if (pageWord == 0)
        {
            /*Find an unused frame or evict a page from some frame*/
            uint64_t frame = getFrame(currentFrame, page_num);
            if (frame == 0)
            {
                return 0;
            }
            //Case: Actual Page table and not a page of page tables
            if (depth + 1 == TABLES_DEPTH)
            {
                /*Restore from disk*/
                PMrestore(frame, page_num);
            }
            else
            {
                /*Write 0's to all rows*/
                clearTable(uint64_t(frame));
            }
            pageWord = int(frame);
            //Update the "parent" with the relevant frame index
            PMwrite(currentFrame * PAGE_SIZE + pageIndexInFrame, pageWord);
        }
        currentFrame = uint64_t(pageWord);
        depth++;
    }

    //This is the resulting page we get from the loop: the index of the last page's frame
    return uint64_t(pageWord);
}

void VMinitialize()
{
    clearTable(0);
}


int VMread(uint64_t virtualAddress, word_t *value)
{
    uint64_t paddresses[TABLES_DEPTH + 1];
    getAddressParts(virtualAddress, paddresses);
    uint64_t addr = translateVaddress((virtualAddress >> OFFSET_WIDTH), paddresses);
    //Case: Virtual address cannot be mapped
    if (addr < 1)
    {
        return 0;
    }
    PMread(addr * PAGE_SIZE + paddresses[TABLES_DEPTH], value);
    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value)
{
    uint64_t paddresses[TABLES_DEPTH + 1];
    getAddressParts(virtualAddress, paddresses);
    uint64_t addr = translateVaddress((virtualAddress >> OFFSET_WIDTH), paddresses);
    //Case: Virtual address cannot be mapped
    if (addr < 1)
    {
        return 0;
    }
    PMwrite(addr * PAGE_SIZE + paddresses[TABLES_DEPTH], value);

    return 1;
}


/*----------------------------------------- General Work Flow -----------------------------------------*/
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


 #When traversing the tree, keep an index of the maximal frame visited and of the page visited
 */


/*
 * helper for printTree
 */
void printSubTree1(uint64_t root, int depth, bool isEmptyMode)
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
    printSubTree1(static_cast<uint64_t>(currValue), depth + 1, isEmptyMode);

    //father
    for (int _ = 0; _ < depth; _++)
    {
        std::cout << '\t';
    }
    if (isEmptyMode)
    {
        std::cout << '_' << '\n';
    }
    else
    {
        if (depth == TABLES_DEPTH - 1)
        {
            word_t a, b;
            PMread(root * PAGE_SIZE + 0, &a);
            PMread(root * PAGE_SIZE + 1, &b);
            std::cout << root << " -> (" << a << ',' << b << ")\n";
        }
        else
        {
            std::cout << root << '\n';
        }
    }

    //left son
    PMread(root
           * PAGE_SIZE + 0, &currValue);
    printSubTree1(static_cast <uint64_t>(currValue), depth + 1, isEmptyMode);
}

/**
 * print's the virtual memory tree. feel free to use this function is Virtual Memory for debuging.
 */
void printTree1()
{
    std::cout << "---------------------" << '\n';
    std::cout << "Virtual Memory:" << '\n';
    printSubTree1(0, 0, false);
    std::cout << "---------------------" << '\n';
}
