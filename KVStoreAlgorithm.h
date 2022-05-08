/**********************************
 * FILE NAME: MP2Node.h
 *
 * DESCRIPTION: MP2Node class header file
 **********************************/

#ifndef MP2NODE_H_
#define MP2NODE_H_

#include "stdincludes.h"
#include "EmulNet.h"
#include "Node.h"
#include "HashTable.h"
#include "Log.h"
#include "Params.h"
#include "Message.h"
#include "MessageQueue.h"
#include "Stabilizer.h"
#include "QuorumTracker.h"

// This class encapsulates all the key-value store functionality including:
//  1) Ring
//  2) Stabilization Protocol
//  3) Server side CRUD APIs
//  4) Client side CRUD APIs
class KVStoreAlgorithm
{
private:
    vector<Node> m_ring;
    HashTable * m_dataStore;
    Member *m_memberNode;
    Params *m_parameters;
    EmulNet * m_networkEmulator;
    Log * m_logger;
    IMessageQueue * m_queue;
    QuorumTracker* m_readMessagesQuorumTracker;
    QuorumTracker* m_quorumTracker;

    vector<Node> getMembershipList();
    size_t hashFunction(string key);

    void sendMessageToReplicas(vector<Node> replicaNodes, MessageType msgType, string key);
    void sendMessageToReplicas(vector<Node> replicaNodes, MessageType msgType, string key, string value);

    // Server side  API. The function does the following:
    //  1) read/create/update/delete key value from/into the local hash table
    //  2) Return true or false based on success or failure
    bool createKeyValue(string key, string value, ReplicaType replica);
    string readKey(string key);
    bool updateKeyValue(string key, string value, ReplicaType replica);
    bool deletekey(string key);

    ReplicaType ConvertToReplicaType(string replicaTypeString);

    // compare current state
    bool isCurrentStateChange(vector<Node> curMemList, vector<Node> ring);
    vector<string> ParseMessageIntoTokens(const string& message, size_t dataSize);

    void processReadMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processCreateMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processUpdateMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processDeleteMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processReplyMessage(bool isCoordinator, const vector<string> &messageParts, int transID);
    void processReadReplyMessage(bool isCoordinator, const vector<string> &messageParts, int transID);

    void updateQuorum(MessageType msgType, string key);

public:
    virtual ~KVStoreAlgorithm();
    KVStoreAlgorithm(
        Member *memberNode,
        Params *par,
        EmulNet *emulNet,
        Log *log,
        Address *addressOfMember,
        IMessageQueue* queue);

    Member * getMemberNode() {
        return this->m_memberNode;
    }

    // ring functionalities
    void updateRing();

    // client side CRUD APIs
    void clientCreate(string key, string value);
    void clientRead(string key);
    void clientUpdate(string key, string value);
    void clientDelete(string key);

    // receive messages from Emulnet
    bool recvLoop();

    // handle messages from receiving queue
    void checkMessages();

    // find the addresses of nodes that are responsible for a key
    vector<Node> findNodes(string key);
};

#endif /* MP2NODE_H_ */
