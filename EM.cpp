//
//  EM.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/26/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#include "EM.h"

EM& EM::operator = (EM &anotherEM) {
    this->nextid = anotherEM.getNextId();
    this->currbuffsize = anotherEM.getCurrBuffSize();
    int i = this->currbuffsize;
    while (i > 0) {
        this->buff[i] = anotherEM.buff[i];
        i--;
    }
    return *this;
}

void EM::incrementNextId(){
    this->nextid++;
}

void EM::resetNextId(){
    this->nextid = 0;
}

int EM::getNextId() const {
    return nextid;
}

int EM::getCurrBuffSize() {
    return currbuffsize;
}

en_msg * EM::getBuffer(int buffsize){
    return this->buff[buffsize];
}

void EM::setNextId(int nextid) {
    this->nextid = nextid;
}

void EM::setCurrBuffSize(int currbuffsize) {
    this->currbuffsize = currbuffsize;
}

void EM::decrementCurrBuffSize(){
    this->currbuffsize--;
}

void EM::incrementCurrBuffSize(){
    this->currbuffsize++;
}

void EM::setBuffer(int buffsize , en_msg * buff){
    this->buff[buffsize] = buff;
}

EM::~EM(){
}

