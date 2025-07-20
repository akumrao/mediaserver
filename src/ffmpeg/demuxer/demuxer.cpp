#include <stdio.h>

#include <string>

#define __STDC_CONSTANT_MACROS

#include "avformat.h"
#include "mathematics.h"
#include "avcodec.h"
//#define USE_H264BSF 1


static double get_fps(AVStream *st)
{
    int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
    int tbr = st->r_frame_rate.den && st->r_frame_rate.num;
    int tbn = st->time_base.den && st->time_base.num;

    if(fps) return av_q2d(st->avg_frame_rate);
    if(tbr) return av_q2d(st->r_frame_rate);
    if(tbn) return 1.0 / av_q2d(st->time_base);
}



   
int main(int argc, char **argv ) {
    AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex = -1, audioindex = -1;
    int frame_index = 0;

    
    char *in_filename;
    char *out_filename_v;// = "sample.h264";
    const char *out_filename_a  = "sample.mp3";
    
    std::string file = "/var/tmp/test.mp4";
    
    if(argc == 2)
    {
        file = argv[1];
    }
   
    in_filename = (char*)file.c_str();
    
    size_t lastindex = file.find_last_of("."); 
    std::string rawname = file.substr(0, lastindex); 
    rawname += ".264";
    
    out_filename_v = (char*)rawname.c_str();
    
   // out_filename_v =  file.trim() 

    av_register_all();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf( "Could not open input file.");
       return;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf( "Failed to retrieve input stream information");
       return;
    }


    //Output
    avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
    //avformat_alloc_output_context2(&ofmt_ctx_v, NULL, "h264", NULL);
   
     
    if (!ofmt_ctx_v) {
        printf( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
       return;
    }
    ofmt_v = ofmt_ctx_v->oformat;



//    avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
//    if (!ofmt_ctx_a) {
//        printf( "Could not create output context\n");
//        ret = AVERROR_UNKNOWN;
//       return;
//    }
//    ofmt_a = ofmt_ctx_a->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
            //Create output AVStream according to input AVStream
            AVFormatContext *ofmt_ctx;
            AVStream *in_stream = ifmt_ctx->streams[i];
            AVStream *out_stream = NULL;

            if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
                videoindex=i;
                out_stream=avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
                ofmt_ctx=ofmt_ctx_v;
                
//                int es =  in_stream->codecpar->extradata_size ;
//                out_stream->codecpar->extradata_size = es ;
//                
//                out_stream->codecpar->extradata = ( uint8_t *)av_malloc(es+ AV_INPUT_BUFFER_PADDING_SIZE);
//                
//                memcpy( out_stream->codecpar->extradata, in_stream->codecpar->extradata, es);
//                
                out_stream->time_base = ifmt_ctx->streams[0]->time_base;
            	out_stream->avg_frame_rate = ifmt_ctx->streams[0]->avg_frame_rate;
                out_stream->r_frame_rate = ifmt_ctx->streams[0]->r_frame_rate;
                
                
                
                    
                
            }else if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
                audioindex=i;
                out_stream=avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
                ofmt_ctx=ofmt_ctx_a;
            }else{
                break;
            }

            if (!out_stream) {
                printf( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
               return;
            }
            //Copy the settings of AVCodecContext
            
            
    

            
            int ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            if (ret < 0) {
                fprintf(stderr, "Failed to copy codec parameters\n");
                return -1;
            }
            
            
            {
            
                                            
                    if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                        printf( "Failed to copy context from input to output stream codec context\n");
                       return;
                    }

                     AVCodecContext *pCodecCtx =  out_stream->codec;

                     int frameRate_ = 1*pCodecCtx->time_base.den/(pCodecCtx->time_base.num*pCodecCtx->ticks_per_frame);
                     int  width_ = pCodecCtx->width;
                     int height_ = pCodecCtx->height;
            
            
            
                }
            
            int es33 =  out_stream->codecpar->extradata_size ;
            
            out_stream->codec->codec_tag = 0;

           // if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            

