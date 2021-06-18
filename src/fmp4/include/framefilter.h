#ifndef framefilter_HEADER_GUARD
#define framefilter_HEADER_GUARD


#include "frame.h"
//#include "framefifo.h"

/** The mother class of all frame filters!  
 * FrameFilters are used to create "filter chains".  These chains can be used to manipulate the frames, feed them to fifo's, copy them, etc.
 * 
 * @ingroup filters_tag
 */

                                                


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
}; 

/** A "hello world" demo class: prints its own name if verbose is set to true.
 * @ingroup filters_tag
 */
class DummyFrameFilter : public FrameFilter { 

public:                                                                                
    DummyFrameFilter(const char *name, bool verbose = true, FrameFilter *next = NULL); 
     ~DummyFrameFilter();
protected:
    bool verbose;
   

protected:
    void go(Frame *frame);
    
    FILE* fp_out;

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
//class FifoFrameFilter : public FrameFilter { 
//
//public: 
//    /** Default constructor
//   * 
//   * @param name       Name
//   * @param framefifo  The FrameFifo where the frames are being written
//   */
//    FifoFrameFilter(const char *name, FrameFifo *framefifo); ///< Default constructor     
//
//protected:
//    FrameFifo *framefifo;
//
//protected:
//    void go(Frame *frame);
//}; 

/** Passes frames to a multiprocessing fifo.
 * 
 * Works like FifoFrameFilter, but blocks if the receiving FrameFifo does not have available frames
 * 
 * @ingroup filters_tag
 * @ingroup queues_tag
 */

#endif
