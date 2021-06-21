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
#define IOBUFSIZE 1024

/**
 * @file
 * libavformat API example.
 *
 * Output a media file in any supported libavformat format. The default
 * codecs are used.
 * @example muxing.c
 */
 
#ifdef av_err2str
#undef av_err2str
#include <string>
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif  // av_err2str


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// #include <libavutil/channel_layout.h>
// #include <libavutil/opt.h>
// #include <libavutil/mathematics.h>
// #include <libavutil/timestamp.h>
// #include <libavcodec/avcodec.h>
// #include <libavformat/avformat.h>
// #include <libswscale/swscale.h>




extern "C" {
 #include <libavcodec/avcodec.h>
#include <libavutil/avassert.h>    
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#ifdef HAVE_FFMPEG_SWRESAMPLE
#include <libswresample/swresample.h>
#else
#include <libavresample/avresample.h>
#endif
}



/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * audio encoding with libavcodec API example.
 *
 * @example encode_audio.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels   = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout    = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

int main(int argc, char **argv)
{
    const char *filename;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVFrame *frame;
    AVPacket pkt;
    int ret, got_output;
    FILE *fout;
    uint16_t *samples;
    //float t, tincr;


    filename = "/tmp/test.aac";
    
   const char* inputFile ="/var/tmp/test.pcm";  

    /* register all the codecs */
    avcodec_register_all();

    /* find the MP2 encoder */
  //  codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    codec = avcodec_find_encoder_by_name("libfdk_aac"); //Specify the use of file encoding type
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(c->sample_fmt));
        exit(1);
    }

    /* select other audio parameters supported by the encoder */
    c->sample_rate    = select_sample_rate(codec);
    c->channel_layout = AV_CH_LAYOUT_STEREO;//select_channel_layout(codec);
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->profile = FF_PROFILE_AAC_LOW;
    /* open it */
    
   //  c->flags |= AV_CODEC_FLAG_GLOBAL_HEADE;
             
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    fout = fopen(filename, "wb");
    if (!fout) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }

    frame->nb_samples     = c->frame_size;
    frame->format         = c->sample_fmt;
    frame->channel_layout = c->channel_layout;

    /* allocate the data buffers */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate audio data buffers\n");
        exit(1);
    }
    
    int size = av_samples_get_buffer_size(NULL, c->channels,c->frame_size,c->sample_fmt, 1);
    uint8_t* frame_buf = (uint8_t *)av_malloc(size);
  

   FILE* in_file = fopen(inputFile,"rb");
    if(in_file){
        av_log(NULL,AV_LOG_INFO,"open file success \n");
    }else{
        av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
        return -1;
    }

///////////////////////////////////////////////////////
    while(1){   
       if (fread(frame_buf, 1, size, in_file) <= 0){
            printf("Failed to read raw data! \n");
            break;
            
            
        }else if(feof(in_file)){
            break;
        }
       
       av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;

        
       ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);
       
       frame->data[0] = frame_buf;  //PCM Data
        //pFrame->pts=i*100;
       // encode(encodeContext,pFrame,&pkt);
        //i++;
       
        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, fout);
            av_packet_unref(&pkt);
        }
    }
   // av_write_trailer(outputContext);

////////////////////////////////////////////////////////////    

//    /* encode a single tone sound */
//    t = 0;
//    tincr = 2 * M_PI * 440.0 / c->sample_rate;
//    for (i = 0; i < 200; i++) {
//        av_init_packet(&pkt);
//        pkt.data = NULL; // packet data will be allocated by the encoder
//        pkt.size = 0;
//
//        /* make sure the frame is writable -- makes a copy if the encoder
//         * kept a reference internally */
//        ret = av_frame_make_writable(frame);
//        if (ret < 0)
//            exit(1);
//        samples = (uint16_t*)frame->data[0];
//
//        for (j = 0; j < c->frame_size; j++) {
//            samples[2*j] = (int)(sin(t) * 10000);
//
//            for (k = 1; k < c->channels; k++)
//                samples[2*j + k] = samples[2*j];
//            t += tincr;
//        }
//        /* encode the samples */
//        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
//        if (ret < 0) {
//            fprintf(stderr, "Error encoding audio frame\n");
//            exit(1);
//        }
//        if (got_output) {
//            fwrite(pkt.data, 1, pkt.size, f);
//            av_packet_unref(&pkt);
//        }
//    }

    /* get the delayed frames */
//    for (got_output = 1; got_output; i++) {
//        ret = avcodec_encode_audio2(c, &pkt, NULL, &got_output);
//        if (ret < 0) {
//            fprintf(stderr, "Error encoding frame\n");
//            exit(1);
//        }
//
//        if (got_output) {
//            fwrite(pkt.data, 1, pkt.size, fout);
//            av_packet_unref(&pkt);
//        }
//    }
    fclose(fout);
    fclose(in_file);
    

    av_frame_free(&frame);
    avcodec_free_context(&c);

    return 0;
}

















