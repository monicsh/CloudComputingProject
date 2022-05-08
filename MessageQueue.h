//
//  MessageQueue.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef MessageQueue_hpp
#define MessageQueue_hpp

#include "stdincludes.h"

#include "RawMessage.h"
#include "IMessageQueue.h"


class MessageQueue : public IMessageQueue
{
private:
    std::queue<RawMessage> m_messages;
    
public:
    MessageQueue();
    virtual ~MessageQueue();
    bool empty() override;
    void enqueue(void *buffer, int size) override;
    RawMessage dequeue() override;
};

#endif /* MessageQueue_hpp */
