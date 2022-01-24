/*****************************************
 * Test code for
 Common Core reusable classes for string, Thread, Token Generation & Expiration, UUID, CoreID, Loggin, Profiling
  
 */

#include "GenToken.hxx"
#include "SecurityToken.hxx"


#include <uuid/uuid.h>  //apt-get install uuid uuid-dev
#include "base/logger.h"

using namespace std;
using namespace base;


int main(int arc, char** argv) {

  
    Logger::instance().add(new ConsoleChannel("debug", Level::Info));


     string permissions("VT");
     string deviceUid("ID");
     string appkey = "appkey";
     
     unsigned long expireSec = 5;

     uuid_t uuid;
      char coreID[64];

      uuid_generate(uuid);
      uuid_unparse_lower(uuid, coreID);
      //char coreID[100];
      uuid_unparse(uuid, coreID);
      deviceUid.assign(coreID);

      
      SInfo << "perm: "  << permissions  << " key: "  <<   appkey   << " dev: " <<  deviceUid << " tocken expire in secs " << expireSec;

   // // gut_log(STR_ERROR, " perm %s, dev %s appkey %s", permissions.c_str(), deviceUid.c_str(), appkey.c_str());


   //  timeval now;
   //  gettimeofday(&now, NULL);

       string token = SecToken::createSecurityToken(deviceUid, permissions, appkey, 5);

       SInfo << "token "  << token;
   //  gut_log(STR_ERROR, "token %s", token.c_str());

      if (token.empty()) {
         SError <<  "Unable to get token for dev "   <<  deviceUid ;
      }

   //  // Validate if Token is expired or Not. Uncomment the following line to test if token is expered after 35 secs
       sleep( 1);
      string perm;
      string msg;
      uint32_t statusCode;


      MS_SecurityToken obj(token);
      obj.validate(appkey, msg, perm, statusCode, false);

     SInfo  << "key validate  msg  "<<  msg <<  ", perm " <<   perm  << ",  code " <<   statusCode  ;




    return 0;
}