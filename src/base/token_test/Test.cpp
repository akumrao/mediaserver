/*****************************************
 * Test code for
 Common Core reusable classes for string, Thread, Token Generation & Expiration, UUID, CoreID, Loggin, Profiling
  
 */

#include "GenToken.h"
#include "SecurityToken.h"

#include "base/uuid.h"

#include "base/logger.h"

using namespace std;
using namespace base;


int main(int arc, char** argv) {

  
    Logger::instance().add(new ConsoleChannel("debug", Level::Info));


    string permissions("VT");
    string deviceUid("ID");
    string appkey = "admin@provigil.com#232dfdf";
     
    unsigned long expireSec = 5;

    //deviceUid.assign(uuid4::uuid());

      
    SInfo << "perm: "  << permissions  << " key: "  <<   appkey   << " dev: " <<  deviceUid << " tocken expire in secs " << expireSec;
    
    string token = SecToken::createSecurityToken(deviceUid, permissions, appkey, 5);

    SInfo << "token "  << token;
       
       
   //  gut_log(STR_ERROR, "token %s", token.c_str());

    if (token.empty()) {
         SError <<  "Unable to get token for dev "   <<  deviceUid ;
    }

   //  // Validate if Token is expired or Not. Uncomment the following line to test if token is expered after 35 secs
  //  sleep( 1);
   string perm;
   string msg;
   uint32_t statusCode;


    MS_SecurityToken obj(token);
    obj.validate(appkey, msg, perm, statusCode, false);

   SInfo  << "key validate  msg  "<<  msg <<  ", perm " <<   perm  << ",  code " <<   statusCode  ;




    return 0;
}
