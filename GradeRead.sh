#!/bin/bash

#################################################
# FILE NAME: KVStoreGrader.sh
#
# DESCRIPTION: Grader for MP2
#
# RUN PROCEDURE:
# $ chmod +x KVStoreGrader.sh
# $ ./KVStoreGrader.sh
#################################################

function contains () {
    local e
    for e in "${@:2}"
  	do
    	if [ "$e" == "$1" ]; then
      		echo 1
      		return 1;
    	fi
  	done
    echo 0
}

####
# Main function
####

verbose=$(contains "-v" "$@")

###
# Global variables
###
SUCCESS=0
FAILURE=-1
RF=3
RFPLUSONE=4
GRADE=0
INVALID_KEY="invalidKey"
READ_OPERATION="READ OPERATION"
READ_SUCCESS="read success"
READ_FAILURE="read fail"
QUORUM=2
QUORUMPLUSONE=3

echo ""
echo "############################"
echo " READ TEST"
echo "############################"
echo ""

READ_TEST1_STATUS="${FAILURE}"
READ_TEST1_SCORE=0
READ_TEST2_STATUS="${FAILURE}"
READ_TEST2_SCORE=0
READ_TEST3_PART1_STATUS="${FAILURE}"
READ_TEST3_PART1_SCORE=0
READ_TEST3_PART2_STATUS="${FAILURE}"
READ_TEST3_PART2_SCORE=0
READ_TEST4_STATUS="${FAILURE}"
READ_TEST4_SCORE=0
READ_TEST5_STATUS="${FAILURE}"
READ_TEST5_SCORE=0

if [ "${verbose}" -eq 0 ]
then
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1
    if [ $? -ne "${SUCCESS}" ]
    then
    	echo "COMPILATION ERROR !!!"
    	exit
    fi
    ./Application ./testcases/read.conf > /dev/null 2>&1
else
	make clean
	make
	if [ $? -ne "${SUCCESS}" ]
	then
    	echo "COMPILATION ERROR !!!"
    	exit
    fi
	./Application ./testcases/read.conf
fi

read_operations=`grep -i "${READ_OPERATION}" dbg.log  | cut -d" " -f3 | tr -s ']' ' '  | tr -s '[' ' ' | sort`

cnt=1
for time in ${read_operations}
do
	if [ ${cnt} -eq 1 ]
	then
		echo "TEST 1: Read a key. Check for correct value being read at least in quorum of replicas"
		read_op_test1_time="${time}"
		read_op_test1_key=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test1_time}" | cut -d" " -f7`
		read_op_test1_value=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test1_time}" | cut -d" " -f9`
	elif [ ${cnt} -eq 2 ]
	then
		echo "TEST 2: Read a key after failing a replica. Check for correct value being read at least in quorum of replicas"
		read_op_test2_time="${time}"
		read_op_test2_key=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test2_time}" | cut -d" " -f7`
		read_op_test2_value=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test2_time}" | cut -d" " -f9`
	elif [ ${cnt} -eq 3 ]
	then
		echo "TEST 3 PART 1: Read a key after failing two replicas. Read should fail"
		read_op_test3_part1_time="${time}"
		read_op_test3_part1_key=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test3_part1_time}" | cut -d" " -f7`
		read_op_test3_part1_value=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test3_part1_time}" | cut -d" " -f9`
	elif [ ${cnt} -eq 4 ]
	then
		echo "TEST 3 PART 2: Read the key after allowing stabilization protocol to kick in. Check for correct value being read at least in quorum of replicas"
		read_op_test3_part2_time="${time}"
		echo "read_op_test3_part2_time", $time
		read_op_test3_part2_key=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test3_part2_time}" | cut -d" " -f7`
		echo "read_op_test3_part2_key", $read_op_test3_part2_key
		read_op_test3_part2_value=`grep -i "${READ_OPERATION}" dbg.log | grep "${read_op_test3_part2_time}" | cut -d" " -f9`
		echo "read_op_test3_part2_value", $read_op_test3_part2_value
	elif [ ${cnt} -eq 5 ]
	then
		echo "TEST 4: Read a key after failing a non-replica. Check for correct value being read at least in quorum of replicas"
		read_op_test4_time="${time}"
		read_op_test4_key="${read_op_test1_key}"
		read_op_test4_value="${read_op_test1_value}"
	elif [ ${cnt} -eq 6 ]
	then
		echo "TEST 5: Attempt read of an invalid key"
		read_op_test5_time="${time}"
	fi
	cnt=$(( ${cnt} + 1 ))
done

read_test1_success_count=0
read_test2_success_count=0
read_test3_part2_success_count=0
read_test4_success_count=0

read_successes=`grep -i "${READ_SUCCESS}" dbg.log | grep ${read_op_test1_key} | grep ${read_op_test1_value} 2>/dev/null`

echo "read_successes", $read_successes




if [ "${read_successes}" ]
then
	while read success
	do
		time_of_this_success=`echo "${success}" | cut -d" " -f2 | tr -s '[' ' ' | tr -s ']' ' '`
