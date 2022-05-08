//
//  Stabilizer.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 1/9/18.
//  Copyright © 2018 Monika Sharma. All rights reserved.
//

#include "Stabilizer.h"

Stabilizer::~Stabilizer()
{
}

Stabilizer::Stabilizer(HashTable *dataStore, Member *memberNode, EmulNet * networkEmulator)
{
    this->m_dataStore = dataStore;
    this->m_memberNode = memberNode;
    this->m_networkEmulator = networkEmulator;
}

int Stabilizer::myPositionInTheRing()
{
    // 1. find out my postition in the ring
    int myPos = -1;
    for (int i = 0; i < this->m_ring.size(); i++) {
        if (m_ring[i].nodeAddress == this->m_memberNode->addr){
            myPos = i;          // found pos

            break;
        }
    }

    return myPos;
}

int Stabilizer::findfirstSuccessorIndex(int myPos)
{
    if (myPos < 0){
        return -1;
    }
    return (myPos+1)%(this->m_ring.size());
}

int Stabilizer::findSecondSuccessorIndex(int myPos)
{
    if (myPos < 0){
        return -1;
    }
    return (myPos+2)%(this->m_ring.size());
}

int Stabilizer::findfirstPredeccesorIndex(int myPos)
{
    if (myPos < 0){
        return -1;
    }
    return (myPos-1 + this->m_ring.size())%(this->m_ring.size());
}

int Stabilizer::findSecondPredeccesorIndex(int myPos)
{
    if (myPos < 0){
        return -1;
    }
    return (myPos-2 + this->m_ring.size())%(this->m_ring.size());
}

void Stabilizer::setHasMyReplicasIfEmpty(int succ_1, int succ_2)
{
    if (!this->m_hasMyReplicas.empty()){
        return;
    }

    this->m_hasMyReplicas.push_back(m_ring[succ_1]);
    this->m_hasMyReplicas.push_back(m_ring[succ_2]);
}

void Stabilizer::setHaveReplicasOfIfEmpty(int pred_1, int pred_2)
{
    if (!this->m_haveReplicasOf.empty()){
        return;
    }
    this->m_haveReplicasOf.push_back(m_ring[pred_1]);
    this->m_haveReplicasOf.push_back(m_ring[pred_2]);
}

void Stabilizer::sendMessageToUpdateReplicaInfoFromPrimary(
     const string &key,
     const string &keyValue,
     int successorFirstIndex,
     int successorSecondIndex)
{
    if (m_ring[successorFirstIndex].getAddress() != m_hasMyReplicas[0].getAddress() and
        m_ring[successorFirstIndex].getAddress() != m_hasMyReplicas[1].getAddress()){
        Address toaddr = m_ring[successorFirstIndex].nodeAddress;
        Message msg = Message(g_transID, m_memberNode->addr, CREATE, key, keyValue, SECONDARY);
        this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());

    }

    if (m_ring[successorSecondIndex].getAddress() != m_hasMyReplicas[0].getAddress() and
        m_ring[successorSecondIndex].getAddress() != m_hasMyReplicas[1].getAddress()){
        Address toaddr = m_ring[successorSecondIndex].nodeAddress;
        Message msg = Message(g_transID, m_memberNode->addr, CREATE, key, keyValue, TERTIARY);
        this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddr, msg.toString());

    }
    g_transID++;
}

void Stabilizer::sendMessageToUpdateReplicaInfoFromSecondary(
    const string &key,
    const string &keyValue,
    int predeccesorFirstIndex,
    int successorFirstIndex)
{
    if (m_ring[successorFirstIndex].getAddress() != m_hasMyReplicas[0].getAddress()){
        if (m_ring[predeccesorFirstIndex].getAddress() != m_haveReplicasOf[0].getAddress()){
            Address toaddrSucc = m_ring[successorFirstIndex].nodeAddress;
            Address toaddrPred = m_ring[predeccesorFirstIndex].nodeAddress;
            Message msgToSucc = Message(g_transID, m_memberNode->addr, CREATE, key, keyValue, TERTIARY);
            Message msgToPred= Message(g_transID, m_memberNode->addr, CREATE, key, keyValue, PRIMARY);
            this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddrSucc, msgToSucc.toString());
            this->m_networkEmulator->ENsend(&m_memberNode->addr, &toaddrPred, msgToPred.toString());
            g_transID++;
        }

    }
}

