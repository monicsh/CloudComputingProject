//
//  Address.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef Address_hpp
#define Address_hpp

#include "stdincludes.h"

/**
 * CLASS NAME: Address
 *
 * DESCRIPTION: Class representing the address of a single node
 */
class Address {
private:
    const static int MAX_BYTES = 6;
public: // TODO: make this private when project wide dependency is resolved on this field
    char m_addr[MAX_BYTES];
public:
    Address();
    Address(const Address &anotherAddress);
    Address& operator =(const Address &anotherAddress);
    bool operator ==(const Address &anotherAddress);
    Address(string address);
    string getAddress();
    string getAddressLogFormatted();
    char getAddrByte(int pos);
};

#endif /* Address_hpp */
