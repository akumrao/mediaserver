
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
FileFrameFilter::FileFrameFilter(const char *name, std::string &cam , FrameFilter *next) : cam(cam),  FrameFilter(name, next)
{

     
}

void FileFrameFilter::Open(std::string fileName)
{

    //SInfo << "open MD file " <<  fileName;
      
 // const char *input_file = "/tmp/test.mp4"; 
    if ((fp_out = fopen(fileName.c_str(), "wb")) == NULL) {
        
        SError << "Could not open " <<  fileName;
        return;
    }

   //frameNo = 0;
       
}

void FileFrameFilter::Close()
{
   if(fp_out)
   fclose(fp_out);
   fp_out = nullptr;
   
  // frameNo = 0;

}

FileFrameFilter::~FileFrameFilter()
{
    Close();
}

void FileFrameFilter::go(Frame *frame) {
   
        // std::cout << "DummyFrameFilter : "<< this->name << " " << verbose << " : got frame : " << *(frame) << std::endl;
       
        //SInfo << "FileFrameFilter : " << this->name << " : got frame : " << *(frame)  <<  " frame no " <<   ++frameNo;



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
        //tolalMp4Size +=ret;

       // SInfo << " Mp4 Wrote: "<<   meta->size << " Toltal Mp4 Size: " << tolalMp4Size ;
 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////



// subclass like this:
DummyFrameFilter::DummyFrameFilter(const char *name, std::string &cam , ReadMp4 *conn, bool verbose, FrameFilter *next) : cam(cam), conn(conn), FrameFilter(name, next), verbose(verbose)
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
         conn->broadcast((const char*)muxframe->payload.data(), meta->size, FRAMETYPE::BINARY , muxframe->frametype, cam);
        
       // SInfo << " Mp4 Wrote: "<<   meta->size << " Toltal Mp4 Size: " << tolalMp4Size ;

 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////



TextFrameFilter::TextFrameFilter(const char *name,  std::string &cam, ReadMp4 *conn, FrameFilter *next) : FrameFilter(name, next), cam(cam), conn(conn)  
{
}

TextFrameFilter::~TextFrameFilter()
{
    
}

void TextFrameFilter::go(Frame *frame) {
        
      TextFrame *txt    =  (TextFrame*) frame;
       SDebug << "Send Text Message : " << this->name << " : got frame : " << txt->txt ;
        
      if(conn)
      {
         txt->txt = cam + ":" +  txt->txt;
         conn->broadcast((const char*)txt->txt.c_str(), txt->txt.size(), txt->frameType, 0, cam );
      }
   
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




H264DumpFrameFilter::H264DumpFrameFilter(const char *name,  std::string &cam, FrameFilter *next) : FrameFilter(name, next), sps(), pps(), cam(cam)
{
}

void H264DumpFrameFilter::go(Frame *frame)
{
}

// #define repeat_ff_verbose 1

void H264DumpFrameFilter::run(Frame *frame)
{


    BasicFrame *basic_frame = static_cast<BasicFrame *>(frame);

    if (basic_frame->codec_id == AV_CODEC_ID_H264)
    {
        // H264SliceType::sps, pps, i
        unsigned slice_type = basic_frame->h264_pars.slice_type;
        if (slice_type == H264SliceType::sps)
        {                              // SPS
            //sps.copyFrom(basic_frame); // cache sps

        } // SPS

        else if (slice_type == H264SliceType::pps)
        {                              // PPS

        } // PPS

        else if (slice_type == H264SliceType::idr)  // arvind it was i 
        { // KEY


            SDebug << "RepeatH264ParsFrameFilter: re-sending sps & pps" << std::endl;

            if ((sps.codec_id != AV_CODEC_ID_NONE) and (pps.codec_id != AV_CODEC_ID_NONE))
            { // so, these have been cached correctly
                sps.mstimestamp = frame->mstimestamp;
                pps.mstimestamp = frame->mstimestamp;

            }
            else
            {
                SError << "RepeatH264ParsFrameFilter: re-sending sps & pps required but they're n/a" << std::endl;
            }


        } 

    }


}

//////////////////////////////////////////////////////////////////////////////////////

FrameFifo::FrameFifo(const char *name, FrameFifoContext ctx) : name(name), ctx(ctx)
{

}

FrameFifo::~FrameFifo()
{
    recycleAll();


}

bool FrameFifo::writeCopy(Frame *f, bool wait)
{
 
    return true;
}

Frame *FrameFifo::read(unsigned short int mstimeout)
{
 return NULL;
}

void FrameFifo::recycle_(Frame *f)
{
//    f->reset();
//    Stack &stack = stacks.at(f->getFrameClass());
//    stack.push_back(f); // take: from the front.  recycle: to the back
}

void FrameFifo::recycle(Frame *f)
{
//    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get out of context
//    recycle_(f);
//    ready_condition.notify_one();
}

void FrameFifo::recycleAll_()
{ // move all frames from fifo to stack
//    auto it = fifo.begin();
//    while (it != fifo.end())
//    {
//        recycle_(*it);       // return to stack
//        it = fifo.erase(it); // and erase from the fifo
//    }
}

void FrameFifo::recycleAll()
{                                                 // move all frames from fifo to stack
//    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get out of context
//    recycleAll_();
}

void FrameFifo::dumpStacks()
{
//    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get out of context
//    Stack stack;
//
//    std::cout << "FrameFifo : dumpStacks : " << std::endl;
//    for (auto it = stacks.begin(); it != stacks.end(); ++it)
//    {
//        std::cout << "FrameFifo : dumpStacks : Stack=" << int(it->first) << std::endl;
//        stack = it->second;
//        for (auto its = stack.begin(); its != stack.end(); ++its)
//        {
//            std::cout << "FrameFifo : dumpStacks :  " << *(*its) << std::endl;
//        }
//    }
//    std::cout << "FrameFifo : dumpStacks : " << std::endl;
}

void FrameFifo::dumpFifo()
{
//    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get out of context
//
//    std::cout << "FrameFifo : dumpFifo : " << std::endl;
//    for (auto it = fifo.begin(); it != fifo.end(); ++it)
//    {
//        std::cout << "FrameFifo : dumpFifo : " << *(*it) << std::endl;
//    }
//    std::cout << "FrameFifo : dumpFifo : " << std::endl;
}

void FrameFifo::diagnosis()
{
//    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get out of context
//    Stack stack;
//
//    std::cout << "FrameFifo : diagnosis : " << std::endl;
//    std::cout << "FrameFifo : diagnosis : fifo  : " << fifo.size() << std::endl;
//    std::cout << "FrameFifo : diagnosis : stack : ";
//    for (auto it = stacks.begin(); it != stacks.end(); ++it)
//    {
//        std::cout << int(it->first) << ":" << (it->second).size() << ", ";
//    }
//    std::cout << std::endl;
//    std::cout << "FrameFifo : diagnosis : " << std::endl;
}

bool FrameFifo::isEmpty()
{
    return fifo.empty();
}





//////////////////////////////////////////////////////////////////////////////////////////////////
FifoFrameFilter::FifoFrameFilter(const char *name, FrameFifo *framefifo) : FrameFilter(name), framefifo(framefifo){};

void FifoFrameFilter::go(Frame *frame)
{
    framefifo->writeCopy(frame);
}



/////////////////////////////////////////////////////////////////////////////////////////////////
}
}
