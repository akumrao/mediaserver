
#include "filethread.h"
//#include "logging.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iterator>
#include "base/logger.h"

// #define RECONNECT_VERBOSE   // by default, disable
// #define LIVE_SIGNAL_FRAMES // experimental

using namespace std::chrono_literals;
using std::this_thread::sleep_for; 

namespace base {
namespace fmp4 {
    
    
  namespace Timeout { ///< Various thread timeouts in milliseconds
  const static long unsigned thread       =250; // Timeout::thread
  const static long unsigned livethread   =250; // Timeout::livethread
  const static long unsigned avthread     =250; // Timeout::avthread
  const static long unsigned openglthread =250; // Timeout::openglthread
  const static long unsigned fswriterthread = 250; // Timeout::fswriterthread
  const static long unsigned fsreaderthread = 250; // Timeout::fswriterthread
  const static long unsigned filecachethread = 1000; // Timeout::cachethread
  // const static long unsigned filecachethread = 500; // Timeout::cachethread
  const static long unsigned usbthread    =250; // Timeout::usbthread
  const static long int filethread        =2000; // Timeout::filethread
  const static long int fdwritethread        =250; // Timeout::filethread
}

    
    static const SlotNumber I_MAX_SLOTS = 255;
    


FileFifo::FileFifo(const char* name, FrameFifoContext ctx) : FrameFifo(name, ctx) {
}

FileFifo::~FileFifo() {
}


void FileFifo::setLiveThread(void *live_thread) { // we need the FileThread so we can call one of its methods..
    this->live_thread=live_thread;
}


bool FileFifo::writeCopy(Frame* f, bool wait) {
    bool do_notify=false;
    bool ok=false;
    
    if (isEmpty()) { // triggerGotFrames => gotFramesEvent => readFrameFifoTask (this one re-registers itself if there are frames in the queue - if queue empty, must be re-registered here)
        do_notify=true; 
    }
    
    ok=FrameFifo::writeCopy(f,wait);
    
    ///*
    if (ok and do_notify) {
        ((FileThread*)live_thread)->triggerGotFrames();
    }
    //*/
    /*
     *  if (ok) {
     *    ((FileThread*)live_thread)->triggerGotFrames();
}
*/
    
    return ok;
    
    /*
     *  if (FrameFifo::writeCopy(f,wait)) {
     *    ((FileThread*)live_thread)->triggerNextFrame();
}
*/
}




#define TIMESTAMP_CORRECTOR // keep this always defined

/*
 * #ifdef TIMESTAMP_CORRECTOR
 * // filterchain: {FrameFilter: inputfilter} --> {TimestampFrameFilter: timestampfilter} --> {FrameFilter: framefilter}
 * ConnectionFile::ConnectionFile(UsageEnvironment& env, const std::string address, SlotNumber slot, FrameFilter& framefilter, long unsigned int msreconnect) : env(env), address(address), slot(slot), framefilter(framefilter), msreconnect(msreconnect), is_playing(false), frametimer(0), timestampfilter("timestamp_filter",&framefilter), inputfilter("input_filter",slot,&timestampfilter)
 * #else
 * // filterchain: {FrameFilter: inputfilter} --> {FrameFilter: framefilter}
 * ConnectionFile::ConnectionFile(UsageEnvironment& env, const std::string address, SlotNumber slot, FrameFilter& framefilter, long unsigned int msreconnect) : env(env), address(address), slot(slot), framefilter(framefilter), msreconnect(msreconnect), is_playing(false), frametimer(0), timestampfilter("timestamp_filter",&framefilter), inputfilter("input_filter",slot,&framefilter)
 * #endif
 */

/*
 * #ifdef TIMESTAMP_CORRECTOR
 * // filterchain: {FrameFilter: inputfilter} --> {TimestampFrameFilter: timestampfilter} --> {FrameFilter: framefilter}
 * ConnectionFile::ConnectionFile(UsageEnvironment& env, LiveConnectionContext& ctx) : env(env), ctx(ctx), is_playing(false), frametimer(0), timestampfilter("timestamp_filter",ctx.framefilter), inputfilter("input_filter",ctx.slot,&timestampfilter)
 * #else
 * // filterchain: {FrameFilter: inputfilter} --> {FrameFilter: framefilter}
 * ConnectionFile::ConnectionFile(UsageEnvironment& env, LiveConnectionContext& ctx) : env(env), ctx(ctx), is_playing(false), frametimer(0), timestampfilter("timestamp_filter",NULL), inputfilter("input_filter",slot,ctx.framefilter)
 * #endif
 * {
 *  if (ctx.msreconnect>0 and ctx.msreconnect<=Timeout::livethread) {
 *   SError << "ConnectionFile: constructor: your requested reconnection time is less than equal to the FileThread timeout.  You will get problems" ;
 *  }  
 * };
 */


ConnectionFile::ConnectionFile(UsageEnvironment& env, LiveConnectionContext& ctx) : env(env), ctx(ctx), is_playing(false), frametimer(0), pendingtimer(0) {
    if       (ctx.time_correction==TimeCorrectionType::none) {
        // no timestamp correction: FileThread --> {SlotFrameFilter: inputfilter} --> ctx.framefilter
//        timestampfilter    = new TimestampFrameFilter2("timestampfilter", NULL); // dummy

          fragmp4_muxer        = ctx.framefilter;
          info = ctx.info;
          txt = ctx.txt;
    }
    else if  (ctx.time_correction==TimeCorrectionType::dummy) {
        // smart timestamp correction:  FileThread --> {SlotFrameFilter: inputfilter} --> {TimestampFrameFilter2: timestampfilter} --> ctx.framefilter
        //timestampfilter    = new DummyTimestampFrameFilter("dummy_timestamp_filter", ctx.framefilter);
        //repeat_sps_filter  = new RepeatH264ParsFrameFilter("repeat_sps_filter", timestampfilter);
      //  inputfilter        = new SlotFrameFilter("input_filter", ctx.slot, repeat_sps_filter);
          fragmp4_muxer        = ctx.framefilter;
          info = ctx.info;
           txt = ctx.txt;
    }
    else { // smart corrector
        // brute-force timestamp correction: FileThread --> {SlotFrameFilter: inputfilter} --> {DummyTimestampFrameFilter: timestampfilter} --> ctx.framefilter
      //  timestampfilter    = new TimestampFrameFilter2("smart_timestamp_filter", ctx.framefilter);
      //  repeat_sps_filter  = new RepeatH264ParsFrameFilter("repeat_sps_filter", timestampfilter);
       // inputfilter        = new SlotFrameFilter("input_filter", ctx.slot, repeat_sps_filter);
          fragmp4_muxer        = ctx.framefilter;
          info = ctx.info;
           txt = ctx.txt;
    }
}


ConnectionFile::~ConnectionFile() {
//    delete timestampfilter;
//    delete inputfilter;
//    delete repeat_sps_filter;
};

void ConnectionFile::reStartStream() {
    stopStream();
    playStream();
}

void ConnectionFile::reStartStreamIf() {
}

SlotNumber ConnectionFile::getSlot() {
    return ctx.slot;
};

bool ConnectionFile::isClosed() {
    return true;
}

void ConnectionFile::forceClose() {
}




// OutFileBound::OutFileBound(UsageEnvironment &env, FrameFifo &fifo, SlotNumber slot, const std::string adr, const unsigned short int portnum, const unsigned char ttl) : env(env), fifo(fifo), slot(slot), adr(adr), portnum(portnum), ttl(ttl) {}
OutFileBound::OutFileBound(UsageEnvironment &env, FrameFifo &fifo, LiveOutboundContext &ctx) : env(env), fifo(fifo), ctx(ctx), setup_ok(false), at_setup(false) {}
OutFileBound::~OutFileBound() {}

void OutFileBound::reinit() {
    setup_ok =false;
    at_setup =false;
    // deallocate session and subsessions
}

void OutFileBound::handleFrame(Frame* f) {
    /* 
     * 
     * The session and subsession setup/reinit logic.
     * The idea is, that we first receive N setup frames.  One for each substream.
     * Once there are no more setup frames coming, we close up the setup and start accepting payload
     * 
     * at_setup  = doing setup
     * setup_ok  = did setup and got first payload frame
     * start with at_setup, setup_ok = false
     * 
     * if (setup frame):
     *  if (setup_ok): REINIT
     *    call reinit:
     *      at_setup=false
     *      setup_ok=false
     *      deallocate
     *    
     *  if (not at_setup): INIT
     *    starting setup again..  create session
     *  
     *  create next subsession (according to subsession_index)
     *  check if subsession_index has been used .. reinit if necessary
     *  at_setup=true
     *  
     * else:
     *  if (at_setup): CLOSE SETUP
     *    were doing setup, but a payload frame arrived.  Close setup
     *    prepare everything for payload frames
     *    setup_ok=true
     *    
     *  if (not setup_ok):
     *    do nothing
     *  else:
     *    write payload
     */
    
//    int subsession_index =f->subsession_index; // alias
//    
//    if ( subsession_index>=2) { // subsession_index too big
//        return;
//    }
//    
//    if (f->getFrameClass()==FrameClass::setup) { // SETUP FRAME
//        if (setup_ok) { // REINIT
//            reinit();
//        } // REINIT
//        
//        if (at_setup==false) { // INIT
//            // create Session
//            at_setup=true;
//        } // INIT
//        
//        // ** create here a Subsession into subsession_index
//        // ** check first that it's not already occupied..
//    }
//    else { // PAYLOAD FRAME
//        if (at_setup) { // CLOSE SETUP
//            // ** do whatever necessary to close up the setup
//            setup_ok=true; 
//        } // CLOSE SETUP
//        
//        if (setup_ok==false) {
//            // ** setup has not been started yet .. write an error message?
//        }
//        else {
//            // ** write payload
//        }
//    } // PAYLOAD FRAME
}



// VideoConnection::VideoConnection(UsageEnvironment& env, const std::string address, SlotNumber slot, FrameFilter& framefilter, long unsigned int msreconnect) : ConnectionFile(env, address, slot, framefilter, msreconnect), livestatus(LiveStatus::none) {};

VideoConnection::VideoConnection(UsageEnvironment& env, LiveConnectionContext& ctx) : ConnectionFile(env, ctx), livestatus(LiveStatus::none) {};


VideoConnection::~VideoConnection() {
    // delete client;
}

/* default copy constructor good enough ..
 * VideoConnection(const VideoConnection &cp) : env(cp.env), address(cp.address), slot(cp.slot), framefilter(cp.framefilter)  { 
 * }
 */


void VideoConnection::playStream() {
    if (is_playing) {
       // SDebug << "VideoConnection : playStream : stream already playing" ;
    }
    else {
        // Here we are a part of the live555 event loop (this is called from periodicTask => handleSignals => stopStream => this method)
        livestatus=LiveStatus::pending;
        frametimer=0;
        VideoClientState scs;
                
        SInfo<< "VideoConnection : playStream " << ctx.address;
        client = VideoFrameSink::createNew(env, scs , fragmp4_muxer, info, txt, "/var/tmp/test.264");
       // client = VideoFrameSink::createNew(env, scs , fragmp4_muxer, info, txt, "/var/tmp/videos/test1.264");

       
    }
    is_playing=true; // in the sense that we have requested a play .. and that the event handlers will try to restart the play infinitely..
}


void VideoConnection::stopStream() {
    // Medium* medium;
    // HashTable* htable;
    // Here we are a part of the live555 event loop (this is called from periodicTask => handleSignals => stopStream => this method)
    SInfo<< "VideoConnection : stopStream" ;
    if (is_playing) {
        // before the RTSPClient instance destroyed itself (!) it modified the value of livestatus
        if (livestatus==LiveStatus::closed) { // so, we need this to avoid calling Media::close on our RTSPClient instance
            SDebug << "VideoConnection : stopStream: already shut down" ;
        }
        else if (livestatus==LiveStatus::pending) { // the event-loop-callback system has not yet decided what to do with this stream ..
            SDebug << "VideoConnection : stopStream: pending" ;
            // we could do .. env.taskScheduler().unscheduleDelayedTask(...);
            // .. this callback chain exits by itself.  However, we'll get problems if we delete everything before that
            // .. this happens typically, when the DESCRIBE command has been set and we're waiting for the reply.
            // an easy solution: set the timeout (i.e. the interval we can send messages to the thread) larger than the time it takes wait for the describe response
            // but what if the user sends lots of stop commands to the signal queue ..?
            // TODO: add counter for pending events .. wait for pending events, etc .. ?
            // better idea: allow only one play/stop command per stream per handleSignals interval
            // possible to wait until handleSignals has been called
        }
        else {
            //VideoFrameSink::shutdownStream(client, 1); // sets LiveStatus to closed   // arvind write code for the same
            SDebug << "VideoConnection : stopStream: shut down" ;
        }
    }
    else {
        SDebug << "VideoConnection : stopStream : stream was not playing" ;
    }
    is_playing=false;
}


void VideoConnection::reStartStreamIf() {
    if (ctx.msreconnect<=0) { // don't attempt to reconnect
        return;
    }
    
    SDebug << "VideoConnection: status: " << int(livestatus) ;

    if (livestatus==LiveStatus::pending or livestatus==LiveStatus::closed) { // stream trying to connect .. waiting for tcp socket most likely
        SDebug << "VideoConnection: reStartStreamIf: pending" ;
        // frametimer=frametimer+Timeout::livethread;
        pendingtimer=pendingtimer+Timeout::livethread;
        if (pendingtimer >= ctx.msreconnect) {
            #ifdef LIVE_SIGNAL_FRAMES
            OfflineSignalContext signal_ctx = OfflineSignalContext();
            SignalFrame signalframe = SignalFrame();
            put_signal_context(&signalframe, signal_ctx, SignalType::offline);
            SDebug << "VideoConnection: restartStreamIf: pending: sending signal frame for slot " << ctx.slot ;
            ctx.framefilter->run(&signalframe);
            #endif
            pendingtimer=0;
        }
        return;
    }

    pendingtimer=0;
    
    if (livestatus==LiveStatus::alive) { // alive
        // std::cout << "VideoConnection: reStartStreamIf: alive" ;
        if (client->scs.gotFrame()) { // there has been frames .. all is well
            client->scs.clearFrame(); // reset the watch flag
            frametimer=0;
        }
        else {
            frametimer=frametimer+Timeout::livethread;
        }
    } // alive
    else if (livestatus==LiveStatus::closed) {
          SInfo << "VideoConnection: reStartStreamIf: closed" ;
          frametimer=frametimer+Timeout::livethread;
    }
    else {
       SError << "VideoConnection: restartStreamIf called without client";
        return;
    }
    
    #ifdef RECONNECT_VERBOSE
    std::cout << "VideoConnection: frametimer=" << frametimer ;
    #endif
    
    if (frametimer>=ctx.msreconnect) {
        // std::cout << "VideoConnection: reStartStreamIf: msreconnect" ;
         SInfo << "VideoConnection: restartStreamIf: restart at slot " << ctx.slot ;
        
        if (livestatus==LiveStatus::alive) {
            #ifdef LIVE_SIGNAL_FRAMES
            // inform downstream that this stream is offline
            OfflineSignalContext signal_ctx = OfflineSignalContext();
            SignalFrame signalframe = SignalFrame();
            put_signal_context(&signalframe, signal_ctx, SignalType::offline);
            SDebug << "VideoConnection: restartStreamIf: sending signal frame for slot " << ctx.slot ;
            ctx.framefilter->run(&signalframe);
            #endif

             SInfo << "VideoConnection: restartStreamIf: sending signal frame for slot " << ctx.slot ;
            stopStream();
        }
        if (livestatus==LiveStatus::closed) {
            is_playing=false; // just to get playStream running ..
            playStream();
        } // so, the stream might be left to the pending state
    }
}

bool VideoConnection::isClosed() { // not pending or playing
    return (livestatus==LiveStatus::closed or livestatus==LiveStatus::none); // either closed or not initialized at all
}

void VideoConnection::forceClose() {
   // MSRTSPClient::shutdownStream(client, 1);
}


//SDPConnection::SDPConnection(UsageEnvironment& env, const std::string address, SlotNumber slot, FrameFilter& framefilter) : ConnectionFile(env, address, slot, framefilter, 0) {};



FileOutbound::FileOutbound(UsageEnvironment& env, FrameFifo &fifo, LiveOutboundContext& ctx) : OutFileBound(env,fifo,ctx) {
//    streams.resize(2, NULL); // we'll be ready for two media streams
}

void FileOutbound::reinit() {
    setup_ok =false;
    at_setup =false;
    // deallocate session and subsessions
//    for (auto it=streams.begin(); it!=streams.end(); ++it) {
//        if (*it!=NULL) {
//            delete *it;
//            *it=NULL;
//        }
//    }
}

void FileOutbound::handleFrame(Frame *f) {
    
     SError << "should never be called "; 
     
//    int subsession_index =f->subsession_index; // alias
//    
//    if (subsession_index>=streams.size()) { // subsession_index too big
//       SError << "FileOutbound :"<<ctx.address<<" : handleFrame :  substream index overlow : "<<subsession_index<<"/"<<streams.size();
//        fifo.recycle(f); // return frame to the stack - never forget this!
//        return;
//    }
//    
//    if (f->getFrameClass()==FrameClass::setup) { // SETUP FRAME
//        SetupFrame* setupframe = static_cast<SetupFrame*>(f);
//        
//        if (setup_ok) { // REINIT
//            reinit();
//        } // REINIT
//        
//        if (at_setup==false) { // INIT
//            // create Session
//            at_setup=true;
//        } // INIT
//        
//        // ** create here a Subsession into subsession_index
//        // ** check first that it's not already occupied..
//        if (streams[subsession_index]!=NULL) {
//            SDebug << "FileOutbound:"<<ctx.address <<" : handleFrame : stream reinit at subsession " << subsession_index ;
//            delete streams[subsession_index];
//            streams[subsession_index]=NULL;
//        }
//        
//        SDebug << "FileOutbound:"<<ctx.address <<" : handleFrame : registering stream to subsession index " <<subsession_index;
//        switch (setupframe->codec_id) { // NEW_CODEC_DEV // when adding new codecs, make changes here: add relevant stream per codec
//            case AV_CODEC_ID_H264:
//                streams[subsession_index]=new H264Stream(env, fifo, ctx.address, ctx.portnum, ctx.ttl);
//                streams[subsession_index]->startPlaying();
//                break;
//            default:
//                //TODO: implement VoidStream
//                // streams[subsession_index]=new VoidStream(env, const char* adr, unsigned short int portnum, const unsigned char ttl=255);
//                break;
//        } // switch
//        
//        fifo.recycle(f); // return frame to the stack - never forget this!
//    } // SETUP FRAME
//    else { // PAYLOAD FRAME
//        if (at_setup) { // CLOSE SETUP
//            // ** do whatever necessary to close up the setup
//            setup_ok=true;
//        } // CLOSE SETUP
//        
//        if (setup_ok==false) {
//            // ** setup has not been started yet .. write an error message?
//            fifo.recycle(f); // return frame to the stack - never forget this!
//        }
//        else { // WRITE PAYLOAD
//            if (streams[subsession_index]==NULL) { // invalid subsession index
//                SInfo << "FileOutbound:"<<ctx.address <<" : handleFrame : no stream was registered for " << subsession_index ;
//                fifo.recycle(f); // return frame to the stack - never forget this!
//            }
//            else if (f->getFrameClass()==FrameClass::none) { // void frame, do nothing
//                fifo.recycle(f); // return frame to the stack - never forget this!
//            }
//            else { // send frame
//                streams[subsession_index]->handleFrame(f); // its up to the stream instance to call recycle
//            } // send frame
//        } // WRITE PAYLOAD
//    } // PAYLOAD FRAME
}


FileOutbound::~FileOutbound() {
    reinit();
}







FileThread::FileThread(const char* name, FrameFifoContext fifo_ctx) :  infifo(name, fifo_ctx), infilter(name, &infifo), exit_requested(false) {
    scheduler = BasicTaskScheduler::createNew();
    env       = BasicUsageEnvironment::createNew(*scheduler);
    eventLoopWatchVariable = 0;
    // this->slots_.resize(n_max_slots,NULL); // Reserve 256 slots!
    this->slots_.resize    (I_MAX_SLOTS+1,NULL);
    this->out_slots_.resize(I_MAX_SLOTS+1,NULL);
    
    scheduler->scheduleDelayedTask(Timeout::livethread*4000,(TaskFunc*)(FileThread::periodicTask),(void*)this); //arvind this migth be enabled
    
    // testing event triggers..
    event_trigger_id_hello_world   = env->taskScheduler().createEventTrigger(this->helloWorldEvent);
    event_trigger_id_frame_arrived = env->taskScheduler().createEventTrigger(this->frameArrivedEvent);
    event_trigger_id_got_frames    = env->taskScheduler().createEventTrigger(this->gotFramesEvent);
    
    infifo.setLiveThread((void*)this);
    fc=0;
}


FileThread::~FileThread() {
    unsigned short int i;
    ConnectionFile* connection;
    
    SInfo<< "FileThread: destructor: " ;
    
   // stopCall(); // stop if not stopped ..
    
    stop();
    
    for (std::vector<ConnectionFile*>::iterator it = slots_.begin(); it != slots_.end(); ++it) {
        connection=*it;
        if (!connection) {
        }
        else {
            SInfo<< "FileThread: destructor: connection ptr : "<< connection ;
            SInfo<< "FileThread: destructor: removing connection at slot " << connection->getSlot() ;
            delete connection;
        }
    }
    
   
    
    bool deleted =env->reclaim(); // ok.. this works.  I forgot to close the RTSPServer !
    SInfo<< "FileThread: deleted BasicUsageEnvironment?: " << deleted ;
    
//    if (!deleted) {
//        SInfo << "FileThread: WARNING: could not delete BasicUsageEnvironment" ;
//    }
    
    /* // can't do this .. the destructor is protected
     *  if (!deleted) { // die, you bastard!
     *    delete env;
}
*/
    delete scheduler; 
}
// 
void FileThread::preRun() {
    exit_requested=false;
}

void FileThread::postRun() {
}

void FileThread::sendSignal(LiveSignalContext signal_ctx) {
    std::unique_lock<std::mutex> lk(this->m);
    this->signal_fifo.push_back(signal_ctx);
}


void FileThread::checkAlive() {
    // std::cout << "FileThread: checkAlive" ;
    ConnectionFile *connection;
    for (std::vector<ConnectionFile*>::iterator it = slots_.begin(); it != slots_.end(); ++it) {
        connection=*it;
        if (connection!=NULL) {
            if (connection->is_playing) {
                connection->reStartStreamIf();
            }
        }
    }
}


void FileThread::handlePending() {
    ConnectionFile* connection;
    auto it=pending.begin();
    while (it!=pending.end()) {
        connection=*it;
        if (connection->is_playing) { // this has been scheduled for termination, without calling stop stream
            connection->stopStream();
        }
        if (connection->isClosed()) {
            SInfo<< "FileThread: handlePending: deleting a closed stream at slot " << connection->getSlot() ;
            it=pending.erase(it);
            delete connection;
        }
        else {
            it++;
        }
    }
}


void FileThread::closePending() { // call only after handlePending
    ConnectionFile* connection;
    for (auto it=pending.begin(); it!=pending.end(); ++it) {
        connection=*it;
        connection->forceClose();
        delete connection;
    }
}


void FileThread::handleSignals() {
    std::unique_lock<std::mutex> lk(this->m);
    unsigned short int i;
    LiveConnectionContext connection_ctx;
    LiveOutboundContext   out_ctx;
    
    
    // if (signal_fifo.empty()) {return;}
    
    // handle pending signals from the signals fifo
    for (auto it = signal_fifo.begin(); it != signal_fifo.end(); ++it) { // it == pointer to the actual object (struct LiveSignalContext)
        
        switch (it->signal) {
            case LiveSignal::exit:
                
                for(i=0;i<=I_MAX_SLOTS;i++) { // stop and deregister all streams
                    connection_ctx.slot=i;
                    deregisterStream(connection_ctx);
                }
                
                for(i=0;i<=I_MAX_SLOTS;i++) { // stop and deregister all streams
                    out_ctx.slot=i;
                    deregisterOutbound(out_ctx);
                }
                
                 this->eventLoopWatchVariable=1;
                exit_requested=true;
                break;
                // inbound streams
            case LiveSignal::register_stream:
                this->registerStream(*(it->connection_context));
                break;
            case LiveSignal::deregister_stream:
                this->deregisterStream(*(it->connection_context));
                break;
            case LiveSignal::play_stream:
                this->playStream(*(it->connection_context));
                break;
            case LiveSignal::stop_stream:
                this->stopStream(*(it->connection_context));
                break;
                // outbound streams
            case LiveSignal::register_outbound:
                this->registerOutbound(*(it->outbound_context));
                break;
            case LiveSignal::deregister_outbound:
                // std::cout << "FileThread : handleSignals : deregister_outbound" ;
                this->deregisterOutbound(*(it->outbound_context));
                break;
            default:
                std::cout << "FileThread : handleSignals : unknown signal " << int(it->signal) ;
                break;
        }
    }
    
    signal_fifo.clear();
}


void FileThread::handleFrame(Frame *f) { // handle an incoming frame ..
    int i;
    int subsession_index;
    OutFileBound* outbound;
//    Stream* stream;
    
    if (safeGetOutboundSlot(f->n_slot,outbound)>0) { // got frame
        #ifdef STREAM_SEND_DEBUG
        std::cout << "FileThread : "<< this->name <<" : handleFrame : accept frame "<<*f ;
        #endif
        outbound->handleFrame(f); // recycling handled deeper in the code
    } 
    else {
        #ifdef STREAM_SEND_DEBUG
        std::cout << "FileThread : "<< this->name <<" : handleFrame : discard frame "<<*f ;
        #endif
        infifo.recycle(f);
    }
}

void FileThread::stop(bool flag)
{
    eventLoopWatchVariable = 1;

    Thread::stop(flag);
}

void FileThread::run() {
    SInfo << " run : live555 loop start " ;
    env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
    SInfo << " run : live555 loop exit " ;
}


/*
 * void FileThread::resetConnectionContext_() {
 * this->connection_ctx.connection_type=FileThread::LiveConnectionType::none;
 * this->connection_ctx.address        =std::string();
 * this->connection_ctx.slot           =0;
 * }
 */


int FileThread::safeGetSlot(SlotNumber slot, ConnectionFile*& con) { // -1 = out of range, 0 = free, 1 = reserved // &* = modify pointer in-place
    ConnectionFile* connection;
    SInfo<< "FileThread: safeGetSlot" ;
    
    if (slot>I_MAX_SLOTS) {
       SError << "FileThread: safeGetSlot: WARNING! Slot number overfow : increase I_MAX_SLOTS in sizes.h" ;
        return -1;
    }
    
    try {
        connection=this->slots_[slot];
    }
    catch (std::out_of_range) {
        SDebug << "FileThread: safeGetSlot : slot " << slot << " is out of range! " ;
        con=NULL;
        return -1;
    }
    if (!connection) {
        SInfo<< "FileThread: safeGetSlot : nothing at slot " << slot ;
        con=NULL;
        return 0;
    }
    else {
        SDebug << "FileThread: safeGetSlot : returning " << slot ;
        con=connection;
        return 1;
    }
}


int FileThread::safeGetOutboundSlot(SlotNumber slot, OutFileBound*& outbound) { // -1 = out of range, 0 = free, 1 = reserved // &* = modify pointer in-place
    OutFileBound* out_;
    SInfo<< "FileThread: safeGetOutboundSlot" ;
    
    if (slot>I_MAX_SLOTS) {
       SError << "FileThread: safeGetOutboundSlot: WARNING! Slot number overfow : increase I_MAX_SLOTS in sizes.h" ;
        return -1;
    }
    
    try {
        out_=this->out_slots_[slot];
    }
    catch (std::out_of_range) {
        SDebug << "FileThread: safeGetOutboundSlot : slot " << slot << " is out of range! " ;
        outbound=NULL;
        return -1;
    }
    if (!out_) {
        SDebug << "FileThread: safeGetOutboundSlot : nothing at slot " << slot ;
        outbound=NULL;
        return 0;
    }
    else {
        SDebug << "FileThread: safeGetOutboundSlot : returning " << slot ;
        outbound=out_;
        return 1;
    }
}


void FileThread::registerStream(LiveConnectionContext &connection_ctx) {
    // semantics:
    // register   : create RTSP/SDPConnection object into the slots_ vector
    // play       : create RTSPClient object in the ConnectionFile object .. start the callback chain describe => play, etc.
    // stop       : start shutting down by calling shutDownStream .. destruct the RTSPClient object
    // deregister : stop (if playing), and destruct RTSP/SDPConnection object from the slots_ vector
    ConnectionFile* connection;
    SInfo<< "FileThread: registerStream" ;
    switch (safeGetSlot(connection_ctx.slot,connection)) {
        case -1: // out of range
            break;
            
        case 0: // slot is free
            switch (connection_ctx.connection_type) {
                
                case LiveConnectionType::rtsp:
                    // this->slots_[connection_ctx.slot] = new VideoConnection(*(this->env), connection_ctx.address, connection_ctx.slot, *(connection_ctx.framefilter), connection_ctx.msreconnect);
                    this->slots_[connection_ctx.slot] = new VideoConnection(*(this->env), connection_ctx);
                    SDebug << "FileThread: registerStream : rtsp stream registered at slot " << connection_ctx.slot << " with ptr " << this->slots_[connection_ctx.slot] ;
                    // this->slots_[connection_ctx.slot]->playStream(); // not here ..
                    break;
                    
                case LiveConnectionType::sdp:
                    // this->slots_[connection_ctx.slot] = new SDPConnection(*(this->env), connection_ctx.address, connection_ctx.slot, *(connection_ctx.framefilter));
                    this->slots_[connection_ctx.slot] = new VideoConnection(*(this->env), connection_ctx);
                    SDebug << "FileThread: registerStream : sdp stream registered at slot "  << connection_ctx.slot << " with ptr " << this->slots_[connection_ctx.slot] ;
                    // this->slots_[connection_ctx.slot]->playStream(); // not here ..
                    break;
                    
                default:
                    SInfo << "FileThread: registerStream : no such LiveConnectionType" ;
                    break;
            } // switch connection_ctx.connection_type
            
            break;
            
                case 1: // slot is reserved
                    SInfo << "FileThread: registerStream : slot " << connection_ctx.slot << " is reserved! " ;
                    break;
    } // safeGetSlot(connection_ctx.slot,connection)
    
}


void FileThread::deregisterStream(LiveConnectionContext &connection_ctx) {
    ConnectionFile* connection;
    SInfo<< "FileThread: deregisterStream" ;
    switch (safeGetSlot(connection_ctx.slot,connection)) {
        case -1: // out of range
            break;
        case 0: // slot is free
            SInfo<< "FileThread: deregisterStream : nothing at slot " << connection_ctx.slot ;
            break;
        case 1: // slot is reserved
            SInfo << "FileThread: deregisterStream : de-registering " << connection_ctx.slot ;
            if (connection->is_playing) {
                connection->stopStream();
            }
            if (!connection->isClosed()) { // didn't close correctly .. queue for stopping
                SDebug << "FileThread: deregisterStream : queing for stopping: " << connection_ctx.slot ;
                pending.push_back(connection);
            }
            else {
                delete connection;
            }
            this->slots_[connection_ctx.slot]=NULL; // case 1
            break;
    } // switch
}


void FileThread::playStream(LiveConnectionContext &connection_ctx) {
    ConnectionFile* connection;
    SInfo<< "FileThread: playStream" ;  
    switch (safeGetSlot(connection_ctx.slot,connection)) {
        case -1: // out of range
            break;
        case 0: // slot is free
            SInfo << "FileThread: playStream : nothing at slot " << connection_ctx.slot ;
            break;
        case 1: // slot is reserved
            SInfo << "FileThread: playStream : playing.. " << connection_ctx.slot ;
            connection->playStream();
            break;
    }
}


void FileThread::stopStream(LiveConnectionContext &connection_ctx) {
    ConnectionFile* connection;
    SInfo<< "FileThread: stopStream" ;
    switch (safeGetSlot(connection_ctx.slot,connection)) {
        case -1: // out of range
            break;
        case 0: // slot is free
            SInfo << "FileThread: stopStream : nothing at slot " << connection_ctx.slot ;
            break;
        case 1: // slot is reserved
            SDebug << "FileThread: stopStream : stopping.. " << connection_ctx.slot ;
            connection->stopStream();
            break;
    }
}


void FileThread::registerOutbound(LiveOutboundContext &outbound_ctx) {
    OutFileBound* outbound;
    switch (safeGetOutboundSlot(outbound_ctx.slot,outbound)) {
        case -1: // out of range
            break;
        case 0: // slot is free
            switch (outbound_ctx.connection_type) {
                
                case LiveConnectionType::sdp:
                    // this->out_slots_[outbound_ctx.slot] = new FileOutbound(*env, infifo, outbound_ctx.slot, outbound_ctx.address, outbound_ctx.portnum, outbound_ctx.ttl);
                    this->out_slots_[outbound_ctx.slot] = new FileOutbound(*env, infifo, outbound_ctx);
                    SDebug << "FileThread: "<<" registerOutbound : sdp stream registered at slot "  << outbound_ctx.slot << " with ptr " << this->out_slots_[outbound_ctx.slot] ;
                    //std::cout << "FileThread : registerOutbound : " << this->out_slots_[2] ;
                    break;
                    
                case LiveConnectionType::rtsp:
                    if (1) {
                       SError << "FileThread: registerOutbound: no RTSP server initialized" ;
                    }
                    else {
                        //this->out_slots_[outbound_ctx.slot] = new RTSPOutbound(*env, *server, infifo, outbound_ctx);
                        SDebug << "FileThread: "<<" registerOutbound : rtsp stream registered at slot "  << outbound_ctx.slot << " with ptr " << this->out_slots_[outbound_ctx.slot] ;
                    }
                    break;
                    
                default:
                    SInfo << "FileThread: "<<" registerOutbound : no such LiveConnectionType" ;
                    break;
            } // switch outbound_ctx.connection_type
            break;
            
                case 1: // slot is reserved
                    SInfo << "FileThread: "<<" registerOutbound : slot " << outbound_ctx.slot << " is reserved! " ;
                    break;
    }
}


void FileThread::deregisterOutbound(LiveOutboundContext &outbound_ctx) {
    OutFileBound* outbound;
    //std::cout << "FileThread : deregisterOutbound" ;
    //std::cout << "FileThread : deregisterOutbound : " << this->out_slots_[2] ;
    switch (safeGetOutboundSlot(outbound_ctx.slot,outbound)) {
        case -1: // out of range
            break;
        case 0: // slot is free
            SInfo<< "FileThread: deregisterOutbound : nothing at slot " << outbound_ctx.slot ;
            break;
        case 1: // slot is reserved
            SDebug << "FileThread: deregisterOutbound : de-registering " << outbound_ctx.slot ;
            // TODO: what else?
            delete outbound;
            this->out_slots_[outbound_ctx.slot]=NULL;
            break;
    }
}


void FileThread::periodicTask(void* cdata) {
    FileThread* livethread = (FileThread*)cdata;
    SDebug<< "FileThread: periodicTask" ;
    livethread->handlePending(); // remove connections that were pending closing, but are ok now
    // std::cout << "FileThread: periodicTask: pending streams " << livethread->pending.size() ;
    // stopCall => handleSignals => loop over deregisterStream => stopStream
    // if isClosed, then delete the connection, otherwise put into the pending list
    
    if (livethread->pending.empty() and livethread->exit_requested) {
        SInfo<< "FileThread: periodicTask: exit: nothing pending" ;
        livethread->eventLoopWatchVariable=1;
    }
    else if (livethread->exit_requested) { // tried really hard to close everything in a clean way .. but sockets etc. might still be hanging 
        SInfo<< "FileThread: periodicTask: exit: closePending" ;
        livethread->closePending(); // eh.. we really hope the eventloop just exits and does nothing else: some MSRTSPClient pointers have been nulled and these might be used in the callbacks
        livethread->eventLoopWatchVariable=1; 
    }
    
    if (!livethread->exit_requested) {
        livethread->checkAlive();
        livethread->handleSignals(); // WARNING: sending commands to live555 must be done within the event loop
        livethread->scheduler->scheduleDelayedTask(Timeout::livethread*4000,(TaskFunc*)(FileThread::periodicTask),(void*)livethread); // re-schedule itself
    }
}



// *** API ***

void FileThread::registerStreamCall(LiveConnectionContext &connection_ctx) {
    LiveSignalContext signal_ctx = {LiveSignal::register_stream, &connection_ctx, NULL};
    sendSignal(signal_ctx);
}

void FileThread::deregisterStreamCall(LiveConnectionContext &connection_ctx) {
    LiveSignalContext signal_ctx = {LiveSignal::deregister_stream, &connection_ctx, NULL};
    sendSignal(signal_ctx);
}


void FileThread::playStreamCall(LiveConnectionContext &connection_ctx) {
    LiveSignalContext signal_ctx = {LiveSignal::play_stream, &connection_ctx, NULL};
    sendSignal(signal_ctx);
    
    //registerOutbound( );
    
    // playStream(connection_ctx);
    
}

void FileThread::stopStreamCall(LiveConnectionContext &connection_ctx) {
    LiveSignalContext signal_ctx = {LiveSignal::stop_stream, &connection_ctx, NULL};
    sendSignal(signal_ctx);
}


void FileThread::registerOutboundCall(LiveOutboundContext &outbound_ctx) {
    LiveSignalContext signal_ctx = {LiveSignal::register_outbound, NULL, &outbound_ctx};
    sendSignal(signal_ctx);
}


void FileThread::deregisterOutboundCall(LiveOutboundContext &outbound_ctx) {
    // std::cout << "FileThread : deregisterOutboundCall" ;
    LiveSignalContext signal_ctx = {LiveSignal::deregister_outbound, NULL, &outbound_ctx};
    sendSignal(signal_ctx);
}


void FileThread::requestStopCall() {
//    threadlogger.log(LogLevel::crazy) << "FileThread: requestStopCall: "<< this->name <<std::endl;
//    if (!this->has_thread) { return; } // thread never started
 //   if (stop_requested) { return; } // can be requested only once
//    stop_requested = true;
    
    LiveSignalContext signal_ctx;
    signal_ctx.signal=LiveSignal::exit;
    
   // threadlogger.log(LogLevel::crazy) << "FileThread: sending exit signal "<< this->name <<std::endl;
    this->sendSignal(signal_ctx);
}



/*
 * FileFifo &FileThread::getFifo() {
 *  return infifo;
 * }
 */


FifoFrameFilter &FileThread::getFrameFilter() {
    return infilter;
}


void FileThread::setRTSPServer(int portnum) {

       
        
}


void FileThread::helloWorldEvent(void* clientData) {
    // this is the event identified by event_trigger_id_hello_world
    std::cout << "Hello world from a triggered event!" ;
}


void FileThread::frameArrivedEvent(void* clientData) {
    Frame* f;
    FileThread *thread = (FileThread*)clientData;
    // this is the event identified by event_trigger_id_frame
    // std::cout << "FileThread : frameArrived : New frame has arrived!" ;
    f=thread->infifo.read(1); // this should not block..
    thread->fc+=1;
    std::cout << "FileThread: frameArrived: frame count=" << thread->fc << " : " << *f ;
    // std::cout << "FileThread : frameArrived : frame :" << *f ;
    thread->infifo.recycle(f);
}


void FileThread::gotFramesEvent(void* clientData) { // registers a periodic task to the event loop
    #ifdef STREAM_SEND_DEBUG
    std::cout << "FileThread: gotFramesEvent " ;
    #endif
    FileThread *thread = (FileThread*)clientData;
    thread->scheduler->scheduleDelayedTask(0,(TaskFunc*)(FileThread::readFrameFifoTask),(void*)thread); 
}


void FileThread::readFrameFifoTask(void* clientData) {
    Frame* f;
    FileThread *thread = (FileThread*)clientData;
    #ifdef STREAM_SEND_DEBUG
    std::cout << "FileThread: readFrameFifoTask: read" ;
    thread->infifo.diagnosis();
    #endif
    if (thread->infifo.isEmpty()) { // this task has been scheduled too many times .. nothing yet to read from the fifo
        std::cout << "FileThread: readFrameFifoTask: underflow" ;
        return; 
    }
    f=thread->infifo.read(); // this blocks
    thread->fc+=1;
    #ifdef STREAM_SEND_DEBUG
    std::cout << "FileThread: readFrameFifoTask: frame count=" << thread->fc << " : " << *f ;
    #endif
    
    thread->handleFrame(f);
    // thread->infifo.recycle(f); // recycling is handled deeper in the code
    
    ///*
    if (thread->infifo.isEmpty()) { // no more frames for now ..
    }
    else {
        thread->scheduler->scheduleDelayedTask(0,(TaskFunc*)(FileThread::readFrameFifoTask),(void*)thread); // re-registers itself
    }
    //*/
    
}


void FileThread::testTrigger() {
    // http://live-devel.live555.narkive.com/MSFiseCu/problem-with-triggerevent
    scheduler->triggerEvent(event_trigger_id_hello_world,(void*)(NULL));
}


void FileThread::triggerGotFrames() {
    #ifdef STREAM_SEND_DEBUG
    std::cout << "FileThread: triggerGotFrames" ;
    #endif
    // scheduler->triggerEvent(event_trigger_id_frame_arrived,(void*)(this));
    scheduler->triggerEvent(event_trigger_id_got_frames,(void*)(this)); 
}


}
}
