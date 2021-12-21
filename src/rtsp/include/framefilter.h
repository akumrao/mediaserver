#ifndef framefilter_HEADER_GUARD
#define framefilter_HEADER_GUARD


#include "muxframe.h"

#include <map>
#include <deque>
#include <atomic>
#include <condition_variable>
// #include "net/netInterface.h"
// #include "http/HttpsClient.h"


/** The mother class of all frame filters!  
 * FrameFilters are used to create "filter chains".  These chains can be used to manipulate the frames, feed them to fifo's, copy them, etc.
 * 
 * @ingroup filters_tag
 */

                                                
namespace base {
namespace fmp4 {

class ReadMp4;    
    
struct FrameFifoContext {                                                                                                                                       // <pyapi>
                                                                                                                   // <pyapi>
  int n_basic;     ///< data at payload                                                                                                                         // <pyapi>
  int n_avpkt;     ///< data at ffmpeg avpkt                                                                                                                    // <pyapi>
  int n_avframe;   ///< data at ffmpeg av_frame and ffmpeg av_codec_context                                                                                     // <pyapi>
  int n_yuvpbo;    ///< data at yuvpbo struct                                                                                                                   // <pyapi>
  int n_setup;     ///< setup data                                                                                                                              // <pyapi>
  int n_signal;    ///< signal to AVThread or OpenGLThread                                                                                                      // <pyapi>
  int n_marker;    ///< marks start/end of frame emission.  defaults to n_signal                                                                                // <pyapi>    
  bool flush_when_full; ///< Flush when filled                                                                                                                  // <pyapi>
};   

/** A thread-safe combination of a fifo (first-in-first-out) queue and an associated stack.
 * 
 * Frame instances are placed into FrameFifo with FrameFifo::writeCopy that draws a Frame from the stack and performs a copy of the frame.
 * 
 * If no frames are available, an "overflow" occurs.  The behaviour at overflow event can be defined (see FrameFifoContext).
 * 
 * When Frame has been used, it should be returned to the FrameFifo by calling FrameFifo::recycle.  This returns the Frame to the stack.
 * 
 * @ingroup queues_tag
 */
class FrameFifo {                                                                                   

public:                                                                                             
  FrameFifo(const char *name, FrameFifoContext ctx =FrameFifoContext()); ///< Default ctor          
  virtual ~FrameFifo();                                                  ///< Default virtual dtor  
//  ban_copy_ctor(FrameFifo);
 // ban_copy_asm(FrameFifo);
  
protected:
  std::string      name;
  FrameFifoContext ctx;   ///< Parameters defining the stack and overflow behaviour
  
protected: // reservoir, stack & fifo queue
 // std::map<FrameClass,Reservoir>  reservoirs;   ///< The actual frames
 // std::map<FrameClass,Stack>      stacks;       ///< Pointers to the actual frames, sorted by FrameClass
   typedef std::deque<Frame *> Fifo;
  Fifo                            fifo;         ///< The fifo queue
  
  
protected: // mutex synchro
  std::mutex mutex;                         ///< The Lock
  std::condition_variable condition;        ///< The Event/Flag
  std::condition_variable ready_condition;  ///< The Event/Flag for FrameFifo::ready_mutex
    
protected:
  virtual void recycle_(Frame* f);  ///< Return Frame f back into the stack.  Update target_size if necessary
  virtual void recycleAll_();       ///< Recycle all frames back to the stack
  
public:
   // Reservoir &getReservoir(FrameClass cl) {return this->reservoirs[cl];}  ///< Get the reservoir .. in the case you want to manipulate the frames
  
public:
  virtual bool writeCopy(Frame* f, bool wait=false);     ///< Take a frame "ftmp" from the stack, copy contents of "f" into "ftmp" and insert "ftmp" into the beginning of the fifo (i.e. perform "copy-on-insert").  The size of "ftmp" is also checked and set to target_size, if necessary.  If wait is set to true, will wait until there are frames available in the stack.
  virtual Frame* read(unsigned short int mstimeout=0);   ///< Pop a frame from the end of the fifo when available
  virtual void recycle(Frame* f);                        ///< Like FrameFifo::recycle_ but with mutex protection
  virtual void recycleAll();                             ///< Recycle all frames from fifo back to stack (make a "flush")
  virtual void dumpStacks();    ///< Dump frames in the stacks
  virtual void dumpFifo();      ///< Dump frames in the fifo
  virtual void diagnosis();     ///< Print a resumen of fifo and stack usage
  bool isEmpty();               ///< Tell if fifo is empty
};                  
    

    
    
    
class FrameFilter { 

public: 
    /** Default constructor
   * 
   * @param name  Name of the filter
   * @param next  Next FrameFilter instance in the filter chain
   * 
   */
    FrameFilter(const char *name, FrameFilter *next = NULL); // don't include into the python api (this class is abstract)
    virtual ~FrameFilter();                                  ///< Virtual destructor 

public:
    std::string name;

public:                // sometimes ffmpeg static functions need to access this! (in muxing at least)
    FrameFilter *next; ///< The next frame filter in the chain to be applied

protected: 
    // does the filtering
    virtual void go(Frame *frame) = 0; ///< Does the actual filtering/modification to the Frame.  Define in subclass

public: // API
    /** Calls this->go(Frame* frame) and then calls the this->next->run(Frame* frame) (if this->next != NULL)
   */
    virtual void run(Frame *frame);
    
