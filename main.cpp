#include <iostream>
#include <bitset>
#include "MemoryConstants.h"
#include "VirtualMemory.h"

#include <math.h>
#include <vector>

using namespace std;

int main()
{
//    unsigned long long int moveit = ((1 << 4) - 1);
//    int num = 889180;
//    auto m = moveit;
    vector<unsigned int long long> arr;
//
//
// while (num != 0)
//    {
//        unsigned long long int a = num & m;
//        arr.push_back(a);
//        num>>=4;
//    }
//
//    for (auto i: arr)
//        std::cout << i << ' ';

    readAddress(889180,arr);
    for (auto i: arr)
        std::cout << i << ' ';
    return 0;
}