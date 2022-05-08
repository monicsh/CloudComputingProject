//
//  IMessageQueueu.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef IMessageQueue_hpp
#define IMessageQueue_hpp

#include "RawMessage.h"

//Interface Q
class IMessageQueue {
public:
    virtual bool empty()=0;
    virtual void enqueue(void *buffer, int size)=0;
    virtual RawMessage dequeue()=0;
    
};

#endif /* IMessageQueue_hpp */

