//
// Created by root on 28/12/19.
//

#ifndef SA_SOCKETIO_H
#define SA_SOCKETIO_H



typedef void (*LOCALSDPREADYTOSEND_CALLBACK)(const char* type, const char* sdp);
typedef void (*MESSAGE_CALLBACK)(const char* type, const char* msg);

namespace sa {



  	void  connect(const char* signalip, const int signalport, const char* roomid, const char** turn_urls,
					   const int no_of_urls,
					   const char* username,
					   const char* credential
					  );

    	void  stop( );


	//void createoffer( const std::string& type, const std::string& sdp);
    
	 void sendSDP(const char* type, const char* sdp   );

	 bool RegisterOnLocalSdpReadytoSend(LOCALSDPREADYTOSEND_CALLBACK callback);
	 bool RegisterOnMessage(MESSAGE_CALLBACK callback);

}// end sa



#endif //MEDIAAPP_UPLOAD_H
