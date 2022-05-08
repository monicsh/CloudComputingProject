//
//  MP2NodeTest.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/15/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <execinfo.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <queue>
#include <fstream>


#define CATCH_CONFIG_MAIN
#include "catch.hpp"


#include "HashTable.h"


TEST_CASE("hashtable read correctly"){
	
	
	HashTable * ht = new HashTable();
	string val1 = ht->read("key1");
	REQUIRE(val1 == "");
	
	bool f = ht->create("key1", "x:y:z");
	REQUIRE(f==TRUE);
	
	val1 = ht->read("key1");
	REQUIRE(val1 == "v:y:z");
	
}

TEST_CASE("Read API : Sucessfully reading"){
	string key = "ms";
	string keyval = "monika";
	
	
}

