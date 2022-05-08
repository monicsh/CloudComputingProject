/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MembershipProtocol.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MembershipProtocol::MembershipProtocol(Member *member, Params *params, EmulNet *emul, Log *log, Address *address, IMessageQueue* queue) {
    for( int i = 0; i < 6; i++ ) {
        NULLADDR[i] = 0;
    }
    this->m_memberNode = member;
    this->m_emulNet = emul;
    this->m_log = log;
    this->m_par = params;
    this->m_memberNode->addr = *address;
    this->m_queue = queue;
}

/**
 * Destructor of the MP1Node class
 */
MembershipProtocol::~MembershipProtocol() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MembershipProtocol::recvLoop() {
	if ( m_memberNode->bFailed ) {
		return false;
	}
	else {
        return m_emulNet->ENrecv(&(m_memberNode->addr), this->m_queue, NULL, 1);
	}
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MembershipProtocol::nodeStart(char *servaddrstr, short servport) {
	Address joinaddr;
	joinaddr = getJoinAddress();
	
	// Self booting routines
	if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
		m_log->LOG(&m_memberNode->addr, "init_thisnode failed. Exit.");
#endif
		exit(1);
	}
	
	if( !introduceSelfToGroup(&joinaddr) ) {
		finishUpThisNode();
#ifdef DEBUGLOG
		m_log->LOG(&m_memberNode->addr, "Unable to join self to group. Exiting.");
#endif
		exit(1);
	}
	
	return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MembershipProtocol::initThisNode(Address *joinaddr) {
	/*
	 * This function is partially implemented and may require changes
	 */
	int id = *(int*)(&m_memberNode->addr.m_addr);
	int port = *(short*)(&m_memberNode->addr.m_addr[4]);
	
	m_memberNode->bFailed = false;
	m_memberNode->inited = true;
	m_memberNode->inGroup = false;
	// node is up!
	m_memberNode->nnb = 0;
	m_memberNode->heartbeat = 0;
	m_memberNode->pingCounter = TFAIL;
	m_memberNode->timeOutCounter = -1;
	initMemberListTable(m_memberNode);
	
	return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MembershipProtocol::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
#ifdef DEBUGLOG
	static char s[1024];
#endif
	
	if ( 0 == memcmp((char *)&(m_memberNode->addr.m_addr), (char *)&(joinaddr->m_addr), sizeof(m_memberNode->addr.m_addr))) {
		// I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
		m_log->LOG(&m_memberNode->addr, "Starting up group...");
#endif
		m_memberNode->inGroup = true;
		
		// make an own entry into the list
		int myId = atoi(m_memberNode->addr.getAddress().substr(0,m_memberNode->addr.getAddress().find(":")).c_str());
		short myPort = atoi(m_memberNode->addr.getAddress().substr(m_memberNode->addr.getAddress().find(":")+1).c_str());
		MemberListEntry myEntry =  MemberListEntry(myId, myPort, m_memberNode->heartbeat, m_par->getcurrtime());
		m_memberNode->memberList.push_back(myEntry);
		m_log->logNodeAdd(&m_memberNode->addr, &m_memberNode->addr);
		auto  pos = m_memberNode->memberList.begin();
		m_memberNode->myPos = pos;
		
	}
	else {
		size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->m_addr) + sizeof(long) + 1;
		msg = (MessageHdr *) malloc(msgsize * sizeof(char));
		
		// create JOINREQ message: format of data is {struct Address myaddr}
		msg->msgType = JOINREQ;
		memcpy((char *)(msg+1), &m_memberNode->addr.m_addr, sizeof(m_memberNode->addr.m_addr));
		memcpy((char *)(msg+1) + 1 + sizeof(m_memberNode->addr.m_addr), &m_memberNode->heartbeat, sizeof(long));
		
#ifdef DEBUGLOG
		sprintf(s, "Trying to join...");
		m_log->LOG(&m_memberNode->addr, s);
#endif
		
		// make an own entry into the list
		int myId = atoi(m_memberNode->addr.getAddress().substr(0,m_memberNode->addr.getAddress().find(":")).c_str());
		short myPort = atoi(m_memberNode->addr.getAddress().substr(m_memberNode->addr.getAddress().find(":")+1).c_str());
		MemberListEntry myEntry =  MemberListEntry(myId, myPort, m_memberNode->heartbeat, m_par->getcurrtime());
		m_memberNode->memberList.push_back(myEntry);
		m_log->logNodeAdd(&m_memberNode->addr, &m_memberNode->addr);
		auto  pos = m_memberNode->memberList.begin();
		m_memberNode->myPos = pos;
		
		// send JOINREQ message to introducer member
		m_emulNet->ENsend(&m_memberNode->addr, joinaddr, (char *)msg, (int)msgsize);
		//std::cout << "\nDEBUG: ENSend from: " << memberNode->addr.getAddress() << ", to: " << joinaddr->getAddress() << "; msg type: " << msg->msgType << " ;heartbeat: " << memberNode->heartbeat << std::endl;
		
		free(msg);
	}
	
	return 1;
	
}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MembershipProtocol::finishUpThisNode() {
	/*
	 * Your code goes here
	 */
	return 0;
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MembershipProtocol::nodeLoop() {
	if (m_memberNode->bFailed) {
		return;
	}
	
	// Check my messages
	checkMessages();
	
	// Wait until you're in the group...
	if( !m_memberNode->inGroup ) {
		return;
	}
	
	// ...then jump in and share your responsibilites!
	nodeLoopOps();
	
	return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MembershipProtocol::checkMessages() {
	void *ptr;
	int size;
	
	// Pop waiting messages from memberNode's mp1q
	while ( !this->m_queue->empty() ) {
        RawMessage msg = this->m_queue->dequeue();
		ptr = msg.elt;
		size = msg.size;
		recvCallBack((void *)m_memberNode, (char *)ptr, size);
	}
	return;
}


/** FUNCTION NAME : sendMsgBack
 *  Introducer response ro JOINREQ with JOINREP
 *
 */
void MembershipProtocol::sendMsgBack(Address* toNode, MsgTypes msgType) {
	size_t msgsize = sizeof(MessageHdr) + sizeof(toNode->m_addr) + sizeof(long) + 1;
	MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));
	
	// create JOINREP message: format of data is {struct Address myaddr}
	msg->msgType = msgType;
	memcpy((char *)(msg+1), &m_memberNode->addr.m_addr, sizeof(m_memberNode->addr.m_addr));
	memcpy((char *)(msg+1) + 1 + sizeof(m_memberNode->addr.m_addr), &m_memberNode->heartbeat, sizeof(long));
	
	// send JOINREP message to new member
	m_emulNet->ENsend(&m_memberNode->addr, toNode, (char *)msg, (int)msgsize);
	free(msg);
}

