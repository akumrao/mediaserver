/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "ffparse.h"
#include <thread>
#include "intreadwrite.h"


// #include "ff/ff.h"
// #include "ff/mediacapture.h"
// #include "base/define.h"
// #include "base/test.h"
#include "tools.h"
#include "fmp4.h"

extern "C"
{
//#include <libavutil/timestamp.h>
#include <avformat.h>
#include <avcodec.h>
#include <channel_layout.h>    
#include <mathematics.h>
}

#define MAX_CHUNK_SIZE 10240*8
// maximum send buffer 262144  =1024 *256

#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576




#define IOBUFSIZE 40960
//40960*6


/////////////////////////////////// h264 parser//////////////////////////////////////////////

#define HAVE_FAST_UNALIGNED 1

#define HAVE_FAST_64BIT 1

enum {
    H264_NAL_SLICE           = 1,
    H264_NAL_DPA             = 2,
    H264_NAL_DPB             = 3,
    H264_NAL_DPC             = 4,
    H264_NAL_IDR_SLICE       = 5,
    H264_NAL_SEI             = 6,
    H264_NAL_SPS             = 7,
    H264_NAL_PPS             = 8,
    H264_NAL_AUD             = 9,
    H264_NAL_END_SEQUENCE    = 10,
    H264_NAL_END_STREAM      = 11,
    H264_NAL_FILLER_DATA     = 12,
    H264_NAL_SPS_EXT         = 13,
    H264_NAL_AUXILIARY_SLICE = 19,
};

typedef struct H2645NAL {
    uint8_t *rbsp_buffer;
    unsigned int rbsp_buffer_size;

    int size;
    const uint8_t *data;

    /**
     * Size, in bits, of just the data, excluding the stop bit and any trailing
     * padding. I.e. what HEVC calls SODB.
     */
    int size_bits;

    int raw_size;
    const uint8_t *raw_data;

  //  GetBitContext gb;

    /**
     * NAL unit type
     */
    int type;

    /**
     * HEVC only, nuh_temporal_id_plus_1 - 1
     */
    int temporal_id;

    int skipped_bytes;
    int skipped_bytes_pos_size;
    int *skipped_bytes_pos;
    /**
     * H.264 only, nal_ref_idc
     */
    int ref_idc;
} H2645NAL;


static inline int get_nalsize(int nal_length_size, const uint8_t *buf,
                              int buf_size, int *buf_index, void *logctx)
{
    int i, nalsize = 0;

    if (*buf_index >= buf_size - nal_length_size) {
        // the end of the buffer is reached, refill it
        return AVERROR(EAGAIN);
    }

    for (i = 0; i < nal_length_size; i++)
        nalsize = ((unsigned)nalsize << 8) | buf[(*buf_index)++];
    if (nalsize <= 0 || nalsize > buf_size - *buf_index) {
        av_log(logctx, AV_LOG_ERROR,
               "Invalid NAL unit size (%d > %d).\n", nalsize, buf_size - *buf_index);
        return AVERROR_INVALIDDATA;
    }
    return nalsize;
}



const uint8_t *avpriv_find_start_code(const uint8_t * p,
                                      const uint8_t *end,
                                      uint32_t * state)
{
    int i;

    assert(p <= end);
    if (p >= end)
        return end;

    for (i = 0; i < 3; i++) {
        uint32_t tmp = *state << 8;
        *state = tmp + *(p++);
        if (tmp == 0x100 || p == end)
            return p;
    }

    while (p < end) {
        if      (p[-1] > 1      ) p += 3;
        else if (p[-2]          ) p += 2;
        else if (p[-3]|(p[-1]-1)) p++;
        else {
            p++;
            break;
        }
    }

    p = FFMIN(p, end) - 4;
    *state = AV_RB32(p);

    return p + 4;
}

static inline int find_start_code(const uint8_t *buf, int buf_size,
                           int buf_index, int next_avc)
{
    uint32_t state = -1;

    buf_index = avpriv_find_start_code(buf + buf_index, buf + next_avc + 1, &state) - buf - 1;

    return FFMIN(buf_index, buf_size);
}

#define MAX_MBPAIR_SIZE (256*1024) // a tighter bound could be calculated if someone cares about a few bytes

