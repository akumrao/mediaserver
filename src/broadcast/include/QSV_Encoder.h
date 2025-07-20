

#ifndef QSV_Encoder_H
#define QSV_Encoder_H

#include "H264_Encoder.h"

namespace base {
    namespace web_rtc {
class QSV_Encoder : public H264_Encoder {

 public:
  QSV_Encoder(en_EncType &encType);                         /* pass in a callback function that is called whenever we decoded a video frame, make sure to call `readFrame()` repeatedly */
  ~QSV_Encoder();                                                                       /* d'tor, cleans up the allocated objects and closes the codec context */
   
  void encodeFrame(AVCodecContext *dec_ctx, AVFrame *hw_frame, int w , int h );
  
  int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,   AVCodecContext *enc_ctx, const char *filter_spec);
  
  int init_filters(AVCodecContext *dec_ctx);

 // int lastFrormat{0};
 
};
}}
#endif
