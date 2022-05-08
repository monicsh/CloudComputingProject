//
//  MemberListEntry.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef MemberListEntry_hpp
#define MemberListEntry_hpp

#include "stdincludes.h"

/**
 * CLASS NAME: MemberListEntry
 *
 * DESCRIPTION: Entry in the membership list
 */
class MemberListEntry {
private:
    int id;
    short port;
    long heartbeat;
    long timestamp;
public:
    MemberListEntry(int id, short port, long heartbeat, long timestamp);
    MemberListEntry(int id, short port);
    MemberListEntry(): id(0), port(0), heartbeat(0), timestamp(0) {}
    MemberListEntry(const MemberListEntry &anotherMLE);
    MemberListEntry& operator =(const MemberListEntry &anotherMLE);
    int getid();
    short getport();
    long getheartbeat();
    long gettimestamp();
    void setid(int id);
    void setport(short port);
    void setheartbeat(long hearbeat);
    void settimestamp(long timestamp);
};

#endif /* MemberListEntry_hpp */
