
#include "framefilter.h"
#include "tools.h"
#include "base/logger.h"
#include "fmp4.h"

namespace base {
namespace fmp4 {

//#define DUMPFMP4 1

// #define TIMESTAMPFILTER_DEBUG // keep this commented

FrameFilter::FrameFilter(const char *name, FrameFilter *next) : name(name), next(next){};

FrameFilter::~FrameFilter(){};

void FrameFilter::run(Frame *frame)
{
    this->go(frame); // manipulate frame
    if (!this->next)
    {
        return;
    } // call next filter .. if there is any
    (this->next)->run(frame);
}

// subclass like this:
DummyFrameFilter::DummyFrameFilter(const char *name,  ReadMp4 *conn, bool verbose, FrameFilter *next) : conn(conn), FrameFilter(name, next), verbose(verbose)
{
    #if DUMPFMP4 
    // std::cout << ">>>>>>" << verbose << std::endl;
    const char *input_file = "/tmp/test.mp4"; 
    if ((fp_out = fopen(input_file, "wb")) == NULL) {
        fprintf(stderr, "fopen %s failed.\n", input_file);
        // goto ret7;
        return;
    }
     #endif
}
DummyFrameFilter::~DummyFrameFilter()
{
    #if DUMPFMP4 
    fclose(fp_out);
    #endif
}

void DummyFrameFilter::go(Frame *frame) {
   
        // std::cout << "DummyFrameFilter : "<< this->name << " " << verbose << " : got frame : " << *(frame) << std::endl;
        STrace << "DummyFrameFilter : " << this->name << " : got frame : " << *(frame) ;



        if (frame->type() != "MuxFrame") {
            std::cout << "FragMP4ShmemFrameFilter: go: ERROR: MuxFrame required" << std::endl;
            return;
        }
        MuxFrame *muxframe = static_cast<MuxFrame*> (frame);
        if (muxframe->meta_type != MuxMetaType::fragmp4) {
            std::cout << "FragMP4ShmemFrameFilter::go: needs MuxMetaType::fragmp4"
                    << std::endl;
            return;
        }

    
        FragMP4Meta* meta = (FragMP4Meta*) (muxframe->meta_blob.data());
       // *meta = *meta_;
       
        #if DUMPFMP4 
        int ret = fwrite(muxframe->payload.data(), meta->size, 1, fp_out);
        tolalMp4Size +=ret;
        #endif
        
        if(conn)
         conn->broadcast((const char*)muxframe->payload.data(), meta->size, true );
        
        STrace << " Mp4 Wrote: "<<   meta->size << " Toltal Mp4 Size: " << tolalMp4Size ;

 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////



TextFrameFilter::TextFrameFilter(const char *name,  ReadMp4 *conn, FrameFilter *next) : FrameFilter(name, next),conn(conn) 
{
}

TextFrameFilter::~TextFrameFilter()
{
    
}

void TextFrameFilter::go(Frame *frame) {
        
      TextFrame *txt    =  (TextFrame*) frame;
       SDebug << "Send Text Message : " << this->name << " : got frame : " << txt->txt ;
        
      if(conn)
         conn->broadcast((const char*)txt->txt.c_str(), txt->txt.size(), false );
   
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////







InfoFrameFilter::InfoFrameFilter(const char *name, FrameFilter *next) : FrameFilter(name, next)
{
}

void InfoFrameFilter::go(Frame *frame)
{
    SInfo << "InfoFrameFilter: " << name << " start dump>> " ;
    SInfo << "InfoFrameFilter: FRAME   : " << *(frame) ;
    SInfo << "InfoFrameFilter: PAYLOAD : [";
    SInfo << frame->dumpPayload();
    SInfo << "]" << std::endl;
    // std::cout << "InfoFrameFilter:<" << frame->dumpAVFrame() << ">" << std::endl;
   // std::cout << "InfoFrameFilter: timediff: " << frame->mstimestamp - getCurrentMsTimestamp() << std::endl;
    SInfo<< "InfoFrameFilter: " << name << " <<end dump   " ;
}


    }
}
