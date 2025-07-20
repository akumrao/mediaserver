
#include "MVExtractor.h"
#ifdef _MV_DEBUG_
#include<opencv2/highgui.hpp>
extern char run_mode;
#endif

std::vector<cv::Point> coords;
bool quitkey=false;
bool polygon_complete=false;

void MVExtractor::CallBackFunc(int event, int x, int y, int flags, void* userdata) 
{
     if ( event == cv::EVENT_LBUTTONDBLCLK) //start polygon 
     { 
         std::cout << "POLYGON START" << std::endl;
         coords.clear();
         polygon_complete=false;
         coords.push_back(cv::Point(x,y));
     }   
     
     else if  ( event == cv::EVENT_LBUTTONDOWN ) //build polygon, end when point clicked is close to first point
     {
        if (!polygon_complete) { 
         if (coords.size() <=2 ) {
             if (coords.size()>0)
               if ((abs(x-coords[0].x) > 5) || (abs(y-coords[0].y) > 5 )) {
                  coords.push_back(cv::Point(x,y));
               } 
                   
             
         } 
         else if (coords.size() > 2) 
         {
             if  ((abs(x-coords[0].x) < 5) && (abs(y-coords[0].y) < 5 ))  {
                 polygon_complete=true;
                 std::cout << "POLYGON COMPLETE" << std::endl;
             } else {
               coords.push_back(cv::Point(x,y));
             }  
         } 
        } 
         
     }
     
     else if  ( event == cv::EVENT_RBUTTONDOWN ) //delete the last point
     {    
         if (!polygon_complete) { 
             coords.pop_back();
         }
     }
     
     
     else if  ( event == cv::EVENT_RBUTTONDBLCLK) //if coordinate within the polygon, delete the polygon
     {
         if (polygon_complete && (coords.size()>2))
          //if (pnpoly(coords.size(), coords, cv::Point(x,y) ) )
         {
             std::cout << "POLYGON DELETED" << std::endl;
             coords.clear();
             polygon_complete=false;
         } 
     }
     else if  ( event == cv::EVENT_MBUTTONDBLCLK) //if coordinate within the polygon, delete the polygon
     {   quitkey=true;
     
     }
     
    
} 

MVExtractor::MVExtractor(const char *src_filename) {
    int ret = 0;

    cv::namedWindow("occupancy", 1);
    cv::setMouseCallback("occupancy", CallBackFunc, NULL);
     
    read(src_filename);

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    open_codec_context(fmt_ctx, AVMEDIA_TYPE_VIDEO);



    if (!video_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        ret = 1;
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        exit(1);
    }

    printf("framenum,source,blockw,blockh,srcx,srcy,dstx,dsty,flags\n");

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;


    
}

MVExtractor::~MVExtractor() {
    if (mv_detector) {
        delete mv_detector;
        mv_detector = nullptr;
    }

    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);

}

int MVExtractor::read(const char *src_filename) {



    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    av_dump_format(fmt_ctx, 0, src_filename, 0);

    return 0;
}

