#include "VirtualMemory.h"

#include <cstdio>
#include <cassert>
#include <iostream>

int main(int argc, char **argv) {
    VMinitialize();
    //(2 * NUM_FRAMES)
//    for (uint64_t i = 0; i < 4; ++i) {
//        printf("writing to %llu\n", i);
//        VMwrite(5 * i * PAGE_SIZE, i);
//    }
//
//    for (uint64_t i = 0; i < 4; ++i) {
//        word_t value;
//        VMread(5 * i * PAGE_SIZE, &value);
//        printf("reading from %llu %d\n", i, value);
//        std::cout<<value<<std::endl;
////        assert(uint64_t(value) == i);
//    }


//    word_t value;
//    VMwrite(0 , 0);
//    VMread(0 , &value);
//    std::cout<<value<<std::endl;
//    VMwrite(80 , 1);
//    VMread(80 , &value);
//    std::cout<<value<<std::endl;
//    VMwrite(160 , 2);
//    VMread(160 , &value);
//    std::cout<<value<<std::endl;
//    VMwrite(240 , 3);
//    VMread(240 , &value);
//    std::cout<<value<<std::endl;


    word_t value;
    VMwrite(80 , 1);
    uint64_t framesInUse = dfs(0,0);
//    VMread(0 , &value);
//    std::cout<<value<<std::endl;
    VMwrite(96 , 3);
//    VMread(80 , &value);
//    std::cout<<value<<std::endl;
    VMread(80 , &value);
    std::cout<<value<<std::endl;
    VMread(96 , &value);
    std::cout<<value<<std::endl;
//    VMread(83 , &value);
//    std::cout<<value<<std::endl;



    printf("success\n");

    return 0;
}