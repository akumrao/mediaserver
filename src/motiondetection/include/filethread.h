#ifndef FILETHREAD_HEADER_GUARD 
#define FILETHREAD_HEADER_GUARD


#include "fileH264.h"

#include "live.h"
//#include "liveserver.h"
#include "base/thread.h"
#include "framefilter.h"
#include <list>

#include "livethread.h"

//#include "framefifo.h"



namespace base {
namespace fmp4 {


/** This is a special FrameFifo class for feeding frames *into* live555, i.e. for sending them to the network.
 * 
 * Should not be instantiated by the user, but requested from FileThread with FileThread::getFifo()
 * 
 * There is a single FileFifo instance per FileThread
 * 
 * @ingroup livethread_tag
 * @ingroup queues_tag
 */

                                                                    


class FileFifo : public FrameFifo {                       
  
public:                                                    
  /** Default constructor */
  FileFifo(const char* name, FrameFifoContext ctx);       
  /** Default virtual destructor */
  ~FileFifo();                                            
  
protected:
  void* live_thread;
  
public:
  void setLiveThread(void* live_thread);
  bool writeCopy(Frame* f, bool wait=false);
};                                                        





/** A base class that unifies all kinds of connections (RTSP and SDP).
 * 
 * Methods of this class are used by the FileThread class and they are called from within the Live555 event loop.
 * 
 * Connect typically has a small, default internal filterchain to correct for the often-so-erroneous timestamps (see the cpp file for more details):
 * 
 * Filterchain: --> {FrameFilter: ConnectionFile::inputfilter} --> {TimestampFrameFilter2: ConnectionFile::timestampfilter} --> {FrameFilter: ConnectionFile::framefilter} -->
 * 
 * @ingroup livethread_tag
 * 
 */ 
class ConnectionFile {
  
public:
  /** Default constructor
   * 
   * @param env   See ConnectionFile::env
   * @param ctx   See ConnectionFile::ctx
   * 
   * @ingroup livethread_tag
   */
  ConnectionFile(UsageEnvironment& env, LiveConnectionContext& ctx);
  virtual ~ConnectionFile(); ///< Default destructor
  
protected:
  LiveConnectionContext   &ctx;           ///< LiveConnectionContext identifying the stream source (address), it's destination (slot and target framefilter), etc. 
  
  // internal framefilter chain.. if we'd like to modify the frames before they are passed to the API user
  // more framefilters could be generated here and initialized in the constructor init list
  // the starting filter should always be named as "inputfilter" .. this is where Live555 writes the frames
  // TimestampFrameFilter2   timestampfilter; ///< Internal framefilter: correct timestamp
  // SlotFrameFilter         inputfilter;     ///< Internal framefilter: set slot number
  
 // FrameFilter*            timestampfilter;
    //FrameFilter*            inputfilter;
  //FrameFilter*            repeat_sps_filter;        ///< Repeat sps & pps packets before i-frame (if they were not there before the i-frame)
  
    FrameFilter *fragmp4_muxer;
    FrameFilter *info;
    FrameFilter *txt;
  
