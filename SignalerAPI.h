

#ifndef SignalerAPI_H
#define SignalerAPI_H


//#define WEBRTC_WIN

#if defined(WEBRTC_WIN)
#define WEBRTC_PLUGIN_API __declspec(dllexport)
#elif defined(WEBRTC_ANDROID)
#define WEBRTC_PLUGIN_API __attribute__((visibility("default")))
#else
#define WEBRTC_PLUGIN_API __attribute__((visibility("default")))
#endif

typedef void (*LOCALSDPREADYTOSEND_CALLBACK)(const char* type, const char* sdp);
typedef void (*MESSAGE_CALLBACK)(const char* type, const char* msg);

extern "C" {
// Create a peerconnection and return a unique peer connection id.
// WEBRTC_PLUGIN_API int CreatePeerConnection(const char* ip, const int port, const char* roomid, const char** turn_urls,
//                                            const int no_of_urls,
//                                            const char* username,
//                                            const char* credential       );
// // Close a peerconnection.


 WEBRTC_PLUGIN_API    void  connectSignalServer
 (const char* signalip, const int signalport, const char* roomid, const char** turn_urls,
                       const int no_of_urls,
                       const char* username,
                       const char* credential
                      );

 WEBRTC_PLUGIN_API    void  stop( );


    //void createoffer( const std::string& type, const std::string& sdp);
    
 WEBRTC_PLUGIN_API     void sendSDP(const char* type, const char* sdp   );


 WEBRTC_PLUGIN_API bool RegisterOnLocalSdpReadytoSend(LOCALSDPREADYTOSEND_CALLBACK callback);
 WEBRTC_PLUGIN_API bool RegisterOnMessage(MESSAGE_CALLBACK callback);

}

#endif
