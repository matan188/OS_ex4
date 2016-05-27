//
// Created by root on 5/23/16.
//

#ifndef OS_EX4_COUNTCHAIN_H
#define OS_EX4_COUNTCHAIN_H
#include <vector>
#include "CDE.h"
#include <iostream>

using namespace std;

class CountChain {
private:
    vector<pair<CDE *, CDE *>*> _countChain;
public:
    CountChain(int CMax);
    void insert(CDE * cde, int pos);
    void increment(CDE * cde);
    void remove(CDE * cde);
    CDE * getItemToRemove();
    void printCountChain();
};


#endif //OS_EX4_COUNTCHAIN_H
