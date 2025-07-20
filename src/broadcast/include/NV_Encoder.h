

#ifndef NV_Encoder_H
#define NV_Encoder_H

#include "H264_Encoder.h"

namespace base {
    namespace web_rtc {
class NV_Encoder : public H264_Encoder {

 public:
  NV_Encoder(en_EncType &encType);                         /* pass in a callback function that is called whenever we decoded a video frame, make sure to call `readFrame()` repeatedly */
  ~NV_Encoder();                                                                       /* d'tor, cleans up the allocated objects and closes the codec context */
   
  void encodeFrame(AVCodecContext *dec_ctx, AVFrame *hw_frame, int w , int h );

  int lastFrormat{0};
 
};
    }}
#endif