/** FUNCTION NAME : sendMemberListEntry
 *
 *  example- send new member info as NEWMEMBER(msgtype)t o all member
 */
void MembershipProtocol::sendMemberListEntry(MemberListEntry* entry, Address* toNode, MsgTypes msgType) {
	size_t msgsize = sizeof(MessageHdr) + (sizeof(Address)+1) + sizeof(long) +  1;
	MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));
	
	// create NEWMEMBER message
	string newMemberAddressString = to_string(entry->getid()) + ":" + to_string(entry->getport());
	Address newMemberAddress = Address(newMemberAddressString);
	long newMemberHeartbeat = entry->getheartbeat();
	
	msg->msgType = msgType;
	memcpy((char *)(msg+1), (char *)(newMemberAddress.m_addr), sizeof(newMemberAddress.m_addr));
	memcpy((char *)(msg+1)+1 + sizeof(newMemberAddress.m_addr), &newMemberHeartbeat ,sizeof(long));
	
	m_emulNet->ENsend(&m_memberNode->addr, toNode, (char *)msg, (int)msgsize);
	free(msg);
	
}

/** FUNCTION NAME: updateHeartbeat
 *
 * DESCRIPTION: check heartbeat of a received entry in regular message whether it need to be updated in my list
 *              make a new entry in the list if does not find in the list
 */
