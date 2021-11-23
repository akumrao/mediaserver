 
#include "fileH264.h"
#include "constant.h"
#include "tools.h"
#include "base/logger.h"
#include <thread>

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

// Implementation of "VideoClientState":

VideoClientState::VideoClientState() : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), pingGetParameterTask(NULL), duration(0.0), subsession_index(-1), frame_flag(false) {
}


void VideoClientState::close() {
  MediaSubsessionIterator iter2(*session);
  while ((subsession = iter2.next()) != NULL) {
    if (subsession->sink != NULL) {
     SInfo << "VideoClientState : closing subsession" <<std::endl;
      Medium::close(subsession->sink);
     SInfo << "VideoClientState : closed subsession" <<std::endl;
      subsession->sink = NULL;
    }
  }
}


VideoClientState::~VideoClientState() {
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


// Implementation of "VideoFrameSink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 500000

/*
VideoFrameSink* VideoFrameSink::createNew(UsageEnvironment& env, MediaSubsession& subsession, FrameFilter& framefilter, int subsession_index, char const* streamId) {
  return new VideoFrameSink(env, subsession, framefilter, subsession_index, streamId);
}

VideoFrameSink::VideoFrameSink(UsageEnvironment& env, MediaSubsession& subsession, FrameFilter& framefilter, int subsession_index, char const* streamId) : MediaSink(env), fSubsession(subsession), framefilter(framefilter), subsession_index(subsession_index), on(true), nbuf(0) 
*/

VideoFrameSink* VideoFrameSink::createNew(UsageEnvironment& env, VideoClientState& scs, FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, char const* streamId) {
   
    
    return new VideoFrameSink(env, scs, fragmp4_muxer,info, txt, streamId);
}

void afterPlaying(void* clientData) {
    
   VideoFrameSink* obj =  (VideoFrameSink*) clientData;
   
   obj->stopPlaying();
    
//  *env << "...done reading from file\n";
//  videoSink->stopPlaying();
    Medium::close(obj->videoSource);
  // Note that this also closes the input file that this source read from.

  // Start playing once again:
    obj->Play();
}

void VideoFrameSink::Play()
{
     ByteStreamFileSource* fileSource  = ByteStreamFileSource::createNew(env, fStreamId);
     
   /*  
    std::string tmp =  "/var/tmp/videos/test1.264";
   
    
    
    std::string tmp1 =  "/var/tmp/test.264";
     
    
    static int nCount = 0;
   
    if( nCount++ % 2 == 0 )
    fStreamId = strDup((char*)tmp.c_str());
    else
    fStreamId = strDup((char*)tmp1.c_str());    
   */ 
     
    
    
    if (fileSource == NULL) {
    env << "Unable to open file \"" << fStreamId
         << "\" as a byte-stream file source\n";
     exit(1);
    }

    FramedSource* videoES = fileSource;

    // Create a framer for the Video Elementary Stream:
    videoSource = H264VideoStreamFramer::createNew(env, videoES);

    // Finally, start playing:
    env << "Beginning to read from file...\n";
    
    this->startPlaying(*videoSource, afterPlaying, this);
    
    
    const char* codec_name= "H264";

    SInfo << "VideoFrameSink: constructor: codec_name =" << codec_name << ", subsession_index =" << subsession_index << std::endl;

    foundsps = false;
    foundpps = false;        
    // https://ffmpeg.org/doxygen/3.0/avcodec_8h_source.html
    if (strcmp(codec_name, "H264") == 0) { // NEW_CODEC_DEV // when adding new codecs, make changes here

        fragmp4_muxer->deActivate();

        // WARNING: force subsession index to 0
        subsession_index = 0;

        SDebug << "VideoFrameSink: init H264 Frame" << std::endl;
        // prepare payload frame
        basicframe.media_type = AVMEDIA_TYPE_VIDEO;
        basicframe.codec_id = AV_CODEC_ID_H264;
        basicframe.stream_index = subsession_index;
        // prepare setup frame
        setupframe.sub_type = SetupFrameType::stream_init;
        setupframe.media_type = AVMEDIA_TYPE_VIDEO;
        setupframe.codec_id = AV_CODEC_ID_H264; // what frame types are to be expected from this stream
        setupframe.stream_index = subsession_index;
        setupframe.mstimestamp = CurrentTime_milliseconds();
        // send setup frame
        //info->run(&setupframe);
        fragmp4_muxer->run(&setupframe);
        //setReceiveBuffer(DEFAULT_PAYLOAD_SIZE_H264); // sets nbuf
    } else {
        SInfo << "VideoFrameSink: constructor: codec_name =" << codec_name << " not supported yet" << std::endl;
        return; // no return here!  You won't do sendParameteSets() ..!
    }
            
    
    
    
    
    
    
}

VideoFrameSink::VideoFrameSink(UsageEnvironment& env, VideoClientState& scs,  FrameFilter* fragmp4_muxer, FrameFilter *info, FrameFilter *txt, char const* streamId) : env(env), MediaSink(env), scs(scs), fragmp4_muxer(fragmp4_muxer),info(info),txt(txt), on(true), fSubsession(*(scs.subsession))
{
    
    fStreamId = strDup(streamId);
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
      
   
    Play();
    
  // some aliases:
  // MediaSubsession &subsession = *(scs.subsession);
  int subsession_index        = scs.subsession_index;
  
 
 
  
  
  // some beautiful day enable audio.  At the moment, it messes up some things (for example, re-transmitting streams, etc.)
  
  
  /*
    else if (strcmp(codec_name,"PCMU")==0) {
   SDebug << "VideoFrameSink: init PCMU Frame"<<std::endl;
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
   SDebug << "VideoFrameSink: WARNING: unknown codec "<<codec_name<<std::endl;
    basicframe.media_type           =AVMEDIA_TYPE_UNKNOWN;
    basicframe.codec_id             =AV_CODEC_ID_NONE;
    basicframe.subsession_index     =subsession_index;
    setReceiveBuffer(DEFAULT_PAYLOAD_SIZE); // sets nbuf
  }
  */
  
#ifdef SEND_PARAMETER_SETS
///  sendParameterSets();
#endif
  
 SDebug << "VideoFrameSink: constructor: internal_frame= "<< basicframe <<std::endl;
}

VideoFrameSink::~VideoFrameSink() {
  SInfo << "VideoFrameSink: destructor :"<<std::endl;
   delete[] fReceiveBuffer;
  delete[] fStreamId;
  SInfo << "VideoFrameSink: destructor : bye!"<<std::endl;
}

void VideoFrameSink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds) {
  VideoFrameSink* sink = (VideoFrameSink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
// #define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void VideoFrameSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
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
       // fragmp4_muxer->resetParser = false;
   }

   if (basicframe.h264_pars.slice_type == H264SliceType::sps ||  basicframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
   {
                       
       unsigned num_units_in_tick, time_scale;

     
        //  analyze_seq_parameter_set_data(buffer,sz, num_units_in_tick, time_scale);
         
       
       if( !foundsps &&  basicframe.h264_pars.slice_type == H264SliceType::sps  )
       {    
           
           obj.analyze_seq_parameter_set_data(fReceiveBuffer , frameSize, num_units_in_tick, time_scale);
       
           fps = obj.fps ;
           height = obj.height;        
           width = obj.width;        
                   
           
           
           basicframe.fps = obj.fps ;
           basicframe.height = obj.height;        
           basicframe.width = obj.width;
           
           SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;
                   
           info->run(&basicframe);
           fragmp4_muxer->run(&basicframe); // starts the frame filter chain
           basicframe.payload.resize(basicframe.payload.capacity());
           
           foundsps  = true;
           
       }
       
       if( !foundpps &&  basicframe.h264_pars.slice_type == H264SliceType::pps  )
       {    
          
           
            SInfo <<  " Got PPS fps ";
            
            info->run(&basicframe);
            fragmp4_muxer->run(&basicframe); // starts the frame filter chain
            basicframe.payload.resize(basicframe.payload.capacity());
            
             foundpps  = true;
           
       }
               
      
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
//   SDebug << "VideoFrameSink : growing reserved size to "<< target_size << " bytes" ;
//    setReceiveBuffer(target_size);
//  }
  
   // recovers maximum size .. must set maximum size before letting live555 to write into the memory area
  
  // Then continue, to request the next frame of data:
  if (on) {continuePlaying();}
}


//void VideoFrameSink::afterGettingHeader(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
//  // We've just received a frame of data.  (Optionally) print out information about it:
//#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
//  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
//  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
//  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
//  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
//  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
//  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
//  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
//    envir() << " !"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
//  }
//#ifdef DEBUG_PRINT_NPT
//  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
//#endif
//  envir() << "\n";
//#endif
//  
//  
//   basicframe.copyBuf(fReceiveBuffer, frameSize );
//  // mstimestamp=presentationTime.tv_sec*1000+presentationTime.tv_usec/1000;
//  // std::cout << "afterGettingFrame: mstimestamp=" << mstimestamp <<std::endl;
//  basicframe.mstimestamp=(presentationTime.tv_sec*1000+presentationTime.tv_usec/1000);
//  basicframe.fillPars();
//  // std::cout << "afterGettingFrame: " << basicframe ;
//  
// // basicframe.payload.resize(checkBufferSize(frameSize)); // set correct frame size .. now information about the packet length goes into the filter chain
//  
//                    
//  
//  scs.setFrame(); // flag that indicates that we got a frame
//  
//  // std::cerr << "BufferSource: IN0: " << basicframe ;
//  info->run(&basicframe);
//  fragmp4_muxer->run(&basicframe); // starts the frame filter chain
//  
////  if (numTruncatedBytes>0) {// time to grow the buffer..
////   SDebug << "VideoFrameSink : growing reserved size to "<< target_size << " bytes" ;
////    setReceiveBuffer(target_size);
////  }
//  
//  basicframe.payload.resize(basicframe.payload.capacity()); // recovers maximum size .. must set maximum size before letting live555 to write into the memory area
//  
//  // Then continue, to request the next frame of data:
//  if (on) {continuePlaying();}
//}

Boolean VideoFrameSink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)
  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  // fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
  uint64_t currentTime =  CurrentTime_microseconds();
  
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
  
  if(fps)
  {
  uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
    //std::this_thread::sleep_for(std::chrono::microseconds(300000 - deltaTimeMillis));
  std::this_thread::sleep_for(std::chrono::microseconds((1000000 / fps) - deltaTimeMillis));
  }
  
  return True;
}


