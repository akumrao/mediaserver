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


namespace base {
namespace wrtc {
    

  class FRawFrameBuffer : public webrtc::VideoFrameBuffer
  {
    public:
	 FRawFrameBuffer(wrtc::Peer *peer, uint frameNo);

	//
	// webrtc::VideoFrameBuffer interface
	//
	Type type() const override
	{
		return Type::kNative;
	}

	virtual int width() const override
	{
		return 720;
	}

	virtual int height() const override
	{
		return 576;
	}

	rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override
	{
                assert(1);
		return nullptr;
	}

	//
	// Own methods
	//
	  wrtc::Peer * GetPlayer() 
	{
		return peer;
	}
        
//        void push( BasicFrame & f );
//        
//        BasicFrame&  pop();
        
      

        uint frameNo;	
private:
    
        wrtc::Peer *peer;
	//BasicFrame bframe;
        
	
};

} } // namespace wrtc


#endif /* RAWVIDEOFRAME_H */

