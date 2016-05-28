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
    //cout << "\t$ insert"<< endl;

    --pos;
    CDE * head = _countChain.at(pos)->first;
    CDE * tail = _countChain.at(pos)->second;

    if(head != nullptr) {
        cde->setCountNext(head);
        head->setCountPrev(cde);
    }
    _countChain.at(pos)->first = cde;
    cout << _countChain.at(1)->first << endl;

    if(tail == nullptr) {
        _countChain.at(pos)->second = cde;
    }
    //printCountChain();
}

void CountChain::increment(CDE * cde) {
    //cout << "\t$ increment"<< endl;
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
    //printCountChain();
}

void CountChain::remove(CDE * cde) {
    //cout << "\t$ remove "<< cde->getFileName() << endl;
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
    //printCountChain();
}

CDE * CountChain::getItemToRemove() {
    //cout << "\t$ getItemToRemove"<< endl;
    for(auto it : _countChain) {
        if(it->second != nullptr) {
            if(it->second->getIsOld()) {
                return it->second;
            }
        }

    }
    //printCountChain();
    return nullptr;
}

void CountChain::printCountChain() {
    //cout << "print count" << endl;

    for(int i = 0; i < (int) _countChain.size(); ++i) {
        CDE * cde = _countChain.at(i)->first;
        cout << "\t(" << i + 1 << ") : ";

        while(cde != nullptr) {
            cout << cde->getFileName() << " -> ";
            cde = cde->getCountNext();
        }
        cout << endl;
    }
}