void MembershipProtocol::updateHeartbeat(int id, short port, long heartbeat){
	bool found = false;
	for(auto  it = m_memberNode->memberList.begin(); it != m_memberNode->memberList.end(); it++){
		if (it->getid() == id and it->getport() == port){
			if (it->getheartbeat() < heartbeat) {
				it->setheartbeat(heartbeat);
				it->settimestamp(m_par->getcurrtime());
			}
			
			found = true;
			break;
		}
	}
	
	// new entry
	if (!found){
		MemberListEntry newEntry = MemberListEntry(id, port, heartbeat, m_par->getcurrtime());
		m_memberNode->memberList.push_back(newEntry);
		
		Address addToLog = Address(to_string(id) + ":" + to_string(port));
		m_log->logNodeAdd(&m_memberNode->addr, &addToLog);
	}
	
	return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MembershipProtocol::recvCallBack(void *env, char *data, int size ) {
	
	Member* member = (Member*)env;
	MessageHdr* msg = (MessageHdr*)data;
	data += sizeof(MessageHdr);
	Address* address = (Address*)data;
	data += 7;
	long heartbeat = (long)(*data);
	
	// Extract id and port from Address
	int addId = atoi(address->getAddress().substr(0,address->getAddress().find(":")).c_str());
	short addPort = atoi(address->getAddress().substr(address->getAddress().find(":")+1).c_str());
	
	// JOINREQ : response the request with JOINREP, send new member's info to others, update own memberlist
	if (msg->msgType == JOINREQ){
		
		// 1. acknoledge this new node with JOINREP message
		sendMsgBack(address, JOINREP);
		
		// 2. Add new entry in own memberList
		MemberListEntry newEntry = MemberListEntry(addId, addPort, heartbeat, m_par->getcurrtime());
		member->memberList.push_back(newEntry);
		
		Address addToLog = Address(to_string(addId) + ":" + to_string(addPort));
		m_log->logNodeAdd(&m_memberNode->addr, &addToLog);
		
		// 3. send info about new join member to all other memberss (NEWMEMBER)
		for (auto  it = member->memberList.begin(); it != member->memberList.end(); it++){
			string address = to_string(it->getid()) + ":" + to_string(it->getport());
			Address toNode = Address(address);
			if (m_memberNode->memberList[0].getid() != it->getid()){
				sendMemberListEntry(&newEntry, &toNode, NEWMEMBER);
			}
		}
		
	} else if (msg->msgType == JOINREP){
		
		m_memberNode->inGroup = true;
		m_memberNode->inited = true;
		
		//make own MemberListEntry in memberList
		MemberListEntry coordinatorEntry = MemberListEntry(addId, addPort, heartbeat, m_par->getcurrtime());
		
		bool isDuplicate = false;
		for (auto  it = member->memberList.begin(); it != member->memberList.end(); it++){
			if ( it->getid() == addId){
				// duplicate message
				isDuplicate = true;
			}
			
		}
		if (!isDuplicate){
			m_memberNode->memberList.push_back(coordinatorEntry);
			
			Address addToLog = Address(to_string(addId)+":"+to_string(addPort));
			m_log->logNodeAdd(&m_memberNode->addr, &addToLog);
		}
		
	} else if (msg->msgType == NEWMEMBER){
		//std::cout<<"I am: " <<memberNode->addr.getAddress()<<"new member is: "<< address->getAddress()<<endl;
		if (m_memberNode->memberList[0].getid() != addId and m_memberNode->memberList[0].getid() != 1){
			MemberListEntry newEntry = MemberListEntry(addId, addPort, heartbeat, m_par->getcurrtime());
			
			bool isDuplicate = false;
			for (auto  it = member->memberList.begin(); it != member->memberList.end(); it++){
				if ( it->getid() == addId){
					// duplicate message
					isDuplicate = true;
				}
				
			}
			
			if (!isDuplicate){
				member->memberList.push_back(newEntry);
				
				Address addToLog = Address(to_string(addId)+":"+to_string(addPort));
				m_log->logNodeAdd(&m_memberNode->addr, &addToLog);
			}
		}
		
	} else if (msg->msgType == REGULAR){
		//update memberList for the member if the incoming heartbeat > heartbeat
		//std::cout<<"REGULAR message: from: "<<addId<<endl;
		if (!m_memberNode->memberList.empty() && m_memberNode->memberList[0].getid() != addId){
			updateHeartbeat(addId, addPort,heartbeat);
		}
		
	} else if (msg->msgType == LEAVEGROUP){
		// whichone?
		//addId and addPort
	}
	
	/*
	 else if (msg->msgType == LEAVEGROUP){
	 //delete the member from the list
	 if (deleteLeaveMemberEntry(addId, addPort)) {
	 
	 //Address nodeToRemove(to_string(addId) + ":" + to_string(addPort));
	 //log->logNodeRemove(&memberNode->addr, &nodeToRemove);
	 
	 //propogate the message to all members
	 for (auto it = memberNode->memberList.begin() + 1; it != memberNode->memberList.end(); it++) {
	 Address toNode(to_string(it->getid()) + ":" + to_string(it->getport()));
	 sendLeaveMessage(&toNode , addId, addPort, LEAVEGROUP);
	 }
	 
	 }
	 
	 }
	 */
	
	return true;
}


/**
 * FUNCTION NAME: sendLeaveMessage
 */
void MembershipProtocol::sendLeaveMessage(Address* toNode, int addId, short addPort, MsgTypes msgType) {
	size_t msgsize = sizeof(MessageHdr) + sizeof(toNode->m_addr) + sizeof(long) + 1;
	MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));
	
	Address leaveMemberAddress(to_string(addId) + ":" + to_string(addPort));
	
	// create LEAVEGROUP message: format of data is {struct Address myaddr}
	msg->msgType = msgType;
	memcpy((char *)(msg+1), leaveMemberAddress.m_addr, sizeof(Address));
	memcpy((char *)(msg+1) + 1 + sizeof(Address), &m_memberNode->heartbeat, sizeof(long));
	
	// send LEAVEGROUP message to new member
	m_emulNet->ENsend(&m_memberNode->addr, toNode, (char *)msg, (int)msgsize);
	free(msg);
}


