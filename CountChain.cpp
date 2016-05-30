//
// Created by root on 5/23/16.
//

#include "CountChain.h"

CountChain::CountChain(int CMax) {
    for(int i = 0; i < CMax; ++i) {
        auto myPair = new pair<CDE *, CDE*>;
        myPair->first = nullptr;
        myPair->second = nullptr;
        _countChain.push_back(myPair);
    }
}

void CountChain::insert(CDE * cde, int pos) {
    --pos;
    CDE * head = _countChain.at(pos)->first;
    CDE * tail = _countChain.at(pos)->second;

    if(head != nullptr) {
        cde->setCountNext(head);
        head->setCountPrev(cde);
    }
    _countChain.at(pos)->first = cde;

    if(tail == nullptr) {
        _countChain.at(pos)->second = cde;
    }
}

void CountChain::increment(CDE * cde) {
    if(!cde->getIsNew()) { // increase only if not new
        int count = cde->getCount();
        if((size_t) count <= _countChain.size()) {
            remove(cde);
            if((size_t) count < _countChain.size()) {
                insert(cde, count + 1);
            }
        }
        cde->setCountPrev(nullptr);
        cde->increaseCount();
    }
}

void CountChain::remove(CDE * cde) {
    if((size_t) cde->getCount() > _countChain.size()) {
        return;
    }
    if(cde->getCountPrev() != nullptr) {
        cde->getCountPrev()->setCountNext(cde->getCountNext());
    } else {
        _countChain.at(cde->getCount() - 1)->first = cde->getCountNext();
    }

    if(cde->getCountNext() != nullptr) {
        cde->getCountNext()->setCountPrev(cde->getCountPrev());
        cde->setCountNext(nullptr);
    } else {
        _countChain.at(cde->getCount() - 1)->second = cde->getCountPrev();
    }
}

CDE * CountChain::getItemToRemove() {
    for(auto it : _countChain) {
        if(it->second != nullptr) {
            if(it->second->getIsOld()) {
                return it->second;
            }
        }

    }
    return nullptr;
}