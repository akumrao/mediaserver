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
#include "muxframe.h"

  class FRawFrameBuffer : public webrtc::VideoFrameBuffer
  {
    public:
	 FRawFrameBuffer(Frame *frame, uint frameNo):frameNo(frameNo)
	{
              BasicFrame *basic_frame = static_cast<BasicFrame *>(frame);
              bframe.payload = basic_frame->payload;
	}

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
	BasicFrame& GetBuffer() 
	{
		return bframe;
	}

        uint frameNo;	
private:
	BasicFrame bframe;
        
	
};


#endif /* RAWVIDEOFRAME_H */

