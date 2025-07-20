
#include "MVExtractor.h"
#include "base/logger.h"

#include "base/datetime.h"

#include "base/filesystem.h"
#include "codec.h"
#include "Settings.h"
#include "muxer.h"
#include "tools.h"
#include "MVDetectorContour.h"
#include "MVDetector.h"

#ifdef _MV_DEBUG_
#include<opencv2/highgui.hpp>
char run_mode = 'r';
#endif



//#define store "/mnt/clips-storage/"



namespace base {
namespace fmp4 {

MVExtractor::MVExtractor(const char *name, std::string & cam,  FrameFilter *txt, FrameFilter *muxer, FrameFilter *fileFilter ,FrameFilter *next): cam(cam), txt(txt), muxer(muxer), fileFilter(fileFilter),FrameFilter(name, next)
{
    if(!txt)
      return;
        
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        exit(1);
    }
    
    if (!base::fs::exists(Settings::configuration.storage))
    {
        base::fs::mkdir(Settings::configuration.storage);
    }
    
    
    if (base::fs::exists(Settings::configuration.storage + cam))
    {
        //base::fs::rmdirr(Settings::configuration.storage + cam);
    }
    else
    base::fs::mkdir(Settings::configuration.storage + cam);
    
   // config.load(Settings::configuration.storage + cam + "/"+ cam +".js");
    
    Settings::getMDConfig(cam,mdConfig );
    
    
    