#if 0 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include <stdlib.h>
#include <stdio.h>
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libswresample/swresample.h"
AVFormatContext* inputContext;
AVFormatContext* outputContext;
AVCodecContext* decodeContext;
AVCodecContext* encodeContext;
AVStream* stream;
void init(){
    avcodec_register_all();
}
int open_output_context(char* filename){
   // outputContext = avformat_alloc_context();
    
    AVOutputFormat *outputFormat = av_guess_format("aac", nullptr, nullptr);

    /* allocate the output media context */
    int ret = avformat_alloc_output_context2(&outputContext, outputFormat, NULL, NULL);
    

    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"create outputContext failed \n");
        return -1;
    }else{
        av_log(NULL,AV_LOG_INFO,"create outputContext success \n");
    }
    ret = avio_open(&outputContext->pb,filename,AVIO_FLAG_READ_WRITE);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"open file failed  \n");
        return -1;
    }else{
          av_log(NULL,AV_LOG_INFO,"open file success  \n");
    }
    stream = avformat_new_stream(outputContext,NULL);
    if(!stream){
        av_log(NULL,AV_LOG_ERROR,"avformat_new_stream failed  \n");
        return -1;
    }else{
        av_log(NULL,AV_LOG_INFO,"avformat_new_stream success  \n");
    }
    av_dump_format(outputContext,0,filename,1);
    return ret;
}
void close(){
    if(decodeContext){
        avcodec_close(decodeContext);
    }
    if(inputContext){
        avformat_close_input(&inputContext);
    }
    if(outputContext){
        for(int i = 0 ; i < outputContext->nb_streams; i++){
            AVCodecContext* codecContext = outputContext->streams[i]->codec;
            avcodec_close(codecContext);
        }
        avformat_close_input(&outputContext);
    }
}
int init_encode(AVStream* audio_stream){ 
    encodeContext = audio_stream->codec;
    encodeContext->codec = outputContext->audio_codec;
    encodeContext->codec_id = outputContext->audio_codec_id;
    encodeContext->codec_type = AVMEDIA_TYPE_AUDIO;
    /* check that the encoder supports s16 pcm input */
    encodeContext->sample_fmt = AV_SAMPLE_FMT_S16;
    encodeContext->sample_rate    = 44100;
    encodeContext->channel_layout=AV_CH_LAYOUT_STEREO;
    encodeContext->channels       = av_get_channel_layout_nb_channels(encodeContext->channel_layout);
    encodeContext->bit_rate = 64000; 
    AVCodec* pCodec = avcodec_find_encoder_by_name("libfdk_aac");
     if(pCodec){
        av_log(NULL,AV_LOG_INFO,"avcodec_find_encoder success\n");
     }else{
        av_log(NULL,AV_LOG_ERROR,"avcodec_find_encoder failed\n");
        return -1;
     }
    return avcodec_open2(encodeContext,pCodec,NULL);
}
static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt)
{
    int ret;
    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        av_write_frame(outputContext,pkt);
        av_packet_unref(pkt);
    }
}
int main(){
    init();
    char* inputFile = "/var/tmp/test.pcm";
    char* outputFile = "jichi2.aac";
    FILE* in_file = fopen(inputFile,"rb");
    if(in_file){
        av_log(NULL,AV_LOG_INFO,"open file success \n");
    }else{
        av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
        return 0;
    }

    int ret = open_output_context(outputFile);
    if(ret >= 0){
        av_log(NULL,AV_LOG_INFO,"open_output_context success \n");
    }else{
        av_log(NULL,AV_LOG_ERROR,"open_output_context failed! \n");
         return 0;
    }
   
    if (stream==NULL){
        av_log(NULL,AV_LOG_ERROR,"avformat_new_stream failed \n");
        return 0;
    }else{
        av_log(NULL,AV_LOG_INFO,"avformat_new_stream success \n");
    }

    ret = init_encode(stream);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"init_encoder failed \n");
        return 0;
    }else{
        av_log(NULL,AV_LOG_INFO,"init encoder success \n");
    }
    ret = avcodec_parameters_from_context(stream->codecpar, encodeContext);
    if(ret < 0){
        printf("avcodec_parameters_from_context failed \n");
    }else{
        printf("avcodec_parameters_from_context success \n");
    }

    AVFrame* pFrame = av_frame_alloc();
    pFrame->nb_samples= encodeContext->frame_size;
    pFrame->format= encodeContext->sample_fmt;
 
    int size = av_samples_get_buffer_size(NULL, encodeContext->channels,encodeContext->frame_size,encodeContext->sample_fmt, 1);
    uint8_t* frame_buf = (uint8_t *)malloc(size);
    avcodec_fill_audio_frame(pFrame, encodeContext->channels, encodeContext->sample_fmt,(const uint8_t*)frame_buf, size, 1);
 
    //Write Header
    avformat_write_header(outputContext,NULL);
    AVPacket pkt;
    av_new_packet(&pkt,size);
    int got_frame;
    int i = 0;
    while(1){   
       if (fread(frame_buf, 1, size, in_file) <= 0){
            printf("Failed to read raw data! \n");
            break;
        }else if(feof(in_file)){
            break;
        }
        pFrame->data[0] = frame_buf;  //PCM Data
        pFrame->pts=i*100;
        encode(encodeContext,pFrame,&pkt);
        i++;
    }
    av_write_trailer(outputContext);
    printf("Encoding completed\n");
    AVCodec* encoder = avcodec_find_encoder(outputContext->oformat->audio_codec);
    if(!encoder){
        av_log(NULL,AV_LOG_ERROR,"can't find endcoder \n");
    }else{
        av_log(NULL,AV_LOG_INFO," find endcoder \n");
        av_log(NULL,AV_LOG_ERROR,"encoder name = %s \n",encoder->name);
    }
    av_free(frame_buf);
  
    av_frame_unref(pFrame);
    if(in_file){
        fclose(in_file);
    }
    close();
    return 0;
}
#endif