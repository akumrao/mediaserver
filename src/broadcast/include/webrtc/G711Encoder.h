/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef CODECS_G711_AUDIO_ENCODER_PCM_H_
#define CODECS_G711_AUDIO_ENCODER_PCM_H_

#include <vector>

#include "modules/audio_coding/codecs/g711/audio_encoder_pcm.h"


namespace base {
namespace wrtc {


class AudioEncoderPcmUCAM  : public webrtc::AudioEncoderPcm {
 public:
  struct Config : public AudioEncoderPcm::Config {
    Config() : AudioEncoderPcm::Config(0) {}
  };

  explicit AudioEncoderPcmUCAM(const Config& config);
  


  webrtc::AudioEncoder::EncodedInfo EncodeImpl(  uint32_t rtp_timestamp,   rtc::ArrayView<const int16_t> audio,   rtc::Buffer* encoded) ;
  
  
  
  //webrtc::AudioEncoder::EncodedInfo Encode(  uint32_t rtp_timestamp,    rtc::ArrayView<const int16_t> audio,    rtc::Buffer* encoded);
  


 protected:
  size_t EncodeCall(const int16_t* audio,
                    size_t input_len,
                    uint8_t* encoded) override;

  size_t BytesPerSample() const override;

  AudioEncoder::CodecType GetCodecType() const override;

 private:
  static const int kSampleRateHz = 8000;
  RTC_DISALLOW_COPY_AND_ASSIGN(AudioEncoderPcmUCAM);
  
    FILE* in_file;
     
    uint8_t  frame_buf[160];
     
    int ncount{0};
     
    int64_t _nextFrameTime{0};
     
    int64_t diff{0}; 
    
    
    int payload_type_;
    
    int full_frame_samples_{160};
    
    
    
 // bool first_frame_ {true};
  //uint32_t last_timestamp_ ;
  uint32_t last_rtp_timestamp_ ;

     
};

} } // namespace webrtc

#endif 