int Stabilizer::findMyPosInReplicaSet(vector<Node> &replicaSet)
{
    for (int i = 0; i < replicaSet.size(); i++){
        if (replicaSet[i].nodeAddress == this->m_memberNode->addr){
            return i;
        }
    }

    return -1;
}

ReplicaType Stabilizer::parseReplicaType(string valueToParse)
{
    return Entry(valueToParse).replica;
}

string Stabilizer::parseValue(string valueToParse)
{
    return Entry(valueToParse).value;
}

size_t Stabilizer::hashFunction(string key)
{
    std::hash<string> hashFunc;
    size_t ret = hashFunc(key);
    return ret%RING_SIZE;
}

vector<Node> Stabilizer::findNodes(string key)
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

vector<Node> Stabilizer::getMembershipList()
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

void Stabilizer::initRing(){
    auto curMemList = getMembershipList();
    sort(curMemList.begin(), curMemList.end());

    this->m_ring = curMemList;
    return;
}

void Stabilizer::stabilizationProtocol()
{
    initRing();
    int myPositionInRing = myPositionInTheRing();

    if (myPositionInRing < 0)
    {
        throw new std::runtime_error("stabilization_protocol_error: myPos is -1");
    }

    int successorFirstIndex = findfirstSuccessorIndex(myPositionInRing);
    int successorSecondIndex = findSecondSuccessorIndex(myPositionInRing);
    int predeccesorFirstIndex = findfirstPredeccesorIndex(myPositionInRing);
    int predeccesorSecondIndex = findSecondPredeccesorIndex(myPositionInRing);

    setHasMyReplicasIfEmpty(successorFirstIndex, successorSecondIndex);
    setHaveReplicasOfIfEmpty(predeccesorFirstIndex, predeccesorSecondIndex);

    // iterate key-value hash table
    for (auto it = this->m_dataStore->hashTable.begin();
         it != this->m_dataStore->hashTable.end();
         it++) {

        string key = it->first;
        string value = parseValue(it->second);
        ReplicaType replicaType = parseReplicaType(it->second);

        vector<Node> replicaSet = findNodes(key);

        // todo: fix this for early exit
        if (replicaSet.size() < 3) {
            return;
        }

        //check this node exists in replicaSet
        int myPosInReplica = findMyPosInReplicaSet(replicaSet);

        // node exists in the replicaSet
        if (myPosInReplica < 0) {
            // todo : throw exception ?
            return ;
        }

        // if position changed, update replicatype
        if ((int)(replicaType) != myPosInReplica){
            Entry entry = Entry(it->second);
            entry.replica = (ReplicaType) myPosInReplica;
            it->second = entry.convertToString();
        }

        switch (myPosInReplica) {
            case 0: // i m primary
                sendMessageToUpdateReplicaInfoFromPrimary(key, value, successorFirstIndex, successorSecondIndex);
                break;
            case 1: // i am seconday
                sendMessageToUpdateReplicaInfoFromSecondary(key, value, predeccesorFirstIndex, successorFirstIndex);
                break;
            case 2: // i am tertiary
                // todo
                break;
            default:
                //throw new std::runtime_error("stabilization_protocol_error: myPosInReplica is not known");
                break;
        }

        m_hasMyReplicas.clear();
        m_haveReplicasOf.clear();
        m_hasMyReplicas.push_back(m_ring[successorFirstIndex]);
        m_hasMyReplicas.push_back(m_ring[successorSecondIndex]);
        m_haveReplicasOf.push_back(m_ring[predeccesorFirstIndex]);
        m_haveReplicasOf.push_back(m_ring[predeccesorSecondIndex]);
    }
}
