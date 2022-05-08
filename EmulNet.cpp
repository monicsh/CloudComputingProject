/**********************************
 * FILE NAME: EmulNet.cpp
 *
 * DESCRIPTION: Emulated Network classes definition
 **********************************/

#include "EmulNet.h"


/**
 * Constructor
 */
EmulNet::EmulNet(Params *p)
{
	//trace.funcEntry("EmulNet::EmulNet");
	int i,j;
	m_par = p;
	m_emulnet.setNextId(1);
	m_emulnet.setCurrBuffSize(0);
	m_enInited=0;
	for ( i = 0; i < MAX_NODES; i++ ) {
		for ( j = 0; j < MAX_TIME; j++ ) {
			m_sent_msgs[i][j] = 0;
			m_recv_msgs[i][j] = 0;
		}
	}
	//trace.funcExit("EmulNet::EmulNet", SUCCESS);
}

/**
 * Copy constructor
 */
EmulNet::EmulNet(EmulNet &anotherEmulNet) {
	int i, j;
	this->m_par = anotherEmulNet.m_par;
	this->m_enInited = anotherEmulNet.m_enInited;
	for ( i = 0; i < MAX_NODES; i++ ) {
		for ( j = 0; j < MAX_TIME; j++ ) {
			this->m_sent_msgs[i][j] = anotherEmulNet.m_sent_msgs[i][j];
			this->m_recv_msgs[i][j] = anotherEmulNet.m_recv_msgs[i][j];
		}
	}
	this->m_emulnet = anotherEmulNet.m_emulnet;
}

/**
 * Assignment operator overloading
 */
EmulNet& EmulNet::operator =(EmulNet &anotherEmulNet) {
	int i, j;
	this->m_par = anotherEmulNet.m_par;
	this->m_enInited = anotherEmulNet.m_enInited;
	for ( i = 0; i < MAX_NODES; i++ ) {
		for ( j = 0; j < MAX_TIME; j++ ) {
			this->m_sent_msgs[i][j] = anotherEmulNet.m_sent_msgs[i][j];
			this->m_recv_msgs[i][j] = anotherEmulNet.m_recv_msgs[i][j];
		}
	}
	this->m_emulnet = anotherEmulNet.m_emulnet;
	return *this;
}

/**
 * Destructor
 */
EmulNet::~EmulNet() {}

/**
 * FUNCTION NAME: ENinit
 *
 * DESCRIPTION: Init the emulnet for this node
 */
void *EmulNet::ENinit(Address *myaddr, short port) {
	// Initialize data structures for this member

    *(int *)(myaddr->m_addr) = m_emulnet.getNextId();
    m_emulnet.incrementNextId();
    *(short *)(&myaddr->m_addr[4]) = 0;
	return myaddr;
}

/**
 * FUNCTION NAME: ENsend
 *
 * DESCRIPTION: EmulNet send function
 *
 * RETURNS:
 * size
 */
int EmulNet::ENsend(Address *myaddr, Address *toaddr, char *data, int size) {
	en_msg *em;
	static char temp[2048];
	int sendmsg = rand() % 100;

	if( (m_emulnet.getCurrBuffSize() >= ENBUFFSIZE) || (size + (int)sizeof(en_msg) >= m_par->MAX_MSG_SIZE) || (m_par->dropmsg && sendmsg < (int) (m_par->MSG_DROP_PROB * 100)) ) {
		return 0;
	}

	em = (en_msg *)malloc(sizeof(en_msg) + size);
	em->size = size;

	memcpy(&(em->from.m_addr), &(myaddr->m_addr), sizeof(em->from.m_addr));
	memcpy(&(em->to.m_addr), &(toaddr->m_addr), sizeof(em->from.m_addr));
	memcpy(em + 1, data, size);

	//m_emulnet.buff[m_emulnet.currbuffsize++] = em;
    m_emulnet.setBuffer(m_emulnet.getCurrBuffSize(), em);
    m_emulnet.incrementCurrBuffSize();

	int src = *(int *)(myaddr->m_addr);
	int time = m_par->getcurrtime();

	assert(src <= MAX_NODES);
	assert(time < MAX_TIME);

	m_sent_msgs[src][time]++;

	#ifdef DEBUGLOG 
		sprintf(temp, "Sending 4+%d B msg type %d to %s ", size-4, *(int *)data, toaddr->getAddressLogFormatted().c_str());
	#endif

	return size;
}

