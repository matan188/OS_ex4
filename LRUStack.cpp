//
// Created by root on 5/21/16.
//

#include "LRUStack.h"

using namespace std;

LRUStack::LRUStack() {
    _size = 0;
    _head = nullptr;
    _tail = nullptr;
    _newBoundary = nullptr;
    _oldBoundary = nullptr;
}

LRUStack::~LRUStack() {
    CDE * cde = _head;
    while(cde != nullptr) {
        CDE* tmpCde = cde;
        cde = cde->getNext();
        delete tmpCde;
    }
}

void LRUStack::insert(CDE * newCde) {

    if(_size == 0) {
        _tail = newCde;
    } else {
        _head->setPrev(newCde);
        newCde->setNext(_head);
    }
    _head = newCde;

    ++_size;
    if(_size == (size_t) _newIndex) {
        _newBoundary = _tail;
    } else if(_size == (size_t) _oldIndex) {
        _oldBoundary = _tail;
        _oldBoundary->setIsOld(true);
        _oldBoundary->setIsNew(false);
    }

    if(_size > (size_t) _newIndex) {
        _newBoundary->setIsNew(false);
        _newBoundary = _newBoundary->getPrev();
    }
};

void LRUStack::remove(CDE * cde) {
    _oldBoundary = cde->getPrev(); // prev because insert is coming
    _oldBoundary->setIsOld(true);
    _oldBoundary->setIsNew(false);

    if(_head == _tail) {
        _head = nullptr;
        _tail = nullptr;
    } else if(cde == _head) {
        _head = cde->getNext();
        cde->getNext()->setPrev(nullptr);
    } else if(cde == _tail) {
        _tail = cde->getPrev();
        cde->getPrev()->setNext(nullptr);
    } else {
        cde->getPrev()->setNext(cde->getNext());
        cde->getNext()->setPrev(cde->getPrev());
    }
    --_size;
};

void LRUStack::reinsert(CDE * cde) {

    // update new boundary
    if(cde->getIsNew()) {
        if(_newBoundary == cde) {
            if(cde->getPrev() != nullptr) {
                _newBoundary = cde->getPrev();
            }
        }
    } else {
        if(_newBoundary->getPrev() == nullptr) {
            _newBoundary->setIsNew(false);
            _newBoundary = cde;
        } else {
            _newBoundary->setIsNew(false);
            _newBoundary = _newBoundary->getPrev();
        }
    }

    // update old boundary
    if(cde->getIsOld()) {
        _oldBoundary = _oldBoundary->getPrev(); // prev because insert is coming
        _oldBoundary->setIsOld(true);
        _oldBoundary->setIsNew(false);
    }

    cde->setIsOld(false);
    cde->setIsNew(true);

    if(_head != cde) {
        if(_tail == cde) {
            _tail = cde->getPrev();
            _tail->setNext(nullptr);
        } else {
            cde->getPrev()->setNext(cde->getNext());
            cde->getNext()->setPrev(cde->getPrev());
        }
        cde->setNext(_head);
        _head->setPrev(cde);
        _head = cde;
        _head->setPrev(nullptr);
    }
};

void LRUStack::printLru() {
    CDE * cde = _head;
    if( _head != nullptr && _tail != nullptr) {
        cout << "\thead: <" << _head->getFileName() << "," << _head->getBlockId() << ">" << endl;
        cout << "\ttail: <" << _tail->getFileName() << "," << _tail->getBlockId() << ">" << endl;
        cout << "\t";
    }

    while(cde != nullptr) {
        cout << "<";
        if(cde == _newBoundary) {
            cout << "nb, ";
        }
        if(cde == _oldBoundary) {
            cout << "ob, ";
        }
        cout << cde->getFileName() << "," << cde->getBlockId() << "," << cde->getIsNew() <<"," << cde->getIsOld() << ">" << " -> ";
        cde = cde->getNext();
    }
    cout << endl;
}