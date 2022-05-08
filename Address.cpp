//
//  Address.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#include "Address.h"

/**
 * Constructor
 */
Address::Address() {
    memset(&m_addr, 0, sizeof(m_addr));
}

/**
 * Copy constructor
 */
Address::Address(const Address &anotherAddress) {
    // strcpy(addr, anotherAddress.addr);
    memcpy(&m_addr, &anotherAddress.m_addr, sizeof(m_addr));
}

/**
 * Assignment operator overloading
 */
Address& Address::operator =(const Address& anotherAddress) {
    // strcpy(addr, anotherAddress.addr);
    memcpy(&m_addr, &anotherAddress.m_addr, sizeof(m_addr));
    return *this;
}

/**
 * Compare two Address objects
 */
bool Address::operator ==(const Address& anotherAddress) {
    return !memcmp(this->m_addr, anotherAddress.m_addr, sizeof(this->m_addr));
}

Address::Address(string address) {
    size_t pos = address.find(":");
    int id = stoi(address.substr(0, pos));
    short port = (short)stoi(address.substr(pos + 1, address.size()-pos-1));
    memcpy(&m_addr[0], &id, sizeof(int));
    memcpy(&m_addr[4], &port, sizeof(short));
}

string Address::getAddressLogFormatted() {
    // %d.%d.%d.%d:%d
    char formatstring[30]={0};
    sprintf(formatstring, "%d.%d.%d.%d:%d", m_addr[0], m_addr[1],m_addr[2],m_addr[3], (short*)m_addr[4]);

    return formatstring;
}

string Address::getAddress() {
    int id = 0;
    short port;
    memcpy(&id, &m_addr[0], sizeof(int));
    memcpy(&port, &m_addr[4], sizeof(short));
    return to_string(id) + ":" + to_string(port);
}

char Address::getAddrByte(int pos) {
    if (pos >= 0 && pos < MAX_BYTES) {
        return m_addr[pos];
    }

    throw new runtime_error("bad byte requested");
}


