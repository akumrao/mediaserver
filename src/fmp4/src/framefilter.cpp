
#include "framefilter.h"
#include "tools.h"


namespace base {
namespace fmp4 {


 #define TIMESTAMPFILTER_DEBUG // keep this commented

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
    // std::cout << ">>>>>>" << verbose << std::endl;
    const char *input_file = "/tmp/test.mp4"; 
    if ((fp_out = fopen(input_file, "wb")) == NULL) {
        fprintf(stderr, "fopen %s failed.\n", input_file);
        // goto ret7;
        return;
    }
    
}
DummyFrameFilter::~DummyFrameFilter()
{
    fclose(fp_out);
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
       
        int ret = fwrite(muxframe->payload.data(), meta->size, 1, fp_out);
        tolalMp4Size +=ret;
        
        if(conn)
         conn->broadcast((const char*)muxframe->payload.data(), meta->size, true );
        
        STrace << " Mp4 Wrote: "<<   meta->size << " Toltal Mp4 Size: " << tolalMp4Size ;

 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////



TextFrameFilter::TextFrameFilter(const char *nm,  ReadMp4 *conn) : name(nm),conn(conn) 
{
}

TextFrameFilter::~TextFrameFilter()
{
    
}

void TextFrameFilter::go(std::string cmd) {

        // std::cout << "DummyFrameFilter : "<< this->name << " " << verbose << " : got frame : " << *(frame) << std::endl;
        SDebug << "TextFrameFilter : " << this->name << " : got frame : " << cmd ;

        
        if(conn)
         conn->broadcast((const char*)cmd.c_str(), cmd.size(), false );
   
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////







InfoFrameFilter::InfoFrameFilter(const char *name, FrameFilter *next) : FrameFilter(name, next)
{
}

void InfoFrameFilter::go(Frame *frame)
{
    std::cout << "InfoFrameFilter: " << name << " start dump>> " << std::endl;
    std::cout << "InfoFrameFilter: FRAME   : " << *(frame) << std::endl;
    std::cout << "InfoFrameFilter: PAYLOAD : [";
    std::cout << frame->dumpPayload();
    std::cout << "]" << std::endl;
    // std::cout << "InfoFrameFilter:<" << frame->dumpAVFrame() << ">" << std::endl;
   // std::cout << "InfoFrameFilter: timediff: " << frame->mstimestamp - getCurrentMsTimestamp() << std::endl;
    std::cout << "InfoFrameFilter: " << name << " <<end dump   " << std::endl;
}


    }
}
