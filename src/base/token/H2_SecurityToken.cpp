/*
 * ------------------------------------------------------------------
 * July 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2013 by Arvind Umrao.
 * All rights reserved.
 * 
 * 
 * ------------------------------------------------------------------
 */

#include  "H2_SecurityToken.h"
#include "Encoders.h"
#include <sys/time.h>
#include <vector>
#include <iostream>
#include "SecFile.h"

#include "GenToken.h"

#include "base/logger.h"
using namespace base;




/*
** MediaServer security token helper functions
**
*/
const int H2_SecurityToken::SECURITY_TOKEN_ITEMS;
const char H2_SecurityToken::SECURITY_TOKEN_SEPARATOR;
bool H2_SecurityToken::parse()
{
    // parse m_token based on the following string format
    //  CameraUID^Permissions^PTZ-Priority^QosLive^QosRec^Expiration^H2
    std::vector<string> sdata;
    string::size_type spos = 0, pos = 0;
    if (m_token.empty()) {
        SError << "Security token is missing from the request.";
        return false;
    }
    while (spos < m_token.length()) {
        pos = m_token.find(SECURITY_TOKEN_SEPARATOR, spos);
        if (pos == string::npos) break;
        sdata.push_back(m_token.substr(spos, pos - spos));
        spos = pos + 1;
    }
    if (spos < m_token.length())  {
        sdata.push_back(m_token.substr(spos));
    }
    if (sdata.size() < SECURITY_TOKEN_ITEMS)  {
        return false;
    }

    m_cameraUID = sdata[CAMUID];
    m_permissions = sdata[PERM];
    
    m_ptzPriority = std::stoi(sdata[PTZPRI]);
    m_qosl = std::stoi(sdata[QOSL]);
    m_qosa = std::stoi(sdata[QOSA]);
    m_expirationTime = std::stoi(sdata[EXPIRATION]);
    
    
    m_h2 = sdata[H2];

    sdata.clear();
    return true;

}




bool H2_SecurityToken::validateFromFile(string& emsg, uint32_t& statusCode)
{
    // validate the format of the data
    statusCode = H2_SecurityToken::OK;
    emsg.assign("200 OK");
    if (!parse()) {
        emsg.assign("401 Unauthorized");
        statusCode = H2_SecurityToken::UNAUTHORIZED;
        return false;
    }
    // validate permissions

    

    timeval now;
    gettimeofday(&now, NULL);
    if (m_expirationTime < now.tv_sec) {
        emsg.assign("402 Authorization Expired");
        statusCode = H2_SecurityToken::AUTHORIZATION_EXPIRED;
        return false;
    }

    
    std::string fileName = m_cameraUID + "/" + m_cameraUID;
    
    std::string content;
            
    SecFile::readFile(fileName, content);
  
    string  sectoken = SecToken::createSecurityH2Token( content,  m_expirationTime );
    
    if( sectoken.find(m_h2) == string::npos)
    {
        emsg.assign("401 Unauthorized");
        statusCode = H2_SecurityToken::UNAUTHORIZED;
         return false;
    }
    
    
    return true;
 
}





bool H2_SecurityToken::validate(const string& key,
            string& emsg,
             string& perm,
            uint32_t& statusCode,
            bool perms, bool exp )
{
    // validate the format of the data
    statusCode = H2_SecurityToken::OK;
    emsg.assign("200 OK");
    if (!parse()) {
        emsg.assign("401 Unauthorized");
        statusCode = H2_SecurityToken::UNAUTHORIZED;
        return false;
    }
    // validate permissions
    if (!perm.empty()) {
        // rtps checks for at least one permissions from perm to be in m_permissions
        if (!perms) {
            for (uint8_t i = 0; i < perm.length(); i++) {
                if (m_permissions.find(perm[i]) != string::npos)
                    break;
            }
        } else {
            // http permissions
            for (uint8_t i = 0; i < perm.length(); i++)
                if (m_permissions.find(perm[i]) == string::npos)  {
                    emsg.assign("403 Forbidden");
                    statusCode = H2_SecurityToken::FORBIDDEN;
                    return false;
                }
        }
    }
    // if expiration time is in the near future
    
    if(exp)
    {
        timeval now;
        gettimeofday(&now, NULL);
        if (m_expirationTime < now.tv_sec) {
            emsg.assign("402 Authorization Expired");
            statusCode = H2_SecurityToken::AUTHORIZATION_EXPIRED;
            return false;
        }
    }

    //calculate H2
    string comp_h2;
    comp_h2 = calculateH2(m_cameraUID, m_permissions, m_ptzPriority, m_qosl, m_qosa, key, m_expirationTime);
    bool ret = (comp_h2 == m_h2);
    if (!ret) {
        emsg.assign("401 Unauthorized");
        statusCode = H2_SecurityToken::UNAUTHORIZED;
    }
    else
    {
         if (perm.empty()) 
             perm = m_permissions;
         
    }
    return ret;
}





// stub function. Don't give parameter name for unused parms to avoid unused
// parameter errors.





string H2_SecurityToken::calculateH2(
    string& uid,
    string& permissions,
    uint8_t ptz,
    uint8_t qosl,
    uint8_t qosa,
    const string& key,
    uint32_t expirationTime)
{
    //calculate H1
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 sha1;

    sha1.init();
    //string work;
    
     char work[2048]={'\0'};
     
    //work.appendf("%s%s%s%d%d%d", key.c_str(), uid.c_str(), permissions.c_str(), ptz, qosl, qosa);
    //sha1.update(work.c_str(), work.length());
     
    snprintf(work,  sizeof(work),"%s%s%s%d%d%d", key.c_str(), uid.c_str(), permissions.c_str(), ptz, qosl, qosa);
    
    sha1.update(work, strlen(work) );
    
    sha1.final(digest);

    string temp, temp2;
    temp.assign((const char*)digest, sizeof(digest));
    HexEncoder::encode(temp2, temp);

    //calculate H2 = SHA1(H1, expiration time)
    sha1.init();
   // work.clear();
    //work.appendf("%s%u", temp2.c_str(), expirationTime);
    //sha1.update(work.c_str(), work.length());
    
    
    
    snprintf(work,  sizeof(work),"%s%u", temp2.c_str(), expirationTime);
    
    sha1.update(work, strlen(work) );
    
    sha1.final(digest);

    // hex encode the digest and append to the date string
    string sdigest, comp_h2;
    sdigest.assign((const char*)digest, sizeof(digest));
    HexEncoder::encode(comp_h2, sdigest);

    return comp_h2;
}
