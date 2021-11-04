#ifndef muxer_HEADER_GUARD
#define muxer_HEADER_GUARD

#include "muxframe.h"
//#include "constant.h"
#include "framefilter.h"
#include <condition_variable>
#include <thread>
#include <chrono>
 

#define STREAM_FRAME_RATE 25 
#define SAMPLINGRATE 44100
#define AUDIOSAMPLE 1024


namespace base {
namespace fmp4 {
    

class MuxFrameFilter:  public FrameFilter {      
    
public:                                                              //
    MuxFrameFilter(const char* name, FrameFilter *next = NULL);      //
    virtual ~MuxFrameFilter();                                       //
    
protected:
   // bool active {false};                       ///< Writing to muxer has been requested
    bool has_extraVideodata{false};                ///< Got "extradata" (sps & pps)
    bool has_extraAudiodata{false};                ///< Got AAC audio config spec 
    int extradata_count;               ///< Check that we have the sps => pps sequence
    //bool ready;                        ///< Got enough setup frames & extradata
    /** After ready & active, initMux is called 
     * (set streams, codec ctx, etc. & initialized is set */
    bool initialized;                  
   // long int mstimestamp0{0};             ///< Time of activation (i.e. when the recording started)
   // long int zerotime{false};                 ///< Start time set explicitly by the user
   
    bool zerotimeset;
    bool testflag;
    // bool sps_ok, pps_ok;

    std::string format_name;
    
public: // so that child static methods can access..
    uint32_t missing, ccf;
    
protected: //libav stuff
    AVFormatContext               *av_format_ctx;
    AVIOContext                   *avio_ctx;
    uint8_t                       *avio_ctx_buffer;
//    AVRational                    timebase;
    std::vector<AVCodecContext*>  codec_contexes;
    std::vector<int64_t>  prevpts;
    
    std::vector<AVStream*>        streams;
    AVFormatContext               *av_format_context;
    AVPacket                      *avpkt;
    AVDictionary                  *av_dict;
    //AVBufferRef                   *avbuffer;

    static const size_t avio_ctx_buffer_size = 4096;
  
protected: //mutex stuff
    std::mutex              mutex;     ///< Mutex protecting the "active" boolean
    std::condition_variable condition; ///< Condition variable for the mutex

protected: //frames
    std::vector<SetupFrame>     setupframes;        ///< deep copies of the arrived setup frames
    
public:
    //BasicFrame                  internal_basicframe; ///< 
    MuxFrame                    internal_frame;      ///< outgoing muxed frame
    BasicFrame                  extradata_videoframe;     ///< capture decoder extradata here
    BasicFrame                  extradata_audioframe;     ///< capture decoder extradata here
    virtual void run(Frame* frame);
protected:
    virtual void defineMux() = 0; ///< Define container format (format_name) & muxing parameters (av_dict).  Define in child classes.
    virtual void go(Frame* frame);
    
    void initMux();           ///< Open file, reserve codec_contexes, streams, write preamble, set initialized=true if success
    void closeMux();          ///< Close file, dealloc codec_contexes, streams
    void writeFrame(BasicFrame* frame);
  
public: // API calls                                                                           
    // setFileName(const char* fname); ///< Sets the output filename                           
//    void activate(long int zerotime=0);       ///< Request streaming to asap (when config frames have arrived) 
     void deActivate();                                           ///< Stop streaming           
//    
// 
    
protected:
    static int write_packet(void *opaque, uint8_t *buf, int buf_size); // define separately in child classes
    static int read_packet(void *opaque, uint8_t *buf, int buf_size) {return 0;} // dummy function
    static int64_t seek(void *opaque, int64_t offset, int whence) {return 0;} // dummy function
};                                                                                             


class FragMP4MuxFrameFilter : public MuxFrameFilter {                       
    
public:                                                                     
    FragMP4MuxFrameFilter(const char* name, FrameFilter *next = NULL);      
    virtual ~FragMP4MuxFrameFilter();                                       

public:
    bool                        got_ftyp, got_moov;
    MuxFrame                    ftyp_frame, moov_frame;

protected:
    virtual void defineMux();

public: // API calls    
    void sendMeta();
   
protected:
    static int write_packet(void *opaque, uint8_t *buf, int buf_size_);
    static int read_packet(void *opaque, uint8_t *buf, int buf_size) {return 0;} // {std::cout << "muxer: dummy read packet" << std::endl; return 0;} // dummy function
    static int64_t seek(void *opaque, int64_t offset, int whence) {return 0;}// {std::cout << "muxer: dummy seek" << std::endl; return 0;} // dummy function
};                                                                           


// helper functions for minimal MP4 parsing
// the right way to do this:
// class: MP4Box with subclasses like moov, moof, etc.
// all boxes have pointers to the point in memory where they start
// MP4Box has a child-parent structure, can iterate over children, etc.


/** Gives length and name of an MP4 box
 */
void getLenName(uint8_t* data, uint32_t& len, char* name);

/** Gives index of a certain MP4 sub-box, as identified by the box type's name
 */
uint32_t getSubBoxIndex(uint8_t* data, const char name[4]);

/* Example usage:

char name[4];
getLenName(payload.data(), len, &name[0])

if strcmp(name, "moof") == 0 {
    i_traf = getSubBoxIndex(payload.data(), "traf")
    i_trun = getSubBoxIndex(payload.data() + i, "trun")
    getLenName(payload.data() + i_trun, len, name) # this shoud be "trun"
    # now inspect the payload.data() + i_trun
}

*/

bool moofHasFirstSampleFlag(uint8_t* data);
    }
}
#endif
