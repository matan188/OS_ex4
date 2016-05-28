//
// Created by root on 5/21/16.
//

#ifndef OS_EX4_LRUSTACK_H
#define OS_EX4_LRUSTACK_H

#include "CDE.h"
#include <iostream>

class LRUStack {
private:
    CDE * _head;
    CDE * _tail;
    CDE * _newBoundary;
    CDE * _oldBoundary;
    size_t _size;
    int _newIndex, _oldIndex;
public:
    LRUStack();
    ~LRUStack();

    size_t getSize() { return _size; };
    void insert(CDE * cde);
    void remove(CDE * cde);
    void reinsert(CDE * cde);

    void setNewIndex(int newIndex) { _newIndex = newIndex; };
    void setOldIndex(int oldIndex) { _oldIndex = oldIndex; };

    void setNewBoundary(CDE * cde) { _newBoundary = cde; };
    void setOldBoundary(CDE * cde) { _oldBoundary = cde; };

    CDE * setNewBoundary() { return _newBoundary; };
    CDE * setOldBoundary() { return _oldBoundary; };

    CDE * getTail() { return _tail; };
    CDE * getHead() { return _head; };

    void printLru();
};


#endif //OS_EX4_LRUSTACK_H
