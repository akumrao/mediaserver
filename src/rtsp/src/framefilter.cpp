
#include "framefilter.h"
#include "tools.h"


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




RepeatH264ParsFrameFilter::RepeatH264ParsFrameFilter(const char *name, FrameFilter *next) : FrameFilter(name, next), sps(), pps(), phase(-1)
{
}

void RepeatH264ParsFrameFilter::go(Frame *frame)
{
}

// #define repeat_ff_verbose 1

void RepeatH264ParsFrameFilter::run(Frame *frame)
{
    if (!next)
    {
        return;
    }

    if (frame->type()  != "BasicFrame")
    { // all other frames than BasicFrame, just pass-through
        (this->next)->run(frame);
    }
    else
    {

#ifdef repeat_ff_verbose
        std::cout << ">>> RepeatH264ParsFrameFilter: got frame" << std::endl;
#endif

        BasicFrame *basic_frame = static_cast<BasicFrame *>(frame);

        if (basic_frame->codec_id == AV_CODEC_ID_H264)
        {
            // H264SliceType::sps, pps, i
            unsigned slice_type = basic_frame->h264_pars.slice_type;
            if (slice_type == H264SliceType::sps)
            {                              // SPS
                //sps.copyFrom(basic_frame); // cache sps
#ifdef repeat_ff_verbose
                std::cout << ">>> RepeatH264ParsFrameFilter: cached sps" << std::endl;
#endif
                phase = 0;
            } // SPS

            else if (slice_type == H264SliceType::pps)
            {                              // PPS
               // pps.copyFrom(basic_frame); // cache pps
#ifdef repeat_ff_verbose
                std::cout << ">>> RepeatH264ParsFrameFilter: cached pps" << std::endl;
#endif
                if (phase == 0)
                { // so, last packet was sps
                    phase = 1;
                }
                else
                {
                    phase = -1;
                }
            } // PPS

            else if (slice_type == H264SliceType::idr)  // arvind it was i 
            { // KEY
                if (phase == 1)
                { // all fine - the packets came in the right order: sps, pps and now i-frame
                }
                else
                {
                    SDebug << "RepeatH264ParsFrameFilter: re-sending sps & pps" << std::endl;

                    if ((sps.codec_id != AV_CODEC_ID_NONE) and (pps.codec_id != AV_CODEC_ID_NONE))
                    { // so, these have been cached correctly
                        sps.mstimestamp = frame->mstimestamp;
                        pps.mstimestamp = frame->mstimestamp;
                        (this->next)->run((Frame *)(&sps));
                        (this->next)->run((Frame *)(&pps));
                    }
                    else
                    {
                        SError << "RepeatH264ParsFrameFilter: re-sending sps & pps required but they're n/a" << std::endl;
                    }
                }
                phase = -1;
            } // KEY

            (this->next)->run(frame); // send the original frame

#ifdef repeat_ff_verbose
            std::cout << ">>> RepeatH264ParsFrameFilter: phase=" << phase << std::endl;
#endif
        }
        else
        { // just passthrough
            (this->next)->run(frame);
        }
    }
}



}
}
