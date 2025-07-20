/*
 * ------------------------------------------------------------------
 * July 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2013 by Arvind Umrao.
 * All rights reserved.
 * 
 * Generate security tocken
 * ------------------------------------------------------------------
 */

#ifndef GenToken_HPP
#define	GenToken_HPP


#include "H1_SecurityToken.h"
#include "H2_SecurityToken.h"

//#ifdef H1SECURITY_CODE

class SecToken : public H1_SecurityToken, H2_SecurityToken
{
public:

    static string createSecurityToken(string& uid,
            string& permissions,
            const string& key, unsigned long sec,
            uint8_t ptz = 0,
            uint8_t qosl = 0,
            uint8_t qosa = 0);

    static string createSecurityH2Token(string& H1token, unsigned long sec);

    static string createSecurityH1Token(string& uid,
            string& permissions,
            const string& key,
            uint8_t ptz = 0,
            uint8_t qosl = 0,
            uint8_t qosa = 0);
            
    static bool SaveAndValidate(string& uid, string& token);
      
};


#endif	/* GenToken_HPP */

