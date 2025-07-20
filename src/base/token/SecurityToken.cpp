/*
 * Copyright (c) 2011-2013 by Arvind Umrao.
 * 
 */

#include "SecurityToken.h"
#include "Encoders.h"
#include <sys/time.h>
#include <vector>
#include <iostream>


#include "base/logger.h"
using namespace base;

/*
 *  create a security token for inter-server communication
 *  param - data to be encrypted in the token
 */
string SecurityToken::createServerToken(
    string& value)
{
    // create a date string
    timeval tv;
    gettimeofday(&tv, NULL);
    char date[1024]={'\0'};
     snprintf(date,sizeof(date), "%x%06x", tv.tv_sec, tv.tv_usec);
    // get salt value for SHA1 from date string
    string salt = date;
    
    SDebug <<  "date: " <<  date  << " elapse time in sec: " << tv.tv_sec  ;
      
    int datelen = salt.length();
     
    salt = salt.substr(salt.length()-10);

    // create a SHA1 digest
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 work;

    work.init();
    work.update(salt.c_str(), salt.length());
    work.update(value.c_str(), value.length());
    work.final(digest);

    // hex encode the digest and append to the date string
    for ( uint8_t i=0; i < SHA1_DIGESTSIZE; i++ )
    {
        snprintf(date + datelen + 2*i ,sizeof(date),"%02x", digest[i]);
    }

     SDebug <<  "ServerToken: " <<  date  << " Salt: " <<  salt;

    // return new date string as the token
    return date;
}





/*
 *  validate a security token for inter-server communication
 *  param - data that was encrypted in the token
 */
bool SecurityToken::isValidServerToken(
    string& token,
    string& value,
    uint32_t timeDiff)
{
    // get the date string from the token
    string date;
    date = token.substr(0, token.length()-(SHA1_DIGESTSIZE*2));

    // get the salt string from the date
    string salt;
    salt = date.substr(date.length()-10);

    SDebug <<  "token: " << token  << " salt: " <<   salt << " date:" << date ;

    // the date is a hex encode time value of the form seconds:microseconds
    string seconds;
    time_t secs{0};
    // extract seconds from date
    seconds = date.substr(0, date.size()-6);
    sscanf(seconds.c_str(), "%x",  &secs);

    // check time is within 60 seconds of our time
    time_t now = time(NULL);
    time_t diff = now - secs;
    SDebug  << "seconds: "<<  seconds << " secs: " << secs  <<   " now: " << now << " diff: " <<  diff;
    if ( ::labs(diff) > timeDiff ) {
        SError  <<  "Token expired " <<  diff ;
        return false;
    }

    // create the SHA1 digest
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 work;

    work.init();
    work.update(salt.c_str(), salt.length());
    work.update(value.c_str(), value.length());
    work.final(digest);

    // hex encode the digest and append to the date
    
    char tmpDate[1024];
    
    snprintf(tmpDate, sizeof(tmpDate), "%s", date.c_str());
    
    for ( uint8_t i=0; i < SHA1_DIGESTSIZE; i++ )
      snprintf(tmpDate + date.size() + 2*i, sizeof(tmpDate), "%02x", digest[i]);

    SDebug << "ServerToken: " <<  tmpDate  ;

    return (tmpDate == token);
}





