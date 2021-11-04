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
	 FRawFrameBuffer(Frame *frame):frame(frame)
	{
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
		return 800;
	}

	virtual int height() const override
	{
		return 600;
	}

	rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override
	{
                assert(1);
		return nullptr;
	}

	//
	// Own methods
	//
	Frame* GetBuffer() 
	{
		return frame;
	}

	
private:
	Frame *frame;
	
};


#endif /* RAWVIDEOFRAME_H */