    this->avpkt = new AVPacket();
    av_init_packet(this->avpkt);
    
     
}



MVExtractor::~MVExtractor() {
    if (mv_detector) {
        delete mv_detector;
        mv_detector = nullptr;
    }

    av_frame_free(&frame);
    avcodec_free_context(&av_codec_context);
    av_packet_unref(avpkt);
    delete avpkt;


}

//int MVExtractor::read(const char *src_filename) {
//
//
//    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
//        fprintf(stderr, "Could not open source file %s\n", src_filename);
//        exit(1);
//    }
//
//    av_dump_format(fmt_ctx, 0, src_filename, 0);
//
//    return 0;
//}



int MVExtractor::Init( BasicFrame *bs) 
{
    
    fragmp4_muxer   =  (FragMP4MuxFrameFilter*)muxer;

    fileSave        =  (FileFrameFilter*)fileFilter;



    AVStream *av_stream;

    av_codec_context = avcodec_alloc_context3(avcodec_find_decoder(AV_CODEC_ID_H264));
    av_codec_context->width =  bs->width; // dummy values .. otherwise mkv muxer refuses to co-operate
    av_codec_context->height =  bs->height;
    av_codec_context->bit_rate = 1024 * 1024;


    AVRational tb;
    tb.num = 1;

    tb.den = bs->fps;

    av_codec_context->time_base = tb;//  (AVRational){ 1, STREAM_FRAME_RATE };



    av_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    ///*
    //av_codec_context->extradata = extradata_videoframe.payload.data();
   // av_codec_context->extradata_size = extradata_videoframe.payload.size();


   // av_codec_context->extradata = ( uint8_t *)av_malloc(extradata_videoframe.payload.size() + AV_INPUT_BUFFER_PADDING_SIZE);
    //memcpy(av_codec_context->extradata, extradata_videoframe.payload.data(), extradata_videoframe.payload.size());





    ///AVStream *st;

    AVCodec *dec = NULL;
    int ret;
    AVDictionary *opts = NULL;

//        st = fmt_ctx->streams[0];
//
//       
//
//       int ret = avcodec_parameters_to_context(video_dec_ctx, st->codecpar);
//        if (ret < 0) {
//            fprintf(stderr, "Failed to copy codec parameters to codec context\n");
//            return ret;
//        }

    /* Init the video decoder */
    av_dict_set(&opts, "flags2", "+export_mvs", 0);
    if ((ret = avcodec_open2(av_codec_context, dec, &opts)) < 0) {
        fprintf(stderr, "Failed to open codec\n");
        return ret;
    }
        

    if(mdConfig.contourArea)
    {
        mv_detector = new MVDetectorContour(std::make_pair( bs->width,  bs->height) ,mdConfig , mdConfig.contourArea);
    }
    else
    {
        mv_detector = new MVDetector(std::make_pair( bs->width,  bs->height) ,mdConfig );
    }
    
    
     
    dst_width = bs->width;    
    dst_height = bs->height;
    initialized = true;
     
  
}


int MVExtractor::send()
{
    
    if( continueMotion &&  !iFrameCount)
    {
        ++iFrameCount ;
        continueMotion = 0;
      
       // SInfo << "Send ";
        
       // std::time_t ct = std::time(0);
       // std::string time = std::ctime(&ct);
        
        std::string res;
        Timestamp ts;
        DateTimeFormatter::append(res, ts, DateTimeFormat::SORTABLE_FORMAT);
                    
       //  #if BroswerStream
        txtFrame.txt= "MD_Start_" + cam + "_" + res + "_FNO_" + std::to_string(priClip.size());
        
        //config.setRaw(txtFrame.txt, "") ;
        //txtFrame.frameType = FRAMETYPE::TEXT;
        txt->run( &txtFrame);
        SDebug <<  txtFrame.txt ;

        txtFrame.txt= "MD_End_" + cam + "_" + res+  "_FNO_" + std::to_string(priClip.size());

       // #endif
        
//        basicframe.copyBuf((uint8_t*)sps.data(), sps.size() );
//        
//        if(fragmp4_muxer)
//           fragmp4_muxer->run(&basicframe); 
//         
//        basicframe.copyBuf((uint8_t*)pps.data(), pps.size() );
//        
//        if(fragmp4_muxer)
//           fragmp4_muxer->run(&basicframe); 
        
        fileSave->Open( Settings::configuration.storage + cam + "/" +res + ".mp4");
       
        parseH264Header();
          
        fragmp4_muxer->run(&sps); 
        fragmp4_muxer->run(&pps); 
        
        for( auto clip : priClip)
        {
            ++iFrameCount;
            //SInfo << "Pri Clip  frame count " << iFrameCount << "  clipe type " <<  clip.h264_pars.slice_type;
        
            #if FPSONE 
            if(clip.iFrame)
            #endif
            fragmp4_muxer->run(&clip); 
        }
        
    }
    
}


void MVExtractor::go(Frame *frame)
{
    
  //  this->go(frame);
  // chaining of run is done at write_packet
    BasicFrame *bs = static_cast<BasicFrame*> (frame);
    
    if(!initialized )
    {
        Init(bs);
    }
     
     
    
    if(bs->h264_pars.slice_type  ==   H264SliceType::sps )
    {
       sps = *bs;
       
       #if FPSONE
       if(Settings::configuration.DROP_P_Frame)
         sps.fps = 1;
       #endif  
       
    }
   
    
    if(bs->h264_pars.slice_type  ==   H264SliceType::pps )
    {
       pps = *bs;
       #if FPSONE
       if(Settings::configuration.DROP_P_Frame)
         pps.fps = 1;
       #endif
       
    }
    
    bs->fillAVPacket(avpkt);
    extract(avpkt, bs->h264_pars.slice_type , bs);
     
   
    
    
}


bool MVExtractor::extract( AVPacket *pk ,  short unsigned slice_type , BasicFrame *bs ){
    
   

    int decoded = pk->size;

    int got_frame = 0;
    
    bool mp4ResetTimeLine =false; 
    
   
    
   /*  
    else if(slice_type  ==   H264SliceType::idr )
    {
        SInfo << " IDr frame "; 
    }
    else if(slice_type  ==   H264SliceType::nonidr )
    {
         SInfo << " Nont IDr frame "; 
    }
    */
        

  //  if (pk->stream_index == video_stream_idx) 
       {
        int ret = avcodec_decode_video2(av_codec_context, frame, &got_frame, pk);
        if (ret < 0) {
           // fprintf(stderr, "Error decoding video frame \n");
            if(!(bs->h264_pars.slice_type  ==   H264SliceType::sps || (bs->h264_pars.slice_type  ==   H264SliceType::pps )))
            SWarn <<  "Error decoding video frame";

            return ret;
        }

        if (got_frame) {

            char pict_type = av_get_picture_type_char(frame->pict_type);
            
            #if FPSONE

            if(frame->pict_type  == AVPictureType::AV_PICTURE_TYPE_I)
            {
                 bs->iFrame = true;
                 incPframe = 0;
                 if(!foundGop &&  gop)
                 {
                    foundGop = true;
                     
                    float  noIFrame= (bs->fps*20)/gop;  
                     
                    if(20 - noIFrame > 0)
                    {
                        pFrame=  (20 - noIFrame)/noIFrame;
                    }
                     
                 }
            }
            else if( incPframe < pFrame )
            {
                 bs->iFrame = true;
                 ++incPframe;
            }
            else
            {
                bs->iFrame = false;
            }
            
            if(!foundGop)
            {   
                ++gop;
                return 0;
            }
            
            #endif       
            
            if( iFrameCount && ( slice_type  == H264SliceType::idr  ||  iFrameCount >  (mdConfig.CLIPSIZE * bs->fps +35)) )
            {
                
                if( ++iFrameCount >  mdConfig.CLIPSIZE * bs->fps +35  )
                {
                   
                    
                    /////////////////
                    #if BroswerStream
                    
                    std::string res;
                    Timestamp ts;
                    DateTimeFormatter::append(res, ts, DateTimeFormat::SORTABLE_FORMAT);
                    
                    base::fs::savefile( Settings::configuration.storage + cam + "/" +res +".264", (const char*) h264.data(),h264.size() );
                    
                    //config.setRaw(txtFrame.txt,  Settings::configuration.storage + cam + "/" +res +".264") ;
                    //config.save();
                    
                    
                    SInfo << "saved H264 Cip " << res;
                                     
                    h264.clear();
                    if(sps.payload.size() && pps.payload.size())
                        h264 += std::string( (char*) sps.payload.data(), sps.payload.size()) + std::string( (char*)pps.payload.data(), pps.payload.size());
                    #endif    

                    
                   // SInfo << " save Clip clip size " << priClip.size()  << " iFrameCount "  << iFrameCount;
                    
                    iFrameCount = 0;
                    
                    fileSave->Close();
                   // priClip.clear();
                    
                    
                    txt->run( &txtFrame);
                    SDebug <<  txtFrame.txt ;
                    
                    mp4ResetTimeLine =true; 
                    
                }
            }
            
            if( slice_type  == H264SliceType::idr )
            {
              // int x = priClip.size();
               
               
               
               if(gop )
               {
                    priClipGop.push(gop);
                    int gopfromFront= priClipGop.front();
               
                
                    while( priClip.size() >   Settings::configuration.pre_clip * bs->fps -4   && priClip.size()  >= gopfromFront )
                    {

                       // SInfo << " Before Erase buffer "  << priClip.size() << " type " <<  priClip.begin()->h264_pars.slice_type ;


                        priClip.erase( priClip.begin() , priClip.begin() + gopfromFront);

                        priClipGop.pop();

                        //SInfo << "After Erase "  << priClip.size()  << " gop " << gopfromFront  << " type " <<  priClip.begin()->h264_pars.slice_type ;
                        
                        gopfromFront= priClipGop.front();
                    }
                }
              // if(mx)
              // mx->clipData.clear();
               #if BroswerStream
               h264.clear();
               if(sps.payload.size() && pps.payload.size())
                    h264 += std::string( (char*) sps.payload.data(), sps.payload.size()) + std::string( (char*)pps.payload.data(), pps.payload.size());
               #endif    
               
               mp4ResetTimeLine =true;
               
               //SInfo << "Erase OLD IDRs and add new " << priClip.size() ;
               
               priClip.push_back( *bs);
               
               gop = 1;


            }
            else if(  priClip.size() &&  (bs->h264_pars.slice_type  ==   H264SliceType::nonidr)  )
            {
                
                ++gop ;
                
                priClip.push_back( *bs);
            }
            
            
            
            if( iFrameCount &&  fileSave->fp_out)
            {
                 ++iFrameCount;
 		 #if FPSONE
                 if(bs->iFrame)
                 #endif  
                 fragmp4_muxer->run(bs); 
            }
            
            #if BroswerStream
            if(h264.size())
            {
             
                h264 = h264 +  std::string( (const char*)  pk->data, pk->size);
            }
            #endif 
            
           

            
          //  printf(" Pic type  %c  %d\n " , pict_type, decoded );
            // get pts
            int64_t pts = frame->pts != AV_NOPTS_VALUE ? frame->pts : (frame->pkt_dts != AV_NOPTS_VALUE ? frame->pkt_dts : pts + 1);


            video_frame_count++;

            if (!iFrameCount) 
            {
                AVFrameSideData *sd;
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
                    bool movement = mv_detector->process_frame(cam, pts, video_frame_count, pict_type, std::vector<AVMotionVector>(mvs, mvs + mvcount));
                    //std::cout << "proc: " << m2.elapsed() << std::endl;

                    //                mv_detector->draw_occupancy(img2);
                    //                mv_detector->draw_motion_vectors(img3);
                    //std::cout << "avg_movment=" << movement  << "  iFrameCount " <<   iFrameCount   << "  priClip.size()" <<  priClip.size() << std::endl;

                    if (movement && priClip.size() >=   Settings::configuration.pre_clip * bs->fps -5) {

                        //  cv::putText(img2, "Movement", cv::Point(10, 200), cv::FONT_HERSHEY_SIMPLEX, 2, CV_RGB(0, 0, 255), 2, cv::LINE_AA);
                        if (continueMotion < 1) {
                            continueMotion++;

                            // if( continueMotion == MOTIONCOUNT)
                            //  iFrameCount++; 

                            base::SInfo << "trigger Motion detection events for frame " << video_frame_count << " frame type " << pict_type;
                        }
                    } else {
                        continueMotion = 0;
                        // std::cout << " frame " << video_frame_count << " frame type " << pict_type << std::endl << std::flush;
                    }
                    #ifdef _MV_DEBUG_ 

                                cv::Mat tmp_img = cv::Mat::zeros( dst_height*3/2, dst_width, CV_8UC1 );    
                                memcpy( tmp_img.data, frame->data[0], dst_width*dst_height );
                                memcpy( tmp_img.data + dst_width*dst_height, frame->data[1], dst_width*dst_height/4 );
                                memcpy( tmp_img.data + dst_width*dst_height*5/4, frame->data[2], dst_width*dst_height/4 );

                               // cv::imshow( "yuv_show", tmp_img ); gray scale 
                                
                                
                                cv::Mat bgr;
                                cv::cvtColor( tmp_img, bgr, CV_YUV2BGR_I420 );
                                //cv::imshow( "bgr_show", bgr );
                                
                                
                                //sws_scale(swsctx, frame->data, frame->linesize, 0, frame->height, frameFill->data, frameFill->linesize);


                        //  printf("diennv0 %d %d %d %d\n", dst_height, dst_width, dst_height*dst_width*3, frame->linesize[0]);
                                    //cv::Mat img(dst_height, dst_width, CV_8UC3, framebuf.data(), frame->linesize[0]);
                               
                                //cv::Mat img(dst_height, dst_width, CV_8UC3, framebuf.data(), frameFill->linesize[0]);
                                cv::Mat img2 = bgr.clone();
                                cv::Mat img3 = bgr.clone();
            


//                                mv_detector->draw_occupancy(img2);
                                mv_detector->draw_motion_vectors(img3);

                                
//                               if (!polygon_complete)
//                               {
//                                    if (coords.size() == 0) {
//                                      cv::putText(img2, "Double Left CLick to Start Polygon inclusion zone", cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 1,  cv::Scalar(0,0,255,255));
//                                    } else {
//                                      for (int i=0; i<coords.size() ; i++  ) {
//                                        cv::circle( img2, coords[i], 5.0, cv::Scalar( 0, 255, 0 ), 5, 8 ); 
//                                        cv::putText(img2, "Single Left CLick to add polygon vertex, close by clicking first vertex", cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 1,  cv::Scalar(0,0,255,255));
//                                      }
//                                    }
//                                    
//                                    mv_detector->xmask.clear();
//                                    mv_detector->ymask.clear();
//                               } else { 
//                                    cv::putText(img2, "Double Right CLick inside polygon to delete Polygon", cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 1,  cv::Scalar(0,0,255,255));
//                                    if (coords.size()>2)
//                                    {
//                                        
//                                        cv::polylines(img2, coords, true, cv::Scalar( 110, 220, 0 ),  2, 8);
//                                        
//                                        if(  !mv_detector->xmask.size() &&   !mv_detector->ymask.size())
//                                        {
//                                            for (int i=0; i<coords.size() ; i++  )
//                                            {
//                                                mv_detector->xmask.push_back( coords[i].x/ mv_detector->_grid_step ); 
//                                                mv_detector->ymask.push_back( coords[i].y/ mv_detector->_grid_step ); 
//                                            }
//                                            
//                                            int x = 1;
//                                        }
//                                    }
//
//                               }
         
                
                               cv::imshow("motion vectors", img3);
  //                             cv::imshow("occupancy", img2);
            
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
                    mv_detector->process_frame(cam, pts, video_frame_count, pict_type, std::vector<AVMotionVector>());
                }

            }


        }
    }
    
    
    
    send();

    

    return mp4ResetTimeLine;
}



bool MVExtractor::parseH264Header() 
{
    int ret = 0;
   // AVCodec *codec = NULL;
  //  AVCodecContext *cdc_ctx = NULL;

    BasicFrame  basicframe; 

    SetupFrame        setupframe;  ///< This frame is used to send subsession information



    basicframe.media_type           =AVMEDIA_TYPE_VIDEO;
    basicframe.codec_id             =AV_CODEC_ID_H264;
    basicframe.stream_index     =0;
    // prepare setup frame
    
    
    setupframe.sub_type             =SetupFrameType::stream_init;
    setupframe.media_type           =AVMEDIA_TYPE_VIDEO;
    setupframe.codec_id             =AV_CODEC_ID_H264;   // what frame types are to be expected from this stream
    setupframe.stream_index     = 0;
    setupframe.mstimestamp      = CurrentTime_milliseconds();
    // send setup frame
    //info->run(&setupframe);
    fragmp4_muxer->run(&setupframe);

    //cur_size = fread(in_buffer, 1, in_buffer_size, fileVideo);
    //cur_ptr = in_buffer;

//           avcodec_close(cdc_ctx);
//            avcodec_free_context(&cdc_ctx);
    return  true;

}


}
}
