//
//  Stabilizer.h
//  MembershipProtocol
//
//  Created by Monika Sharma on 1/9/18.
//  Copyright © 2018 Monika Sharma. All rights reserved.
//

#ifndef Stabilizer_h
#define Stabilizer_h

#include <stdio.h>
#include "stdincludes.h"
#include "HashTable.h"
#include "EmulNet.h"
#include "Member.h"
#include "IMessageQueue.h"
#include "Node.h"
#include "KVStoreAlgorithm.h"

class Stabilizer
{
private:
    HashTable *m_dataStore;
    Member *m_memberNode;
    EmulNet *m_networkEmulator;
    vector<Node> m_hasMyReplicas;
    vector<Node> m_haveReplicasOf;
    vector<Node> m_ring;

    size_t hashFunction(string key);
    vector<Node> findNodes(string key);
    vector<Node> getMembershipList();
    void initRing();
    int myPositionInTheRing();
    int findfirstSuccessorIndex(int myPos);
    int findSecondSuccessorIndex(int myPos);
    int findfirstPredeccesorIndex(int myPos);
    int findSecondPredeccesorIndex(int myPos);
    void setHasMyReplicasIfEmpty(int succ_1, int succ_2);
    void setHaveReplicasOfIfEmpty(int pred_1, int pred_2);
    void sendMessageToUpdateReplicaInfoFromPrimary(const string &key,const string &keyValue,int successorFirstIndex,int successorSecondIndex);
    void sendMessageToUpdateReplicaInfoFromSecondary(const string &key, const string &keyValue, int predeccesorFirstIndex, int successorFirstIndex);
    int findMyPosInReplicaSet(vector<Node> &replicaSet);
    ReplicaType parseReplicaType(string valueToParse);
    string parseValue(string valueToParse);

public:
    virtual ~Stabilizer();
    Stabilizer(HashTable *dataStore, Member *memberNode, EmulNet * networkEmulator);

    void stabilizationProtocol();
};

#endif /* Stabilizer_h */

