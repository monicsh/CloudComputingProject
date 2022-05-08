/**********************************
 * FILE NAME: MP2Node.cpp
 *
 * DESCRIPTION: MP2Node class definition
 **********************************/

#include "KVStoreAlgorithm.h"

KVStoreAlgorithm::KVStoreAlgorithm(
    Member *memberNode,
    Params *par,
    EmulNet * emulNet,
    Log * log,
    Address * address,
    IMessageQueue * queue)
{
    this->m_memberNode = memberNode;
    this->m_parameters = par;
    this->m_networkEmulator = emulNet;
    this->m_logger = log;
    this->m_queue = queue;

    // TODO (monicsh): inject from ctor
    this->m_dataStore = new HashTable();
    this->m_readMessagesQuorumTracker = new QuorumTracker(memberNode->addr, par, log);
    this->m_quorumTracker = new QuorumTracker(memberNode->addr, par, log);

    this->m_memberNode->addr = *address;
}

KVStoreAlgorithm::~KVStoreAlgorithm()
{
    delete m_dataStore;
    delete m_memberNode;
}

ReplicaType KVStoreAlgorithm::ConvertToReplicaType(string replicaTypeString)
{
    if (std::stoi(replicaTypeString) == 1) {
        return SECONDARY;

    } else if (std::stoi(replicaTypeString) == 2) {
        return TERTIARY;
    }

    return PRIMARY;
}


 // FUNCTION NAME: updateRing

 // DESCRIPTION: This function does the following:
 //  1) Gets the current membership list from the Membrship Protocol (MP1Node)
 //     The membership list is returned as a vector ofNodes. See Node class in Node.h
 //  2) Constructs the ring based on the membership list
 //  3) Calls the Stabilization Protocol
void KVStoreAlgorithm::updateRing()
{
    Stabilizer stabilizer(m_dataStore, m_memberNode, m_networkEmulator);

    // Step 1. Get the current membership list from Membership Protocol (mp1)
    auto curMemList = getMembershipList();

    // Step 2. Construct the ring. Sort the list based on the hashCode
    sort(curMemList.begin(), curMemList.end());

    // has ring changed?
    size_t length_of_ring = m_ring.size();
    size_t length_of_memList = curMemList.size();

    bool hasRingChanged = (length_of_ring != length_of_memList);
    if (!hasRingChanged) {
        for (int i = 0; i < length_of_memList; i++) {
            auto hashCurrML = curMemList[i].getHashCode();
            auto hashRing = m_ring[i].getHashCode();
            if ( hashCurrML != hashRing ) {
                hasRingChanged = true;
                break;
            }
        }
    }

    // Step 3: Run the stabilization protocol IF REQUIRED
    // Run stabilization protocol if the hash table size is greater than zero
    // and if there has been a changed in the ring
    if (hasRingChanged) {

        // update ring
        this->m_ring = curMemList;
        if (!this->m_dataStore->isEmpty()) {
            stabilizer.stabilizationProtocol();
        }
    }
}

/**
 * compare two vectors
 * return : true if  state changed
 *                      false on state unchanged
 */
bool KVStoreAlgorithm::isCurrentStateChange(
    vector<Node> curMemList,
    vector<Node> ring)
{
    size_t length_of_ring = ring.size();
    size_t length_of_memList = curMemList.size();

    if (length_of_ring != length_of_memList){
        return true;
    }

    for (int i = 0; i< length_of_memList; i++) {
        if (curMemList[i].getHashCode() != ring[i].getHashCode()) {
            return true;
        }
    }

    return false;
}

/**
 * FUNCTION NAME: getMemberhipList
 *
 * DESCRIPTION: This function goes through the membership list from the Membership protocol/MP1 and
 *                               i) generates the hash code for each member
 *                               ii) populates the ring member in MP2Node class
 *                               It returns a vector of Nodes. Each element in the vector contain the following fields:
 *                               a) Address of the node
 *                               b) Hash code obtained by consistent hashing of the Address
 */
vector<Node> KVStoreAlgorithm::getMembershipList()
{
    vector<Node> curMemList;

    for (auto i = 0 ; i < this->m_memberNode->memberList.size(); i++ ) {

        int id = this->m_memberNode->memberList.at(i).getid();
        short port = this->m_memberNode->memberList.at(i).getport();

        Address addressOfThisMember;
        memcpy(&addressOfThisMember.m_addr[0], &id, sizeof(int));
        memcpy(&addressOfThisMember.m_addr[4], &port, sizeof(short));

        curMemList.emplace_back(Node(addressOfThisMember));
    }

    return curMemList;
}