int ff_h2645_extract_rbsp(const uint8_t *src, int length,
                          H2645NAL *nal, int small_padding)
{
    int i, si, di;
    uint8_t *dst;
    int64_t padding = small_padding ? 0 : MAX_MBPAIR_SIZE;

    nal->skipped_bytes = 0;
#define STARTCODE_TEST                                                  \
        if (i + 2 < length && src[i + 1] == 0 && src[i + 2] <= 3) {     \
            if (src[i + 2] != 3 && src[i + 2] != 0) {                   \
                /* startcode, so we must be past the end */             \
                length = i;                                             \
            }                                                           \
            break;                                                      \
        }
#if HAVE_FAST_UNALIGNED
#define FIND_FIRST_ZERO                                                 \
        if (i > 0 && !src[i])                                           \
            i--;                                                        \
        while (src[i])                                                  \
            i++
#if HAVE_FAST_64BIT
    for (i = 0; i + 1 < length; i += 9) {
        if (!((~AV_RN64A(src + i) &
               (AV_RN64A(src + i) - 0x0100010001000101ULL)) &
              0x8000800080008080ULL))
            continue;
        FIND_FIRST_ZERO;
        STARTCODE_TEST;
        i -= 7;
    }
#else
    for (i = 0; i + 1 < length; i += 5) {
        if (!((~AV_RN32A(src + i) &
               (AV_RN32A(src + i) - 0x01000101U)) &
              0x80008080U))
            continue;
        FIND_FIRST_ZERO;
        STARTCODE_TEST;
        i -= 3;
    }
#endif /* HAVE_FAST_64BIT */
#else
    for (i = 0; i + 1 < length; i += 2) {
        if (src[i])
            continue;
        if (i > 0 && src[i - 1] == 0)
            i--;
        STARTCODE_TEST;
    }
#endif /* HAVE_FAST_UNALIGNED */

    if (i >= length - 1 && small_padding) { // no escaped 0
        nal->data     =
        nal->raw_data = src;
        nal->size     =
        nal->raw_size = length;
        return length;
    } else if (i > length)
        i = length;

    av_fast_padded_malloc(&nal->rbsp_buffer, &nal->rbsp_buffer_size,
                          length + padding);
    if (!nal->rbsp_buffer)
        return AVERROR(ENOMEM);

    dst = nal->rbsp_buffer;

    memcpy(dst, src, i);
    si = di = i;
    while (si + 2 < length) {
        // remove escapes (very rare 1:2^22)
        if (src[si + 2] > 3) {
            dst[di++] = src[si++];
            dst[di++] = src[si++];
        } else if (src[si] == 0 && src[si + 1] == 0 && src[si + 2] != 0) {
            if (src[si + 2] == 3) { // escape
                dst[di++] = 0;
                dst[di++] = 0;
                si       += 3;

                if (nal->skipped_bytes_pos) {
                    nal->skipped_bytes++;
                    if (nal->skipped_bytes_pos_size < nal->skipped_bytes) {
                        nal->skipped_bytes_pos_size *= 2;
                        assert(nal->skipped_bytes_pos_size >= nal->skipped_bytes);
                        av_reallocp_array(&nal->skipped_bytes_pos,
                                nal->skipped_bytes_pos_size,
                                sizeof(*nal->skipped_bytes_pos));
                        if (!nal->skipped_bytes_pos) {
                            nal->skipped_bytes_pos_size = 0;
                            return AVERROR(ENOMEM);
                        }
                    }
                    if (nal->skipped_bytes_pos)
                        nal->skipped_bytes_pos[nal->skipped_bytes-1] = di - 1;
                }
                continue;
            } else // next start code
                goto nsc;
        }

        dst[di++] = src[si++];
    }
    while (si < length)
        dst[di++] = src[si++];

nsc:
    memset(dst + di, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    nal->data = dst;
    nal->size = di;
    nal->raw_data = src;
    nal->raw_size = si;
    return si;
}

/*888************************************ h264 end */

namespace base {
    namespace fmp4 {


        // based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html


        FFParse::FFParse(  const char* audioFile, const char* videofile,  FrameFilter *fragmp4_muxer , FrameFilter *info , FrameFilter *txt ):  fragmp4_muxer(fragmp4_muxer) , info(info) , txt(txt)  {

     
            
            fileAudio = fopen(audioFile,"rb");
            if(fileAudio){
                av_log(NULL,AV_LOG_INFO,"open file success \n");
            }else{
                 SError << "can't open file! " <<  audioFile;
                return;
            }
           
            
            fileVideo = fopen(videofile,"rb");
            if(fileVideo){
                av_log(NULL,AV_LOG_INFO,"open file success \n");
            }else{
                SError << "can't open file! " <<  videofile;
            }
                
          
             //FILE *fp_in, *fp_out;
             

        }

        FFParse::~FFParse() {
            SInfo << "~FFParse( )";
            fclose(fileAudio);
            fclose(fileVideo);
        }

        /*
        void FFParse::run() {

            int64_t startTime = time::hrtime();



            std::ifstream bunnyFile;
            bunnyFile.open("/var/tmp/test.mp4", std::ios_base::in | std::ios_base::binary);

            char buf[ MAX_CHUNK_SIZE];

            memset(buf, 'A', MAX_CHUNK_SIZE);

            while (!stopped()) {


                // dc->sendDataMsg("ravind");

                pc->sendDataBinary((const uint8_t *) buf, MAX_CHUNK_SIZE);

                do {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                     SInfo << "slee for 5 mlsec";
                } while (pc->data_channel_->buffered_amount() > highWaterMark);

                // while( pc->data_channel_->buffered_amount()  > highWaterMark )
                // std::this_thread::sleep_for(std::chrono::milliseconds(10));

                int64_t lastTimestamp = time::hrtime(); //nanosecondtime
                auto nsdelay = lastTimestamp - startTime;
                

                SInfo << "Sent message seed MByte " <<  (pc->data_channel_->bytes_sent()*1000)/nsdelay ;
            }





            SInfo << "fmp4 thread exit";

            // fileName = "/var/tmp/videos/test.mp4";
            fileName = "/var/tmp/kunal720.mp4";
            //fmp4(fileName.c_str(), "fragTmp.mp4");
            //fmp4(fileName.c_str());
        }
         */

        void FFParse::run() {

            stream_index = 0;
            //audio only
//            if (parseAACHeader()) {
//                 ++stream_index;
//                parseAACContent();
//            }
//            return;

            //video only
//            if (parseH264Header()) {
//                ++stream_index;
//                parseH264Content();
//            }
//            return;

            //Vidoe and Audio mux

            while(  !stopped())
            { 
               // reopen();
                keeprunning = true;
                stream_index = 0;
                
                fragmp4_muxer->deActivate();
                if(!mute )
                {   

                    if (parseH264Header()) {
                        ++stream_index;
                        if (parseAACHeader()) {
                            ++stream_index;
                            parseMuxContent();
                        }
                       
                    }
               }
               else
               {   

                    if (parseH264Header()) {
                    ++stream_index;
                    parseH264Content();
                    }

               }
                

            }
////            
           // startAudio();
            //parseAACHeader("/var/tmp/songs/hindi.pcm");
            
            //fmp4("/experiment/fmp4/test.264", "fragTmp.mp4");
//            
//                        std::ifstream bunnyFile;
//                        bunnyFile.open("/tmp/output2.mp4", std::ios_base::in | std::ios_base::binary);
//            
//                        char buf[ MAX_CHUNK_SIZE];
//            
//                        while (bunnyFile.good() && !stopped() ) {
//                          bunnyFile.read(buf,  MAX_CHUNK_SIZE);
//                          int nRead = bunnyFile.gcount();
//                          if (nRead > 0) {
//                           // dc->sendDataMsg("ravind");
//            
//                            pc->sendDataBinary((const uint8_t *)buf, nRead);
//            
//                            do
//                            {
//                                std::this_thread::sleep_for(std::chrono::milliseconds(15));
//                            } while( pc->data_channel_->buffered_amount()  >  highWaterMark );
//                    
//                            // while( pc->data_channel_->buffered_amount()  >  12 * 1024 * 1024 )
//                             // std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                    
//                          }
//            
//                          SInfo << "Sent message of size " << nRead ;
//                        }


            SInfo << "fmp4 thread exit";

            // fileName = "/var/tmp/videos/test.mp4";
         
            //fmp4(fileName.c_str(), "fragTmp.mp4");
            //fmp4(fileName.c_str());
        }


 

        
        bool FFParse::parseAACHeader() {
          
            SetupFrame        setupframe;  ///< This frame is used to send subsession information
          
            basicaudioframe.media_type           =AVMEDIA_TYPE_AUDIO;
            basicaudioframe.codec_id             =AV_CODEC_ID_AAC;
            basicaudioframe.stream_index     =stream_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_AUDIO;
            setupframe.codec_id             =AV_CODEC_ID_AAC;   // what frame types are to be expected from this stream
            setupframe.stream_index     =stream_index;
            setupframe.mstimestamp          = CurrentTime_milliseconds();
            // send setup frame
            
            //info->run(&setupframe);
            fragmp4_muxer->run(&setupframe);
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            const AVCodec *codec;
            AVCodecContext *c = nullptr ;
           
           
                
            ///////////////////////////////////////////////////////
          ///  AVFormatContext *oc;
            AVDictionary *opt = NULL;
          //  av_dict_set(&opt, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof", 0);
            
    
            //codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            codec = avcodec_find_encoder_by_name("libfdk_aac"); //Specify the use of file encoding type
            if (!codec) {
                fprintf(stderr, "Codec not found\n");
               return false ;
            }

            c = avcodec_alloc_context3(codec);
            if (!c) {
                fprintf(stderr, "Could not allocate audio codec context\n");
                return  false;
            }

            /* put sample parameters */
            c->bit_rate = 64000;

            /* check that the encoder supports s16 pcm input */
            c->sample_fmt = AV_SAMPLE_FMT_S16;


            /* select other audio parameters supported by the encoder */

            c->sample_rate = SAMPLINGRATE;//  

            c->channel_layout = AV_CH_LAYOUT_STEREO;//select_channel_layout(codec);
            c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
            c->profile = FF_PROFILE_AAC_LOW;
            
            //if (oc->oformat->flags & AVFMT_GLOBALHEADER)
             c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            
               
            /* open it */
            if (avcodec_open2(c, codec, &opt) < 0) {
                fprintf(stderr, "Could not open codec\n");
               return false;
            }
             
             startTime=  setupframe.mstimestamp;
                
             int extrasize = c->extradata_size;
             basicaudioframe.payload.resize(extrasize);
             memcpy( basicaudioframe.payload.data(),  c->extradata, extrasize) ;
             basicaudioframe.codec_id = codec->id;
             basicaudioframe.mstimestamp = startTime ;
             fragmp4_muxer->run(&basicaudioframe);
             basicaudioframe.payload.resize(basicaudioframe.payload.capacity());
             audioContext = c;
             
             return true;
        }
        
        
        void FFParse:: parseAACContent()
        {
            
            AVPacket audiopkt;
            
            int ret, got_output;
             
            AVFrame *frame;
             
                    /* frame containing input raw audio */
            frame = av_frame_alloc();
            if (!frame) {
                fprintf(stderr, "Could not allocate audio frame\n");
                return ;
            }

            frame->nb_samples     = audioContext->frame_size;
            frame->format         = audioContext->sample_fmt;
            frame->channel_layout = audioContext->channel_layout;

            /* allocate the data buffers */
            ret = av_frame_get_buffer(frame, 0);
            if (ret < 0) {
                fprintf(stderr, "Could not allocate audio data buffers\n");
               return ;
            }
    
            int audiosize = av_samples_get_buffer_size(NULL, audioContext->channels,audioContext->frame_size,audioContext->sample_fmt, 1);
            uint8_t* frame_audobuf = (uint8_t *)av_malloc(audiosize);

            
            long framecount =0;
            while(!stopped())
            {   
               if (fread(frame_audobuf, 1, audiosize, fileAudio) <= 0){
                    printf("Failed to read raw data! \n");
                    break;


                }else if(feof(fileAudio)){
                    
                     if (fseek(fileAudio, 0, SEEK_SET))
                    return;
                    continue;
                    
                }

               av_init_packet(&audiopkt);
                audiopkt.data = NULL; // packet data will be allocated by the encoder
                audiopkt.size = 0;


               ret = av_frame_make_writable(frame);
                if (ret < 0)
                    return ;

               frame->data[0] = frame_audobuf;  //PCM Data
                //pFrame->pts=i*100;
               // encode(encodeContext,pFrame,&pkt);
                //i++;

                //if (c->oformat->flags & AVFMT_GLOBALHEADER)
	       //c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


                ret = avcodec_encode_audio2(audioContext, &audiopkt, frame, &got_output);
                if (ret < 0) {
                    fprintf(stderr, "Error encoding audio frame\n");
                    return ;
                }
                if (got_output) 
                {
                  
                    basicaudioframe.copyFromAVPacket(&audiopkt);
                  
                    basicaudioframe.mstimestamp = startTime + framecount;
                   
 
                    framecount = framecount + AUDIOSAMPLE ;
                    fragmp4_muxer->run(&basicaudioframe);

                    basicaudioframe.payload.resize(basicaudioframe.payload.capacity());
                                       
                    av_packet_unref(&audiopkt);
                    
                     std::this_thread::sleep_for(std::chrono::microseconds(21000));
                    
                }
            }
                    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            
           // fclose(fout);
            
           // av_dict_free(&opt);
            av_free(frame_audobuf);
            av_frame_free(&frame);
            // if you open any thing at ffmpeg do not forget to close it
            avcodec_close(audioContext);
            avcodec_free_context(&audioContext);
        }
        
        
        long FFParse::get_nal_size(uint8_t *buf, long size,  uint8_t **poutbuf, int *poutbuf_size) {
            long pos = 3;
            while ((size - pos) > 3) {
                if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 1)
                {
                    *poutbuf = buf;
                    *poutbuf_size  = pos;
                    return pos;
                }
                if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 0 && buf[pos + 3] == 1)
                {
                    *poutbuf = buf;
                    *poutbuf_size  = pos;
                    return pos;
                }
                pos++;
            }
            
             *poutbuf_size  = 0;
            return size;
        }

        
        
        void FFParse::reset() {
        
//            if(mute)
//            {
//                 if(hd)
//                        mediaContent("vhd");
//                    else
//                         mediaContent("vsd"); 
//                
//                SDebug<< " FFParse::reset()"<<  "Stream only Video";
//            }
//            else
//            {
//               if(hd)
//                        mediaContent("avhd");
//                    else
//                         mediaContent("avsd"); 
//                
//               SDebug<< " FFParse::reset()"<<  "Stream both Video & Audio";
//            }
              
            
        }


        void FFParse::restart(bool muteme)
        {
            
            keeprunning = false;
            mute = muteme;
        }
        
        
        
        void FFParse::reopen()
        {
            fclose(fileAudio);
            fileAudio = nullptr;
            
            fclose(fileVideo);
            fileVideo = nullptr;

            if (hd)
            {
                hd= true;
                SInfo << "Open HD";
                
                const char* audioFile = AUDIOFILE1;
                const char* videofile = VIDEOFILE1;
                
                fileAudio = fopen(audioFile,"rb");
                if(fileAudio){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                   // av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
                    SError << "can't open file! " <<  audioFile;
                    return;
                }


                fileVideo = fopen(videofile,"rb");
                if(fileVideo){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                    SError << "can't open file! " <<  videofile;
                }
            }
            else
            {
                hd= false;
                const char* audioFile = AUDIOFILE;
                const char* videofile =VIDEOFILE;
                
            
                SInfo << "Open SD";
                fileAudio = fopen(audioFile,"rb");
                if(fileAudio){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                    av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
                    return;
                }


                fileVideo = fopen(videofile,"rb");
                if(fileVideo){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                    av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
                }
            }
            
        }

        void FFParse::resHD(bool _hd)
        {
            hd =_hd;
           restart(mute);
        }

        
        bool FFParse::parseH264Header() {
            //int ret = 0;
           // AVCodec *codec = NULL;
          //  AVCodecContext *cdc_ctx = NULL;
            AVPacket *pkt = NULL;
            
            H2645NAL nal = { NULL };

            SetupFrame        setupframe;  ///< This frame is used to send subsession information
          

            basicvideoframe.media_type          =AVMEDIA_TYPE_VIDEO;
            basicvideoframe.codec_id            =AV_CODEC_ID_H264;
            basicvideoframe.stream_index     =stream_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_VIDEO;
            setupframe.codec_id             =AV_CODEC_ID_H264;   // what frame types are to be expected from this stream
            setupframe.stream_index     =stream_index;
            setupframe.mstimestamp          = CurrentTime_milliseconds();
            // send setup frame
            
            info->run(&setupframe);
            fragmp4_muxer->run(&setupframe);

  
            if ((pkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return false;
            }

            
             if (fseek(fileVideo, 0, SEEK_END))
               return false;
            long fileSize = (long)ftell(fileVideo);
            if (fileSize < 0)
                return false;
            if (fseek(fileVideo, 0, SEEK_SET))
                return false;
    
            //SInfo << "H264 file Size " << fileSize;
          

           // av_init_packet(pkt);

            const int in_buffer_size=fileSize;
            unsigned char *in_buffer = (unsigned char*)malloc(in_buffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_ptr;
            int cur_size;
            
            startTime=    setupframe.mstimestamp;
            //long int deltatime =   1000000/25;  //25 frames persecs

           
            bool foundsps = false;
            bool foundpps =false;
            
            cur_size = fread(in_buffer, 1, in_buffer_size, fileVideo);
            cur_ptr = in_buffer;

            int buf_index, next_avc;
            
            buf_index     = 0;
            next_avc      =  cur_size;
    
           int state = -1;

            while (true)
            {
                
             //    ret = get_nal_size( cur_ptr, cur_size, &pkt->data, &pkt->size);
                 
              //  const SPS *sps;
                 int src_length, consumed, nalsize = 0;

                if (buf_index >= next_avc) {
                    nalsize = get_nalsize(4, cur_ptr, cur_size, &buf_index, nullptr);
                    if (nalsize < 0)
                        break;
                    next_avc = buf_index + nalsize;
                } else {
                    buf_index = find_start_code(cur_ptr, cur_size, buf_index, next_avc);
                    if (buf_index >= cur_size)
                        break;
                    if (buf_index >= next_avc)
                        continue;
                }
                src_length = next_avc - buf_index;

                state = cur_ptr[buf_index];
                switch (state & 0x1f) {
                case H264_NAL_SLICE:
                case H264_NAL_IDR_SLICE:
                    // Do not walk the whole buffer just to decode slice header
                    if ((state & 0x1f) == H264_NAL_IDR_SLICE || ((state >> 5) & 0x3) == 0) {
                        /* IDR or disposable slice
                         * No need to decode many bytes because MMCOs shall not be present. */
                        if (src_length > 60)
                            src_length = 60;
                    } else {
                        /* To decode up to MMCOs */
                        if (src_length > 1000)
                            src_length = 1000;
                    }
                    break;
                }
                
                consumed = ff_h2645_extract_rbsp(cur_ptr + buf_index, src_length, &nal, 1);
                if (consumed < 0)
                        break;

                buf_index += consumed;;


//                 ret = get_nal_size( cur_ptr, cur_size, &pkt->data, &pkt->size);
//                 if (ret < 4) {
//                    cur_ptr += 1;
//                    cur_size -= 1;
//                    continue;
//                }
//
//
//                // avcodec_decode_video2
//
//                cur_ptr += ret;
//                cur_size -= ret;

 //               if (pkt->size == 0)
 //                   continue;

                basicvideoframe.copyBuf((u_int8_t*) nal.data , nal.size);


                basicvideoframe.mstimestamp = startTime ;
                basicvideoframe.fillPars();

                if(  basicvideoframe.h264_pars.slice_type ==  H264SliceType::sps) //AUD Delimiter
                {
                      foundsps = true;
                      
                      info->run(&basicvideoframe);

                     fragmp4_muxer->run(&basicvideoframe);
                
                }

                else if(  basicvideoframe.h264_pars.slice_type ==  H264SliceType::pps) //AUD Delimiter
                {
                      foundpps = true;
                      
                      info->run(&basicvideoframe);

                    fragmp4_muxer->run(&basicvideoframe);
                
                }
               
                
                
                
                 

                


                basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                //std::this_thread::sleep_for(std::chrono::microseconds(10000));

                if( foundsps && foundpps )
                    break;
            }

            av_freep(&nal.rbsp_buffer);
        
            free(in_buffer);
            av_packet_free(&pkt);
 //           avcodec_close(cdc_ctx);
//            avcodec_free_context(&cdc_ctx);
            return foundsps & foundpps;

        }
        
        
        void FFParse::parseH264Content() {
            AVPacket *videopkt = NULL;
             //int ret = 0;
            if ((videopkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
            }


            if (fseek(fileVideo, 0, SEEK_END))
                return;
            long videofileSize = (long) ftell(fileVideo);
            if (videofileSize < 0)
                return;
            if (fseek(fileVideo, 0, SEEK_SET))
                return;

            //SInfo << "H264 file Size " << videofileSize;


            // av_init_packet(pkt);

            const int in_videobuffer_size = videofileSize;
            unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_videoptr = nullptr;
            int cur_videosize=0;

            long framecount =0;
            
             int buf_index, next_avc;
            
            int state = -1;
            
            H2645NAL nal = { NULL };

            while (!stopped() && keeprunning) {
                 uint64_t currentTime =  CurrentTime_microseconds();
                if (cur_videosize > 0) {

                    
                int src_length, consumed, nalsize = 0;

                if (buf_index >= next_avc) {
                    nalsize = get_nalsize(4, cur_videoptr, cur_videosize, &buf_index, nullptr);
                    if (nalsize < 0)
                        break;
                    next_avc = buf_index + nalsize;
                } else {
                    buf_index = find_start_code(cur_videoptr, cur_videosize, buf_index, next_avc);
                    if (buf_index >= cur_videosize)
                        break;
                    if (buf_index >= next_avc)
                        continue;
                }
                src_length = next_avc - buf_index;

                state = cur_videoptr[buf_index];
                switch (state & 0x1f) {
                case H264_NAL_SLICE:
                case H264_NAL_IDR_SLICE:
                    // Do not walk the whole buffer just to decode slice header
                    if ((state & 0x1f) == H264_NAL_IDR_SLICE || ((state >> 5) & 0x3) == 0) {
                        /* IDR or disposable slice
                         * No need to decode many bytes because MMCOs shall not be present. */
                        if (src_length > 60)
                            src_length = 60;
                    } else {
                        /* To decode up to MMCOs */
                        if (src_length > 1000)
                            src_length = 1000;
                    }
                    break;
                }
                
                consumed = ff_h2645_extract_rbsp(cur_videoptr + buf_index, src_length, &nal, 1);
                if (consumed < 0)
                        break;

                buf_index += consumed;;


//                 ret = get_nal_size( cur_ptr, cur_size, &pkt->data, &pkt->size);
//                 if (ret < 4) {
//                    cur_ptr += 1;
//                    cur_size -= 1;
//                    continue;
//                }
//
//
//                // avcodec_decode_video2
//
//                cur_ptr += ret;
//                cur_size -= ret;

 //               if (pkt->size == 0)
 //                   continue;

                basicvideoframe.copyBuf((u_int8_t*) nal.data , nal.size);
                
//                    ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
//                    if (ret < 4) {
//                        cur_videoptr += 1;
//                        cur_videosize -= 1;
//                        continue;
//                    }
//
//
//                    // avcodec_decode_video2
//
//                    cur_videoptr += ret;
//                    cur_videosize -= ret;
//
//                    if (videopkt->size == 0)
//                        continue;

                    //Some Info from AVCodecParserContext

                    //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                    // BasicFrame        basicframe;
                   // basicvideoframe.copyFromAVPacket(videopkt);
                    basicvideoframe.mstimestamp = startTime +  framecount;
                    basicvideoframe.fillPars();

                    if ( basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
                    {
                        fragmp4_muxer->sendMeta();
                       // resetParser = false;
                    }
                    
                    if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                    {
                        continue;
                    }
                    else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                        continue;
                    }
                    

                    framecount++;

                    info->run(&basicvideoframe);

                    fragmp4_muxer->run(&basicvideoframe);


                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                    uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
                    std::this_thread::sleep_for(std::chrono::microseconds(100000 - deltaTimeMillis));
                    //

                }
                else
                {
                     if (fseek(fileVideo, 0, SEEK_SET))
                    return;

                    cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);

                    STrace << "Read H264 filee " << cur_videosize;

                    if (cur_videosize == 0)
                        break;
                    cur_videoptr = in_videobuffer;
                    
                    buf_index     = 0;
                    next_avc      =  cur_videosize;
                    state = -1;
            
                    
                }
            }

            av_packet_free(&videopkt);

            free(in_videobuffer);
            av_freep(&nal.rbsp_buffer);
        }
        
       void FFParse::parseMuxContent() 
       {
           AVPacket *videopkt = NULL;
           int ret = 0;
           if ((videopkt = av_packet_alloc()) == NULL) {
               fprintf(stderr, "av_packet_alloc failed.\n");
               //goto ret3;
               return;
           }


           if (fseek(fileVideo, 0, SEEK_END))
               return;
           long videofileSize = (long) ftell(fileVideo);
           if (videofileSize < 0)
               return;
           if (fseek(fileVideo, 0, SEEK_SET))
               return;

           SInfo << "H264 file Size " << videofileSize;


           // av_init_packet(pkt);

           const int in_videobuffer_size = videofileSize;
           unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
           unsigned char *cur_videoptr = nullptr;;
           int cur_videosize=0;

           
           
           ////////////////////////////////////
           AVPacket audiopkt;
            
    
           
           int got_output;
            
           AVFrame *frame;
            
                   /* frame containing input raw audio */
           frame = av_frame_alloc();
           if (!frame) {
               fprintf(stderr, "Could not allocate audio frame\n");
               return ;
           }

           frame->nb_samples     = audioContext->frame_size;
           frame->format         = audioContext->sample_fmt;
           frame->channel_layout = audioContext->channel_layout;

           /* allocate the data buffers */
           ret = av_frame_get_buffer(frame, 0);
           if (ret < 0) {
               fprintf(stderr, "Could not allocate audio data buffers\n");
              return ;
           }
   
           int audiosize = av_samples_get_buffer_size(NULL, audioContext->channels,audioContext->frame_size,audioContext->sample_fmt, 1);
           uint8_t* frame_audobuf = (uint8_t *)av_malloc(audiosize);
           ///////////////////////////////
           
           
           int64_t videoframecount=0;
           int64_t audioframecount=0;
           
           AVRational  videotimebase;//= (AVRational){ 1, };
           videotimebase.num = 1;
           videotimebase.den = STREAM_FRAME_RATE;

           AVRational  audiotimebase ;//= (AVRational){ 1,SAMPLINGRATE };
           audiotimebase.num = 1;
           audiotimebase.den = SAMPLINGRATE;
                            
                   
           while (!stopped() && keeprunning)
           {
               uint64_t currentTime =  CurrentTime_microseconds();

               if ( av_compare_ts(videoframecount, videotimebase,  audioframecount, audiotimebase) <= 0)
               {
                   if (cur_videosize > 0)
                   {

                       ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
                       if (ret < 4) {
                           cur_videoptr += 1;
                           cur_videosize -= 1;
                           continue;
                       }


                       // avcodec_decode_video2

                       cur_videoptr += ret;
                       cur_videosize -= ret;

                       if (videopkt->size == 0)
                           continue;

                       //Some Info from AVCodecParserContext

                       //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                       // BasicFrame        basicframe;
                       basicvideoframe.copyFromAVPacket(videopkt);
                       basicvideoframe.mstimestamp = startTime +  videoframecount;
                       basicvideoframe.fillPars();

                       if ( basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
                       {
                           fragmp4_muxer->sendMeta();

                       }

                       if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                       {
                           continue;
                       }
                       else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                           continue;
                       }


                       videoframecount++;

                       info->run(&basicvideoframe);

                       fragmp4_muxer->run(&basicvideoframe);
                       basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                    
                   
                   }
                   else 
                   {

                       if (fseek(fileVideo, 0, SEEK_SET))
                       return;

                       cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);

                       STrace << "Read H264 filee " << cur_videosize;

                       if (cur_videosize == 0)
                           break;
                       cur_videoptr = in_videobuffer;
                       continue;
                   }
               }
               else //audio 
               {
                    if (fread(frame_audobuf, 1, audiosize, fileAudio) <= 0){
                   printf("Failed to read raw data! \n");
                   break;


                   }else if(feof(fileAudio)){

                        if (fseek(fileAudio, 0, SEEK_SET))
                       return;
                       continue;

                   }

                   av_init_packet(&audiopkt);
                   audiopkt.data = NULL; // packet data will be allocated by the encoder
                   audiopkt.size = 0;


                   ret = av_frame_make_writable(frame);
                    if (ret < 0)
                        return ;

                   frame->data[0] = frame_audobuf;  //PCM Data
                    //pFrame->pts=i*100;
                   // encode(encodeContext,pFrame,&pkt);
                    //i++;

                    //if (c->oformat->flags & AVFMT_GLOBALHEADER)
                   //c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


                   ret = avcodec_encode_audio2(audioContext, &audiopkt, frame, &got_output);
                   if (ret < 0) {
                       fprintf(stderr, "Error encoding audio frame\n");
                       return ;
                   }
                   if (got_output) 
                   {

                       basicaudioframe.copyFromAVPacket(&audiopkt);

                       basicaudioframe.mstimestamp = startTime + audioframecount;

//                        if( resetParser ) 
//                        {
//                              fragmp4_muxer->sendMeta();
//                              resetParser =false;
//                        }
                       audioframecount = audioframecount + AUDIOSAMPLE;
                       fragmp4_muxer->run(&basicaudioframe);

                       basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

                       av_packet_unref(&audiopkt);

                   //     std::this_thread::sleep_for(std::chrono::microseconds(21000));

                   }
               }//audio 
           
               uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
               std::this_thread::sleep_for(std::chrono::microseconds(14500 - deltaTimeMillis));
           } //end while

           free(in_videobuffer);
           
           av_free(frame_audobuf);
           av_packet_free(&videopkt);

           av_frame_free(&frame);
           // if you open any thing at ffmpeg do not forget to close it
           avcodec_close(audioContext);
           avcodec_free_context(&audioContext);

       }
       
        

      }// ns mp4

}// nsbase


                    /*Only input video data*/
//                    if ((ret = av_parser_parse2(parser, cdc_ctx, &pkt->data, &pkt->size,
//                            cur_ptr, cur_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)) < 0) {
//                        fprintf(stderr, "av_parser_parse2 failed.\n");
//                        //goto ret8;
//                        return;
//                    }

//                    printf("[Packet]Size:%6d\t", pkt->size);
//                    switch (parser->pict_type) {
//                        case AV_PICTURE_TYPE_I: printf("Type:I\t");
//                            break;
//                        case AV_PICTURE_TYPE_P: printf("Type:P\t");
//                            break;
//                        case AV_PICTURE_TYPE_B: printf("Type:B\t");
//                            break;
//                        default: printf("Type:Other\t");
//                            break;
//                    };
//                    printf("Number:%4d\n", parser->output_picture_number);

//                    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
//                    if (ret < 0) {
//                        printf("Decode Error.\n");
//                        return ret;
//                    }
//                    if (got_picture) {
//                    }
