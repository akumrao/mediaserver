
#include "framefilter.h"
#include "tools.h"

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
DummyFrameFilter::DummyFrameFilter(const char *name, bool verbose, FrameFilter *next) : FrameFilter(name, next), verbose(verbose)
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
    if (verbose) {
        // std::cout << "DummyFrameFilter : "<< this->name << " " << verbose << " : got frame : " << *(frame) << std::endl;
        std::cout << "DummyFrameFilter : " << this->name << " : got frame : " << *(frame) << std::endl;



        if (frame->getFrameClass() != FrameClass::mux) {
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
        int x =  muxframe->payload.size();
       
       // meta->size = std::min(x, 10); // correct meta->size if there was more than allowed n_bytes
       // memcpy(payload, f->payload.data(), meta->size);


        fwrite(muxframe->payload.data(), meta->size, 1, fp_out);

    }
}


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
    std::cout << "InfoFrameFilter: timediff: " << frame->mstimestamp - getCurrentMsTimestamp() << std::endl;
    std::cout << "InfoFrameFilter: " << name << " <<end dump   " << std::endl;
}
