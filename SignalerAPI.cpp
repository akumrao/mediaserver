#include "SignalerAPI.h"
#include "SocketIO.h"
#include <iostream>

void connectSignalServer(const char* ip, const int port, const char* roomid, const char** turn_urls,
                 const int no_of_urls,
                 const char* username,
                 const char* credential )
{
    sa::connect(ip, port,roomid, turn_urls, no_of_urls, username,credential);
}


void  stop( )
{
  sa::stop();
}


void sendSDP(const char* type, const char* sdp   )
{
     sa::sendSDP(type, sdp);
}



bool RegisterOnLocalSdpReadytoSend(LOCALSDPREADYTOSEND_CALLBACK callback) {

 return sa::RegisterOnLocalSdpReadytoSend( callback);
}


bool RegisterOnMessage(MESSAGE_CALLBACK callback)
{
      
      sa::RegisterOnMessage( callback);

      return true;
}