/**
 * FUNCTION NAME: hashFunction
 *
 * DESCRIPTION: This functions hashes the key and returns the position on the ring
 *                              HASH FUNCTION USED FOR CONSISTENT HASHING
 *
 * RETURNS:
 * size_t position on the ring
 */
size_t KVStoreAlgorithm::hashFunction(string key)
{
    std::hash<string> hashFunc;
    size_t ret = hashFunc(key);
    return ret%RING_SIZE;
}

void KVStoreAlgorithm::sendMessageToReplicas(
    vector<Node> replicaNodes,
    MessageType msgType,
    string key)
{
    for (auto it = replicaNodes.begin(); it != replicaNodes.end(); it++){
        Address toaddr = it->nodeAddress;
        Message msg = Message(g_transID, m_memberNode->addr, msgType, key);

        this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());
    }
}

void KVStoreAlgorithm::sendMessageToReplicas(
    vector<Node> replicaNodes,
    MessageType msgType,
    string key,
    string value)
{
    int replica =  0;
    for (auto it = replicaNodes.begin(); it != replicaNodes.end(); it++){
        Address toaddr = it->nodeAddress;
        Message msg = Message(g_transID, m_memberNode->addr, msgType, key, value, static_cast<ReplicaType>(replica++));

        this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());
    }
}

void KVStoreAlgorithm::updateQuorum(MessageType msgType, string key)
{
    this->m_quorumTracker->updateQuorum(g_transID, msgType, key);

    g_transID++;
}

void KVStoreAlgorithm::clientCreate(string key, string value)
{
    sendMessageToReplicas(findNodes(key), CREATE, key, value);
    updateQuorum(CREATE, key);
}

void KVStoreAlgorithm::clientRead(string key)
{
    sendMessageToReplicas(findNodes(key), READ, key);
    this->m_readMessagesQuorumTracker->updateQuorum(g_transID, READ, key);
    g_transID++;
}

void KVStoreAlgorithm::clientUpdate(string key, string value)
{
    sendMessageToReplicas(findNodes(key), UPDATE, key, value);
    updateQuorum(UPDATE, key);
}

void KVStoreAlgorithm::clientDelete(string key){
    sendMessageToReplicas(findNodes(key), DELETE, key);
    updateQuorum(DELETE, key);
}

bool KVStoreAlgorithm::createKeyValue(
    string key,
    string value,
    ReplicaType replica)
{
    Entry entry = Entry(value, m_parameters->getcurrtime(), replica);
    return this->m_dataStore->create(key, entry.convertToString());
}

string KVStoreAlgorithm::readKey(string key)
{
    string valueTuple = m_dataStore->read(key);

    if (valueTuple.empty()) {
        return "";
    }

    size_t pos = valueTuple.find(":");
    if (pos != std::string::npos) {
        return valueTuple.substr(0, pos);
    }

    return "";
}

bool KVStoreAlgorithm::updateKeyValue(
    string key,
    string value,
    ReplicaType replica)
{
    Entry entry = Entry(value, m_parameters->getcurrtime(), replica);
    return this->m_dataStore->update(key, entry.convertToString());
}

bool KVStoreAlgorithm::deletekey(string key)
{
    return this->m_dataStore->deleteKey(key);
}

vector<string> KVStoreAlgorithm::ParseMessageIntoTokens(
    const string& message,
    size_t dataSize)
{
    const string delimiter = "::";
    string token;
    size_t pos = 0;
    string messageCopy = message;
    vector<string> messageParts;

    //parse the message
    while ((pos = messageCopy.find(delimiter)) != std::string::npos) {
        token = messageCopy.substr(0, pos);
        //std::cout << token << std::endl;
        messageCopy.erase(0, pos + delimiter.length());
        messageParts.push_back(token);
    }

    token =  messageCopy.substr(0, dataSize-1);
    messageParts.push_back(token);

    return messageParts;
}

