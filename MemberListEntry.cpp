//
//  MemberListEntry.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#include "MemberListEntry.h"

/**
 * Constructor
 */
MemberListEntry::MemberListEntry(int id, short port, long heartbeat, long timestamp): id(id), port(port), heartbeat(heartbeat), timestamp(timestamp) {}

/**
 * Constuctor
 */
MemberListEntry::MemberListEntry(int id, short port): id(id), port(port) {}

/**
 * Copy constructor
 */
MemberListEntry::MemberListEntry(const MemberListEntry &anotherMLE) {
    this->heartbeat = anotherMLE.heartbeat;
    this->id = anotherMLE.id;
    this->port = anotherMLE.port;
    this->timestamp = anotherMLE.timestamp;
}

/**
 * Assignment operator overloading
 */
MemberListEntry& MemberListEntry::operator =(const MemberListEntry &anotherMLE) {
    MemberListEntry temp(anotherMLE);
    swap(heartbeat, temp.heartbeat);
    swap(id, temp.id);
    swap(port, temp.port);
    swap(timestamp, temp.timestamp);
    return *this;
}

/**
 * FUNCTION NAME: getid
 *
 * DESCRIPTION: getter
 */
int MemberListEntry::getid() {
    return id;
}

/**
 * FUNCTION NAME: getport
 *
 * DESCRIPTION: getter
 */
short MemberListEntry::getport() {
    return port;
}

/**
 * FUNCTION NAME: getheartbeat
 *
 * DESCRIPTION: getter
 */
long MemberListEntry::getheartbeat() {
    return heartbeat;
}

/**
 * FUNCTION NAME: gettimestamp
 *
 * DESCRIPTION: getter
 */
long MemberListEntry::gettimestamp() {
    return timestamp;
}

/**
 * FUNCTION NAME: setid
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::setid(int id) {
    this->id = id;
}

/**
 * FUNCTION NAME: setport
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::setport(short port) {
    this->port = port;
}

/**
 * FUNCTION NAME: setheartbeat
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::setheartbeat(long hearbeat) {
    this->heartbeat = hearbeat;
}

/**
 * FUNCTION NAME: settimestamp
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::settimestamp(long timestamp) {
    this->timestamp = timestamp;
}
