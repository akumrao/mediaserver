
#include "framefilter.h"
#include "tools.h"
#include "base/logger.h"
#include "restApi.h"
#include "base/datetime.h"
#include "base/filesystem.h"
#include "Settings.h"

namespace base
{
namespace web_rtc
{

//#define DUMP264 1

// #define TIMESTAMPFILTER_DEBUG // keep this commented

FrameFilter::FrameFilter(const char *name, FrameFilter *next) : name(name), next(next){};

FrameFilter::~FrameFilter(){};

void FrameFilter::run(Frame *frame)
{
    this->go(frame);  // manipulate frame
    if (!this->next) { return; }  // call next filter .. if there is any
    (this->next)->run(frame);
}

// subclass like this:
DummyFrameFilter::DummyFrameFilter(const char *name, std::string &cam , RestApi *conn, bool verbose, FrameFilter *next) : cam(cam), FrameFilter(name, next), verbose(verbose)
{
    
    if (!base::fs::exists(Settings::configuration.storage))
    {
        base::fs::mkdir(Settings::configuration.storage);
    }

    camera= "CAM" + cam;

   
    if (!base::fs::exists(Settings::configuration.storage + camera))
    {
        base::fs::mkdir(Settings::configuration.storage + camera);
    }
        
  

//    metadata = Settings::configuration.storage + camera + "/metadata";
//    if (!base::fs::exists(metadata))
//    {
//       base::fs::mkdir(metadata );
//    }
   
    
}
DummyFrameFilter::~DummyFrameFilter()
{
    if(in_file)
    {
        fclose(in_file);
        in_file = nullptr;
    }
    mf.save();
}

void DummyFrameFilter::go(Frame *frame)
{

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


   // if(conn)
//         conn->broadcast((const char*)muxframe->payload.data(), meta->size, true , muxframe->frametype, cam);

    std::string boxName = std::string( meta->name, 4);
   // SInfo << " Mp4 Wrote: "<<   meta->size << " box name " << boxName << "  muxframe->frametype " <<   muxframe->frametype  << " cam " << cam ;

    Timestamp ts;

    Timestamp::TimeVal time = ts.epochMicroseconds();

    int milli = int(time % 1000000) / 1000;


    std::time_t time1 = ts.epochTime();
    struct std::tm* tms = std::localtime(&time1);


    char date[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
    int len = std::strftime(date, sizeof (date), "%Y-%m-%d", tms);

    if( dayDate != date)
    {   dayDate = date;
        std::string tmp = Settings::configuration.storage + camera + "/" + dayDate + "/" ;
        if (!base::fs::exists(tmp ))
        {
           mkdir(tmp.c_str(),0777);
        }


        mf.load(tmp + "manifest.js");

        if( mf.root.is_null() )
        {
           mf.root[dayDate] = json::object();
        }

        mf.root["fps"]= muxframe->fps;
    }



    char timeHr[100]= {'\0'};
    len = std::strftime(timeHr, sizeof (timeHr), "%H", tms);

    if( HR != timeHr)
    {   HR = timeHr;
        HRFullPath = Settings::configuration.storage + camera + "/" + dayDate + "/" + HR;
        if (!base::fs::exists(HRFullPath))
        {
           //base::fs::mkdir(HRFullPath); crashes
           mkdir(HRFullPath.c_str(),0777);
        }

        if( mf.root[dayDate][HR].is_null())
        {
           mf.root[dayDate][HR] = json::object();
        }
    }


    char timeSec[100]= {'\0'};
    len = std::strftime(timeSec, sizeof (timeSec), "%H-%M-%S", tms);


    char currentTime[84] = {'\0'};
    len = sprintf(currentTime, "%s_%d", timeSec, milli);

    std::string filePath = HRFullPath + "/" + currentTime + ".mp4";

    if( muxframe->frametype == 1  && !keycount  )
    {
        if(in_file)
        {
            fclose(in_file);
            in_file = nullptr;
        }

        in_file = fopen(filePath.c_str(),"wb");
        if(!in_file){
               SError << "can't open file! " <<  in_file;
        }

        if( mf.root[dayDate][HR][currentTime].is_null())
        {
            mf.root[dayDate][HR][currentTime] = json::array();

            activeFileName = currentTime; 
        }
        else
        {
            std::cout << "not a possible state";
        }

    }

    if( muxframe->frametype == 3 )
    {
     //  filePath = filePath + "key" ;

       if( boxName == "moof")

        if( (keycount %  Settings::configuration.SegSize_key ) == 0)
        {
            int len = ftell(in_file);

            mf.root[dayDate][HR][activeFileName].push_back(len  );// = std::string(currentTime) + "." + boxName;
            mf.save();
        }

       if(++keycount >  Settings::configuration.Mp4Size_Key )
           keycount = 0;
       //}
    }

    fwrite((const char*) muxframe->payload.data(), 1, meta->size, in_file);
    //fs::savefile(filePath ,(const char*) muxframe->payload.data(), meta->size, false);



}


/////////////////////////////////////////////////////////////////////////////////////////////////////

TextFrameFilter::TextFrameFilter(const char *name,  std::string &cam, RestApi *conn, FrameFilter *next) : FrameFilter(name, next), cam(cam), conn(conn)  
{
}

TextFrameFilter::~TextFrameFilter() {}

void TextFrameFilter::go(Frame *frame)
{
    TextFrame *txt = (TextFrame *) frame;
    SDebug << "Send Text Message : " << this->name << " : got frame : " << txt->txt;
    
    if(conn)
    conn->postAppMessage(cam, txt->frameType, txt->txt );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////


InfoFrameFilter::InfoFrameFilter(const char *name, FrameFilter *next) : FrameFilter(name, next) {}

void InfoFrameFilter::go(Frame *frame)
{
    SInfo << "InfoFrameFilter: " << name << " start dump>> ";
    SInfo << "InfoFrameFilter: FRAME   : " << *(frame);
    SInfo << "InfoFrameFilter: PAYLOAD : [";
    SInfo << frame->dumpPayload();
    SInfo << "]" << std::endl;
    // std::cout << "InfoFrameFilter:<" << frame->dumpAVFrame() << ">" << std::endl;
    // std::cout << "InfoFrameFilter: timediff: " << frame->mstimestamp - getCurrentMsTimestamp() << std::endl;
    SInfo << "InfoFrameFilter: " << name << " <<end dump   ";
}


RepeatH264ParsFrameFilter::RepeatH264ParsFrameFilter(const char *name, FrameFilter *next)
    : FrameFilter(name, next),
      sps(),
      pps(),
      phase(-1)
{}

void RepeatH264ParsFrameFilter::go(Frame *frame) {}

// #define repeat_ff_verbose 1

void RepeatH264ParsFrameFilter::run(Frame *frame)
{
    if (!next) { return; }

    if (frame->type() != "BasicFrame")
    {  // all other frames than BasicFrame, just pass-through
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
            {  // SPS
               // sps.copyFrom(basic_frame); // cache sps
#ifdef repeat_ff_verbose
                std::cout << ">>> RepeatH264ParsFrameFilter: cached sps" << std::endl;
#endif
                phase = 0;
            }  // SPS

            else if (slice_type == H264SliceType::pps)
            {  // PPS
               // pps.copyFrom(basic_frame); // cache pps
#ifdef repeat_ff_verbose
                std::cout << ">>> RepeatH264ParsFrameFilter: cached pps" << std::endl;
#endif
                if (phase == 0)
                {  // so, last packet was sps
                    phase = 1;
                }
                else
                {
                    phase = -1;
                }
            }  // PPS

            else if (slice_type == H264SliceType::idr)  // arvind it was i
            {  // KEY
                if (phase == 1)
                {  // all fine - the packets came in the right order: sps, pps and now i-frame
                }
                else
                {
                    SDebug << "RepeatH264ParsFrameFilter: re-sending sps & pps" << std::endl;

                    if ((sps.codec_id != AV_CODEC_ID_NONE) and (pps.codec_id != AV_CODEC_ID_NONE))
                    {  // so, these have been cached correctly
                        sps.mstimestamp = frame->mstimestamp;
                        pps.mstimestamp = frame->mstimestamp;
                        (this->next)->run((Frame *) (&sps));
                        (this->next)->run((Frame *) (&pps));
                    }
                    else
                    {
                        SError << "RepeatH264ParsFrameFilter: re-sending sps & pps required but they're n/a"
                               << std::endl;
                    }
                }
                phase = -1;
            }  // KEY

            (this->next)->run(frame);  // send the original frame

#ifdef repeat_ff_verbose
            std::cout << ">>> RepeatH264ParsFrameFilter: phase=" << phase << std::endl;
#endif
        }
        else
        {  // just passthrough
            (this->next)->run(frame);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////

FrameFifo::FrameFifo(const char *name, FrameFifoContext ctx) : name(name), ctx(ctx) {}

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
    //    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get
    //    out of context recycle_(f); ready_condition.notify_one();
}

void FrameFifo::recycleAll_()
{  // move all frames from fifo to stack
   //    auto it = fifo.begin();
   //    while (it != fifo.end())
   //    {
   //        recycle_(*it);       // return to stack
   //        it = fifo.erase(it); // and erase from the fifo
   //    }
}

void FrameFifo::recycleAll()
{  // move all frames from fifo to stack
   //    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get
   //    out of context recycleAll_();
}

void FrameFifo::dumpStacks()
{
    //    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get
    //    out of context Stack stack;
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
    //    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get
    //    out of context
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
    //    std::unique_lock<std::mutex> lk(this->mutex); // this acquires the lock and releases it once we get
    //    out of context Stack stack;
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
FifoFrameFilter::FifoFrameFilter(const char *name, FrameFifo *framefifo)
    : FrameFilter(name),
      framefifo(framefifo){};

void FifoFrameFilter::go(Frame *frame)
{
    framefifo->writeCopy(frame);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace web_rtc
}  // namespace base
