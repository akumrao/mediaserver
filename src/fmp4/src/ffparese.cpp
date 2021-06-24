/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "ffparse.h"
#include <thread>



#include "ff/ff.h"
#include "ff/mediacapture.h"
#include "base/define.h"
#include "base/test.h"
#include "tools.h"
#include "fmp4.h"

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#define MAX_CHUNK_SIZE 10240*8
// maximum send buffer 262144  =1024 *256

#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576




#define IOBUFSIZE 40960
//40960*6

namespace base {
    namespace fmp4 {


        // based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html


        FFParse::FFParse( base::fmp4::ReadMp4 *conn, const char* audioFile, const char* videofile) :  fragmp4_filter("fragmp4",conn ), fragmp4_muxer("fragmp4muxer", &fragmp4_filter), info("info", nullptr) {

            fragmp4_muxer.activate();
            
            
            
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

//            //Vidoe and Audio mux
            if (parseH264Header()) {
                ++stream_index;
                if (parseAACHeader()) {
                    ++stream_index;
                    parseMuxContent();
                }
               
            }
//            
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
            setupframe.mstimestamp          = getCurrentMsTimestamp();
            // send setup frame
            
            info.run(&setupframe);
            fragmp4_muxer.run(&setupframe);
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            const AVCodec *codec;
            AVCodecContext *c = nullptr ;
           
           
           // int ret =0;
          //  FILE *fout;
            uint16_t *samples;
    //float t, tincr;
            
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
            c->sample_rate    = 44100;//select_sample_rate(codec);
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
             fragmp4_muxer.run(&basicaudioframe);
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
            while(1)
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
                   
                    if( resetParser ) 
                    {
                          fragmp4_muxer.sendMeta();
                          resetParser =false;
                    }
                    framecount = framecount + AUDIOSAMPLE ;
                    fragmp4_muxer.run(&basicaudioframe);

                    basicaudioframe.payload.resize(basicaudioframe.payload.capacity());
                                       
                    av_packet_unref(&audiopkt);
                    
                     std::this_thread::sleep_for(std::chrono::microseconds(21000));
                    
                }
            }
                    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            
           // fclose(fout);
            
           // av_dict_free(&opt);

