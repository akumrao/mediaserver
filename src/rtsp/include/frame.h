#ifndef frame_HEADER_GUARD
#define frame_HEADER_GUARD


#include <iostream>
#include  <vector>

//#include <algorithm>
//#include <iterator>
//#include  <vector>

// #include "ff/ff.h"
 //#include "ff/mediacapture.h"
extern "C"
{
//#include <libavutil/timestamp.h>
#include "avformat.h"
}

//#include "micro.h"
#include "codec.h"

#include "constant.h"

/** Enumeration of Frame classes 
 * 
 * @ingroup frames_tag
 */
enum class FrameClass
{
    none, ///< unknown

    basic, ///< data at payload

    avpkt, ///< data at ffmpeg avpkt

    avmedia,  ///< data at ffmpeg av_frame and ffmpeg av_codec_context
    avbitmap, ///< child of avmedia: video
    // avbitmap_np,  ///< child of avmedia: video, non-planar

    avaudio, ///< child of avmedia: audio

    avrgb, ///< rgb interpolated from yuv

    yuv, ///< data at the GPU

    rgb, ///< our own RGB24 data structure

    setup,  ///< setup data
    signal, ///< signal to AVThread or OpenGLThread.  Also custom signals to custom Threads

    marker, ///< Used when sending blocks of frames: mark filesystem and block start and end

    mux, ///< Muxed streams, for example, MP4 or matroska

    First = none,
    Last = mux
};

/** Methods to correct frame timestamps
 * 
 */
enum class TimeCorrectionType 
{          
    none,  
    smart, 
    dummy  
};         

/** Frame: An abstract queueable class.
 * 
 * Instances of this class can be placed into FrameFifos and passed through FrameFilters (see the FrameFifo and FrameFilter classes).  They have characteristic internal data, defined in child classes.
 * 
 * The internal data can be paylod, information for decoder setup, signals to threads, etc.
 * 
 * FrameFifos and FrameFilters are responsible for checking the exact type of the Frame, performing correct typecast and using/discarding/modificating the Frame.
 * 
 * All Frame classes can be put through FrameFilters, but not all Frame classes are copyable/queueable.  Copyable Frame instances can be placed in a FrameFifo (that creates a copy of the Frame before queuing it).
 * 
 * A FrameFifo can also decide to transform the frame to another type before placing it into the queue.
 * 
 * The internal data (and state of the object) consists typically of managed objects, created with external libraries (say, FFmpeg/libav) and "helper" objects/members.  In the case of FFmpeg these are auxiliary members that make the use of the underlying FFmpeg objects more transparent.  State of the managed objects and the helper objects must be kept consistent.
 * 
 * When the state of the managed object(s) is changed, call "updateFrom" with const parameters.  This also makes it more transparent, which parameters trigger updates in helper (and managed) objects.
 * 
 * @ingroup frames_tag
 */

static const int METADATA_MAX_SIZE = 10*1024; // 10 kB

enum class MuxMetaType
{
    none, ///< unknown
    fragmp4
};

struct FragMP4Meta {                                  
    FragMP4Meta() : is_first(false),                   
    size(0), slot(0), mstimestamp(0) {}               
    char name[4];                                     
    bool is_first;                                    
    std::size_t size; ///< Actual size copied         
    SlotNumber slot;                                  
    long int mstimestamp;                             
};    

enum class AbstractFileState
{
    none,
    error,
    seek, // seek start
    stop, // stream stopped
    play  // stream is playing
};

class Frame
{

public:
    Frame();          ///< Default ctor
    virtual ~Frame(); ///< Default virtual dtor
    //frame_essentials(FrameClass::none, Frame);
    //frame_essentials(FrameClass::none, Frame);
    