    virtual void deActivate(){};
    virtual void sendMeta(){};
    //std::atomic< bool > resetParser { false };
}; 

/** A "hello world" demo class: prints its own name if verbose is set to true.
 * @ingroup filters_tag
 */
//namespace base {
//namespace fmp4 {
//    class ReadMp4;
//    
//}
//}


class DummyFrameFilter : public FrameFilter { 

public:                                                                                
    DummyFrameFilter(const char *name,  base::fmp4::ReadMp4 *conn , bool verbose = true, FrameFilter *next = NULL); 
     ~DummyFrameFilter();

     base::fmp4::ReadMp4 *conn; 
protected:
    bool verbose;
   

protected:
    void go(Frame *frame);
    
    FILE* fp_out;
    long  tolalMp4Size;
}; 


class TextFrameFilter: public FrameFilter  { 

public:                                                                                
    TextFrameFilter(const char *name,  base::fmp4::ReadMp4 *conn,  FrameFilter *next = NULL ); 
     ~TextFrameFilter();

     base::fmp4::ReadMp4 *conn;
     
  
public:
    
    protected:
    void go(Frame *frame);    

    //std::string name;
}; 

/** Dump the beginning of Frame's payload into stdout
 * @ingroup filters_tag
 */
class InfoFrameFilter : public FrameFilter { 

public:                                                          
    InfoFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    void go(Frame *frame);
}; 

/** Dump the beginning of Frame's payload into stdout in a one-liner
 * @ingroup filters_tag
 */
class BriefInfoFrameFilter : public FrameFilter { 

public:                                                               
    BriefInfoFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    void go(Frame *frame);
}; 

/** FrameFilter s that are fed from various different threads, should be protected with this
 * 
 * 
 * Thread --> framefilter --+
 *                          + --> ThreadSafeFrameFilter --> Final frame filter
 * Thread --> framefilter --+
 * 
 * @ingroup filters_tag
 */
class ThreadSafeFrameFilter : public FrameFilter { 

private:
    std::mutex mutex;

public:                                                                
    ThreadSafeFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);
}; 

/** Replicates frame flow to two filters
 * Use this frame filter to create frame filter tree structures
 * @ingroup filters_tag
 */
class ForkFrameFilter : public FrameFilter { 

public: 
    /** @copydoc FrameFilter::FrameFilter
   * 
   *  @param next2 Yet another next FrameFilter instance to be applied in the chain
   * 
   */
    ForkFrameFilter(const char *name, FrameFilter *next = NULL, FrameFilter *next2 = NULL); 

protected:
    FrameFilter *next2;

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);
}; 

/** Replicates frame flow to three filters
 * Use this frame filter to create frame filter tree structures
 * @ingroup filters_tag
 */
class ForkFrameFilter3 : public FrameFilter { 

public: 
    /** @copydoc FrameFilter::FrameFilter
   * 
   *  @param next2 Yet another next FrameFilter instance to be applied in the chain
   *  @param next3 Still yet another next FrameFilter instance to be applied in the chain
   * 
   */
    ForkFrameFilter3(const char *name, FrameFilter *next = NULL, FrameFilter *next2 = NULL, FrameFilter *next3 = NULL); 

protected:
    FrameFilter *next2;
    FrameFilter *next3;

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);
}; 

/** Replicates frame flow to arbitrary number of outputs
 * 
 * - Terminals are added after the instance has been created
 *
 * @ingroup filters_tag 
 */
class ForkFrameFilterN : public FrameFilter { 

public: 
    /** Default ctor
   * 
   * @param name  Name identifying this FrameFilter
   * 
   */
    ForkFrameFilterN(const char *name); 
    /** Default virtual dtor
   */
    virtual ~ForkFrameFilterN(); 

protected:
    std::mutex mutex;
    std::map<std::string, FrameFilter *> framefilters; ///< nametag to connecting FrameFilter mapping

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame); ///< called by other FrameFilter(s)

public: 
    /** Connect a new terminal FrameFilter.  Tag the connection with a name.
   * 
   * @param tag     Nametag for this connection
   * @param filter  FrameFilter for the connection
   * 
   */
    bool connect(const char *tag, FrameFilter *filter); 
    /** Disconnect a connection tagged with a name
   * 
   * @param tag     Nametag for this connection
   * 
   */
    bool disconnect(const char *tag); 
};                                    



/** Dumps each received packet to a file: use with care!  For debugging purposes only.
 * @ingroup filters_tag
 */
