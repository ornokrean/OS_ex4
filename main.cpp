#include <iostream>
#include <bitset>
#include "MemoryConstants.h"
#include "VirtualMemory.h"


using namespace std;

int main()
{
    VMinitialize();
    VMwrite(13,3);
    printTree();

    word_t value;

    VMread(6, &value);
    printTree();


    VMread(31, &value);
    printTree();
    return 0;
}