#echo "time_of_this_success"
#echo $time_of_this_success


		if [ "${time_of_this_success}" -ge "${read_op_test1_time}" -a "${time_of_this_success}" -lt "${read_op_test2_time}" ]
		then
			read_test1_success_count=`expr ${read_test1_success_count} + 1`
		elif [ "${time_of_this_success}" -ge "${read_op_test2_time}" -a "${time_of_this_success}" -lt "${read_op_test3_part1_time}" ] 
		then
			read_test2_success_count=`expr ${read_test2_success_count} + 1`
		elif [ "${time_of_this_success}" -ge "${read_op_test3_part2_time}" -a "${time_of_this_success}" -lt "${read_op_test4_time}" ]  
		then
				echo $read_op_test3_part2_time, $read_op_test4_time
				echo
				read_test3_part2_success_count=`expr ${read_test3_part2_success_count} + 1`
		elif [ "${time_of_this_success}" -ge "${read_op_test4_time}" ]
		then
			read_test4_success_count=`expr ${read_test4_success_count} + 1`
		fi
	done <<<"${read_successes}"
fi

read_test3_part1_fail_count=0
read_test5_fail_count=0

read_fails=`grep -i "${READ_FAILURE}" dbg.log 2>/dev/null`
if [ "${read_fails}" ]
then
	while read fail
	do
		time_of_this_fail=`echo "${fail}" | cut -d" " -f2 | tr -s '[' ' ' | tr -s ']' ' '`
		if [ "${time_of_this_fail}" -ge "${read_op_test3_part1_time}" -a "${time_of_this_fail}" -lt "${read_op_test3_part2_time}" ]
		then
			actual_key=`echo "${fail}" | grep "${read_op_test3_part1_key}" | wc -l`
			if [ "${actual_key}"  -eq 1 ]
			then	
				read_test3_part1_fail_count=`expr ${read_test3_part1_fail_count} + 1`
			fi
		elif [ "${time_of_this_fail}" -ge "${read_op_test5_time}" ]
		then
			actual_key=`echo "${fail}" | grep "${INVALID_KEY}" | wc -l`
			if [ "${actual_key}" -eq 1 ]
			then
				read_test5_fail_count=`expr ${read_test5_fail_count} + 1`
			fi
		fi
	done <<<"${read_fails}"
fi

if [ "${read_test1_success_count}" -eq "${QUORUMPLUSONE}" -o "${read_test1_success_count}" -eq "${RFPLUSONE}" ]
then
	READ_TEST1_STATUS="${SUCCESS}"
fi
if [ "${read_test2_success_count}" -eq "${QUORUMPLUSONE}" ]
then
	READ_TEST2_STATUS="${SUCCESS}"
fi
if [ "${read_test3_part1_fail_count}" -eq 1 ]
then
	READ_TEST3_PART1_STATUS="${SUCCESS}"
fi

echo "read_test3_part2_success_count"
echo $read_test3_part2_success_count


if [ "${read_test3_part2_success_count}" -eq "${QUORUMPLUSONE}" -o "${read_test3_part2_success_count}" -eq "${RFPLUSONE}" ]
then
	READ_TEST3_PART2_STATUS="${SUCCESS}"
fi
if [ "${read_test4_success_count}" -eq "${QUORUMPLUSONE}" -o "${read_test4_success_count}" -eq "${RFPLUSONE}" ]
then
	READ_TEST4_STATUS="${SUCCESS}"
fi
if [ "${read_test5_fail_count}" -eq "${QUORUMPLUSONE}" -o "${read_test5_fail_count}" -eq "${RFPLUSONE}" ]
then
	READ_TEST5_STATUS="${SUCCESS}"
fi

if [ "${READ_TEST1_STATUS}" -eq "${SUCCESS}" ]
then
	READ_TEST1_SCORE=3
fi
if [ "${READ_TEST2_STATUS}" -eq "${SUCCESS}" ]
then
	READ_TEST2_SCORE=9
fi
if [ "${READ_TEST3_PART1_STATUS}" -eq "${SUCCESS}" ]
then
	READ_TEST3_PART1_SCORE=9
fi
if [ "${READ_TEST3_PART2_STATUS}" -eq "${SUCCESS}" ]
then
	READ_TEST3_PART2_SCORE=10
fi
if [ "${READ_TEST4_STATUS}" -eq "${SUCCESS}" ]
then
	READ_TEST4_SCORE=6
fi
if [ "${READ_TEST5_STATUS}" -eq "${SUCCESS}" ]
then
	READ_TEST5_SCORE=3
fi

# Display score
echo "TEST 1 SCORE..................: ${READ_TEST1_SCORE} / 3"
echo "TEST 2 SCORE..................: ${READ_TEST2_SCORE} / 9"
echo "TEST 3 PART 1 SCORE..................: ${READ_TEST3_PART1_SCORE} / 9"
echo "TEST 3 PART 2 SCORE..................: ${READ_TEST3_PART2_SCORE} / 10"
echo "TEST 4 SCORE..................: ${READ_TEST4_SCORE} / 6"
echo "TEST 5 SCORE..................: ${READ_TEST5_SCORE} / 3"
# Add to grade
GRADE=`expr ${GRADE} + ${READ_TEST1_SCORE}`
GRADE=`expr ${GRADE} + ${READ_TEST2_SCORE}`
GRADE=`echo ${GRADE} ${READ_TEST3_PART1_SCORE} | awk '{print $1 + $2}'`
GRADE=`echo ${GRADE} ${READ_TEST3_PART2_SCORE} | awk '{print $1 + $2}'`
GRADE=`echo ${GRADE} ${READ_TEST4_SCORE} | awk '{print $1 + $2}'`
GRADE=`echo ${GRADE} ${READ_TEST5_SCORE} | awk '{print $1 + $2}'`

#echo ""
#echo "############################"
#echo " READ TEST ENDS"
#echo "############################"
#echo ""

echo ""
echo "TOTAL GRADE: ${GRADE} / 40"
echo ""