  long int                frametimer;      ///< Measures time when the last frame was received
  long int                pendingtimer;    ///< Measures how long stream has been pending
  
public:
  UsageEnvironment &env;                   ///< UsageEnvironment identifying the Live555 event loop (see \ref live555_page)
  bool is_playing;
  
public:
  virtual void playStream() =0;   ///< Called from within the live555 event loop
  virtual void stopStream() =0;   ///< Stops stream and reclaims it resources.  Called from within the live555 event loop
  virtual void reStartStream();   ///< Called from within the live555 event loop
  virtual void reStartStreamIf(); ///< Called from within the live555 event loop
  virtual bool isClosed();        ///< Have the streams resources been reclaimed after stopping it?
  virtual void forceClose();      ///< Normally, stopStream reclaims the resources.  This one forces the delete.
  SlotNumber getSlot();           ///< Return the slot number
};


/** A base class that unifies all kinds of outgoing streams (i.e. streams sent by live555).  Analogical to ConnectionFile (that is for incoming streams).
 * 
 * @param env    See OutFileBound::env
 * @param fifo   See OutFileBound::fifo
 * @param ctx    See OutFileBound::ctx
 * 
 * @ingroup livethread_tag
 */
class OutFileBound { // will leave this quite generic .. don't know at this point how the rtsp server is going to be // analogy: AVThread
  
public:
  OutFileBound(UsageEnvironment& env, FrameFifo& fifo, LiveOutboundContext& ctx);  ///< Default constructor
  virtual ~OutFileBound(); ///< Default virtual destructor
  
public: // init'd at constructor time
  LiveOutboundContext  &ctx;     ///< Identifies the connection type, stream address, etc.  See LiveOutboundContext
  UsageEnvironment     &env;     ///< Identifies the live555 event loop
  FrameFifo            &fifo;    ///< Outgoing fFrames are being read and finally recycled here
  
protected:
  bool setup_ok, at_setup; ///< Flags used by OutFileBound::handleFrame
  
public:
  virtual void reinit();              ///< Reset session and subsessions
  virtual void handleFrame(Frame *f); ///< Setup session and subsessions, writes payload
};


/** A negotiated RTSP connection
 * 
 * Uses the internal MSRTSPClient instance which defines the RTSP client behaviour, i. e. the events and callbacks that are registered into the Live555 event loop (see \ref live_tag)
 * 
 * @ingroup livethread_tag
 */
class VideoConnection : public ConnectionFile {

public:
  /** @copydoc ConnectionFile::ConnectionFile */
  VideoConnection(UsageEnvironment& env, LiveConnectionContext& ctx);
  ~VideoConnection();
  // VideoConnection(const VideoConnection& cp); ///< Copy constructor .. nopes, default copy constructor good enough
  
  
private:
  VideoFrameSink* client; ///< MSRTSPClient defines the behaviour (i.e. event registration and callbacks) of the RTSP client (see \ref live_tag)
  LiveStatus livestatus;    ///< Reference of this variable is passed to MSRTSPClient.  We can see outside of the live555 callback chains if VideoConnection::client has deallocated itself
  
public:
  void playStream();      ///< Uses MSRTSPClient instance to initiate the RTSP negotiation
  void stopStream();      ///< Uses MSRTSPClient instance to shut down the stream
  void reStartStreamIf(); ///< Restarts the stream if no frames have been received for a while
  bool isClosed();        ///< Have the streams resources been reclaimed?
  void forceClose();
};


/** ConnectionFile is is defined in an SDP file
 * 
 * @ingroup livethread_tag
 */



/** Sending a stream without rtsp negotiation (i.e. without rtsp server) to certain ports
 * 
 * @param env    See OutFileBound::env
 * @param fifo   See OutFileBound::fifo
 * @param ctx    See OutFileBound::ctx
 * 
 * @ingroup livethread_tag
 */
class FileOutbound : public OutFileBound {
  
public: 
  FileOutbound(UsageEnvironment &env, FrameFifo &fifo, LiveOutboundContext& ctx);
  ~FileOutbound();  
  
public: // virtual redefined
  void reinit();
  void handleFrame(Frame *f);
  
public:
 // std::vector<Stream*> streams;  ///< SubStreams of the outgoing streams (typically two, e.g. video and sound)
};


/** Sending a stream using the on-demand rtsp server
 * 
 * @param env    See OutFileBound::env
 * @param fifo   See OutFileBound::fifo
 * @param ctx    See OutFileBound::ctx
 * 
 * @ingroup livethread_tag
 */




/** Live555, running in a separate thread
 * 
 * This class implements a "producer" thread that outputs frames into a FrameFilter (see \ref threading_tag)
 * 
 * This Thread has its own running Live555 event loop.  It registers a callback into the Live555 event loop which checks periodically for signals send to the thread.  Signals to this thread are sent using the FileThread::sendSignal method.
 * 
 * API methods take as parameter either LiveConnectionContext or LiveOutboundContext instances that identify the stream (type, address, slot number, etc.)
 *
 * @ingroup livethread_tag
 * @ingroup threading_tag
 * 
 */


                                                                                                                                                           


class FileThread : public Thread { 
  
public:                           
  static void periodicTask(void* cdata); ///< Used to (re)schedule FileThread methods into the live555 event loop

public:                                                
  /** Default constructor
   * 
   * @param name          Thread name
   * @param n_max_slots   Maximum number of connections (each ConnectionFile instance is placed in a slot)
   * 
   */
  FileThread(const char* name, FrameFifoContext fifo_ctx=FrameFifoContext());  
  ~FileThread();                                                               
  
protected: // frame input
  FileFifo          infifo;     ///< A FrameFifo for incoming frames
  FifoFrameFilter   infilter;   ///< A FrameFilter for writing incoming frames
  
protected: // redefinitions
  std::deque<LiveSignalContext> signal_fifo;    ///< Redefinition of signal fifo (Thread::signal_fifo becomes hidden)
  
protected:
  TaskScheduler*    scheduler;               ///< Live555 event loop TaskScheduler
  UsageEnvironment* env;                     ///< Live555 UsageEnvironment identifying the event loop
  char              eventLoopWatchVariable;  ///< Modifying this, kills the Live555 event loop
  std::vector<ConnectionFile*>   slots_;         ///< A constant sized vector.  Book-keeping of the connections (RTSP or SDP) currently active in the live555 thread.  Organized in "slots".
  std::vector<OutFileBound*>     out_slots_;     ///< Book-keeping for the outbound connections
  std::list<ConnectionFile*>     pending;        ///< Incoming connections pending for closing
  bool                       exit_requested; ///< Exit asap
  EventTriggerId    event_trigger_id_hello_world;
  EventTriggerId    event_trigger_id_frame_arrived;
  EventTriggerId    event_trigger_id_got_frames;
  int fc;                                             ///< debugging: incoming frame counter
  
protected: // rtsp server for live and/or recorded stream
  //UserAuthenticationDatabase  *authDB;
  //RTSPServer                  *server;
  
public: // redefined virtual functions
  void run();
  void stop(bool flag = true);
  void preRun();
  void postRun();
  /** @copydoc Thread::sendSignal */
  void sendSignal(LiveSignalContext signal_ctx);
  
protected:
  void handlePending();       ///< Try to close streams that were not properly closed (i.e. idling for the tcp socket while closing).  Used by FileThread::periodicTask
  void checkAlive();          ///< Used by FileThread::periodicTask
  void closePending();        ///< Force close all pending connections
  void handleSignals();       ///< Handle pending signals in the signals queue.  Used by FileThread::periodicTask
  void handleFrame(Frame* f); ///< Handle incoming frames.  See \ref live_streaming_page
  
private: // internal
  int  safeGetSlot         (SlotNumber slot, ConnectionFile*& con);
  int  safeGetOutboundSlot (SlotNumber slot, OutFileBound*& outbound);
  // inbound streams
  void registerStream   (LiveConnectionContext &connection_ctx);
  void deregisterStream (LiveConnectionContext &connection_ctx);
  void playStream       (LiveConnectionContext &connection_ctx);
  // outbound streams
  void registerOutbound    (LiveOutboundContext &outbound_ctx); 
  void deregisterOutbound  (LiveOutboundContext &outbound_ctx);
  // thread control
  void stopStream       (LiveConnectionContext &connection_ctx);
  
public: // *** C & Python API *** .. these routines go through the condvar/mutex locking                                                
  // inbound streams
  void registerStreamCall   (LiveConnectionContext &connection_ctx); ///< API method: registers a stream                                 
  void deregisterStreamCall (LiveConnectionContext &connection_ctx); ///< API method: de-registers a stream                             
  void playStreamCall       (LiveConnectionContext &connection_ctx); ///< API method: starts playing the stream and feeding frames      
  void stopStreamCall       (LiveConnectionContext &connection_ctx); ///< API method: stops playing the stream and feeding frames       
  // outbound streams
  void registerOutboundCall   (LiveOutboundContext &outbound_ctx);   ///< API method: register outbound stream                          
  void deregisterOutboundCall (LiveOutboundContext &outbound_ctx);   ///< API method: deregister outbound stream                        
  // thread control
  void requestStopCall();                                            ///< API method: Like Thread::stopCall() but does not block        
  // FileFifo &getFifo();                                            ///< API method: get fifo for sending frames with live555          
  FifoFrameFilter &getFrameFilter();                                 ///< API method: get filter for sending frames with live555        
  void setRTSPServer(int portnum=8554);                              ///< API method: activate the RTSP server at port portnum          
  
public: // live555 events and tasks
  static void helloWorldEvent(void* clientData);   ///< For testing/debugging  
  static void frameArrivedEvent(void* clientData); ///< For debugging
  static void gotFramesEvent(void* clientData);    ///< Triggered when an empty fifo gets a frame.  Schedules readFrameFifoTask.  See \ref live_streaming_page
  static void readFrameFifoTask(void* clientData); ///< This task registers itself if there are frames in the fifo

  std::mutex m;
public:  
  void testTrigger();         ///< See \ref live_streaming_page
  void triggerGotFrames();    ///< See \ref live_streaming_page
 
  
}; 



}
}
#endif