void KVStoreAlgorithm::processReadMessage(
    const Address &fromaddr,
    bool isCoordinator,
    const vector<string> &messageParts,
    int transID)
{
    const string key = messageParts[3];
    const string value = readKey(key);

    if (value.empty()) {
        this->m_logger->logReadFail(&m_memberNode->addr, isCoordinator, transID, key);
        return;
    }

    //READREPLY
    Message msg = Message(transID, m_memberNode->addr, value);
    Address toaddr = fromaddr;

    this->m_logger->logReadSuccess(&m_memberNode->addr, isCoordinator, transID, key, value);
    this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());
}

void KVStoreAlgorithm::processCreateMessage(
    const Address &fromaddr,
    bool isCoordinator,
    const vector<string> &messageParts,
    int transID)
{
    const string key = messageParts[3];
    const string value = messageParts[4];
    const ReplicaType replicaType = ConvertToReplicaType(messageParts[5]);

    if (!createKeyValue(key, value, replicaType)) {
        this->m_logger->logCreateFail(&m_memberNode->addr, isCoordinator, transID, key, value);
        return;
    }

    //REPLY  send acknoledgement to sender
    Message msg = Message(transID, m_memberNode->addr, MessageType::REPLY , true);
    Address toaddr = fromaddr;

    this->m_logger->logCreateSuccess(&m_memberNode->addr, isCoordinator, transID, key, value);
    this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());
}

void KVStoreAlgorithm::processUpdateMessage(
    const Address &fromaddr,
    bool isCoordinator,
    const vector<string> &messageParts,
    int transID)
{
    const string key = messageParts[3];
    const string value = messageParts[4];
    const ReplicaType replicaType = ConvertToReplicaType(messageParts[5]);

    if (!updateKeyValue(key, value, replicaType))
    {
        Message msg = Message(transID, m_memberNode->addr, MessageType::REPLY , false);
        Address toaddr = fromaddr;

        this->m_logger->logUpdateFail(&m_memberNode->addr, isCoordinator, transID, key, value);
        this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());

        return;
    }

    //REPLY  send acknoledgement to sender
    Message msg = Message(transID, m_memberNode->addr, MessageType::REPLY , true);
    Address toaddr = fromaddr;

    this->m_logger->logUpdateSuccess(&m_memberNode->addr, isCoordinator, transID, key, value);
    this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());
}

void KVStoreAlgorithm::processDeleteMessage(
    const Address &fromaddr,
    bool isCoordinator,
    const vector<string> &messageParts,
    int transID)
{
    const string key = messageParts[3];

    if (!deletekey(key)) {
        Message msg = Message(transID, m_memberNode->addr, MessageType::REPLY , false);
        Address toaddr = fromaddr;

        this->m_logger->logDeleteFail(&m_memberNode->addr, isCoordinator, transID, key);
        this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());

        return;
    }

    //REPLY  send acknoledgement to sender
    Message msg = Message(transID, m_memberNode->addr, MessageType::REPLY , true);
    Address toaddr = fromaddr;

    this->m_logger->logDeleteSuccess(&m_memberNode->addr, isCoordinator, transID, key);
    this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());
}

void KVStoreAlgorithm::processReplyMessage(
    bool isCoordinator,
    const vector<string> &messageParts,
    int transID)
{
    const bool success("1" == messageParts[3]);
    if (!success){
        return;
    }

    auto quoromExists = this->m_quorumTracker->quorumExists(transID);
    if (!quoromExists) {
        return; // not found
    }

    auto record = this->m_quorumTracker->getQuorum(transID);

    //replyCounter increment while reply message found and check quorum
    record.replyCounter++;
    this->m_quorumTracker->saveQuorum(transID, record);

    if (record.replyCounter == 3) {

        // quorom met; remove this triplet

        if (record.transMsgType == CREATE){
            this->m_logger->logCreateSuccess(&(this->m_memberNode->addr), isCoordinator, transID, record.reqKey, "somevalue");
        }
        this->m_quorumTracker->removeQuorum(transID);

        return;
    }

    // log as success on basis of message type; but dont remove
    if(record.replyCounter == 2) {
        if (record.transMsgType == UPDATE){
            this->m_logger->logUpdateSuccess(&(this->m_memberNode->addr), isCoordinator, transID, record.reqKey, "somevalue");

        } else if (record.transMsgType == DELETE){
            this->m_logger->logDeleteSuccess(&(this->m_memberNode->addr), isCoordinator, transID, record.reqKey);
        }

        return;
    }

    // TODO : for =<1
    return;
}