int MVExtractor::decode_packet(int *got_frame, int cached) {
    int decoded = pkt.size;

    *got_frame = 0;

    if (pkt.stream_index == video_stream_idx) {
        int ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error decoding video frame \n");
            return ret;
        }

        if (*got_frame) {
            int i;
            AVFrameSideData *sd;

            
            char pict_type = av_get_picture_type_char(frame->pict_type);
            
            printf(" Pic type  %c  %d\n " , pict_type, decoded );
            // get pts
            int64_t pts = frame->pts != AV_NOPTS_VALUE ? frame->pts : (frame->pkt_dts != AV_NOPTS_VALUE ? frame->pkt_dts : pts + 1);
            
            
           #ifdef _MV_DEBUG_ 

            cv::Mat tmp_img = cv::Mat::zeros( dst_height*3/2, dst_width, CV_8UC1 );    
            memcpy( tmp_img.data, frame->data[0], dst_width*dst_height );
            memcpy( tmp_img.data + dst_width*dst_height, frame->data[1], dst_width*dst_height/4 );
            memcpy( tmp_img.data + dst_width*dst_height*5/4, frame->data[2], dst_width*dst_height/4 );

          //  cv::imshow( "yuv_show", tmp_img ); 
            
            
            cv::Mat bgr;
            cv::cvtColor( tmp_img, bgr, CV_YUV2BGR_I420 );
            //cv::imshow( "bgr_show", bgr );
            
            
            //sws_scale(swsctx, frame->data, frame->linesize, 0, frame->height, frameFill->data, frameFill->linesize);


	//	printf("diennv0 %d %d %d %d\n", dst_height, dst_width, dst_height*dst_width*3, frame->linesize[0]);
                //cv::Mat img(dst_height, dst_width, CV_8UC3, framebuf.data(), frame->linesize[0]);
           
            //cv::Mat img(dst_height, dst_width, CV_8UC3, framebuf.data(), frameFill->linesize[0]);
            cv::Mat img2 = bgr.clone();
            cv::Mat img3 = bgr.clone();
	    #endif

            video_frame_count++;
            sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
            //            if (sd) 
            //            {
            //                const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
            //                for (i = 0; i < sd->size / sizeof(*mvs); i++) {
            //                    const AVMotionVector *mv = &mvs[i];
            //                    printf("%d,%2d,%2d,%2d,%4d,%4d,%4d,%4d,0x%"PRIx64"\n",
            //                           video_frame_count, mv->source,
            //                           mv->w, mv->h, mv->src_x, mv->src_y,
            //                           mv->dst_x, mv->dst_y, mv->flags);
            //                    
            //                    
            //                }
            //            }


            if (sd != nullptr) { // sd == nullptr when I frame also
                // reading motion vectors, see ff_print_debug_info2 in ffmpeg's libavcodec/mpegvideo.c for reference and a fresh doc/examples/extract_mvs.c
                AVMotionVector* mvs = (AVMotionVector*) sd->data;
                int mvcount = sd->size / sizeof (AVMotionVector);
                //Measure m2("proc");
                bool movement = mv_detector->process_frame(pts, video_frame_count, pict_type, std::vector<AVMotionVector>(mvs, mvs + mvcount));
                //std::cout << "proc: " << m2.elapsed() << std::endl;

  #ifdef _MV_DEBUG_
                mv_detector->draw_occupancy(img2);
                mv_detector->draw_motion_vectors(img3);
//std::cout << "avg_movment=" << avg_movement << std::endl;  
  #endif
                //https://github.com/diennv/MotionVectorAnalysis/blob/master/mv_detector_test.cpp
                //apt install  libopencv-dev python3-opencv

                if (movement) {
                  //   cv::putText(img2, "Movement", cv::Point(10, 200), cv::FONT_HERSHEY_SIMPLEX, 2, CV_RGB(0, 0, 255), 2, cv::LINE_AA);

                    std::cout << "trigger Motion detection events for frame " << video_frame_count << " frame type " << pict_type << std::endl << std::flush;
                } else {
                    std::cout << " frame " << video_frame_count << " frame type " << pict_type << std::endl << std::flush;
                }
              #ifdef _MV_DEBUG_    
                                
                               if (!polygon_complete)
                               {
                                    if (coords.size() == 0) {
                                      cv::putText(img2, "Double Left CLick to Start Polygon inclusion zone", cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 1,  cv::Scalar(0,0,255,255));
                                    } else {
                                      for (int i=0; i<coords.size() ; i++  ) {
                                        cv::circle( img2, coords[i], 5.0, cv::Scalar( 0, 255, 0 ), 5, 8 ); 
                                        cv::putText(img2, "Single Left CLick to add polygon vertex, close by clicking first vertex", cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 1,  cv::Scalar(0,0,255,255));
                                      }
                                    }
                                    
                                    mv_detector->xmask.clear();
                                    mv_detector->ymask.clear();
                               } else { 
                                    cv::putText(img2, "Double Right CLick inside polygon to delete Polygon", cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 1,  cv::Scalar(0,0,255,255));
                                    if (coords.size()>2)
                                    {
                                        
                                        cv::polylines(img2, coords, true, cv::Scalar( 110, 220, 0 ),  2, 8);
                                        
                                        if(  !mv_detector->xmask.size() &&   !mv_detector->ymask.size())
                                        {
                                            for (int i=0; i<coords.size() ; i++  )
                                            {
                                                mv_detector->xmask.push_back( coords[i].x/ mv_detector->_grid_step ); 
                                                mv_detector->ymask.push_back( coords[i].y/ mv_detector->_grid_step ); 
                                            }
                                        }
                                    }

                               }
         
                
                               cv::imshow("motion vectors", img3);
                               cv::imshow("occupancy", img2);
            
                               switch (cv::waitKey(run_mode == 'r' ? 10 : 0)) {
                                    case 0x1b:
                                        break;
                                    case 'p':
                                        run_mode = cv::waitKey(0) != 'r' ? 'p' : 'r';
                                        break;
                                    default:
                                        run_mode = run_mode != 'r' ? 'p' : 'r';
                                }
                #endif
            } else {
                mv_detector->process_frame(pts, video_frame_count, pict_type, std::vector<AVMotionVector>());
            }
        }
    }

    return decoded;
}

