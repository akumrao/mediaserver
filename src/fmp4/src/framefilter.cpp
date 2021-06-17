
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
}

void DummyFrameFilter::go(Frame *frame)
{
    if (verbose)
    {
        // std::cout << "DummyFrameFilter : "<< this->name << " " << verbose << " : got frame : " << *(frame) << std::endl;
        std::cout << "DummyFrameFilter : " << this->name << " : got frame : " << *(frame) << std::endl;
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