     virtual std::string type(){return "base";}

public:                                                  // redefined virtual
    virtual void print(std::ostream &os) const;          ///< Produces frame output
    virtual std::string dumpPayload();                   ///< Dumps internal payload data
    virtual void dumpPayloadToFile(std::ofstream &fout); ///< Dumps internal payload data into a file
    virtual void updateAux();                        ///< Update internal auxiliary state variables
    virtual void update();                           ///< Update helper points (call always)
    virtual void reset();                                ///< Reset the internal data
    virtual bool isSeekable();                           ///< Can we seek to this frame? (e.g. is it a key-frame .. for H264 sps packets are used as seek markers)

public:
    void copyMetaFrom(Frame *f); ///< Copy metadata (slot, subsession index, timestamp) to this frame

protected:
    FrameClass frameclass; ///< Declares frametype for correct typecast.  Used by Frame::getFrameClass()

public:                   // public metadata
    SlotNumber n_slot;    ///< Slot number identifying the media source
    int stream_index; ///< Media subsession index
    uint64_t mstimestamp; ///< Presentation time stamp (PTS) in milliseconds
};

inline std::ostream &operator<<(std::ostream &os, const Frame &f)
{
    // https://stackoverflow.com/questions/4571611/making-operator-virtual
    f.print(os);
    return os;
}

/** Custom payload Frame
 * 
 * Includes codec info and the payload.  Received typically from LiveThread or FileThread.
 * 
 * Copiable/Queueable : yes
 * 
 * TODO: update to the metadata scheme using metadata.h
 * 
 * @ingroup frames_tag
 */
class BasicFrame : public Frame
{

public:
    BasicFrame();          ///< Default ctor
    virtual ~BasicFrame(); ///< Default virtual dtor
    //frame_essentials(FrameClass::basic, BasicFrame);
    //frame_essentials(FrameClass::basic, BasicFrame);
    /*BasicFrame(const BasicFrame &f); ///< Default copy ctor
  
public: // frame essentials
  virtual FrameClass getFrameClass();         ///< Returns the subclass frame type.  See Frame::frameclass
  virtual void copyFrom(Frame *f);            ///< Copies data to this frame from a frame of the same type
  */

   virtual std::string type(){return "BasicFrame";}
public:                                         // redefined virtual
    virtual void print(std::ostream &os) const; ///< How to print this frame to output stream
    virtual std::string dumpPayload();
    virtual void dumpPayloadToFile(std::ofstream &fout);
    virtual void reset();      ///< Reset the internal data
    virtual bool isSeekable(); ///< for H264 true if sps, other codecs, always true

public:                                // payload handling
    void reserve(std::size_t n_bytes); ///< Reserve space for internal payload
    void resize(std::size_t n_bytes);  ///< Init space for internal payload

public:                           // frame variables
    std::vector<uint8_t> payload; ///< Raw payload data (use .data() to get the pointer from std::vector)
    AVMediaType media_type;       ///< Type of the media (video/audio)
    AVCodecID codec_id;           ///< AVCodeCID of the media

public:                 // codec-dependent parameters
    H264Pars h264_pars; ///< H264 parameters, extracted from the payload

public:                  // codec-dependent functions
    void fillPars();     ///< Fill codec-dependent parameters based on the payload
    void fillH264Pars(); ///< Inspects payload and fills BasicFrame::h264_pars;
    
    void copyBuf( u_int8_t* buf  ,unsigned size );

public:
    void fillAVPacket(AVPacket *avpkt);                                                                    ///< Copy payload to AVPacket structure
    void copyFromAVPacket(AVPacket *avpkt);                                                                ///< Copy data from AVPacket structure
   // void filterFromAVPacket(AVPacket *avpkt, AVCodecContext *codec_ctx, AVBitStreamFilterContext *filter); ///< Copy data from AVPacket structure  //arvind

public:                                                  // frame serialization
    std::size_t calcSize();                              ///< How much this frame occupies in bytes when serialized
   // bool dump(IdNumber device_id, RaWriter &raw_writer); ///< Write the frame to filestream with a certain device id
   // IdNumber read(RawReader &raw_reader);                ///< Read the frame from filestream.  Returns device id
};

/** A muxed packet (in some container format)
 * 
 * TODO: isSeekable / Meta / Init:
 * peek into payload ..
 * .. or these are set at the ctor? (discovered by the ffmpex muxer demangling)
 */