/**
 * FUNCTION NAME: deleteLeaveMemberEntry
 *
 * DESCRIPTION: delete the leaving member from the member list
 **//*
	 bool MP1Node::deleteLeaveMemberEntry(int addId, short addPort)
	 {
	 //bool mismatchedItr = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->myPos->getid());
	 // bool mismatchedIndex = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->memberList.begin()->getid());
	 
	 for(auto it = memberNode->memberList.begin() + 1; it != memberNode->memberList.end(); it++) {
	 
	 if (it->getid() == addId and it->getport() == addPort) {
	 
	 //Address nodeToRemove(AddressToString(*it));
	 //log->logNodeRemove(&memberNode->addr, &nodeToRemove);
	 
	 it = memberNode->memberList.erase(it);
	 it--;
	 return true;
	 }
	 }
	 
	 
	 return false;
	 }
	 */

/**
 * FUNCTION NAME: createRegularMessage
 *
 */
MessageHdr* MembershipProtocol::createRegularMessage(MemberListEntry* entry, MsgTypes msgType, size_t msgsize){
	//size_t msgsize = sizeof(MessageHdr) + (sizeof(Address)+1) + sizeof(long) +  1;
	MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));
	
	// create REGULAR message
	string mleAddressString = to_string(entry->getid()) + ":" + to_string(entry->getport());
	Address mleAddress = Address(mleAddressString);
	long mleHeartbeat = entry->getheartbeat();
	
	msg->msgType = msgType;
	memcpy((char *)(msg+1), (char *)(mleAddress.m_addr), sizeof(mleAddress.m_addr));
	memcpy((char *)(msg+1)+1 + sizeof(mleAddress.m_addr), &mleHeartbeat ,sizeof(long));
	
	return msg;
}

/**
 * FUNCTION NAME: propogateRegularMessage
 *
 * DESCRIPTION : propogate REGULAR message to all members in the list
 */
