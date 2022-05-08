/**********************************
 * FILE NAME: Member.h
 *
 * DESCRIPTION: Definition of all Member related class
 **********************************/

#ifndef MEMBER_H_
#define MEMBER_H_

#include "stdincludes.h"
#include "Address.h"
#include "MemberListEntry.h"


/**
 * CLASS NAME: Member
 *
 * DESCRIPTION: Class representing a member in the distributed system
 */
// Declaration and definition here
class Member {
public:
	// This member's Address
	Address addr;
	// boolean indicating if this member is up
	bool inited;
	// boolean indicating if this member is in the group
	bool inGroup;
	// boolean indicating if this member has failed
	bool bFailed;
	// number of my neighbors
	int nnb;
	// the node's own heartbeat
	long heartbeat;
	// counter for next ping
	int pingCounter;
	// counter for ping timeout
	int timeOutCounter;
	// Membership table
	vector<MemberListEntry> memberList;
	// My position in the membership table
	vector<MemberListEntry>::iterator myPos;
	/**
	 * Constructor
	 */
	Member(): inited(false), inGroup(false), bFailed(false), nnb(0), heartbeat(0), pingCounter(0), timeOutCounter(0) {}
	// copy constructor
	Member(const Member &anotherMember);
	// Assignment operator overloading
	Member& operator =(const Member &anotherMember);
	virtual ~Member() {}
};

#endif /* MEMBER_H_ */