class MuxFrame : public Frame {

public:
    MuxFrame(); ///< Default ctor
    virtual ~MuxFrame(); ///< Default virtual dtor
    //frame_essentials(FrameClass::mux, MuxFrame);
    //frame_essentials(FrameClass::mux, MuxFrame);
        
public: // redefined virtual
    virtual void print(std::ostream& os) const;             ///< Produces frame output
    virtual std::string dumpPayload();                      ///< Dumps internal payload data
    virtual void dumpPayloadToFile(std::ofstream& fout);    ///< Dumps internal payload data into a file
    virtual void reset();                                   ///< Reset the internal data
    //virtual bool isSeekable();                              ///< Can we seek to this frame? 
    virtual std::string type(){return "MuxFrame";}
/*
public:
    virtual bool isInit();  ///< for frag-MP4: ftyp, moov
    virtual bool isMeta();  ///< for frag-MP4: moof
    ///< otherwise its payload
*/

public:                                // payload handling
    void reserve(std::size_t n_bytes); ///< Reserve space for internal payload
    void resize(std::size_t n_bytes);  ///< Init space for internal payload

public:
    std::vector<uint8_t> payload; ///< Raw payload data (use .data() to get the pointer from std::vector)
    AVMediaType media_type;       ///< Type of the media (video/audio) of the underlying elementary stream
    AVCodecID codec_id;           ///< AVCodeCID of the underlying elementary stream

public:
    std::vector<uint8_t> meta_blob; ///< Byte blob that is casted to correct metadata struct
    MuxMetaType          meta_type; ///< Mux type that mandates how meta_blob is casted
};


enum class SetupFrameType
{
    none,
    stream_init,
    stream_state
};


/** Setup frame
 * 
 * "Setup Frame" is not maybe the most transparent name.  This frame class carries general information between Threads
 * 
 * - For decoders and muxers signals instantiation and initialization
 * - May carry additional metadata of the stream if necessary (in the future)
 * - Carries information about file stream states (play, stop, seek, etc.)
 * 
 * Copiable/Queable : yes.  uses default copy-constructor and copy-assignment
 * 
 * @ingroup frames_tag
 */
class SetupFrame : public Frame
{

public:
    SetupFrame();          ///< Default ctor
    virtual ~SetupFrame(); ///< Default virtual dtor
    //frame_essentials(FrameClass::setup, SetupFrame);
    //frame_essentials(FrameClass::setup, SetupFrame);
    /*
  SetupFrame(const SetupFrame &f); ///< Default copy ctor
  
public: // frame essentials
  virtual FrameClass getFrameClass();         ///< Returns the subclass frame type.  See Frame::frameclass
  virtual void copyFrom(Frame *f);            ///< Copies data to this frame from a frame of the same type
  */
     virtual std::string type(){return "SetupFrame";}
public:                                         // redefined virtual
    virtual void print(std::ostream &os) const; ///< How to print this frame to output stream
    virtual void reset();                       ///< Reset the internal data

public:                      // managed objects
    SetupFrameType sub_type; ///< Type of the SetupFrame

    AVMediaType media_type; ///< For subtype stream_init
    AVCodecID codec_id;     ///< For subtype stream_init

    AbstractFileState stream_state; ///< For subtype stream_state
};


/** Decoded Frame in FFmpeg format
 * 
 * - The decoded frame is in FFmpeg/libav format in AVMediaFrame::av_frame
 * - Constructor does not reserve data for frames.  This is done by classes using this class
 * 
 * Copiable/Queable : no
 * 
 * @ingroup frames_tag
 */
class AVMediaFrame : public Frame
{

public:
    AVMediaFrame();          ///< Default ctor
    virtual ~AVMediaFrame(); ///< Default virtual dtor
    ////frame_essentials(FrameClass::avmedia,AVMediaFrame); // now this is a virtual class ..
    ////frame_essentials(FrameClass::avmedia,AVMediaFrame);
    /*AVMediaFrame(const AVMediaFrame &f); ///< Default copy ctor
    
    public: // frame essentials
    virtual FrameClass getFrameClass(); ///< Returns the subclass frame type.  See Frame::frameclass
    virtual void copyFrom(Frame *f);    ///< Copies data to this frame from a frame of the same type
    */

/*
public:
    virtual void updateAux() = 0; ///< update any helper objects
    virtual void update() = 0;
*/
    
