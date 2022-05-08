//
//  QuorumTracker.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 1/13/18.
//  Copyright © 2018 Monika Sharma. All rights reserved.
//

#ifndef QuorumTracker_hpp
#define QuorumTracker_hpp

#include "stdincludes.h"
#include "Log.h"
#include "Message.h"
#include "Params.h"

struct QuoromDetail
{
    QuoromDetail()
        : replyCounter(0)
        , reqTime(0)
        {
        }

    MessageType transMsgType;
    unsigned replyCounter;
    int reqTime;
    string reqKey;
};

class QuorumTracker
{
private:
    Params *m_parameters;
    Log * m_logger;

    // TODO: this is only a logger dependency, remove once logger is independent of addres
    Address m_address;

    // container for tracking quorom for messages
    std::map<int, struct QuoromDetail> m_quorum;

    bool isQuorumExpired(const QuoromDetail& quoromDetail);
    void logFail(int transID, const QuoromDetail& quoromDetail);

public:
    virtual ~QuorumTracker();
    QuorumTracker(
        Address addr,
        Params *par,
        Log *log);

    void updateQuorum(int transID, MessageType msgType, string key);
    void removeExpiredQuorums();
    bool quorumExists(int transID);
    QuoromDetail getQuorum(int transID);
    void removeQuorum(int transID);
    void saveQuorum(int transID, QuoromDetail record);
};

#endif /* QuorumTracker_hpp */
