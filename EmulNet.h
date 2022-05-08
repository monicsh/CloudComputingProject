/**********************************
 * FILE NAME: EmulNet.h
 *
 * DESCRIPTION: Emulated Network classes header file
 **********************************/

#ifndef _EMULNET_H_
#define _EMULNET_H_

#define MAX_NODES 1000
#define MAX_TIME 3600
#define ENBUFFSIZE 30000

#include "stdincludes.h"
#include "Params.h"
#include "Address.h"
#include "IMessageQueue.h"
#include "EM.h"



/**
 * CLASS NAME: EmulNet
 *
 * DESCRIPTION: This class defines an emulated network
 */
class EmulNet
{ 	
private:
	Params* m_par;
	int m_sent_msgs[MAX_NODES + 1][MAX_TIME];
	int m_recv_msgs[MAX_NODES + 1][MAX_TIME];
	int m_enInited;
	EM m_emulnet;
public:
 	EmulNet(Params *p);
 	EmulNet(EmulNet &anotherEmulNet);
 	EmulNet& operator = (EmulNet &anotherEmulNet);
 	virtual ~EmulNet();
	void *ENinit(Address *myaddr, short port);
	int ENsend(Address *myaddr, Address *toaddr, string data);
	int ENsend(Address *myaddr, Address *toaddr, char *data, int size);
    int ENrecv(Address *myaddr, IMessageQueue *queue, struct timeval *t, int times);
	int ENcleanup();
};

#endif /* _EMULNET_H_ */
