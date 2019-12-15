//============================================================================
// Name        : CcuKafkaMessage.cpp
// Author      :Arvind Umrao
// Version     :
// Copyright   : Copyright SUNMobility, Bangalore India
// Description : Common Ccu Kafka Message constructs
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
//#include <stdio.h>
//#include <stdlib.h>
#include <cstring> 
using namespace std; 

#include <cppkafka/producer.h>
#include <cppkafka/consumer.h>
#include <cppkafka/configuration.h>

/*
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
*/

#include <time.h>
#include <sys/time.h>

#include "CcuKafkaMessage.h"

const char* CcuKafkaMessage::MODEL_ALERTDB_ROOT_KEY = "alert-definitions";
const char* CcuKafkaMessage::MODEL_ALERTDB_ALERTID_KEY = "alertid";
const char* CcuKafkaMessage::MODEL_ALERTDB_CATEGORY_KEY = "category";
const char* CcuKafkaMessage::MODEL_ALERTDB_LEVEL_KEY = "level";
const char* CcuKafkaMessage::MODEL_ALERTDB_RCA_KEY = "rca";
const char* CcuKafkaMessage::MODEL_ALERTDB_ACTION_KEY = "action";

int CcuKafkaMessage::MODEL_ALERTDB_NUM_KEYS = 5; // Excludes MODEL_ROOT_KEY



CcuKafkaMessage::CcuKafkaMessage () {
}

CcuKafkaMessage::CcuKafkaMessage (const string & topic, const string & payload, const string & txIdKey) 
{
	mTopic = topic;
	mPayload = payload;
	mTxIdKey = txIdKey;
}

string 
CcuKafkaMessage::createKfMsgKey ()
{
	struct timeval mediaTime;
   	memset (&mediaTime, 0, sizeof(mediaTime));
   	gettimeofday(&mediaTime, 0);
   	time_t mediaTimeMs = (mediaTime.tv_sec)*1000+(mediaTime.tv_usec)/1000;

	string kfMsgKey = std::to_string(mediaTimeMs);

	return kfMsgKey;
}

string 
CcuKafkaMessage::createTxIdKey ()
{
	char bufP[1024]; bufP[0] = '\0';

	struct timeval mediaTime;
   	memset (&mediaTime, 0, sizeof(mediaTime));
   	gettimeofday(&mediaTime, 0);

	sprintf(bufP, "%ld-%ld", mediaTime.tv_sec, mediaTime.tv_usec);
	string txIdKey(bufP);
	
	return txIdKey;
}

void 
CcuKafkaMessage::sendLogMessage (Producer * kfProducer, const string & txIdKey, const string & appName, const string & subsystemName, const string & level, const string & textMessageStr, const string & jsonMessageStr)
{
	// Filter out messages based on level settings
	//static char* logErrorsP = getenv("SMCCULOGERRORS");
	//static char* logDebugsP = getenv("SMCCULOGDEBUGS");
        
        // export SMCCULOGERRORS=1
         //export SMCCULOGDEBUGS=1
    

    static char* logErrorsP = "1";
	static char* logDebugsP = "1";

	// "level": "emergency | alert | critical | error | notice | info | debug",
	if (level.length() < 1) {
		return;
	}
	const char* levelP = level.c_str();
	char levelC = levelP[0];

	// For high efficiency, just look at the first character
	// "level": "emergency | alert | critical | error 
	if ((levelC = 'e') || (levelC == 'a') || (levelC == 'c')) {
		if (!logErrorsP) {
			return;
		}
	}	
	// For high efficiency, just look at the first character
	// "level": "notice | info | debug",
	if ((levelC = 'n') || (levelC == 'i') || (levelC == 'd')) {
		if (!logDebugsP) {
			return;
		}
	}	

	//////////////////////////////
	// 
	// JSON output for Wheeler CAN
	//
	// "media-time": <ISO8601 Timestamp>,
	// "media-txIdKey":"<Tracking UUID>",
	// "media-source" : {
	//	"type":"LOCAL-APP"
	//	"address":"<Application Name>"
	// }
	// "media-recordType": "WHEELER-LOG-MSG",
	// "media-data" : {
	//	"log-message" : {
	//		"subsystem" : "<Subsystem name>",
	//		"level": "emergency | alert | critical | error | notice | info | debug",
	//		"text-message": "<Details in non-JSON text",
	//		"json-message": <Details in pure JSON text>,
	//	}
	// }
	//
	//////////////////////////////

	struct timeval mediaTime;
    memset (&mediaTime, 0, sizeof(mediaTime));
    gettimeofday(&mediaTime, 0);
    time_t mediaTimeMs = (mediaTime.tv_sec)*1000+(mediaTime.tv_usec)/1000;

	string kfPayload = "{\"media-time\":" + std::to_string(mediaTimeMs) + ",";

	if (txIdKey.length() == 0) {
		kfPayload += "\"media-txIdKey\":\"" + CcuKafkaMessage::createTxIdKey() + "\",";
	} else {
		kfPayload += "\"media-txIdKey\":\"" + txIdKey + "\",";
	}
	kfPayload += "\"media-source\":{\"type\":\"LOCAL-APP\",\"address\":\"" + appName + "\"},";
	kfPayload += "\"media-recordType\":\"LOG-MSG\",\"media-data\":{\"log-message\":{";

	kfPayload += "\"subsystem\":\"" + subsystemName + "\",";
	kfPayload += "\"level\":\"" + level + "\",";
	kfPayload += "\"text-message\":\"" + textMessageStr + "\",";
	if (jsonMessageStr.length() == 0) {
		kfPayload += "\"json-message\":{}";
	} else {
		kfPayload += "\"json-message\":" + jsonMessageStr;
	}

	kfPayload += "}}}";

	cout << "SendLogMessage: " << endl << kfPayload << endl;

    string kfMsgKey = CcuKafkaMessage::createKfMsgKey();

    MessageBuilder kfMessageBuilder = MessageBuilder ("ccuapp-to-log");
    kfMessageBuilder.partition(0).key(kfMsgKey).payload(kfPayload);

    kfProducer->produce(kfMessageBuilder);
    kfProducer->flush();
}

