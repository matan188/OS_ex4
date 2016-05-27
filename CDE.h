
#ifndef OS_EX4_CDE_H
#define OS_EX4_CDE_H

#include <string>
#include <cstdlib>
#include <stdlib.h>

using namespace std;

class CDE {
private:
    int _count;
    size_t _dataSize;
    CDE * _countPrev;
    CDE * _countNext;
    CDE * _prev;
    CDE * _next;
    bool _isOld, _isNew;
    int _blockId;
    string _fileName;
    char * _blockData;
public:
    CDE(int blockId, string fileName, char * blockData, size_t dataSize);
    ~CDE();

    char * getData() { return _blockData; };
    size_t getSize() { return _dataSize; };

    void setPrev(CDE * cde) { _prev = cde; };
    void setNext(CDE * cde) { _next = cde; };

    void setCountPrev(CDE * cde) { _countPrev = cde; };
    void setCountNext(CDE * cde) { _countNext = cde; };

    CDE * getPrev() { return _prev; };
    CDE * getNext() { return _next; };

    CDE * getCountPrev() { return _countPrev; };
    CDE * getCountNext() { return _countNext; };

    void increaseCount() { ++_count; };

    void setIsNew(bool b) { _isNew = b; };
    void setIsOld(bool b) { _isOld = b; };

    bool getIsOld() { return _isOld; };
    bool getIsNew() { return _isNew; };

    void setFileName(string fileName) { _fileName = fileName; };

    int getCount() { return _count; };

    int getBlockId() { return _blockId; };
    string getFileName() { return _fileName; };
};


#endif //OS_EX4_CDE_H
