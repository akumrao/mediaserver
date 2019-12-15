//============================================================================
// Name        : CcuKafkaMessage.h
// Author      : Arvind Umrao
// Version     :
// Copyright   : 
// Description : Common Ccu Kafka Message constructs
//============================================================================

#ifndef _CCU_KAFKA_MESSAGE_H_
#define _CCU_KAFKA_MESSAGE_H_

#include "cppkafka/producer.h"
#include "cppkafka/consumer.h"
#include "cppkafka/configuration.h"

#include <time.h>
#include <sys/time.h>

#include <map>



using namespace std;

using namespace cppkafka;


struct AlertDb {
	string mAlertId;
	string mCategory;
	string mLevel;
	string mRca;
	string mAction;
};

class CcuKafkaMessage {

public:

	CcuKafkaMessage ();
	CcuKafkaMessage (const string & topic, const string & payload, const string & txIdKey);

	string getTopic () const
	{
		return mTopic;
	}

	string getPayload () const
	{
		return mPayload;
	}

	string getTxIdKey () const
	{
		return mTxIdKey;
	}

	static string createKfMsgKey ();
	static string createTxIdKey ();

	static void sendLogMessage (Producer * kfProducer, const string & txIdKey, const string & appName, const string & subsystemName, const string & level, const string & textMessageStr, const string & jsonMessageStr);

	//////////////////////////////
	//
	// ALERT LEVELS FOR STATION SW/HW
	//
	// Level:
	// Emergency		emerg		System is unusable								A panic condition.[8]
	// Alert			alert		Action must be taken immediately				A condition that should be corrected immediately, such as a corrupted system database.[8]
	// Critical			crit		Critical conditions	Hard device errors.[8]
	// Error			err			Error conditions	
	// Warning			warning		Warning conditions	
	// Notice			notice		Normal but significant conditions				Conditions that are not error conditions, but that may require special handling.[8]
	// Informational	info		Informational messages	
	// Debug			debug		Debug-level messages							Messages that contain information normally of use only when debugging a program.[8]
	static void sendAlert (Producer * kfProducer, const string & txIdKey, const string & alertId, const string & modelName, bool isActive, const string & textMessageStr, const string & jsonMessageStr);

	static const char* MODEL_ALERTDB_ROOT_KEY;
	static const char* MODEL_ALERTDB_ALERTID_KEY;
	static const char* MODEL_ALERTDB_CATEGORY_KEY;
	static const char* MODEL_ALERTDB_LEVEL_KEY;
	static const char* MODEL_ALERTDB_RCA_KEY;
	static const char* MODEL_ALERTDB_ACTION_KEY;

	static int MODEL_ALERTDB_NUM_KEYS;

private:
	static int loadAlertDb ();

private:
	std::string mTopic;
	std::string mPayload;
	std::string mTxIdKey;

};

#endif // _CCU_KAFKA_MESSAGE_H_

