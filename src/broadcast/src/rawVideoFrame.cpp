
#include "webrtc/rawVideoFrame.h"
#include "base/logger.h"

namespace base
{
namespace web_rtc
{

// FRawFrameBuffer:: FRawFrameBuffer(  std::vector<uint8_t>  &p,  int w ,  int h, uint frameNo, bool idr,
// uint32_t fps): frame(p, w, h, idr),  frameNo(frameNo), fps(fps)
//{
//       //   std::cout <<  " FRawFrameBuffer frame no"  <<   frameNo  <<  " frame size "  << payload.size()  <<
//       std::endl << std::flush;
// }

NULLEncBuffer::NULLEncBuffer(stFrame *qframe, int w, int h, uint32_t fps, std::string &txt)
    : qframe(qframe),
      width_(w),
      height_(h),
      fps(fps),
      txt(txt)
{
    // std::cout <<  " FRawFrameBuffer frame w"  <<   w  <<  " frame h "  << h  << std::endl << std::flush;
}


H264FrameBuffer::H264FrameBuffer(AVCodecContext *dec_ctx, AVFrame *qframe, int w, int h, std::string &txt)
    : dec_ctx(dec_ctx),
      qframe(qframe),
      width_(w),
      height_(h),
      txt(txt)
{
    // std::cout <<  " FRawFrameBuffer frame w"  <<   w  <<  " frame h "  << h  << std::endl << std::flush;

    DecodeContext *decode = (DecodeContext *) dec_ctx->opaque;
    ++decode->NVSurface;

    // frmNo = qframe->pts;
    // SInfo << "H264FrameBuffer() surface " << frmNo << " "  <<   decode->NVSurface<< std::flush;
}

H264FrameBuffer::~H264FrameBuffer()
{
    av_frame_free(&qframe);
    qframe = nullptr;

    DecodeContext *decode = (DecodeContext *) dec_ctx->opaque;
    --decode->NVSurface;
    // SInfo << " H264FrameBuffer~() " << frmNo   << std::flush;
}


stFrame::~stFrame()
{
    // mt.lock();
    uv_rwlock_wrlock(&rwlock_t);
    for (int x = 0; x < queue.size(); ++x)
    {
        if (queue[x])
        {
            delete queue[x];
            queue[x] = nullptr;
        }
    }
    // mt.unlock();
    uv_rwlock_wrunlock(&rwlock_t);

    clear();


    uv_rwlock_destroy(&rwlock_t);

    SInfo << "~stFrame()";
}

stFrame::stFrame()
{
    uv_rwlock_init(&rwlock_t);
}

void stFrame::clear()
{
    ++Ifrm;

    // mt.lock();

    uv_rwlock_wrlock(&rwlock_t);

    mapIt.clear();
    ncount = 0;
    // mt.unlock();

    uv_rwlock_wrunlock(&rwlock_t);

    // SInfo << "clear()";
}


void stFrame::pushsps(std::vector< uint8_t> & sps)
{
    m_sps = sps;
}

void stFrame::pushpps(std::vector< uint8_t> & pps)
{
    m_pps = pps;
}


void stFrame::push(Store *store)
{
    uv_rwlock_wrlock(&rwlock_t);

    // mt.lock();

    if (Ifrm > 1)
    {
        delete queue[ncount];
        queue[ncount] = nullptr;
    }

    queue[ncount] = store;

    ncount++;
    // mt.unlock();

    uv_rwlock_wrunlock(&rwlock_t);

    // SInfo << "push() " <<  queue.size();
}

bool stFrame::key(long ptr, Store *s)
{
   bool ret =  next( ptr, s);
    
    if(!s->idr)
    {
         SWarn << "Not a possible state";
    }
    
   
   char * newBuffer = new char[s->payload.size() + m_sps.size() + m_pps.size()];
   memcpy(newBuffer, &m_sps[0], m_sps.size());
   memcpy(newBuffer + m_sps.size(), &m_pps[0], m_pps.size());
   memcpy(newBuffer + m_sps.size() + m_pps.size(), s->payload.data(), s->payload.size());
   
    s->payload.resize(m_sps.size() + m_pps.size()+ s->payload.size() );
    memcpy( &s->payload[0], newBuffer, m_sps.size() + m_pps.size()+ s->payload.size() );
    delete [] newBuffer;
   return ret;
}
bool stFrame::next(long ptr, Store *s)
{
    // mt.lock();

    uv_rwlock_wrlock(&rwlock_t);

    if (!queue.size())
    {
        // mt.unlock();
        uv_rwlock_wrunlock(&rwlock_t);
        SWarn << "Native queue size is zero";
        return false;
    }

    if (mapIt.find(ptr) == mapIt.end()) mapIt[ptr] = 0;

    int n = (mapIt[ptr])++;
    if (n <= ncount)
        *s = *queue[n];
    else
    {
        uv_rwlock_wrunlock(&rwlock_t);

        SWarn << "Not possible state";
        // mt.unlock();
        return false;
    }


    uv_rwlock_wrunlock(&rwlock_t);
    // mt.unlock();
    return true;
}


}  // namespace web_rtc
}  // namespace base
