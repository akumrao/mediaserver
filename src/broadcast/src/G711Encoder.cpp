#include <webrtc/G711Encoder.h>

/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <cstdint>

#include "modules/audio_coding/codecs/g711/g711_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/time_utils.h"
#include <iostream>

namespace base {
namespace wrtc {


    
 AudioEncoderPcmUCAM::AudioEncoderPcmUCAM(const Config& config): AudioEncoderPcm(config, kSampleRateHz),  payload_type_(config.payload_type)
 {
      
       in_file = fopen("/var/tmp/audio/out.ul", "rb");
       
     //  frame_buf = new uint8_t[3000];
  
  }

//      
//webrtc::AudioEncoder::EncodedInfo  AudioEncoderPcmUCAM::Encode(
//    uint32_t rtp_timestamp,
//    rtc::ArrayView<const int16_t> audio,
//    rtc::Buffer* encoded) {
////  TRACE_EVENT0("webrtc", "AudioEncoder::Encode");
////  RTC_CHECK_EQ(audio.size(),
//  //             static_cast<size_t>(NumChannels() * SampleRateHz() / 100));
//
//  const size_t old_size = encoded->size();
//  
//  webrtc::AudioEncoder::EncodedInfo info = EncodeImpl(rtp_timestamp, audio, encoded);
// // RTC_CHECK_EQ(encoded->size() - old_size, info.encoded_bytes);
//  return info;
//} 
 
 
webrtc::AudioEncoder::EncodedInfo AudioEncoderPcmUCAM::EncodeImpl(  uint32_t rtp_timestamp,   rtc::ArrayView<const int16_t> audio,   rtc::Buffer* encoded) 
{
    
    std::cout << " rtp_timestamp "  << rtp_timestamp << std::endl << std::flush;
    
    
     uint32_t rtp_timestamp1 = rtp_timestamp * 2;
//     
//      first_frame_
//          ? rtp_timestamp
//          : last_rtp_timestamp_ +   160;
// // last_timestamp_ = input_data.input_timestamp;
//  last_rtp_timestamp_ = rtp_timestamp1;
//  first_frame_ = false;
//  
  
    
    //  first_timestamp_in_buffer_ = rtp_timestamp;
  //}
//  speech_buffer_.insert(speech_buffer_.end(), audio.begin(), audio.end());
//  if (speech_buffer_.size() < full_frame_samples_) {
//    return EncodedInfo();
//  }
//  RTC_CHECK_EQ(speech_buffer_.size(), full_frame_samples_);
  webrtc::AudioEncoder::EncodedInfo info;
  info.encoded_timestamp = rtp_timestamp1;
  info.payload_type = payload_type_;
  info.encoded_bytes = encoded->AppendData(     full_frame_samples_ * BytesPerSample(),   [&](rtc::ArrayView<uint8_t> encoded) 
  {
        return EncodeCall(&audio[0], full_frame_samples_,                     encoded.data());     });

  info.encoder_type = GetCodecType();
  return info;


}

//webrtc::AudioEncoder::EncodedInfo AudioEncoderPcmUCAM::EncodeImpl(  uint32_t rtp_timestamp,   rtc::ArrayView<const int16_t> audio,   rtc::Buffer* encoded) 
//{
//  if (speech_buffer_.empty()) {
//    first_timestamp_in_buffer_ = rtp_timestamp;
//  }
//  speech_buffer_.insert(speech_buffer_.end(), audio.begin(), audio.end());
//  if (speech_buffer_.size() < full_frame_samples_) {
//    return EncodedInfo();
//  }
//  RTC_CHECK_EQ(speech_buffer_.size(), full_frame_samples_);
//  EncodedInfo info;
//  info.encoded_timestamp = first_timestamp_in_buffer_;
//  info.payload_type = payload_type_;
//  info.encoded_bytes = encoded->AppendData(
//      full_frame_samples_ * BytesPerSample(),
//      [&](rtc::ArrayView<uint8_t> encoded) {
//        return EncodeCall(&speech_buffer_[0], full_frame_samples_,
//                          encoded.data());
//      });
//  speech_buffer_.clear();
//  info.encoder_type = GetCodecType();
//  return info;
//  
//  
//  }


size_t AudioEncoderPcmUCAM::EncodeCall(const int16_t* audio,
                                    size_t input_len,
                                    uint8_t* encoded) {
    
   // int sz =  WebRtcG711_EncodeU(audio, input_len, encoded);
    
//    memcpy(frame_buf, audio, 160  );
//    
//    char *p = (char*)  frame_buf;
//    
//    char p1 =   frame_buf[0];
//    char p22 =   frame_buf[81];
//    char p12 =   frame_buf[82];
//    char p224 =   frame_buf[83];
//    char p62 =   frame_buf[159];
    
    
    int sz;
    memcpy(encoded, audio, input_len  );
    ////////////////////////////////////////////////////
  
    if(_nextFrameTime )
    {
      
      if(diff == 0 )  
      diff = rtc::TimeMillis() - _nextFrameTime;
      else
      {
          diff = (diff + (rtc::TimeMillis() - _nextFrameTime))/2;
       
          std::cout << "sleep time " << diff << std::endl << std::flush; 
      }
        
    }
    
    _nextFrameTime = rtc::TimeMillis();
     
    
     
    
   // std::cout <<  ncount++ << " bad " <<  (int) encoded[0] << " " <<  (int) encoded[159] <<  std::endl << std::flush;
     
//    for( int x =0; x < 160 ; ++x)
//    {
//        if( encoded[x] !=  x)
//        {
//            std::cout <<  ++ncount << " bad " << std::endl << std::flush;
//            
//        }
//    }
//    
    

     
//    if (( sz = fread(encoded, 1, input_len, in_file)) <= 0)
//    {
//         printf("Failed to read raw data! \n");
//
////            audio = frame_buf;
//
//     }else if(feof(in_file))
//     {
//         fseek(in_file, 0, SEEK_SET);
//          std::cout << " end end" << std::endl << std::flush;
//     }
//
//    
//    if(sz != input_len)
//    {
//         std::cout << " second bad end " << std::endl << std::flush;
//    }
    
//    for(int x =0; x < input_len ; ++x  )
//    {
//        
//        if( encoded[x] !=  frame_buf[x])
//        {
//            std::cout <<  ++ncount << " bad " << std::endl << std::flush;
//            
//        }
//        
//    }
    


//    encoded = frame_buf;
    return input_len;
   


}

size_t AudioEncoderPcmUCAM::BytesPerSample() const {
  return 1;
}

webrtc::AudioEncoder::CodecType AudioEncoderPcmUCAM::GetCodecType() const {
  return AudioEncoder::CodecType::kPcmU;
}

}}  // namespace webrtc