/*
int 
CcuKafkaMessage::loadAlertDb () {
	mAlertDbMutex.Lock ();

	mAlertDb.clear ();

    std::ifstream ifs ((char*)"/ccu/src/AppSw/CcuAppAlertDb.json");
   	IStreamWrapper isw ( ifs );

    Document doc {};
   	doc.ParseStream( isw );

	Value& charListMapping = doc[CcuKafkaMessage::MODEL_ALERTDB_ROOT_KEY];

	int sz = charListMapping.Size ();

	for (int i = 0; i < sz; i++) {
		string alertId (""), category (""), level (""), rca (""), action ("");

		int found = 0; 
		for (Value::ConstMemberIterator iter = charListMapping[i].MemberBegin(); iter != charListMapping[i].MemberEnd(); ++iter) {

            string key = iter->name.GetString();

			if ((strcmp(key.c_str(), CcuKafkaMessage::MODEL_ALERTDB_ALERTID_KEY) == 0) &&
			    (iter->value.IsString())) {
				alertId = iter->value.GetString();
				found++;
			} else if ((strcmp(key.c_str(), CcuKafkaMessage::MODEL_ALERTDB_CATEGORY_KEY) == 0) &&
			    (iter->value.IsString())) {
				category = iter->value.GetString();
				found++;
			} else if ((strcmp(key.c_str(), CcuKafkaMessage::MODEL_ALERTDB_LEVEL_KEY) == 0) &&
			    (iter->value.IsString())) {
				level = iter->value.GetString();
				found++;
			} else if ((strcmp(key.c_str(), CcuKafkaMessage::MODEL_ALERTDB_RCA_KEY) == 0) &&
			    (iter->value.IsString())) {
				rca = iter->value.GetString();
				found++;
			} else if ((strcmp(key.c_str(), CcuKafkaMessage::MODEL_ALERTDB_ACTION_KEY) == 0) &&
			    (iter->value.IsString())) {
				action = iter->value.GetString();
				found++;
			}
		}
		if (found != CcuKafkaMessage::MODEL_ALERTDB_NUM_KEYS) {
			cerr << "!!!! ERROR !!!!: Alert DB mapping: " << alertId << ", : " << category << ", : " << level << ", : " << rca << ", : " << action << endl;
			continue;
		} else {
			// cout << "<<<< Alert DB mapping >>>>: " << alertId << ", : " << category << ", : " << level << ", : " << rca << ", : " << action << endl;
		}

		AlertDb alertDb; 
		alertDb.mAlertId  = alertId;
		alertDb.mCategory = category;
		alertDb.mLevel    = level;
		alertDb.mRca      = rca;
		alertDb.mAction   = action;
	
		mAlertDb[alertId] = alertDb;
	}

	mAlertDbMutex.Unlock ();
	return 0;
}

void 
CcuKafkaMessage::sendAlert (Producer * kfProducer, const string & txIdKey, const string & alertId, const string & modelName, bool isActive, const string & textMessageStr, const string & jsonMessageStr)
{
	static char* mqisSerialNumberP = getenv("MQISSERIALNUMBER");

	if (!mqisSerialNumberP) {
		string textLogMessage = "Failed to generate alert: " + alertId + ", with message: " + textMessageStr + ", as MQISSERIALNUMBER is not defined";
		CcuKafkaMessage::sendLogMessage (kfProducer, txIdKey, "Ccu", "alert-interface", "error", textLogMessage, jsonMessageStr);
		
		return;
	}
	
	string mqisSerialNumber (mqisSerialNumberP);

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

	static bool alertDbLoaded = false;

	if (alertDbLoaded == false) {
		
		if (CcuKafkaMessage::loadAlertDb () == 0) {
			alertDbLoaded = true;
		} else {
			string textLogMessage = "Failed to generate alert: " + alertId + ", with message: " + textMessageStr + ", as ALERT DB is not loaded";
			CcuKafkaMessage::sendLogMessage (kfProducer, txIdKey, "Ccu", "alert-interface", "error", textLogMessage, jsonMessageStr);
		
			return;
		}
	}

	mAlertDbMutex.Lock ();

	map<string, AlertDb>::iterator itr = CcuKafkaMessage::mAlertDb.find (alertId);
	if (itr == CcuKafkaMessage::mAlertDb.end ()) {
		string textLogMessage = "Failed to generate alert: " + alertId + ", with message: " + textMessageStr + ", as ALERT is not defined in DB";
		CcuKafkaMessage::sendLogMessage (kfProducer, txIdKey, "Ccu", "alert-interface", "error", textLogMessage, jsonMessageStr);
		
		mAlertDbMutex.Unlock ();
		return;
	}

	string level    = ((*itr).second).mLevel;
	string category = ((*itr).second).mCategory;

	mAlertDbMutex.Unlock ();

	//////////////////////////////
	// 
	// JSON output for Wheeler CAN
	//
	// "media-time": <ISO8601 Timestamp>,
	// "media-txIdKey":"<Tracking UUID>",
	// "media-source" : {
	//	"type":"LOCAL-APP"
	//	"address":"<Application Name>"
	// }
	// "media-recordType": "WHEELER-ALERT",
	// "media-data" : {
	//	"alert-message" : {
	//		"alertId" : "<Alert ID per JSON>",
	//		"modelName": "<Model name>",
	//		"category" : "<Category per JSON>",
	//		"level": "<As per mapping in AlertDB>",
	//		"text-message": "<Details in non-JSON text",
	//		"json-message": <Details in pure JSON text>,
	//	}
	// }
	//
	//////////////////////////////

	struct timeval mediaTime;
    memset (&mediaTime, 0, sizeof(mediaTime));
    gettimeofday(&mediaTime, 0);
    time_t mediaTimeMs = (mediaTime.tv_sec)*1000+(mediaTime.tv_usec)/1000;

	string kfPayload = "{\"media-time\":" + std::to_string(mediaTimeMs) + ",";

	if (txIdKey.length() == 0) {
		kfPayload += "\"media-txIdKey\":\"" + CcuKafkaMessage::createTxIdKey() + "\",";
	} else {
		kfPayload += "\"media-txIdKey\":\"" + txIdKey + "\",";
	}
	kfPayload += "\"media-source\":{\"type\":\"STATION-SERIAL-NUMBER\",\"address\":\"" + mqisSerialNumber + "\"},";
	kfPayload += "\"media-recordType\":\"WHEELER-ALERT\",\"media-data\":{\"alert-message\":{";

	kfPayload += "\"alertId\":\"" + alertId + "\",";
	kfPayload += "\"modelName\":\"" + modelName + "\",";
	kfPayload += "\"category\":\"" + category + "\",";
	kfPayload += "\"level\":\"" + level + "\",";
	kfPayload += "\"state\":\"" + (isActive?string("set"):string("clear")) + "\",";
	kfPayload += "\"text-message\":\"" + textMessageStr + "\",";
	if (jsonMessageStr.length() == 0) {
		kfPayload += "\"json-message\":{}";
	} else {
		kfPayload += "\"json-message\":" + jsonMessageStr;
	}

	kfPayload += "}}}";

    string kfMsgKey = CcuKafkaMessage::createKfMsgKey();

    MessageBuilder kfMessageBuilder = MessageBuilder ("ccuapp-to-alerts");
    kfMessageBuilder.partition(0).key(kfMsgKey).payload(kfPayload);

   	kfProducer->produce(kfMessageBuilder);
        kfProducer->flush();
}
*/