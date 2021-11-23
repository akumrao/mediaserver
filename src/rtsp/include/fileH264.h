#ifndef file_HEADER_GUARD
#define file_HEADER_GUARD

#include "livedep.h"
#include "frame.h"
//#include "framefifo.h"
#include "framefilter.h"

#include "H264Framer.h"


namespace base {
namespace fmp4 {

UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient);       ///< A function that outputs a string that identifies each stream (for debugging output).
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession);  ///< A function that outputs a string that identifies each subsession (for debugging output).
//Logger& operator<<(Logger& logger, const RTSPClient& rtspClient);                        ///< A function that outputs a string that identifies each stream (for debugging output).
//Logger& operator<<(Logger& logger, const MediaSubsession& subsession);                   ///< A function that outputs a string that identifies each subsession (for debugging output).
void usage(UsageEnvironment& env, char const* progName);





/** Class to hold per-stream state that we maintain throughout each stream's lifetime.
 * 
 * An instance of this class is included in the MSRTSPClient and used in the response handlers / callback chain.
 * 
 * This is a bit cumbersome .. Some of the members are created/managed by the MSRTSPClient instance.  When MSRTSPClient destructs itself (by calling Medium::close on its sinks and MediaSession) in the response-handler callback chain, we need to know that in LiveThread.
 *
 * @ingroup live_tag
 */
class VideoClientState {
public:
  VideoClientState();            ///< Default constructor
  virtual ~VideoClientState();   ///< Default virtual destructor.  Calls Medium::close on the MediaSession object

public:
  MediaSubsessionIterator* iter;  ///< Created by RTSPClient or SDPClient.  Deleted by VideoClientState::~VideoClientState
  int subsession_index;           ///< Managed by RTSPClient or SDPClient
  MediaSession* session;          ///< Created by RTSPClient or SDPClient.  Closed by VideoClientState::~VideoClientState
  MediaSubsession* subsession;    ///< Created by RTSPClient or SDPClient.  Closed by VideoClientState::close
  TaskToken streamTimerTask;
  TaskToken pingGetParameterTask;      ///< Ping the camera periodically with GET_PARAMETER query
  
  double duration;
  bool frame_flag;                ///< Set always when a frame is received
  
public:
  void close();                   ///< Calls Medium::close on the MediaSubsession objects and their sinks
  
public: // setters & getters
  void setFrame()     {this->frame_flag=true;}
  void clearFrame()   {this->frame_flag=false;}
  bool gotFrame()     {return this->frame_flag;}
};





/** Live555 handling of media frames 
 * 
 * When the live555 event loop has composed a new frame, it's passed to VideoFrameSink::afterGettingFrame.  There it is passed to the beginning of valkka framefilter chain.
 * 
 * @ingroup live_tag
 */
class VideoFrameSink: public MediaSink {

public:

  /** Default constructor
   * @param env          The usage environment, i.e. event loop in question
   * @param scs          Info about the stream state
   * @param subsession   Identifies the kind of data that's being received (media type, codec, etc.)
   * @param framefilter  The start of valkka FrameFilter filterchain
   * @param streamId     (optional) identifies the stream itself
   * 
   */
  static VideoFrameSink* createNew(UsageEnvironment& env, VideoClientState& scs, FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, char const* streamId = NULL);

private:
  VideoFrameSink(UsageEnvironment& env, VideoClientState& scs, FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, char const* streamId);
  /** Default virtual destructor */
  virtual ~VideoFrameSink();

  static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds); ///< Called after live555 event loop has composed a new frame
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds); ///< Called by the other afterGettingFrame
 // void afterGettingHeader(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds); ///< Called by the other afterGettingFrame
  
  
  
private:
  void setReceiveBuffer(unsigned target_size);     ///< Calculates receiving memory buffer size
  unsigned checkBufferSize(unsigned target_size);  ///< Calculates receiving memory buffer size
  void sendParameterSets();                        ///< Extracts sps and pps info from the SDP string.  Creates sps and pps frames and sends them to the filterchain.
  
private: // redefined virtual functions:
  virtual Boolean continuePlaying();  ///< Live555 redefined virtual function

private:
  
  u_int8_t*         fReceiveBuffer;
  //long unsigned     nbuf;       ///< Size of bytebuffer
  MediaSubsession&  fSubsession;
  char*             fStreamId;
  UsageEnvironment &env;
  
  FrameFilter *fragmp4_muxer;
  FrameFilter *info;
  FrameFilter *txt;  
  SetupFrame        setupframe;  ///< This frame is used to send subsession information
  BasicFrame        basicframe;  ///< Data is being copied into this frame
  int               subsession_index;

public: // getters & setters
  uint8_t* getReceiveBuffer() {return fReceiveBuffer;}
  void Play();
  
  
public:
  bool on;
  
  VideoClientState &scs; 
  
  H264VideoStreamFramer* videoSource;
  
  bool foundsps{false};
  bool foundpps{false};
  
  int fps{0};
  int width{0};
  int height{0};
    
  
  H264Framer obj;
            
  
  
};

}
}
#endif