//	    out_stream->time_base = ifmt_ctx->streams[0]->time_base;
//            out_stream->avg_frame_rate = ifmt_ctx->streams[0]->avg_frame_rate;
//            out_stream->r_frame_rate = ifmt_ctx->streams[0]->r_frame_rate;
    }

    //Dump Format------------------
//    printf("\n==============Input Video=============\n");
//    av_dump_format(ifmt_ctx, 0, in_filename, 0);
//    printf("\n==============Output Video============\n");
//    av_dump_format(ofmt_ctx_v, 0, out_filename_v, 1);
//    printf("\n==============Output Audio============\n");
//    av_dump_format(ofmt_ctx_a, 0, out_filename_a, 1);
//    printf("\n======================================\n");
    //Open output file
    if (!(ofmt_v->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
            printf( "Could not open output file '%s'", out_filename_v);
           return;
        }
    }
    
    
    
    
    

//    if (!(ofmt_a->flags & AVFMT_NOFILE)) {
//        if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
//            printf( "Could not open output file '%s'", out_filename_a);
//           return;
//        }
//    }

    //Write file header
    if (avformat_write_header(ofmt_ctx_v, NULL) < 0) {
        printf( "Error occurred when opening video output file\n");
       return;
    }

//    if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
//        printf( "Error occurred when opening audio output file\n");
//       return;
//    }
    
    
      
      
      
         
      
#if USE_H264BSF
    AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif

    while (1) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        if (av_read_frame(ifmt_ctx, &pkt) < 0)
            break;
        in_stream  = ifmt_ctx->streams[pkt.stream_index];


        if(pkt.stream_index==videoindex){
            out_stream = ofmt_ctx_v->streams[0];
            ofmt_ctx=ofmt_ctx_v;
            #if USE_H264BSF
                av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
            #endif
            printf("Write Video Packet. size:%d\tpts:%lld\n",pkt.size,pkt.pts);
        }else if(pkt.stream_index==audioindex){
            out_stream = ofmt_ctx_a->streams[0];
            ofmt_ctx=ofmt_ctx_a;
            printf("Write Audio Packet. size:%d\tpts:%lld\n",pkt.size,pkt.pts);
        }else{
            continue;
        }
         
        int fpsSen = get_fps(in_stream);
    
    
        int fpsRec = get_fps(out_stream);
     
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index=0;
        //Write
//        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
//            printf( "Error muxing packet\n");
//            break;
//        }
        
        
        
        int res = av_interleaved_write_frame(ofmt_ctx,  &pkt); // => this calls write_packet
        // std::cout << "MuxFrameFilter : av_write_frame returned " << res << std::endl;
        if (res < 0) {
            printf( "av_interleaved_write_frame failed\n");
            av_write_trailer(ofmt_ctx); // if we don't call this we'll get massive memleaks!  
            //closeMux();
            return -1;
            // std::cout << "MuxFrameFilter: initialized now " << int(initialized) << std::endl;
        } else {
            // used to crasssshh here, but not anymore, after we have defined dummy read & seek functions!
            // int res = av_write_frame(av_format_context, avpkt);
            //std::cout << "res =" << res << std::endl;
            av_write_frame(ofmt_ctx, NULL); // create a custom fragment
            /*
            av_buffer_unref(&(avpkt->buf));
            av_packet_unref(avpkt);
             */
        }
        
        
        
        
        //printf("Write %8d frames to output file\n", frame_index);
         av_packet_unref(&pkt); 
        frame_index++;
    }

#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);  
#endif

    //Write file trailer
    //av_write_trailer(ofmt_ctx_a);
    av_write_trailer(ofmt_ctx_v);

    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_a->pb);

    if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_v->pb);

    avformat_free_context(ofmt_ctx_a);
    avformat_free_context(ofmt_ctx_v);

    if (ret < 0 && ret != AVERROR_EOF) {
        printf("Error occurred.\n");
        return -1;
    }

    return 0;
}
