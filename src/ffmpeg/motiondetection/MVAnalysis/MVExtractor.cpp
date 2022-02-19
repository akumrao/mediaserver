
#include "MVExtractor.h"

MVExtractor::MVExtractor(const char *src_filename) {
    int ret = 0;

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

            // get pts
            int64_t pts = frame->pts != AV_NOPTS_VALUE ? frame->pts : (frame->pkt_dts != AV_NOPTS_VALUE ? frame->pkt_dts : pts + 1);


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

                //                mv_detector->draw_occupancy(img2);
                //                mv_detector->draw_motion_vectors(img3);
                //std::cout << "avg_movment=" << avg_movement << std::endl;

                if (movement) {
                    //  cv::putText(img2, "Movement", cv::Point(10, 200), cv::FONT_HERSHEY_SIMPLEX, 2, CV_RGB(0, 0, 255), 2, cv::LINE_AA);

                    std::cout << "trigger Motion detection events for frame " << video_frame_count << " frame type " << pict_type << std::endl << std::flush;
                } else {
                    std::cout << " frame " << video_frame_count << " frame type " << pict_type << std::endl << std::flush;
                }
                //                cv::imshow("motion vectors", img3);
                //                cv::imshow("occupancy", img2);
                //#if 1
                //                switch (cv::waitKey(run_mode == 'r' ? 10 : 0)) {
                //                    case 0x1b:
                //                        break;
                //                    case 'p':
                //                        run_mode = cv::waitKey(0) != 'r' ? 'p' : 'r';
                //                        break;
                //                    default:
                //                        run_mode = run_mode != 'r' ? 'p' : 'r';
                //                }
                //#endif
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


        mv_detector = new MVDetector(std::make_pair(dst_width, dst_height));

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