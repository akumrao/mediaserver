// based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html
#include "ff/ff.h"
#include "ff/codec.h"
#include "ff/mediacapture.h"
#include "base/define.h"
#include "base/test.h"
#include "base/time.h"
#include "base/logger.h"
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <thread>
#include <string>
#include <vector>
#define IOBUFSIZE 40960

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    // printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
    //        tag,
    //        av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
    //        av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
    //        av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
    //        pkt->stream_index);
}

std::vector<uint8_t> outputData;

int mediaMuxCallback(void *opaque, uint8_t *buf, int bufSize) {
    outputData.insert(outputData.end(), buf, buf + bufSize);
    return bufSize;
}

int main(int argc, char **argv) {

    av_register_all();
    // init network
    avformat_network_init();

    avcodec_register_all();

    const AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;

    if (argc < 3) {
        printf("usage: %s input output\n"
                "API example program to remux a media file with libavformat and libavcodec.\n"
                "The output format is guessed according to the file extension.\n"
                "\n", argv[0]);
        return 1;
    }

    in_filename = argv[1];
    out_filename = argv[2];

    int fragmented_mp4_options = 0;

    if (argc < 3) {
        printf("You need to pass at least two parameters.\n");
        return -1;
    } else if (argc == 4) {
        fragmented_mp4_options = 1;
    }


    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);

        return -1;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        return -1;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    ////////////////////////////////////////////////////////////////////// for file 

    //    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    //    if (!ofmt_ctx) {
    //        fprintf(stderr, "Could not create output context\n");
    //        ret = AVERROR_UNKNOWN;
    //        goto end;
    //    }


    ////////////////////////////////////////////////////////////////////////for buffer 

    std::fstream file(argv[2], std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Couldn't open file " << argv[1] << std::endl;
        return 2;
    }


    AVIOContext *ioCtxt;
    uint8_t *ioBuffer;

    AVOutputFormat *outputFormat = av_guess_format("mp4", nullptr, nullptr);


    avformat_alloc_output_context2(&ofmt_ctx, outputFormat, NULL, NULL);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        // goto end;
        return -1;
    }


    if ((ioBuffer = (uint8_t*) av_malloc(IOBUFSIZE)) == nullptr) {
        std::cout << "Couldn't allocate I/O buffer" << std::endl;
        return 8;
    }
    if ((ioCtxt = avio_alloc_context(ioBuffer, IOBUFSIZE, 1, nullptr, nullptr, mediaMuxCallback, nullptr)) == nullptr) {
        std::cout << "Couldn't initialize I/O context" << std::endl;
        return 9;
    }

    //Set video stream data

    ofmt_ctx->pb = ioCtxt;

    /////////////////////////////////////////////////////////////////////////////////

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = (int*) av_mallocz_array(stream_mapping_size, sizeof (*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        return -1;
    }

    ofmt = ofmt_ctx->oformat;

    int64_t frameInterval;
    
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }
        
        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
             frameInterval =  base::ff::fpsToInterval(int( in_stream->avg_frame_rate.num/in_stream->avg_frame_rate.den ));
            
        }
         
         

        stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return -1;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    //    if (!(ofmt->flags & AVFMT_NOFILE)) {
    //        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
    //        if (ret < 0) {
    //            fprintf(stderr, "Could not open output file '%s'", out_filename);
    //            goto end;
    //        }
    //    }

    AVDictionary* opts = NULL;

    if (fragmented_mp4_options) {
        // https://developer.mozilla.org/en-US/docs/Web/API/Media_Source_Extensions_API/Transcoding_assets_for_MSE
        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
    }
    // 

    ret = avformat_write_header(ofmt_ctx, &opts);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }

     int64_t ptsOffset = 0;
     int64_t dtsOffset = 0;
     int64_t tmpptsOffset = 0;
     int64_t tmpdtsOffset = 0;

    int64_t lastTimestamp = base::time::hrtime();
     
    while (1) {
        AVStream *in_stream, *out_stream;
        
     
        ret = av_read_frame(ifmt_ctx, &pkt);
        
         if (ret == AVERROR_EOF) {
            if(0) {
                auto stream = ifmt_ctx->streams[pkt.stream_index];
                avio_seek(ifmt_ctx->pb, 0, SEEK_SET);
                avformat_seek_file(ifmt_ctx, pkt.stream_index, 0, 0, stream->duration, 0);
                
                 ptsOffset += tmpptsOffset+1024;
                 dtsOffset += tmpdtsOffset+1024;
        
                continue;
            }
        }
        
        if (ret < 0)
            break;

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= stream_mapping_size ||
                stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }
        
        AVCodecParameters *in_codecpar = in_stream->codecpar;

//        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
//        {
//                auto nsdelay = frameInterval - (base::time::hrtime() - lastTimestamp);
//               // LDebug("Sleep delay: ", nsdelay, ", ", (time::hrtime() - lastTimestamp), ", ", frameInterval)
//                std::this_thread::sleep_for(std::chrono::nanoseconds(nsdelay));
//                // base::sleep( nsdelay/1000);
//                lastTimestamp = base::time::hrtime();
//        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, static_cast<AVRounding> (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, static_cast<AVRounding> (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        tmpptsOffset = pkt.pts;
        tmpdtsOffset = pkt.dts;
     
        pkt.pos = -1;
        log_packet(ofmt_ctx, &pkt, "out");
        
        pkt.pts += ptsOffset;
        pkt.dts += dtsOffset;
        
        
      
        
        ret = av_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    //  av_write_trailer(ofmt_ctx);

    ret = av_write_frame(ofmt_ctx, nullptr); //Flush if something has been left

    //Write media data in container to file
    // file.open("my_mp4.mp4", std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Couldn't open output file " << std::endl;
        return 12;
    }

    file.write((char*) outputData.data(), outputData.size());
    if (file.fail()) {
        std::cout << "Couldn't write to file" << std::endl;
        return 13;
    }


    file.close();




    avformat_close_input(&ifmt_ctx);

    ///////////////// for buffer
    av_free(ioBuffer);
    av_free(ioCtxt);
    ////////////////////
    /////////////////for enabl following
    /* close output */
    //   if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    //        avio_closep(&ofmt_ctx->pb);


    avformat_free_context(ofmt_ctx);

    av_freep(&stream_mapping);

    if (ret < 0 && ret != AVERROR_EOF) {
        //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }


    return 0;
}