void MembershipProtocol::propogateRegularMessage(){
	MemberListEntry* entry;
	size_t msgsize = sizeof(MessageHdr) + (sizeof(Address)+1) + sizeof(long) +  1;
	
	for(auto  it = m_memberNode->memberList.begin(); it != m_memberNode->memberList.end(); it++){
		
		if (it->getid() == m_memberNode->memberList[0].getid()){
			//cout<<"this is my entry and here my info : "<<it->getid()<<"HB : "<<it->getheartbeat()<<endl;
			m_memberNode->memberList[0].setheartbeat(m_memberNode->memberList[0].getheartbeat()+1);
		}
		entry = &*it;
		MessageHdr* msg = createRegularMessage(entry, REGULAR, msgsize);
		
		//disseminateRegularMessage(MessageHdr* msg, msgsize );
		for(auto  toNodeIt = m_memberNode->memberList.begin() + 1; toNodeIt != m_memberNode->memberList.end(); toNodeIt++){
			//if (toNodeIt != memberNode->memberList[0]){
			string toNodeAddressString = to_string(toNodeIt->getid()) + ":" + to_string(toNodeIt->getport());
			Address toNodeAddress = Address(toNodeAddressString);
			
			m_emulNet->ENsend(&m_memberNode->addr, &toNodeAddress, (char *)msg, (int)msgsize);
			//}
		}
	}
}




/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MembershipProtocol::nodeLoopOps() {
	
	/*
	 * Your code goes here
	 */
	
	checkFailure();
	
	//1. check all membership enties in MEmberList whether && 2. if the entry got timeout-- delete it
	/*
	 
	 for(auto  it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++){
	 if (it->getid() == memberNode->myPos->getid()){
	 //cout<<"ooOO it's me"<<it->getid()<<endl;
	 //memberNode->myPos->setheartbeat(memberNode->myPos->getheartbeat()+1);
	 //memberNode->myPos->settimestamp(par->getcurrtime());
	 
	 } else if ((par->getcurrtime() - it->gettimestamp()) > 4*(memberNode->pingCounter)){
	 //cout<<"time: "<<par->getcurrtime()<<"ts: "<<it->gettimestamp()<<"id: "<<it->getid()<<"port: "<<it->getport()<<endl;
	 //cout<<"member being erased : "<<it->getid()<< "heartbeat : "<<it->getheartbeat()<<endl;
	 memberNode->memberList.erase(it);
	 }
	 }
	 */
	;
	//3. Propogate MemberList with REGULAR msg
	propogateRegularMessage();
	
	return;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MembershipProtocol::isNullAddress(Address *addr) {
	return (memcmp(addr->m_addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MembershipProtocol::getJoinAddress() {
	Address joinaddr;
	
	memset(&joinaddr, 0, sizeof(Address));
	*(int *)(&joinaddr.m_addr) = 1;
	*(short *)(&joinaddr.m_addr[4]) = 0;
	
	return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MembershipProtocol::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MembershipProtocol::printAddress(Address *addr)
{
	printf("%d.%d.%d.%d:%d \n",  addr->m_addr[0],addr->m_addr[1],addr->m_addr[2],
		   addr->m_addr[3], *(short*)&addr->m_addr[4]) ;
}


/**
 *
 */
void MembershipProtocol::printMemberList()
{
	std::cout << "=== [id:" << m_memberNode->myPos->getid() << "] ";
	
	printAddress(&m_memberNode->addr);
	
	for (auto& mbr : m_memberNode->memberList)
	{
		std::cout << mbr.getid() << " "
		<< mbr.getport() << " "
		<< mbr.getheartbeat() << " "
		<< mbr.gettimestamp() << endl;
	}
	
	std::cout << endl;
}

/* FUNCTION NAME: checkFailure
 *
 * DESCRIPTION: check all membership enties in MEmberList whether && if the entry got timeout-delete it
 */
void MembershipProtocol::checkFailure()
{
	
	// bool mismatchedItr = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->myPos->getid());
	// bool mismatchedIndex = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->memberList.begin()->getid());
	
	/*if (mismatchedItr) {
	 printMemberList();
	 }*/
	
	for(auto it = m_memberNode->memberList.begin() + 1; it != m_memberNode->memberList.end(); it++) {
		// bool mismatchedItr = (it->getid() != memberNode->myPos->getid());
		//115 - 50
		if (((m_par->getcurrtime() - it->gettimestamp()) >= (3 * m_memberNode->pingCounter))) {
			
			Address nodeToRemove(AddressToString(*it));
			m_log->logNodeRemove(&m_memberNode->addr, &nodeToRemove);
			
			it = m_memberNode->memberList.erase(it);
			it--;
		}
	}
	
}