class DumpFrameFilter : public FrameFilter { 

public:                                                          
    DumpFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    int count;

protected:
    void go(Frame *frame);
}; 

/** Counts frames passed through this filter
 * @ingroup filters_tag
 */

class CountFrameFilter : public FrameFilter { 

public:                                                           
    CountFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    int count;

protected:
    void go(Frame *frame);
}; 

/** Corrects erroneous timestamps (while preserving timestamp distances).
 * @ingroup filters_tag
 */

/** Corrects erroneous timestamps (while preserving timestamp distances).  Reset correction every 10 minutes.
 * @ingroup filters_tag
 */

/** Substitute timestamps with the time they arrive to the client.
 * @ingroup filters_tag
 */
class DummyTimestampFrameFilter : public FrameFilter { 

public:                                                                    
    DummyTimestampFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    void go(Frame *frame);
}; 

/** For H264, some cameras don't send sps and pps packets again before every keyframe.  In that case, this filter sends sps and pps before each keyframe.
 * 
 * WARNING: UNTESTED!  // TODO: make tests with dummy H264 packets
 * 
 * @ingroup filters_tag
 */
class RepeatH264ParsFrameFilter : public FrameFilter { 

public:                                                                    
    RepeatH264ParsFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    BasicFrame sps, pps;
    int phase;

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);

}; 

/** When turned on, passes frames.  When turned off, frames are not passed.  
 * 
 * - Configuration frames (FrameType::setup) are passed, even if the gate is unSet
 * - Passing of configuration frames can be turned off (when gate is unSet) by calling noConfigFrames()
 * - Mutex-protected (calls to GateFrameFilter::set and GateFrameFilter::unSet happen during streaming)
 * 
 * @ingroup filters_tag
 */
class GateFrameFilter : public FrameFilter { 

public:                                                          
    GateFrameFilter(const char *name, FrameFilter *next = NULL); 

protected:
    bool on;
    bool config_frames;
    std::mutex mutex;

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);

public:                      
    void set();              
    void unSet();            
    void passConfigFrames(); 
    void noConfigFrames();   
};                           

/** Passes frame to one of the two terminals
 * 
 * 
 */
class SwitchFrameFilter : public FrameFilter { 

public:                                                                                        
    SwitchFrameFilter(const char *name, FrameFilter *next1 = NULL, FrameFilter *next2 = NULL); 

protected:
    FrameFilter *next1;
    FrameFilter *next2;

protected:
    int index;
    std::mutex mutex;

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);

public:
    void set1(); 
    void set2(); 
};               

/** Passes through frames of certain type only
 * 
 * - Not part of the Python API
 * 
 */
class TypeFrameFilter : public FrameFilter
{

public:
    TypeFrameFilter(const char *name, FrameClass frameclass, FrameFilter *next = NULL);

protected:
    FrameClass frameclass;

protected:
    void go(Frame *frame);
    void run(Frame *frame);
};

/** Caches SetupFrame s
 * 
 * Like GateFrameFilter, but caches SetupFrame s and re-emits them always when the gate is activated
 * 
 */
               

/** Changes the slot number of the Frame
 *
 * Mutex-protected (calls to SetSlotFrameFilter::setSlot happen during streaming)
 * 
 * This is a mutex-protected version of SlotFrameFilter.  Not used that much at.  Should deprecate this.
 * 
 * @ingroup filters_tag 
 */

/** Pass frames, but not all of them - only on regular intervals.  This serves for downshifting the fps rate.
 *
 * Should be used, of course, only for decoded frames..!
 * 
 * @param name          A name identifying the frame filter
 * @param mstimedelta   Time interval in milliseconds
 * @param next          Next filter in chain
 * 
 * @ingroup filters_tag
 */
class TimeIntervalFrameFilter : public FrameFilter { 

public:                                                                                        
    TimeIntervalFrameFilter(const char *name, long int mstimedelta, FrameFilter *next = NULL); 

protected:
    long int mstimedelta;
    long int prevmstimestamp;

protected:
    void go(Frame *frame);

public:
    void run(Frame *frame);

}; 

/** Passes frames to a FrameFifo
 * 
 * Typically, the terminal point for the frame filter chain, so there is no next filter = NULL.
 * 
 * @ingroup filters_tag
 * @ingroup queues_tag
 */
class FifoFrameFilter : public FrameFilter { 

public: 
    /** Default constructor
   * 
   * @param name       Name
   * @param framefifo  The FrameFifo where the frames are being written
   */
    FifoFrameFilter(const char *name, FrameFifo *framefifo); ///< Default constructor     

protected:
    FrameFifo *framefifo;

protected:
    void go(Frame *frame);
}; 

/** Passes frames to a multiprocessing fifo.
 * 
 * Works like FifoFrameFilter, but blocks if the receiving FrameFifo does not have available frames
 * 
 * @ingroup filters_tag
 * @ingroup queues_tag
 */


    }
}

#endif
