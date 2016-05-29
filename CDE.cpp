#include <cstring>
#include "CDE.h"
#include <iostream>

CDE::CDE(int blockId, string fileName, size_t dataSize,
         char * blockData) {
    //cout << "\tCDE CONSTRACTOR START" << endl;
    _dataSize = dataSize;
    _fileName = fileName;
    _blockId = blockId;
    _blockData = (char *) malloc(sizeof(char) * dataSize);
    if(_blockData == NULL) {
        cout << "malloc ERROR" << endl;
    }
    memcpy(_blockData, blockData, dataSize);
    _isOld = false;
    _isNew = true;
    _count = 1;
    _countNext = nullptr;
    _countPrev = nullptr;
    _next = nullptr;
    _prev = nullptr;
    //cout << "\tCDE CONSTRACTOR FINISH" << endl;
};

CDE::~CDE() {
    free(_blockData);
};