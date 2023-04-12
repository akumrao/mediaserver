/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef AUDIO_ENCODER_G711_H_
#define AUDIO_ENCODER_G711_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_encoder.h"
#include "api/audio_codecs/audio_format.h"
#include "rtc_base/system/rtc_export.h"
#include "api/audio_codecs/g711/audio_encoder_g711.h"

namespace base {
namespace wrtc {
    

// G711 encoder API for use as a template parameter to
// CreateAudioEncoderFactory<...>().
struct  AudioEncoderG711_Cam {
  struct Config {
    enum class Type { kPcmU, kPcmA };
    bool IsOk() const {
      return (type == Type::kPcmU || type == Type::kPcmA) &&
             frame_size_ms > 0 && frame_size_ms % 10 == 0 && num_channels >= 1;
    }
    Type type = Type::kPcmU;
    int num_channels = 1;
    int frame_size_ms = 20;
  };
  static absl::optional< AudioEncoderG711_Cam::Config> SdpToConfig(
      const webrtc::SdpAudioFormat& audio_format);
  static void AppendSupportedEncoders(std::vector< webrtc::AudioCodecSpec>* specs);
  static  webrtc::AudioCodecInfo QueryAudioEncoder(const Config& config);
  static std::unique_ptr< webrtc::AudioEncoder> MakeAudioEncoder(
      const Config& config,
      int payload_type,
      absl::optional< webrtc::AudioCodecPairId> codec_pair_id = absl::nullopt);
};

}
}

#endif  // API_AUDIO_CODECS_G711_AUDIO_ENCODER_G711_H_