            av_frame_free(&frame);
            avcodec_free_context(&audioContext);
        }
        
        
        ssize_t FFParse::get_nal_size(uint8_t *buf, ssize_t size,  uint8_t **poutbuf, int *poutbuf_size) {
            ssize_t pos = 3;
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
        
            resetParser = true;
        }
        
        bool FFParse::parseH264Header() {
            int ret = 0;
           // AVCodec *codec = NULL;
          //  AVCodecContext *cdc_ctx = NULL;
            AVPacket *pkt = NULL;

            SetupFrame        setupframe;  ///< This frame is used to send subsession information
          

            basicvideoframe.media_type          =AVMEDIA_TYPE_VIDEO;
            basicvideoframe.codec_id            =AV_CODEC_ID_H264;
            basicvideoframe.stream_index     =stream_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_VIDEO;
            setupframe.codec_id             =AV_CODEC_ID_H264;   // what frame types are to be expected from this stream
            setupframe.stream_index     =stream_index;
            setupframe.mstimestamp          = getCurrentMsTimestamp();
            // send setup frame
            
            info.run(&setupframe);
            fragmp4_muxer.run(&setupframe);

  
            if ((pkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return false;
            }

            
             if (fseek(fileVideo, 0, SEEK_END))
               return false;
            ssize_t fileSize = (ssize_t)ftell(fileVideo);
            if (fileSize < 0)
                return false;
            if (fseek(fileVideo, 0, SEEK_SET))
                return false;
    
            SInfo << "H264 file Size " << fileSize;
          

           // av_init_packet(pkt);

            const int in_buffer_size=fileSize;
            unsigned char *in_buffer = (unsigned char*)malloc(in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_ptr;
            int cur_size;
            
            startTime=    setupframe.mstimestamp;
            //long int deltatime =   1000000/25;  //25 frames persecs

           
            bool foundsps = false;
            bool foundpps =false;
            
            cur_size = fread(in_buffer, 1, in_buffer_size, fileVideo);
            cur_ptr = in_buffer;


            while (cur_size > 0)
            {


                 ret = get_nal_size( cur_ptr, cur_size, &pkt->data, &pkt->size);
                 if (ret < 4) {
                    cur_ptr += 1;
                    cur_size -= 1;
                    continue;
                }


                // avcodec_decode_video2

                cur_ptr += ret;
                cur_size -= ret;

                if (pkt->size == 0)
                    continue;

                basicvideoframe.copyFromAVPacket(pkt);


                basicvideoframe.mstimestamp = startTime ;
                basicvideoframe.fillPars();

                if(  basicvideoframe.h264_pars.slice_type ==  H264SliceType::sps) //AUD Delimiter
                {
                      foundsps = true;
                }

                else if(  basicvideoframe.h264_pars.slice_type ==  H264SliceType::pps) //AUD Delimiter
                {
                      foundpps = true;
                }
                else
                {
                    continue;
                }

                //info.run(&basicframe);

                fragmp4_muxer.run(&basicvideoframe);


                basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                //std::this_thread::sleep_for(std::chrono::microseconds(10000));

                if( foundsps && foundpps )
                    break;
            }


        
            free(in_buffer);
            av_packet_free(&pkt);
 //           avcodec_close(cdc_ctx);
//            avcodec_free_context(&cdc_ctx);
            return foundsps & foundpps;

        }
        
        
        void FFParse::parseH264Content() {
            AVPacket *videopkt = NULL;
             int ret = 0;
            if ((videopkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
            }


            if (fseek(fileVideo, 0, SEEK_END))
                return;
            ssize_t videofileSize = (ssize_t) ftell(fileVideo);
            if (videofileSize < 0)
                return;
            if (fseek(fileVideo, 0, SEEK_SET))
                return;

            SInfo << "H264 file Size " << videofileSize;


            // av_init_packet(pkt);

            const int in_videobuffer_size = videofileSize;
            unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + FF_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_videoptr;
            int cur_videosize=0;

            long framecount =0;

            while (1) {

                if (cur_videosize > 0) {

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
                    basicvideoframe.mstimestamp = startTime +  framecount;
                    basicvideoframe.fillPars();

                    if (resetParser && basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
                    {
                        fragmp4_muxer.sendMeta();
                        resetParser = false;
                    }
                    
                    if (!basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                    {
                        continue;
                    }
                    else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                        continue;
                    }
                    

                    framecount++;

                    info.run(&basicvideoframe);

                    fragmp4_muxer.run(&basicvideoframe);


                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                    std::this_thread::sleep_for(std::chrono::microseconds(10000));
                    //

                }
                else
                {
                     if (fseek(fileVideo, 0, SEEK_SET))
                    return;

                    cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);


                    SInfo << "Read H264 filee " << cur_videosize;

                    if (cur_videosize == 0)
                        break;
                    cur_videoptr = in_videobuffer;
                    }
            }

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
            ssize_t videofileSize = (ssize_t) ftell(fileVideo);
            if (videofileSize < 0)
                return;
            if (fseek(fileVideo, 0, SEEK_SET))
                return;

            SInfo << "H264 file Size " << videofileSize;


            // av_init_packet(pkt);

            const int in_videobuffer_size = videofileSize;
            unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + FF_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_videoptr;
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
            
            AVRational  videotimebase= (AVRational){ 1,STREAM_FRAME_RATE };
            AVRational  audiotimebase = (AVRational){ 1,SAMPLINGRATE };
            
                             
                    
            while (1) {

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

                        if (resetParser && basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
                        {
                            fragmp4_muxer.sendMeta();
                            resetParser = false;
                        }

                        if (!basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                        {
                            continue;
                        }
                        else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                            continue;
                        }


                        videoframecount++;

                        info.run(&basicvideoframe);

                        fragmp4_muxer.run(&basicvideoframe);


                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                     
                    
                    }
                    else 
                    {

                        if (fseek(fileVideo, 0, SEEK_SET))
                        return;

                        cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);


                        SInfo << "Read H264 filee " << cur_videosize;

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
//                              fragmp4_muxer.sendMeta();
//                              resetParser =false;
//                        }
                        audioframecount = audioframecount + AUDIOSAMPLE;
                        fragmp4_muxer.run(&basicaudioframe);

                        basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

                        av_packet_unref(&audiopkt);

                    //     std::this_thread::sleep_for(std::chrono::microseconds(21000));

                    }
                }//audio 
            
                   std::this_thread::sleep_for(std::chrono::microseconds(13500));
            }

        }
        
        
        
        
        
        
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////

     
#if 0
        /* Add an output stream. */
        void  FFParse::add_stream(OutputStream *ost, AVFormatContext *oc,
                AVCodec **codec,
                enum AVCodecID codec_id) {
            AVCodecContext *c;
            int i;

            /* find the encoder */
             *codec = avcodec_find_encoder_by_name("libfdk_aac"); //Specify the use of file encoding type
           // *codec = avcodec_find_encoder(codec_id);
            if (!(*codec)) {
                fprintf(stderr, "Could not find encoder for '%s'\n",
                        avcodec_get_name(codec_id));
               return;
            }

            ost->st = avformat_new_stream(oc, NULL);
            if (!ost->st) {
                fprintf(stderr, "Could not allocate stream\n");
               return;
            }
            ost->st->id = oc->nb_streams - 1;
            c = avcodec_alloc_context3(*codec);
            if (!c) {
                fprintf(stderr, "Could not alloc an encoding context\n");
               return;
            }
            ost->enc = c;

            switch ((*codec)->type) {
                case AVMEDIA_TYPE_AUDIO:
                     c->sample_fmt = AV_SAMPLE_FMT_S16;
                   // c->sample_fmt = (*codec)->sample_fmts ?
                      //      (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_S16;
                    c->bit_rate = 64000;
                    c->sample_rate = 44100;
                    if ((*codec)->supported_samplerates) {
                        c->sample_rate = (*codec)->supported_samplerates[0];
                        for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                            if ((*codec)->supported_samplerates[i] == 44100)
                                c->sample_rate = 44100;
                        }
                    }
                    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
                    if ((*codec)->channel_layouts) {
                        c->channel_layout = (*codec)->channel_layouts[0];
                        for (i = 0; (*codec)->channel_layouts[i]; i++) {
                            if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                                c->channel_layout = AV_CH_LAYOUT_STEREO;
                        }
                    }
                    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
                    ost->st->time_base = (AVRational){1, c->sample_rate};
                    break;

                case AVMEDIA_TYPE_VIDEO:
                       break;

                default:
                    break;
            }

            /* Some formats want stream headers to be separate. */
            if (oc->oformat->flags & AVFMT_GLOBALHEADER)
                c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        /**************************************************************/

        /* audio output */

        AVFrame * FFParse::alloc_audio_frame(enum AVSampleFormat sample_fmt,
                uint64_t channel_layout,
                int sample_rate, int nb_samples) {
            AVFrame *frame = av_frame_alloc();
            int ret;

            if (!frame) {
                fprintf(stderr, "Error allocating an audio frame\n");
               return nullptr;
            }

            frame->format = sample_fmt;
            frame->channel_layout = channel_layout;
            frame->sample_rate = sample_rate;
            frame->nb_samples = nb_samples;

            if (nb_samples) {
                ret = av_frame_get_buffer(frame, 0);
                if (ret < 0) {
                    fprintf(stderr, "Error allocating an audio buffer\n");
                   return nullptr;
                }
            }

            return frame;
        }

        void  FFParse::open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg) {
            AVCodecContext *c;
            int nb_samples;
            int ret;
            AVDictionary *opt = NULL;

            c = ost->enc;

            /* open it */
            av_dict_copy(&opt, opt_arg, 0);
            c->profile = FF_PROFILE_AAC_LOW;
            ret = avcodec_open2(c, codec, &opt);
            av_dict_free(&opt);
            if (ret < 0) {
                //fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
               return;
            }


            if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
                nb_samples = 10000;
            else
                nb_samples = c->frame_size;

            ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                    c->sample_rate, nb_samples);
            

            /* copy the stream parameters to the muxer */
            ret = avcodec_parameters_from_context(ost->st->codecpar, c);
            if (ret < 0) {
                fprintf(stderr, "Could not copy the stream parameters\n");
               return;
            }


        }

        /* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
         * 'nb_channels' channels. */
        AVFrame * FFParse::get_audio_frame(OutputStream *ost) {
            AVFrame *frame = ost->frame;
//            int j, i, v;
//            int16_t *q = (int16_t*) frame->data[0];

            //    /* check if we want to generate more frames */
            //    if (av_compare_ts(ost->next_pts, ost->enc->time_base,
            //                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
            //        return NULL;

//            for (j = 0; j < frame->nb_samples; j++) {
//                v = (int) (sin(ost->t) * 10000);
//                for (i = 0; i < ost->enc->channels; i++)
//                    *q++ = v;
//                ost->t += ost->tincr;
//                ost->tincr += ost->tincr2;
//            }
            AVCodecContext *c;
            c = ost->enc;
            
            int size = av_samples_get_buffer_size(NULL, c->channels,c->frame_size,c->sample_fmt, 1);
            uint8_t* frame_buf = (uint8_t *)av_malloc(size);

            if(!ost->in_file)
            {
                ost->in_file = fopen("/var/tmp/test.pcm","rb");
                if(ost->in_file){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                    av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
                    return nullptr;
                }
            }

            
           
            if (fread(frame_buf, 1, size, ost->in_file) <= 0){
                 printf("Failed to read raw data! \n");
                 return nullptr;


             }else if(feof(ost->in_file)){
                 
                 
                 fseek(ost->in_file, 0, SEEK_SET);
                 printf("End of audio file \n"); 
                //return;
                return nullptr;
             }
            
             int ret = av_frame_make_writable(frame);
                if (ret < 0)
                   return 0;
             

            frame->data[0] = frame_buf;  //PCM Data
 
            frame->pts = ost->next_pts;
            ost->next_pts += frame->nb_samples;

            return frame;
        }

        /*
         * encode one audio frame and send it to the muxer
         * return 1 when encoding is finished, 0 otherwise
         */
        int  FFParse::write_audio_frame(AVFormatContext *oc, OutputStream *ost) {
            AVCodecContext *c;
            AVPacket pkt = {0}; // data and size must be 0;
            AVFrame *frame;
            int ret;
            int got_packet;
            int dst_nb_samples;

            av_init_packet(&pkt);
            c = ost->enc;

            frame = get_audio_frame(ost);
//
//            if (frame) {
//                /* convert samples from native format to destination codec format, using the resampler */
//                /* compute destination number of samples */
//                dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
//                        c->sample_rate, c->sample_rate, AV_ROUND_UP);
//                av_assert0(dst_nb_samples == frame->nb_samples);
//
//                /* when we pass a frame to the encoder, it may keep a reference to it
//                 * internally;
//                 * make sure we do not overwrite it here
//                 */
//                ret = av_frame_make_writable(ost->frame);
//                if (ret < 0)
//                   return 0;
//
//                /* convert to destination format */
//                ret = swr_convert(ost->swr_ctx,
//                        ost->frame->data, dst_nb_samples,
//                        (const uint8_t **) frame->data, frame->nb_samples);
//                if (ret < 0) {
//                    fprintf(stderr, "Error while converting\n");
//                   return 0;
//                }
//                frame = ost->frame;
//
//                frame->pts = av_rescale_q(ost->samples_count, (AVRational) {
//                    1, c->sample_rate
//                }, c->time_base);
//                ost->samples_count += dst_nb_samples;
//            }

            ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
            if (ret < 0) {
              //  fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
               return 0;
            }

            if (got_packet) {
                std::cout << "got_packet " << std::endl << std::flush;

                
                ////////////////////////////////////////////////////
                
                
                             // BasicFrame        basicframe;
                    basicaudioframe.copyFromAVPacket(&pkt);
                                      

                   // info.run(&basicaudioframe);
                    fragmp4_muxer.run(&basicaudioframe);

                    basicaudioframe.payload.resize(basicaudioframe.payload.capacity());
                
                
                              
                
                ////////////////////////////////////////////////////
                
//                ret = write_frame(oc, &c->time_base, ost->st, &pkt);
//                if (ret < 0) {
//                    fprintf(stderr, "Error while writing audio frame: %s\n",
//                            av_err2str(ret));
//                   return;
//                }
            }

            return (frame || got_packet) ? 0 : 1;
        }

        /**************************************************************/

        /* video output */

     
        void  FFParse::close_stream(AVFormatContext *oc, OutputStream *ost) {
            avcodec_free_context(&ost->enc);
            av_frame_free(&ost->frame);
//            av_frame_free(&ost->tmp_frame);
 //           sws_freeContext(ost->sws_ctx);
  //          swr_free(&ost->swr_ctx);
        }

        
        
        
        
        
        
        /* media file output */

        int FFParse::startAudio( )
        {
            
            SetupFrame        setupframe;  ///< This frame is used to send subsession information
           
            int               stream_index;
            
            stream_index = 0;
            basicaudioframe.media_type           =AVMEDIA_TYPE_AUDIO;
            basicaudioframe.codec_id             =AV_CODEC_ID_AAC;
            basicaudioframe.stream_index     =stream_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_AUDIO;
            setupframe.codec_id             =AV_CODEC_ID_AAC;   // what frame types are to be expected from this stream
            setupframe.stream_index     =stream_index;
            setupframe.mstimestamp          = getCurrentMsTimestamp();
            // send setup frame
            
            info.run(&setupframe);
            fragmp4_muxer.run(&setupframe);

            
            OutputStream video_st = { 0 }, audio_st = { 0 };
            const char *filename;
            AVOutputFormat *fmt;
            AVFormatContext *oc;
            AVCodec *audio_codec, *video_codec;
            int ret;
            int have_video = 0, have_audio = 0;
            int encode_video = 0, encode_audio = 0;
            AVDictionary *opt = NULL;
            int i;

            /* Initialize libavcodec, and register all codecs and formats. */
            

//            filename = argv[1];
//            for (i = 2; i+1 < argc; i+=2) {
//                if (!strcmp(argv[i], "-flags") || !strcmp(argv[i], "-fflags"))
//                    av_dict_set(&opt, argv[i]+1, argv[i+1], 0);
//            }

           // av_dict_set(&opt, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
            av_dict_set(&opt, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof", 0);



            AVIOContext *ioCtxt;
            uint8_t *ioBuffer;



            AVOutputFormat *outputFormat = av_guess_format("mp4", nullptr, nullptr);

            /* allocate the output media context */
            avformat_alloc_output_context2(&oc, outputFormat, NULL, NULL);
            if (!oc) {
                printf("Could not deduce output format from file extension: using MPEG.\n");
                avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
            }
            if (!oc)
                return 1;

            fmt = oc->oformat;


            if ((ioBuffer = (uint8_t*) av_malloc(IOBUFSIZE)) == nullptr) {
                std::cout << "Couldn't allocate I/O buffer" << std::endl;
                return 8;
            }
            if ((ioCtxt = avio_alloc_context(ioBuffer, IOBUFSIZE, 1, nullptr, nullptr, nullptr, nullptr)) == nullptr) {
                std::cout << "Couldn't initialize I/O context" << std::endl;
                return 9;
            }


            oc->pb = ioCtxt;

            /* Add the audio and video streams using the default format codecs
        //     * and initialize the codecs. */
        //    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        //        add_stream(&video_st, oc, &video_codec, fmt->video_codec);
        //        have_video = 1;
        //        encode_video = 1;
        //    }
            if (fmt->audio_codec != AV_CODEC_ID_NONE) {
                add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec);
                have_audio = 1;
                encode_audio = 1;
            }

            /* Now that all the parameters are set, we can open the audio and
             * video codecs and allocate the necessary encode buffers. */
//            if (have_video)
//                open_video(oc, video_codec, &video_st, opt);
             
            if (have_audio)
                open_audio(oc, audio_codec, &audio_st, opt);

            av_dump_format(oc, 0, filename, 1);

            /* open the output file, if needed */
        //    if (!(fmt->flags & AVFMT_NOFILE)) {
        //        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        //        if (ret < 0) {
        //            fprintf(stderr, "Could not open '%s': %s\n", filename,
        //                    av_err2str(ret));
        //            return 1;
        //        }
        //    }

            /* Write the stream header, if any. */
            ret = avformat_write_header(oc, &opt);
            if (ret < 0) {
              //  fprintf(stderr, "Error occurred when opening output file: %s\n",v_err2str(ret));
                return 1;
            }
            
            long int startTime=  setupframe.mstimestamp;
            long framecount =0;
            while (encode_video || encode_audio) {
                /* select the stream to encode */
//                if (encode_video && (!encode_audio || av_compare_ts(video_st.next_pts, video_st.enc->time_base,  audio_st.next_pts, audio_st.enc->time_base) <= 0)) {
//                    encode_video = !write_video_frame(oc, &video_st);
//                } else {
                
                // std::cout << "afterGettingFrame: mstimestamp=" << mstimestamp <<std::endl;
                    basicaudioframe.mstimestamp = startTime + 10.4 * framecount;
                   // basicframe.fillPars();
                    
//                    if( !framecount &&  basicframe.h264_pars.slice_type == H264SliceType::aud) //AUD Delimiter
//                    {
//                          continue;
//                    }
                    framecount++;
                    
                    //if(framecount == 700 )
                     //   break;
                    
                    std::cout << "encode and write audio" << std::endl << std::flush;
                    encode_audio = !write_audio_frame(oc, &audio_st);
               // }
            }

            /* Write the trailer, if any. The trailer must be written before you
             * close the CodecContexts open when you wrote the header; otherwise
             * av_write_trailer() may try to use memory that was freed on
             * av_codec_close(). */
            av_write_trailer(oc);

//
//            std::fstream file(filename, std::ios::out | std::ios::binary);
//            if (!file.is_open()) {
//                std::cout << "Couldn't open file " << argv[1] << std::endl;
//                return 2;
//            }
//
//            // file.open("my_mp4.mp4", std::ios::out | std::ios::binary);
//            if (!file.is_open()) {
//                std::cout << "Couldn't open output file " << std::endl;
//                return 12;
//            }

//            file.write((char*) outputData.data(), outputData.size());
//            if (file.fail()) {
//                std::cout << "Couldn't write to file" << std::endl;
//                return 13;
//            }
//
//
//            file.close();


            /* Close each codec. */
            if (have_video)
                close_stream(oc, &video_st);
            if (have_audio)
                close_stream(oc, &audio_st);

        //    if (!(fmt->flags & AVFMT_NOFILE))
        //        /* Close the output file. */
        //        avio_closep(&oc->pb);

            /* free the stream */
            avformat_free_context(oc);

            return 0;
        }

#endif
        
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        

    }

}


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
