/*
 * ------------------------------------------------------------------
 * July 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2019  Arvind Umrao
 * All rights reserved.
 * H1 security token = Device Id:userpermissions:ptz Priority:Live QOS:Archive QOS:H1
 * Generate security token

 * ------------------------------------------------------------------
 */

#include <errno.h>
#include <stdio.h>

#include <string.h>
#include "GenToken.h"
#include "Encoders.h"
#include "SecFile.h"
#include "base/filesystem.h"
#include <sys/time.h>
#include <vector>
#include <iostream>

//#ifdef H1SECURITY_CODE

string SecToken::createSecurityToken(
        string& uid,
        string& permissions,
        const string& key, unsigned long sec,
        uint8_t ptz,
        uint8_t qosl,
        uint8_t qosa)
{


    string h1;
    h1 = createSecurityH1Token(uid, permissions,key, ptz, qosl, qosa);
    
    H1_SecurityToken obj(h1);
    obj.parse();
    string H1 = obj.getH1();

    timeval now;
    gettimeofday(&now, NULL);

    uint32_t expir = now.tv_sec + sec;
    string h2;
    h2 = calcH2(H1, expir);

    char token[2048]={'\0'};
    snprintf(token, sizeof(token),"%s^%s^%d^%d^%d^%u^%s", obj.getCameraUID().c_str(), obj.getPermissions().c_str(), obj.getPTZPriority(), obj.getQosl(), obj.getQosa(), expir, h2.c_str());

    return token;
}

string SecToken::createSecurityH2Token(string& H1token, unsigned long expir)
{
    H1_SecurityToken obj(H1token);
    obj.parse();
    string H1 = obj.getH1();

    timeval now;
    gettimeofday(&now, NULL);

    string h2;
    if( expir < 3600*12) // make sure expire time should not be bigger than one day.
    {
        expir = now.tv_sec + expir;
        h2 = calcH2(H1, expir);
    }
    else // TOBD make separate  function for m_expire time found with token
    {
        h2 = calcH2(H1, expir);
    }

    char token[2048]={'\0'};
    snprintf(token, sizeof(token),"%s^%s^%d^%d^%d^%u^%s", obj.getCameraUID().c_str(), obj.getPermissions().c_str(), obj.getPTZPriority(), obj.getQosl(), obj.getQosa(), expir, h2.c_str());

    return token; 
}

string SecToken::createSecurityH1Token(string& uid,
            string& permissions,
            const string& key,
            uint8_t ptz,
            uint8_t qosl,
            uint8_t qosa)

{
    //calculate H1 
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 sha1;

    sha1.init();
   // string work;
   // work.appendf("%s%s%s%d%d%d", key.c_str(), uid.c_str(), permissions.c_str(), ptz, qosl, qosa);
    //sha1.update(work.c_str(), work.length());
    
    char work[2048]={'\0'};
    
    snprintf(work,  sizeof(work),"%s%s%s%d%d%d", key.c_str(), uid.c_str(), permissions.c_str(), ptz, qosl, qosa);
    
    sha1.update(work, strlen(work) );
    
    sha1.final(digest);

    string temp, temp2;
    temp.assign((const char*) digest, sizeof (digest));
    HexEncoder::encode(temp2, temp);
    
    
    char token[2048]={'\0'};
    snprintf(token, sizeof(token), "%s^%s^%d^%d^%d^%s", uid.c_str(), permissions.c_str(), ptz, qosl, qosa, temp2.c_str());
        
    return token;
}



bool SecToken::SaveAndValidate(string& uid, string& token)
{

    std::string fileName = uid + "/" + uid;

    if(  !base::fs::exists(uid) ||!base::fs::exists(fileName))
    {
        if(!base::fs::exists(uid))
          base::fs::mkdir(uid);
        SecFile::writeFile(fileName,  token);

    }
    else
    {
        std::string content;
        SecFile::readFile(fileName,  content);

        std::string msg;
        uint32_t statusCode;


        if(token != content)
        {   
            return false;      
        }

    }
    
    return true;      
}