/**
 * FUNCTION NAME: ENsend
 *
 * DESCRIPTION: EmulNet send function
 *
 * RETURNS:
 * size
 */
int EmulNet::ENsend(Address *myaddr, Address *toaddr, string data) {
	char * str = (char *) malloc(data.length() * sizeof(char));
	memcpy(str, data.c_str(), data.size());
	int ret = this->ENsend(myaddr, toaddr, str, (data.length() * sizeof(char)));
	free(str);
	return ret;
}

/**
 * FUNCTION NAME: ENrecv
 *
 * DESCRIPTION: EmulNet receive function
 *
 * RETURN:
 * 0
 */
int EmulNet::ENrecv(Address *myaddr, IMessageQueue *queue, struct timeval *t, int times){
    // times is always assumed to be 1
    int i;
    char* tmp;
    int sz;
    en_msg *emsg;
    
    for( i = m_emulnet.getCurrBuffSize() - 1; i >= 0; i-- ) {
        //emsg = m_emulnet.buff[i];
        emsg = m_emulnet.getBuffer(i);
        
        if ( 0 != strcmp(emsg->to.m_addr, myaddr->m_addr) ) {
            continue;
        }
        
        sz = emsg->size;
        tmp = (char *) malloc(sz * sizeof(char));
        memcpy(tmp, (char *)(emsg+1), sz);
        
        m_emulnet.setBuffer(i, m_emulnet.getBuffer(m_emulnet.getCurrBuffSize()-1));
        m_emulnet.decrementCurrBuffSize();

        queue->enqueue(tmp, sz);
        
        free(emsg);
        
        int dst = *(int *)(myaddr->m_addr);
        int time = m_par->getcurrtime();
        
        assert(dst <= MAX_NODES);
        assert(time < MAX_TIME);
        
        m_recv_msgs[dst][time]++;

    }
    
    return 0;
}

/**
 * FUNCTION NAME: ENcleanup
 *
 * DESCRIPTION: Cleanup the EmulNet. Called exactly once at the end of the program.
 */
int EmulNet::ENcleanup() {
	m_emulnet.resetNextId();
	int i, j;
	int sent_total, recv_total;

	FILE* file = fopen("msgcount.log", "w+");

	while(m_emulnet.getCurrBuffSize() > 0) {
		//free(m_emulnet.buff[--m_emulnet.currbuffsize]);
        m_emulnet.decrementCurrBuffSize();
        free(m_emulnet.getBuffer(m_emulnet.getCurrBuffSize()));
	}

	for ( i = 1; i <= m_par->EN_GPSZ; i++ ) {
		fprintf(file, "node %3d ", i);
		sent_total = 0;
		recv_total = 0;

		for (j = 0; j < m_par->getcurrtime(); j++) {

			sent_total += m_sent_msgs[i][j];
			recv_total += m_recv_msgs[i][j];
			if (i != 67) {
				fprintf(file, " (%4d, %4d)", m_sent_msgs[i][j], m_recv_msgs[i][j]);
				if (j % 10 == 9) {
					fprintf(file, "\n         ");
				}
			}
			else {
				fprintf(file, "special %4d %4d %4d\n", j, m_sent_msgs[i][j], m_recv_msgs[i][j]);
			}
		}
		fprintf(file, "\n");
		fprintf(file, "node %3d sent_total %6u  recv_total %6u\n\n", i, sent_total, recv_total);
	}

	fclose(file);
	return 0;
}
