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

//#ifdef H1SECURITY_CODE

class SecToken : public H1_SecurityToken
{
public:

    static string createSecurityToken(string& uid,
            string& permissions,
            const string& key, unsigned long sec,
            uint8_t ptz = 0,
            uint8_t qosl = 0,
            uint8_t qosa = 0);

    static string calculateH1(string& uid,
            string& permissions,
            uint8_t ptz,
            uint8_t qosl,
            uint8_t qosa,
            const string& key);
};

class GenToken
{
public:
    GenToken();
  //  static string getToken(string &deviceUid);
    virtual ~GenToken();
private:

};

//#endif

#endif	/* GenToken_HPP */

