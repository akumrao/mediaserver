/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   rawVideoFrame.h
 * Author: root
 *
 * Created on November 4, 2021, 10:35 PM
 */

#ifndef RAWVIDEOFRAME_H
#define RAWVIDEOFRAME_H
#include "common_video/include/video_frame_buffer.h"
#include "webrtc/peer.h"
#include <assert.h>
#include <deque>
#include <uv.h>

namespace base
{
namespace web_rtc
{

typedef struct DecodeContext
{
    AVBufferRef *hw_device_ref;
    int NVSurface{0};
} DecodeContext;

class Store
{
public:
    Store(unsigned char *p, int sz, int w, int h, uint64_t vframecount, bool idr)
        : width_(w),
          height_(h),
          vframecount(vframecount),
          idr(idr)
    {
        payload.reserve(360000);  // __memmove_avx_unaligned segfaul
             
        // if we reserve the std::vector first and the problem is gone, big chances that this is the reason behind.
        //there might be a memeory relocation problem, say: one thread insert a value, causing the vector to enlarge it's size and relocate all it's elements in another memory range. However, another thread stll owns some ref(pointer) to the original memory range of this vector, this might cause the seg fault.

        payload.resize(sz);
        memcpy(payload.data(), p, sz);

        if (idr)
            _frameType = webrtc::VideoFrameType::kVideoFrameKey;
        else
            _frameType = webrtc::VideoFrameType::kVideoFrameDelta;
    }

    Store() {
        payload.reserve(360000); 
    }

    webrtc::VideoFrameType _frameType{webrtc::VideoFrameType::kVideoFrameDelta};
    std::vector<uint8_t> payload;
    int width_;
    int height_;

    uint64_t vframecount{0};

    bool idr;
    //      uint32_t timestamp ;
    //      int64_t ntp_time_ms_;
    //      int64_t capture_time_ms_ ;
};

class stFrame
{
public:
    stFrame();

    void push(Store *store);

    ~stFrame();

    //            int size() {
    //                int sz = 0;
    //                mt.lock();
    //                sz = queue.size();
    //                mt.unlock();
    //
    //                return sz;
    //            }

    bool next(long ptr, Store *s);
    bool key(long ptr, Store *s);
    
    void pushsps(std::vector< uint8_t> & sps);

    void pushpps(std::vector< uint8_t> & pps);


    void clear();

private:
    uv_rwlock_t rwlock_t;
    // std::mutex mt;
    std::map<int, Store *> queue;

    std::map<long, int> mapIt;

    int ncount{0};

    int Ifrm{0};
    std::vector< uint8_t> m_sps;
    std::vector< uint8_t> m_pps;
    
};

class NULLEncBuffer : public webrtc::VideoFrameBuffer
{
public:
    // FRawFrameBuffer( std::vector<uint8_t> & f , int w ,  int h, uint frameNo, bool idr, uint32_t fps);
    NULLEncBuffer(stFrame *qframe, int w, int h, uint32_t fps, std::string &txt);

    Type type() const override { return Type::kNative; }

    virtual int width() const override { return width_; }

    virtual int height() const override { return height_; }

    rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override
    {
        assert(1);
        return nullptr;
    }

    //        std::vector<uint8_t> & getFrame(  )
    //        {
    //            return payload1;
    //        }

    uint32_t fps;
    int width_{0};
    int height_{0};

    stFrame *qframe{nullptr};

    /// std::vector<uint8_t> payload1;
    // webrtc::VideoFrameType _frameType1{webrtc::VideoFrameType::kVideoFrameDelta};
    std::string txt;

private:
};

class H264FrameBuffer : public webrtc::VideoFrameBuffer
{
public:
    // FRawFrameBuffer( std::vector<uint8_t> & f , int w ,  int h, uint frameNo, bool idr, uint32_t fps);
    H264FrameBuffer(AVCodecContext *dec_ctx, AVFrame *qframe, int w, int h, std::string &txt);

    ~H264FrameBuffer();

    Type type() const override { return Type::kNative; }

    virtual int width() const override { return width_; }

    virtual int height() const override { return height_; }

    rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override
    {
        assert(1);
        return nullptr;
    }

    //        std::vector<uint8_t> & getFrame(  )
    //        {
    //            return payload1;
    //        }

    uint32_t fps;
    int width_{0};
    int height_{0};

    // int frmNo;

    AVFrame *qframe{nullptr};

    AVCodecContext *dec_ctx{nullptr};

    /// std::vector<uint8_t> payload1;
    // webrtc::VideoFrameType _frameType1{webrtc::VideoFrameType::kVideoFrameDelta};
    std::string txt;

private:
};


}  // namespace web_rtc
}  // namespace base


#endif /* RAWVIDEOFRAME_H */