unsigned VideoFrameSink::checkBufferSize(unsigned target_size) {// add something to the target_size, if needed (for H264, the nalstamp)
   if (basicframe.codec_id==AV_CODEC_ID_H264) {
     target_size+=nalstamp.size();
   }
   // target_size+=8; // receive extra mem to avoid ffmpeg decoder over-read // nopes!  This can screw up the decoder completely!
   return target_size;
}


//void VideoFrameSink::setReceiveBuffer(unsigned target_size) {
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


//void VideoFrameSink::sendParameterSets() {
//  SPropRecord* pars;
//  unsigned i,num;
//  struct timeval frametime;
//  gettimeofday(&frametime, NULL);
//  
//  SInfo << "Sending parameter sets!\n";
//  pars=parseSPropParameterSets(fSubsession.fmtp_spropparametersets(),num);
//  SInfo << "Found " << num << " parameter sets\n";
//  for(i=0;i<num;i++) {
//    if (pars[i].sPropLength>0) {
//      SInfo << "Sending parameter set " << i << " " << pars[i].sPropLength ;
//      memcpy(fReceiveBuffer, pars[i].sPropBytes, pars[i].sPropLength);
//      
//      afterGettingHeader(pars[i].sPropLength, 0, frametime, 0);
//    }
//  }
//  delete[] pars;
//}



}
}
