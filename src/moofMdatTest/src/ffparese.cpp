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



// #include "ff/ff.h"
// #include "ff/mediacapture.h"
// #include "base/define.h"
// #include "base/test.h"
#include "tools.h"
#include "fmp4.h"


#define MAX_CHUNK_SIZE 10240*8
// maximum send buffer 262144  =1024 *256

#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576




#define IOBUFSIZE 40960
//40960*6

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

           
            while(  !stopped())
            { 
               // reopen();
                keeprunning = true;
               
                
              //audio only is broken please send ftype and moove box after reset or onconnect. Onneect would be better option
//            if (parseAACHeader()) {
//                 ++stream_index;
//                parseAACContent();
//            }
//            return;

                
                
                fragmp4_muxer->deActivate();
                if(!mute )
                {   
//                    if(hd)
//                        mediaContent("avhd");
//                    else
//                         mediaContent("avsd"); 
                    
                    
                    
                    basicvideoframe.media_type = AVMEDIA_TYPE_VIDEO;
                    basicvideoframe.codec_id = AV_CODEC_ID_H264;
                    basicvideoframe.stream_index = 0;
                        
                    if( initAAC())
                     parseMuxContent();
               }
               else
               {   

                        basicvideoframe.media_type = AVMEDIA_TYPE_VIDEO;
                        basicvideoframe.codec_id = AV_CODEC_ID_H264;
                        basicvideoframe.stream_index = 0;
                       
                        parseH264Content();


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

       
         
        bool FFParse::initAAC()
        {

          //  av_dict_set(&opt, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof", 0);
            
            AVCodecContext *c = nullptr ;
            AVDictionary *opt = NULL;
            //codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            audiocodec = avcodec_find_encoder_by_name("libfdk_aac"); //Specify the use of file encoding type
            if (!audiocodec) {
                fprintf(stderr, "Codec not found\n");
               return false ;
            }

            c = avcodec_alloc_context3(audiocodec);
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
            if (avcodec_open2(c, audiocodec, &opt) < 0) {
                fprintf(stderr, "Could not open codec\n");
               return false;
            }
             
     
             audioContext = c;

             
             return true ;
        }
 

        
        bool FFParse::parseAACHeader(int stream_index) {
          
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
            
            
                          
            startTime=  setupframe.mstimestamp;
                
             int extrasize = audioContext->extradata_size;             
             basicaudioframe.payload.resize(extrasize);
             memcpy( basicaudioframe.payload.data(),  audioContext->extradata, extrasize) ;
             basicaudioframe.codec_id = audiocodec->id;
             basicaudioframe.mstimestamp = startTime ;
             fragmp4_muxer->run(&basicaudioframe);
             basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

             
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
            fragmp4_muxer->deActivate();
            fclose(fileAudio);
            fileAudio = nullptr;
            
            fclose(fileVideo);
            fileVideo = nullptr;
            hd = (++hd)%2;
            if (hd)
            {
               
                SInfo << "Open HD";
                
                const char* audioFile = AUDIOFILE;
                const char* videofile = VIDEOFILE;
                
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
              
                const char* audioFile = AUDIOFILE1;
                const char* videofile =VIDEOFILE1;
                
            
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

        
        bool FFParse::parseH264Header(int stream_index) 
        {
            int ret = 0;
           // AVCodec *codec = NULL;
          //  AVCodecContext *cdc_ctx = NULL;
            fragmp4_muxer->deActivate();
            foundsps = false;
            foundpps = false;

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
            setupframe.mstimestamp          = CurrentTime_milliseconds();
            // send setup frame
            
            //info->run(&setupframe);
            fragmp4_muxer->run(&setupframe);

  
           

            //cur_size = fread(in_buffer, 1, in_buffer_size, fileVideo);
            //cur_ptr = in_buffer;

    

        
           
 //           avcodec_close(cdc_ctx);
//            avcodec_free_context(&cdc_ctx);
            return  true;

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
            int gop = 0;
            while (!stopped() && keeprunning) 
            {
                uint64_t currentTime =  CurrentTime_microseconds();
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
                    basicvideoframe.mstimestamp = startTime +  framecount;
                    basicvideoframe.fillPars();

       

////////////////////

                    if ( foundsps && foundpps && ( basicvideoframe.h264_pars.frameType == H264SframeType::i &&  basicvideoframe.h264_pars.slice_type == H264SliceType::idr)) //AUD Delimiter
                    {
                        fragmp4_muxer->sendMeta();
                        
                       
                        
                    #if(_DEBUG)
                        SInfo << " Key " << basicvideoframe.payload.size();
                        if (gop)
                        {
                            SInfo << "Gop " << gop;
                        }
                        gop = 1;
                    #endif
                    }

                    if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                    {
                                       
                       unsigned num_units_in_tick, time_scale;


                        if(  basicvideoframe.h264_pars.slice_type == H264SliceType::sps  )
                        {    
                            obj.analyze_seq_parameter_set_data(videopkt->data + 4, videopkt->size - 4, num_units_in_tick, time_scale);

                            if (fps != obj.fps || width != obj.width || height != obj.height)
                            {
                                    parseH264Header(0);
                                    SInfo << "reset parser, with fps " << obj.fps << " width "  <<   obj.width << " height"  << obj.height;
                            }

//uint8_t *p = videopkt->data +4;


                            if (!foundpps)
                            {
                                    fps = obj.fps;
                                    height = obj.height;
                                    width = obj.width;

                                    basicvideoframe.fps = obj.fps;
                                    basicvideoframe.height = obj.height;
                                    basicvideoframe.width = obj.width;

                                    // SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;

                                    //info->run(&basicvideoframe);
                                    fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                            }
                           
                           foundsps  = true;
                           
                        }

                        if( !foundpps &&  basicvideoframe.h264_pars.slice_type == H264SliceType::pps  )
                        {    
                          
                           
                           // SInfo <<  " Got PPS fps ";
                            
                            //info->run(&basicvideoframe);
                            fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                            basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                            
                             foundpps  = true;

                           
                        }
                       
                        continue;

                    }
                    else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                    //info->run(&basicvideoframe);
                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
			 continue;
                    }
                    else if (foundsps && foundpps  )
                    {
#if(_DEBUG)
                        if (basicvideoframe.h264_pars.frameType == H264SframeType::p)
                        {
                            ++gop;
                            SInfo << " P frame " << basicvideoframe.payload.size();
                        }
                        else if (basicvideoframe.h264_pars.frameType == H264SframeType::b)
                        {
                            ++gop;
                            SInfo << " B frame " << basicvideoframe.payload.size();
                        }
                        else if (basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type != H264SliceType::idr)
                        {
                            SInfo << " I frame " << basicvideoframe.payload.size() << " gop " << gop;
                            gop = 1;
                            
                        }
                      
#endif


                              


                        //info->run(&basicvideoframe);
                        fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                         framecount++;

			             uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
                         std::this_thread::sleep_for(std::chrono::microseconds(100000 - deltaTimeMillis));

                     }
		        }
                else
                {
                    reopen();
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

	    av_packet_free(&videopkt);	
            free(in_videobuffer);

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
           videotimebase.den = 25;   /// 1000000/25 = 40000

           AVRational  audiotimebase ;//= (AVRational){ 1,SAMPLINGRATE };
           audiotimebase.num = 1;
           audiotimebase.den = SAMPLINGRATE;  /// 1000000*1024/ 44100 = 23219.954648526  // almost half of video when 25 frames per secconds
                            
//          uint64_t start =  CurrentTime_microseconds();
          
          int nCount= 0;
           while (!stopped() && keeprunning)
           {
               
               uint64_t currentTime =  CurrentTime_microseconds();
//               
//               uint64_t v = (uint64_t)videoframecount* (uint64_t)1000000 / (uint64_t)videotimebase.den;
//               uint64_t a = (uint64_t)audioframecount*(uint64_t)1000000 / (uint64_t)audiotimebase.den;
//               
//               uint64_t tdelta = currentTime -  start ;
//               
//               std::cout << nCount++ << " delta :" << tdelta <<  " video no :" <<   videoframecount << " video  t :"  << v   <<  " audio no :" <<  audioframecount/AUDIOSAMPLE << " audio t: " <<  a << std::endl << std::flush;
//               
                if ( av_compare_ts(videoframecount, videotimebase,  audioframecount, audiotimebase) <= 0)
              // if ( tdelta >= v )
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

                      
                       
                        if ( foundsps && foundpps && ( basicvideoframe.h264_pars.frameType == H264SframeType::i &&  basicvideoframe.h264_pars.slice_type == H264SliceType::idr)) //AUD Delimiter
                        {
                            fragmp4_muxer->sendMeta();



                        #if(_DEBUG_1)
                            SInfo << " Key " << basicvideoframe.payload.size();
                            if (gop)
                            {
                                SInfo << "Gop " << gop;
                            }
                            gop = 1;
                        #endif
                        }

                        if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                        {

                           unsigned num_units_in_tick, time_scale;


                            if(  basicvideoframe.h264_pars.slice_type == H264SliceType::sps  )
                            {    
                                obj.analyze_seq_parameter_set_data(videopkt->data + 4, videopkt->size - 4, num_units_in_tick, time_scale);

                                if (fps != obj.fps || width != obj.width || height != obj.height)
                                {
                                    videotimebase.den =  obj.fps;
                                    
                                    parseH264Header(0);
                                        
                                    parseAACHeader(1);
                                        
                                    SInfo << "reset parser, with fps " << obj.fps << " width "  <<   obj.width << " height"  << obj.height;
                                }

                                //uint8_t *p = videopkt->data +4;

                                if (!foundpps)
                                {
                                    fps = obj.fps;
                                    height = obj.height;
                                    width = obj.width;

                                    basicvideoframe.fps = obj.fps;
                                    basicvideoframe.height = obj.height;
                                    basicvideoframe.width = obj.width;

                                     SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;

                                        //info->run(&basicvideoframe);
                                    fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                                }

                               foundsps  = true;

                            }

                            if( !foundpps &&  basicvideoframe.h264_pars.slice_type == H264SliceType::pps  )
                            {    

                                // SInfo <<  " Got PPS fps ";
                                //info->run(&basicvideoframe);
                                fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                                foundpps  = true;

                            }
                           
                           continue;


                        }
                        else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                        //info->run(&basicvideoframe);
                            //basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                             continue;
                        }
                        else if (foundsps && foundpps  )
                        {
    #if(_DEBUG_1)
                            if (basicvideoframe.h264_pars.frameType == H264SframeType::p)
                            {
                                ++gop;
                                SInfo << " P frame " << basicvideoframe.payload.size();
                            }
                            else if (basicvideoframe.h264_pars.frameType == H264SframeType::b)
                            {
                                ++gop;
                                SInfo << " B frame " << basicvideoframe.payload.size();
                            }
                            else if (basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type != H264SliceType::idr)
                            {
                                SInfo << " I frame " << basicvideoframe.payload.size() << " gop " << gop;
                                gop = 1;

                            }

    #endif


                            //info->run(&basicvideoframe);
                            fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                            basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                             videoframecount++;

                            // //int64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
                            // std::this_thread::sleep_for(std::chrono::microseconds(100000 - deltaTimeMillis));

                         }
                    
                   
                   }
                   else 
                   {
		        reopen();
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

               else if (foundsps && foundpps  )       //if (tdelta  >= a)//audio 
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
                   if (got_output && basicaudioframe.stream_index) 
                   {

                       basicaudioframe.copyFromAVPacket(&audiopkt);
                       basicaudioframe.mstimestamp = startTime + audioframecount;

//                     if( resetParser ) 
//                     {
//                              fragmp4_muxer->sendMeta();
//                              resetParser =false;
//                     }
                       audioframecount = audioframecount + AUDIOSAMPLE;
                       fragmp4_muxer->run(&basicaudioframe);

                       basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

                       av_packet_unref(&audiopkt);

                       //std::this_thread::sleep_for(std::chrono::microseconds(21000));

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
