 
#include "live.h"
#include "constant.h"
#include "tools.h"
#include "base/logger.h"

#define SEND_PARAMETER_SETS // keep this always defined
 static const unsigned LIVE_GET_PARAMETER_PING = 50;
 
 enum PayloadSizes {
  DEFAULT_PAYLOAD_SIZE            = 1024,    ///< Default buffer size in Live555 for h264 // debug // not used anymore
  
  // DEFAULT_PAYLOAD_SIZE_H264       = 1024*100, ///< Default buffer size in Live555 for h264
  DEFAULT_PAYLOAD_SIZE_H264       = 1024*500, ///< Default buffer size in Live555 for h264 
  // DEFAULT_PAYLOAD_SIZE_H264       = 1024*500, ///< Default buffer size in Live555 for h264
  // DEFAULT_PAYLOAD_SIZE_H264       = 1024*10,  ///< Default buffer size in Live555 for h264 // debug
  // DEFAULT_PAYLOAD_SIZE_H264       = 1024, // use this small value for debugging (( debug
  
  DEFAULT_PAYLOAD_SIZE_PCMU       = 1024,    ///< Default buffer size in Live555 for pcmu
};

 
namespace base {
    namespace fmp4 {
// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
//Logger& operator<<(Logger& logger, const RTSPClient& rtspClient) {
//  return logger.log(logger.current_level) << "[URL:\"" << rtspClient.url() << "\"]: ";
//}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
//Logger& operator<<(Logger& logger, const MediaSubsession& subsession) {
//  return logger.log(logger.current_level) << subsession.mediumName() << "/" << subsession.codecName();
//}
//

void usage(UsageEnvironment& env, char const* progName) {
  SInfo << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
  SInfo << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

// Implementation of "MSRTSPClient":

MSRTSPClient* MSRTSPClient::createNew(UsageEnvironment& env, const std::string rtspURL, FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, LiveStatus* livestatus, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new MSRTSPClient(env, rtspURL, fragmp4_muxer, info, txt, livestatus, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

MSRTSPClient::MSRTSPClient(UsageEnvironment& env, const std::string rtspURL, FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, LiveStatus* livestatus, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) : RTSPClient(env, rtspURL.c_str(), verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1), fragmp4_muxer(fragmp4_muxer),info(info), txt(txt), livestatus(livestatus), request_multicast(false), request_tcp(false), recv_buffer_size(0), reordering_time(0) {
}


MSRTSPClient::~MSRTSPClient() {
}


void MSRTSPClient::continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  LiveStatus* livestatus = ((MSRTSPClient*)rtspClient)->livestatus; // alias
  
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((MSRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      
        SInfo << "MSRTSPClient: "  << "Failed to get a SDP description: " << resultString ;
        
       MSRTSPClient* client = (MSRTSPClient*)rtspClient;
       
       TextFrame txtFrame;
       
       txtFrame.txt =  std::string("Failed to get a SDP description: ") + resultString;
       client->txt->run(&txtFrame);
      
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    // SInfo << "Got a SDP description:\n" << sdpDescription ;
   SInfo << "MSRTSPClient: Got a SDP description:\n" << sdpDescription ;

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      // SInfo  << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() ;
      SInfo << "MSRTSPClient: Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() ;
      break;
    } else if (!scs.session->hasSubsessions()) {
      // SInfo  << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      SInfo << "MSRTSPClient: This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient); // sets *livestatus=LiveStatus::closed;
}

void MSRTSPClient::continueAfterGET_PARAMETER(RTSPClient* rtspClient, int resultCode, char* resultString) {
    // do nothing! (or maybe something)
    std::cout << "MSRTSPClient::continueAfterGET_PARAMETER\n";
    if (resultCode != 0) {
      SInfo << "MSRTSPClient: " << "Failed to get GET_PARAMETER description: " << resultString ;
      delete[] resultString;
      return;
    }

    char* const sdpDescription = resultString;
    // SInfo  << "Got a SDP description:\n" << sdpDescription ;
   SDebug << "MSRTSPClient: Got GET_PARAMETER description:\n" << sdpDescription ;
}


void MSRTSPClient::setupNextSubsession(RTSPClient* rtspClient) {
  // aliases:
  UsageEnvironment& env    = rtspClient->envir();
  StreamClientState& scs   = ((MSRTSPClient*)rtspClient)->scs;
  MSRTSPClient* client = (MSRTSPClient*)rtspClient;
  LiveStatus* livestatus   = ((MSRTSPClient*)rtspClient)->livestatus; // alias
  bool ok_subsession_type = false;
  
  scs.subsession = scs.iter->next();
  scs.subsession_index++;
  
  // CAM_EXCEPTION : UNV-1 == MANUFACTURER: UNV MODEL: IPC312SR-VPF28 
  // CAM_EXCEPTION : UNV-1 : Some UNV cameras go crazy if you try to issue SETUP on the "metadata" subsession which they themselves provide
  // CAM_EXCEPTION : UNV-1 : I get "failed to setup subsession", which is ok, but when "PLAY" command is issued (not on the metadata subsession, but just to normal session) 
  // CAM_EXCEPTION : UNV-1 : we get "connection reset by peer" at continueAfterPlay
  
  if (scs.subsession != NULL) { // has subsession
    
   SInfo << "MSRTSPClient: handling subsession " << scs.subsession->mediumName() ;
    ok_subsession_type = (strcmp(scs.subsession->mediumName(),"video")==0 or strcmp(scs.subsession->mediumName(),"audio")==0); // CAM_EXCEPTION : UNV-1
    
    if (ok_subsession_type) { // a decent subsession
    
      if (!scs.subsession->initiate()) {
        SInfo << "MSRTSPClient: "  << "Failed to initiate the \""  << "\" subsession: " << env.getResultMsg() ;
        setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
      } else { // subsession ok
       SInfo << "MSRTSPClient: "  << " Initiated the \""  << "\" subsession (";
        if (scs.subsession->rtcpIsMuxed()) {
         SInfo << "client port " << scs.subsession->clientPortNum();
        } else {
         SInfo << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
        }
       SInfo << ")\n";

        // adjust receive buffer size and reordering treshold time if requested
        if (scs.subsession->rtpSource() != NULL) {
          if (client->reordering_time>0) {
            scs.subsession->rtpSource()->setPacketReorderingThresholdTime(client->reordering_time);
            SInfo << "MSRTSPClient: packet reordering time now " << client->reordering_time << " microseconds " ;
          }
          if (client->recv_buffer_size>0) {
            int socketNum = scs.subsession->rtpSource()->RTPgs()->socketNum();
            unsigned curBufferSize = getReceiveBufferSize(env, socketNum);
            unsigned newBufferSize = setReceiveBufferTo  (env, socketNum, client->recv_buffer_size);
            SInfo << "MSRTSPClient: receiving socket size changed from " << curBufferSize << " to " << newBufferSize ;
          }
        }
        
        // Continue setting up this subsession, by sending a RTSP "SETUP" command:
        rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, client->request_tcp, client->request_multicast);
        
        /*
        unsigned RTSPClient::sendSetupCommand 	( 	MediaSubsession &  	subsession,
                  responseHandler *  	responseHandler,
                  Boolean  	streamOutgoing = False,
                  Boolean  	streamUsingTCP = False,
                  Boolean  	forceMulticastOnUnspecified = False,
                  Authenticator *  	authenticator = NULL 
          ) 	
        */
        
      } // subsession ok
    }
    else { // decent subsession
     SInfo << "MSRTSPClient: discarded subsession " << scs.subsession->mediumName() ;
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } // decent subsession
    return; // we have either called this routine again with another subsession or sent a setup command
  } // has subsession
    
  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void MSRTSPClient::continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  LiveStatus* livestatus = ((MSRTSPClient*)rtspClient)->livestatus; // alias
  
  do {
    UsageEnvironment& env    = rtspClient->envir(); // alias
    StreamClientState& scs   = ((MSRTSPClient*)rtspClient)->scs; // alias
    FrameFilter *fragmp4_muxer = ((MSRTSPClient*)rtspClient)->fragmp4_muxer;
    FrameFilter *info = ((MSRTSPClient*)rtspClient)->info;
    FrameFilter *txt = ((MSRTSPClient*)rtspClient)->txt;
    
    if (resultCode != 0) {
      SInfo << "MSRTSPClient: "  << "Failed to set up the \""  << "\" subsession: " << resultString ;
      break;
    }

    // SInfo  << "Set up the \""  << "\" subsession (";
   SDebug << "MSRTSPClient: "  << "Set up the \""  << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      // SInfo << "client port " << scs.subsession->clientPortNum();
     SDebug << "client port " << scs.subsession->clientPortNum();
    } else {
      // SInfo << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
     SDebug << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    // SInfo << ")\n";
   SDebug << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    // scs.subsession->sink = FrameSink::createNew(env, *scs.subsession, framefilter, scs.subsession_index, rtspClient->url());
    scs.subsession->sink = FrameSink::createNew(env, scs, fragmp4_muxer, info, txt, rtspClient->url());
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      SInfo << "MSRTSPClient: " << "Failed to create a data sink for the \"" 
	  << "\" subsession: " << env.getResultMsg() ;
      break;
    }

   SDebug << "MSRTSPClient: " << "Created a data sink for the \""  << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession 
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}


void MSRTSPClient::continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;
  LiveStatus* livestatus = ((MSRTSPClient*)rtspClient)->livestatus; // alias
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((MSRTSPClient*)rtspClient)->scs; // alias

  do {

    if (resultCode != 0) {
      SInfo << "MSRTSPClient: " << " Failed to start playing session: " << resultString ;
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

   SDebug << "MSRTSPClient: "  << "Started playing session";
    if (scs.duration > 0) {
     SDebug << " (for up to " << scs.duration << " seconds)";
    }
   SDebug << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
  }
  else {
    *livestatus=LiveStatus::alive;
    // start periodic GET_PARAMETER pinging of the camera.  Required for buggy 3-tier cameras, like AXIS
     SError << "MSRTSPClient: Buggy AXIS firmware does not comply with the RTCP protocol => starting regular GET_PARAMETER pings to the camera" ;
    // ..Sampsa, I commented that stupid pun since it only creates confusion (Petri)
    scs.pingGetParameterTask = env.taskScheduler().scheduleDelayedTask(1000000*LIVE_GET_PARAMETER_PING, (TaskFunc*)pingGetParameter, rtspClient);  // arvind ping imp
  }
}


// Implementation of the other event handlers:
void MSRTSPClient::pingGetParameter(void* clientData) {
    // clientData is an instance of RTSPClient
    MSRTSPClient* client = (MSRTSPClient*)(clientData);
    StreamClientState& scs = client->scs;
    LiveStatus* livestatus = client->livestatus;
    UsageEnvironment& env = client->envir();
    
    //SInfo << "MSRTSPClient: sending GET_PARAMETER ping";
    
    // client->sendGetParameterCommand(*scs.session, MSRTSPClient::continueAfterGET_PARAMETER, "");
    client->sendGetParameterCommand(*scs.session, NULL, ""); // just use this : no callback
    // unsigned sendGetParameterCommand (MediaSession &session, responseHandler *responseHandler, char const *parameterName, Authenticator *authenticator=NULL)
   SDebug << "MSRTSPClient: sent GET_PARAMETER ping ";
    
    if (scs.pingGetParameterTask != NULL and *livestatus == LiveStatus::alive) {
        scs.pingGetParameterTask = env.taskScheduler().scheduleDelayedTask(1000000*LIVE_GET_PARAMETER_PING, (TaskFunc*)pingGetParameter, clientData);
    }
    else {
        scs.pingGetParameterTask = NULL;
    }
}



void MSRTSPClient::subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);
  LiveStatus* livestatus = ((MSRTSPClient*)rtspClient)->livestatus; // alias

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}


void MSRTSPClient::subsessionByeHandler(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

 SDebug << "MSRTSPClient: " << "Received RTCP \"BYE\" on \""  << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void MSRTSPClient::streamTimerHandler(void* clientData) {
  MSRTSPClient* rtspClient = (MSRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

void MSRTSPClient::shutdownStream(RTSPClient* rtspClient, int exitCode) {
  UsageEnvironment& env  =rtspClient->envir(); // alias
  StreamClientState& scs =((MSRTSPClient*)rtspClient)->scs; // alias
  LiveStatus* livestatus = ((MSRTSPClient*)rtspClient)->livestatus; // alias
  
 SInfo << "MSRTSPClient: shutdownStream :" <<std::endl;
  
  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
       SInfo << "MSRTSPClient: shutdownStream : closing subsession" <<std::endl;
	Medium::close(subsession->sink);
       SInfo << "MSRTSPClient: shutdownStream : closed subsession" <<std::endl;
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
     SInfo << "MSRTSPClient: shutdownStream : sending teardown" <<std::endl;
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  SInfo << "MSRTSPClient: "  << " closing the stream.\n";
  *livestatus=LiveStatus::closed;
  
  
  Medium::close(rtspClient);
  // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
  // Uh-oh: how do we tell the event loop that this particular client does not exist anymore..?
  // .. before this RTSPClient deletes itself, we have modified the livestatus member .. that points to
  // a variable managed by the live thread
  /*
  if (--rtspClientCount == 0) {
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you might want to comment this out,
    // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
    exit(exitCode);
  }
  */
}

// Implementation of "StreamClientState":

StreamClientState::StreamClientState() : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), pingGetParameterTask(NULL), duration(0.0), subsession_index(-1), frame_flag(false) {
}


void StreamClientState::close() {
  MediaSubsessionIterator iter2(*session);
  while ((subsession = iter2.next()) != NULL) {
    if (subsession->sink != NULL) {
     SInfo << "StreamClientState : closing subsession" <<std::endl;
      Medium::close(subsession->sink);
     SInfo << "StreamClientState : closed subsession" <<std::endl;
      subsession->sink = NULL;
    }
  }
}


StreamClientState::~StreamClientState() {
  delete iter;
  // return;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias
    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    if (pingGetParameterTask != NULL) {
        env.taskScheduler().unscheduleDelayedTask(pingGetParameterTask);
        pingGetParameterTask = NULL;
    }
    Medium::close(session);
  }
}


// Implementation of "FrameSink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 500000

/*
FrameSink* FrameSink::createNew(UsageEnvironment& env, MediaSubsession& subsession, FrameFilter& framefilter, int subsession_index, char const* streamId) {
  return new FrameSink(env, subsession, framefilter, subsession_index, streamId);
}

FrameSink::FrameSink(UsageEnvironment& env, MediaSubsession& subsession, FrameFilter& framefilter, int subsession_index, char const* streamId) : MediaSink(env), fSubsession(subsession), framefilter(framefilter), subsession_index(subsession_index), on(true), nbuf(0) 
*/

FrameSink* FrameSink::createNew(UsageEnvironment& env, StreamClientState& scs, FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, char const* streamId) {
  return new FrameSink(env, scs, fragmp4_muxer,info, txt, streamId);
}

FrameSink::FrameSink(UsageEnvironment& env, StreamClientState& scs,  FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, char const* streamId) : MediaSink(env), scs(scs), fragmp4_muxer(fragmp4_muxer),info(info),txt(txt), on(true), fSubsession(*(scs.subsession))

{
  // some aliases:
  // MediaSubsession &subsession = *(scs.subsession);
  int subsession_index        = scs.subsession_index;
  
  fStreamId = strDup(streamId);
   fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
  
  const char* codec_name=fSubsession.codecName();
  
  SInfo<< "FrameSink: constructor: codec_name ="<< codec_name << ", subsession_index ="<<subsession_index <<std::endl;
 
  
  
  // https://ffmpeg.org/doxygen/3.0/avcodec_8h_source.html
  if (strcmp(codec_name,"H264")==0) { // NEW_CODEC_DEV // when adding new codecs, make changes here

    fragmp4_muxer->deActivate();

    // WARNING: force subsession index to 0
    subsession_index = 0;

   SDebug << "FrameSink: init H264 Frame"<<std::endl;
    // prepare payload frame
    basicframe.media_type           =AVMEDIA_TYPE_VIDEO;
    basicframe.codec_id             =AV_CODEC_ID_H264;
    basicframe.stream_index     =subsession_index;
    // prepare setup frame
    setupframe.sub_type             =SetupFrameType::stream_init;
    setupframe.media_type           =AVMEDIA_TYPE_VIDEO;
    setupframe.codec_id             =AV_CODEC_ID_H264;   // what frame types are to be expected from this stream
    setupframe.stream_index     = subsession_index;
    setupframe.mstimestamp      = CurrentTime_milliseconds();
    // send setup frame
    //info->run(&setupframe);
    fragmp4_muxer->run(&setupframe);
    //setReceiveBuffer(DEFAULT_PAYLOAD_SIZE_H264); // sets nbuf
  }
  else 
  {
     SInfo<< "FrameSink: constructor: codec_name ="<< codec_name << " not supported yet" <<std::endl;
     return; // no return here!  You won't do sendParameteSets() ..!
  }
  
  // some beautiful day enable audio.  At the moment, it messes up some things (for example, re-transmitting streams, etc.)
  
  
  /*
    else if (strcmp(codec_name,"PCMU")==0) {
   SDebug << "FrameSink: init PCMU Frame"<<std::endl;
    // prepare payload frame
    basicframe.media_type           =AVMEDIA_TYPE_AUDIO;
    basicframe.codec_id             =AV_CODEC_ID_PCM_MULAW;
    basicframe.subsession_index     =subsession_index;
    // prepare setup frame
    setupframe.sub_type             =SetupFrameType::stream_init;
    setupframe.media_type           =AVMEDIA_TYPE_AUDIO;
    setupframe.codec_id             =AV_CODEC_ID_PCM_MULAW;   // what frame types are to be expected from this stream
    setupframe.subsession_index     =subsession_index;
    setupframe.mstimestamp          =getCurrentMsTimestamp();
    // send setup frame
    framefilter.run(&setupframe);
  }
  else {
   SDebug << "FrameSink: WARNING: unknown codec "<<codec_name<<std::endl;
    basicframe.media_type           =AVMEDIA_TYPE_UNKNOWN;
    basicframe.codec_id             =AV_CODEC_ID_NONE;
    basicframe.subsession_index     =subsession_index;
    setReceiveBuffer(DEFAULT_PAYLOAD_SIZE); // sets nbuf
  }
  */
  
#ifdef SEND_PARAMETER_SETS
  sendParameterSets();
#endif
  
 SDebug << "FrameSink: constructor: internal_frame= "<< basicframe <<std::endl;
}

FrameSink::~FrameSink() {
  SInfo << "FrameSink: destructor :"<<std::endl;
   delete[] fReceiveBuffer;
  delete[] fStreamId;
  SInfo << "FrameSink: destructor : bye!"<<std::endl;
}

void FrameSink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds) {
  FrameSink* sink = (FrameSink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
// #define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void FrameSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << " !"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
  
   basicframe.copyBuf(fReceiveBuffer, frameSize );
   
 // unsigned target_size=frameSize+numTruncatedBytes;
  // mstimestamp=presentationTime.tv_sec*1000+presentationTime.tv_usec/1000;
  // std::cout << "afterGettingFrame: mstimestamp=" << mstimestamp <<std::endl;
  basicframe.mstimestamp=(presentationTime.tv_sec*1000+presentationTime.tv_usec/1000);
  basicframe.fillPars();
  
  //SInfo << "afterGettingFrame: " << fragmp4_muxer->resetParser ;
  
 // basicframe.payload.resize(checkBufferSize(frameSize)); // set correct frame size .. now information about the packet length goes into the filter chain
  
   scs.setFrame();
   if ( basicframe.h264_pars.frameType == H264SframeType::i && basicframe.h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
   {
        fragmp4_muxer->sendMeta();
        //fragmp4_muxer->resetParser = false;
   }

   if (basicframe.h264_pars.slice_type == H264SliceType::sps ||  basicframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
   {
       //info->run(&basicframe);
       basicframe.payload.resize(basicframe.payload.capacity());
   }
   else if (!((basicframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicframe.h264_pars.slice_type == H264SliceType::nonidr))) {
        //info->run(&basicframe);
        basicframe.payload.resize(basicframe.payload.capacity());
   }
   else
   {
        //info->run(&basicframe);
        fragmp4_muxer->run(&basicframe); // starts the frame filter chain
        basicframe.payload.resize(basicframe.payload.capacity());
        
   }
                    
  
  // flag that indicates that we got a frame
  
  // std::cerr << "BufferSource: IN0: " << basicframe ;

  
//  if (numTruncatedBytes>0) {// time to grow the buffer..
//   SDebug << "FrameSink : growing reserved size to "<< target_size << " bytes" ;
//    setReceiveBuffer(target_size);
//  }
  
   // recovers maximum size .. must set maximum size before letting live555 to write into the memory area
  
  // Then continue, to request the next frame of data:
  if (on) {continuePlaying();}
}


void FrameSink::afterGettingHeader(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << " !"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
  
  
   basicframe.copyBuf(fReceiveBuffer, frameSize );
  // mstimestamp=presentationTime.tv_sec*1000+presentationTime.tv_usec/1000;
  // std::cout << "afterGettingFrame: mstimestamp=" << mstimestamp <<std::endl;
  basicframe.mstimestamp=(presentationTime.tv_sec*1000+presentationTime.tv_usec/1000);
  basicframe.fillPars();
  // std::cout << "afterGettingFrame: " << basicframe ;
  
 // basicframe.payload.resize(checkBufferSize(frameSize)); // set correct frame size .. now information about the packet length goes into the filter chain
  
                    
  
  scs.setFrame(); // flag that indicates that we got a frame
  
  // std::cerr << "BufferSource: IN0: " << basicframe ;
  //info->run(&basicframe);
  fragmp4_muxer->run(&basicframe); // starts the frame filter chain
  
//  if (numTruncatedBytes>0) {// time to grow the buffer..
//   SDebug << "FrameSink : growing reserved size to "<< target_size << " bytes" ;
//    setReceiveBuffer(target_size);
//  }
  
  basicframe.payload.resize(basicframe.payload.capacity()); // recovers maximum size .. must set maximum size before letting live555 to write into the memory area
  
  // Then continue, to request the next frame of data:
  if (on) {continuePlaying();}
}

Boolean FrameSink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)
  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  // fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
  return True;
}


unsigned FrameSink::checkBufferSize(unsigned target_size) {// add something to the target_size, if needed (for H264, the nalstamp)
   if (basicframe.codec_id==AV_CODEC_ID_H264) {
     target_size+=nalstamp.size();
   }
   // target_size+=8; // receive extra mem to avoid ffmpeg decoder over-read // nopes!  This can screw up the decoder completely!
   return target_size;
}


//void FrameSink::setReceiveBuffer(unsigned target_size) {
//  target_size=checkBufferSize(target_size); // correct buffer size to include nalstamp
//  basicframe.payload.resize(target_size);
//  if (basicframe.codec_id==AV_CODEC_ID_H264) {
//    fReceiveBuffer=basicframe.payload.data()+nalstamp.size(); // pointer for the beginning of the payload (after the nalstamp)
//    nbuf=basicframe.payload.size()-nalstamp.size();           // size left for actual payload (without the prepending 0001)
//    std::copy(nalstamp.begin(),nalstamp.end(),basicframe.payload.begin());
//  }
//  else {
//    fReceiveBuffer=basicframe.payload.data();
//    nbuf=basicframe.payload.size();
//  }
  // std::cout << ">>re-setting receive buffer "<<nbuf<<std::endl;
//}


void FrameSink::sendParameterSets() {
  SPropRecord* pars;
  unsigned i,num;
  struct timeval frametime;
  gettimeofday(&frametime, NULL);
  
  SInfo << "Sending parameter sets!\n";
  pars=parseSPropParameterSets(fSubsession.fmtp_spropparametersets(),num);
  SInfo << "Found " << num << " parameter sets\n";
  for(i=0;i<num;i++) {
    if (pars[i].sPropLength>0) {
      SInfo << "Sending parameter set " << i << " " << pars[i].sPropLength ;
      memcpy(fReceiveBuffer, pars[i].sPropBytes, pars[i].sPropLength);
      
      afterGettingHeader(pars[i].sPropLength, 0, frametime, 0);
    }
  }
  delete[] pars;
}



}
}
