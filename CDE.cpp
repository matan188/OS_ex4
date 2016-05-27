#include <cstring>
#include "CDE.h"

CDE::CDE(int blockId, string fileName, char * blockData, size_t dataSize): _blockId(blockId),
                                                             _fileName(fileName), _dataSize(dataSize) {
    _blockData = (char *) malloc(sizeof(char)*dataSize);
    strncpy(_blockData, blockData, dataSize);
    _isOld = false;
    _isNew = true;
    _count = 1;
    _countNext = nullptr;
    _countPrev = nullptr;
    _next = nullptr;
    _prev = nullptr;
};

CDE::~CDE() {
    free(_blockData);
};