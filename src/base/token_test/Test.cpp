/*****************************************
 * Test code for
 Common Core reusable classes for string, Thread, Token Generation & Expiration, UUID, CoreID, Loggin, Profiling
  
 */

#include "GenToken.h"
#include "SecurityToken.h"

#include "base/uuid.h"
#include "base/application.h"
#include "base/logger.h"

using namespace std;
using namespace base;


int main(int arc, char** argv) {

  
    Logger::instance().add(new ConsoleChannel("debug", Level::Debug));

    Application app;
      
    /******************************************************************************************************************************************/
    // Test Case 1 For   Auth token and Single Sign on 
    
    string permissions("VT");
    string deviceUid("ID");
    string appkey = "admin@provigil.com#232dfdf";
     
    unsigned long expireSec = 25;

    //deviceUid.assign(uuid4::uuid());

      
    SInfo << "perm: "  << permissions  << " key: "  <<   appkey   << " dev: " <<  deviceUid << " tocken expire in secs " << expireSec;
    
    string token = SecToken::createSecurityToken(deviceUid, permissions, appkey, expireSec);

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


    H2_SecurityToken obj(token);
    obj.validate(appkey, msg, perm, statusCode, false);
    
    
    

   SInfo  << "key validate  msg  "<<  msg <<  ", perm " <<   perm  << ",  code " <<   statusCode  ;


   /******************************************************************************************************************************************/
   // Test Case 2  For HTTP 
   // It  will use the SHA1 algorithm to encrypt the password.
   //The other database column is salt. Adding a salt to the hashed password make it more difficult to break the password. 
   //The salt is a random string that is generated and stored for each user. We then add this salt to the password before encrypting it. Every user has a different salt, but we store each user’s salt in the database so that we can authenticate the user’s password.
   /*  Added more test cases */  // this  ( DIGEST ( salt + SHA1)

   std::string uid("12345567891234567891234568"); //
   
  
   string sectoken =  SecurityToken::createServerToken(uid);
   
   bool valid =   SecurityToken::isValidServerToken(sectoken, uid );
   
   
   /******************************************************************************************************************************************/
   // Test Case 3  For Password 
   
   
   deviceUid = "admin";
   
   std::string h1Token = SecToken::createSecurityH1Token(deviceUid, perm,appkey, 0,0,0 );
   
   if( !SecToken::SaveAndValidate(deviceUid, h1Token))
   {
       SError << deviceUid << " does not exist or password wrong";
   }
   else
   {
       //expireSec = 12;                      
       std::string h2token = SecToken::createSecurityH2Token(h1Token, expireSec );
     
        H2_SecurityToken obj(h2token);
        obj.validateFromFile( msg,  statusCode);
        
        SInfo  << "key validate  msg  "<<  msg <<   " code " <<   statusCode  ;
        
   }
   
   
   
   app.run();
   
   
   return 0;
}