void KVStoreAlgorithm::processReadReplyMessage(
    bool isCoordinator,
    const vector<string> &messageParts,
    int transID)
{
    const string value = messageParts[3];

    auto quoromExists = this->m_readMessagesQuorumTracker->quorumExists(transID);
    if (!quoromExists) {
        return; // not found
    }

    auto record = this->m_readMessagesQuorumTracker->getQuorum(transID);

    // else if counter is N-1 (=2 here), logReadSuccess(transID), so that any late message for READ doesnt starts from 1
    // else insert new key with transID with counter 1
    record.replyCounter++;
    this->m_readMessagesQuorumTracker->saveQuorum(transID, record);

    if (record.replyCounter == 3) {

        // quorom met; remove this triplet
        this->m_readMessagesQuorumTracker->removeQuorum(transID);

        return;
    }

    // log as success; but dont remove
    if (record.replyCounter == 2) {
        this->m_logger->logReadSuccess(&(this->m_memberNode->addr), isCoordinator, transID, record.reqKey, value);
        return;
    }

    // TODO : for =<1
    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: This function is the message handler of this node.
 *                              This function does the following:
 *                              1) Pops messages from the queue
 *                              2) Handles the messages according to message types
 */
void KVStoreAlgorithm::checkMessages()
{
    char * data;
    int size;

    // dequeue all messages and handle them
    while ( !m_queue->empty() ) {

        RawMessage item = m_queue->dequeue();
        data = (char *) item.elt;
        size = item.size;

        const string message(data, data + size);
        const vector<string> messageParts = ParseMessageIntoTokens(message, size);
        const int transID = stoi(messageParts[0]);
        const Address fromaddr = Address(messageParts[1]);
        const int messageType = std::stoi(messageParts[2]);
        const bool isCoordinator = (messageParts[1] == m_memberNode->addr.getAddress());

        switch(messageType) {

            case MessageType::CREATE:
            {
                processCreateMessage(fromaddr, isCoordinator, messageParts, transID);
                break;
            }
            case MessageType::READ:
            {
                processReadMessage(fromaddr, isCoordinator, messageParts, transID);
                break;
            }
            case MessageType::UPDATE:
            {
                processUpdateMessage(fromaddr, isCoordinator, messageParts, transID);
                break;
            }
            case MessageType::DELETE:
            {
                processDeleteMessage(fromaddr, isCoordinator, messageParts, transID);
                break;
            }
            case MessageType::REPLY:
            {
                processReplyMessage(isCoordinator, messageParts, transID);
                break;
            }
            case MessageType::READREPLY:
            {
                processReadReplyMessage(isCoordinator, messageParts, transID);
                break;
            }

        } // switch end

        // update quorom state at end
        this->m_quorumTracker->removeExpiredQuorums();
        this->m_readMessagesQuorumTracker->removeExpiredQuorums();
    }
}

/**
 * FUNCTION NAME: findNodes
 *
 * DESCRIPTION: Find the replicas of the given keyfunction
 *                              This function is responsible for finding the replicas of a key
 */
vector<Node> KVStoreAlgorithm::findNodes(string key)
{
    size_t pos = hashFunction(key);
    vector<Node> addr_vec;

    if (m_ring.size() < 3) {
        return addr_vec;
    }

    // if pos <= min || pos > max, the leader is the min
    if (pos <= m_ring.at(0).getHashCode()
        || pos > m_ring.at(m_ring.size()-1).getHashCode()) {

        addr_vec.emplace_back(m_ring.at(0));
        addr_vec.emplace_back(m_ring.at(1));
        addr_vec.emplace_back(m_ring.at(2));

        return addr_vec;
    }

    // go through the ring until pos <= node
    for (int i=1; i<m_ring.size(); i++){
        Node addr = m_ring.at(i);

        if (pos <= addr.getHashCode()) {
            addr_vec.emplace_back(addr);
            addr_vec.emplace_back(m_ring.at((i+1)%m_ring.size()));
            addr_vec.emplace_back(m_ring.at((i+2)%m_ring.size()));

            break;
        }
    }

    return addr_vec;
}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: Receive messages from EmulNet and push into the queue (mp2q)
 */
bool KVStoreAlgorithm::recvLoop()
{
    if (m_memberNode->bFailed) {
        return false;
    }

    return m_networkEmulator->ENrecv(
        &(m_memberNode->addr),
        this->m_queue,
        NULL,
        1);
}