     virtual std::string type(){return "AVMediaFrame";}

public: // redefined virtual
    virtual std::string dumpPayload();
    virtual void print(std::ostream &os) const; ///< How to print this frame to output stream
    virtual void reset();                       ///< Reset the internal data

public:                     // helper objects : values should correspond to member av_frame
    AVMediaType media_type; ///< helper object: media type
    AVCodecID codec_id;     ///< helper object: codec id

public:                // managed objects
    //AVFrame *av_frame; ///< The decoded frame
};

/** Decoded YUV/RGB frame in FFMpeg format
 * 
 * - This FrameClass has decoded video, and is used by the VideoDecoder class. AVThread passes this frame down the filterchain.
 * - It's up to the VideoDecoder to call AVBitmapFrame::update() and to update the helper objects (AVBitmapFrame::bmpars, etc.)
 * - Constructor does not reserve data for frames.  This is done by classes using this class, for example by VideoDecoder.  This is desirable as we don't know the bitmap dimensions yet ..
 * 
 * This is an "intermediate frame".  It is typically copied asap into a YUVFrame.
 * 
 * Copiable/Queable : no
 * 
 * @ingroup frames_tag
;

/** Decoded YUV frame in a non-planar format (thus "NP")
 * 
 * For example, the YUYV422 format (AV_PIX_FMT_YUYV422), where the data layout looks like this:
 * YUYV YUYV YUYV YUYV YUYV YUYV
 * 
 * here we could optimize and copy from YUYV422 directly to YUV420 on the GPU
 * like in YUVFrame::fromAVBitmapFrame
 * maybe someday ..
 * 
 
class AVBitmapFrameNP : public AVMediaFrame {
  
public:
  AVBitmapFrameNP(); ///< Default ctor
  virtual ~AVBitmapFrameNP(); ///< Default virtual dtor
  //frame_essentials(FrameClass::avbitmap_np, AVBitmapFrameNP);
  //frame_essentials(FrameClass::avbitmap_np, AVBitmapFrameNP); // TODO: think about this!
  
public: // redefined virtual
  virtual std::string dumpPayload();
  virtual void print(std::ostream& os) const; ///< How to print this frame to output stream
  virtual void reset();              ///< Reset the internal data
  virtual void update();             ///< Uses AVBitmapFrame::av_frame width and height and AVBitmapFrame::av_pixel_format to calculate AVBitmapFrame::bmpars
  
    
public: // helper objects
  AVPixelFormat  av_pixel_format; ///< From AVCodecContext .. this class implies YUV422P so this is not really required ..
  BitmapPars     bmpars;          ///< Reference parameters for corresponding YUV420P frame
  uint8_t*       payload;         ///< shortcut to the non-planar data
};
*/

class MarkerFrame : public Frame
{

public:
    MarkerFrame();          ///< Default ctor
    virtual ~MarkerFrame(); ///< Default virtual dtor
    //frame_essentials(FrameClass::marker, MarkerFrame);
    ////frame_essentials(FrameClass::marker, MarkerFrame);
    
    virtual std::string type(){return "MarkerFrame";}

public: // redefined virtual
    // virtual std::string dumpPayload();
    virtual void print(std::ostream &os) const; ///< How to print this frame to output stream
    virtual void reset();

public:
    bool fs_start, fs_end; ///< Filesystem start / end  // this controlled better at the python level
    bool tm_start, tm_end; ///< Transmission start / end
};


/** Custom TextFrame Frame
 * 
 * Includes codec info and the payload.  Received typically from LiveThread or FileThread.
 * 
 * @ingroup frames_tag
 */
class TextFrame : public Frame
{

public:
    TextFrame();          ///< Default ctor
    virtual ~TextFrame(){}; ///< Default virtual dtor
    
public:                                                
    std::string txt;
};
#endif