int MVExtractor::open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type) {
    int ret;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file\n",
                av_get_media_type_string(type));
        return ret;
    } else {
        int stream_idx = ret;
        st = fmt_ctx->streams[stream_idx];

        dec_ctx = avcodec_alloc_context3(dec);
        if (!dec_ctx) {
            fprintf(stderr, "Failed to allocate codec\n");
            return AVERROR(EINVAL);
        }

        ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters to codec context\n");
            return ret;
        }

        /* Init the video decoder */
        av_dict_set(&opts, "flags2", "+export_mvs", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }

        video_stream_idx = stream_idx;
        video_stream = fmt_ctx->streams[video_stream_idx];
        video_dec_ctx = dec_ctx;

        dst_width = video_stream->codecpar->width;
        dst_height = video_stream->codecpar->height;
        
         #ifdef _MV_DEBUG_
        
        const AVPixelFormat dst_pix_fmt = AV_PIX_FMT_BGR24;
        
//          swsctx = sws_getCachedContext(nullptr,
//            video_stream->codec->width, video_stream->codec->height,
//            video_stream->codec->pix_fmt,
//            dst_width, dst_height, dst_pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
//            //sws_freeContext(swsctx);
//            if (!swsctx) {
//                std::cerr << "fail to sws_getCachedContext";
//                return 2;
//            }
    
          
        
              // allocate frame buffer for output
       // frameFill = av_frame_alloc();
       // framebuf =    std::vector<uint8_t>(avpicture_get_size(dst_pix_fmt, dst_width, dst_height));
        //avpicture_fill(reinterpret_cast<AVPicture*>(frameFill), framebuf.data(), dst_pix_fmt, dst_width, dst_height);
    
         #endif   
          

        std::cout << "width " << dst_width << " height " << dst_height << std::endl << std::flush;

        mv_detector = new ContDetector(std::make_pair(dst_width, dst_height));

    }

    return 0;
}

void MVExtractor::extract(){

    
    int ret = 0, got_frame;
    
    
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        AVPacket orig_pkt = pkt;
        do {
            ret = decode_packet(&got_frame, 0);
            if (ret < 0)
                break;
            pkt.data += ret;
            pkt.size -= ret;
            
           //printf(" pkt size = %d  ret= %d \n " , pkt.size , ret );
            
        } while (pkt.size > 0);
        av_packet_unref(&orig_pkt);
    }

    /* flush cached frames */
    pkt.data = NULL;
    pkt.size = 0;
    do {
        decode_packet(&got_frame, 1);
    } while (got_frame);
}
