//
//  q_elt.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef q_elt_hpp
#define q_elt_hpp

/**
 * CLASS NAME: q_elt
 *
 * DESCRIPTION: Entry in the queue
 */
class RawMessage {
public:
    void *elt;
    int size;
    RawMessage(void *elt, int size);
};

#endif /* q_elt_hpp */
