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

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#define MAX_CHUNK_SIZE 10240*8
// maximum send buffer 262144  =1024 *256

#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576




#define IOBUFSIZE 40960
//40960*6

namespace base {
    namespace fmp4 {


        // based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html


        FFParse::FFParse( base::net::ClientConnecton *conn) :  fragmp4_filter("fragmp4",conn ), fragmp4_muxer("fragmp4muxer", &fragmp4_filter), info("info", nullptr) {

            fragmp4_muxer.activate();

        }

        FFParse::~FFParse() {
            SInfo << "~FFParse( )";
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


            parseH264("/experiment/live/testProgs/test.264");
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
            fileName = "/var/tmp/kunal720.mp4";
            //fmp4(fileName.c_str(), "fragTmp.mp4");
            //fmp4(fileName.c_str());
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

     
        void FFParse::parseH264(const char *input_file) {
            int ret = 0;
           // AVCodec *codec = NULL;
          //  AVCodecContext *cdc_ctx = NULL;
            AVPacket *pkt = NULL;
            //AVFrame *frame = NULL;
            FILE *fp_in, *fp_out;
           // AVFormatContext *fmt_ctx = NULL;
        //    AVCodecParserContext *parser = NULL;
            
            
         //    u_int8_t*         fReceiveBuffer;
          //  long unsigned     nbuf;       ///< Size of bytebuffer

          //  char*             fStreamId;
           // FrameFilter&      framefilter;
            SetupFrame        setupframe;  ///< This frame is used to send subsession information
            BasicFrame        basicframe;  ///< Data is being copied into this frame
            int               subsession_index;
            
            subsession_index = 0;
            basicframe.media_type           =AVMEDIA_TYPE_VIDEO;
            basicframe.codec_id             =AV_CODEC_ID_H264;
            basicframe.subsession_index     =subsession_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_VIDEO;
            setupframe.codec_id             =AV_CODEC_ID_H264;   // what frame types are to be expected from this stream
            setupframe.subsession_index     =subsession_index;
            setupframe.mstimestamp          = getCurrentMsTimestamp();
            // send setup frame
            
            info.run(&setupframe);
            fragmp4_muxer.run(&setupframe);

  
            if ((pkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
            }


            if ((fp_in = fopen(input_file, "rb")) == NULL) {
                fprintf(stderr, "fopen %s failed.\n", input_file);
                // goto ret7;
                return;
            }
            
             if (fseek(fp_in, 0, SEEK_END))
               return;
            ssize_t fileSize = (ssize_t)ftell(fp_in);
            if (fileSize < 0)
                return;
            if (fseek(fp_in, 0, SEEK_SET))
                return;
    
            SInfo << "H264 file Size " << fileSize;
          

           // av_init_packet(pkt);

            const int in_buffer_size=fileSize;
            unsigned char *in_buffer = (unsigned char*)malloc(in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_ptr;
            int cur_size;
            
            long int startTime=    setupframe.mstimestamp;
            long int deltatime =   1000000/25;  //25 frames persecs

            long int framecount = 0;
            
            
            while (1) {

                if (fseek(fp_in, 0, SEEK_SET))
                return;
                
                cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
               
                
                SInfo << "Read H264 filee " << cur_size;
                
                if (cur_size == 0)
                    break;
                cur_ptr = in_buffer;

                while (cur_size > 0) {
                    /*Only input video data*/
//                    if ((ret = av_parser_parse2(parser, cdc_ctx, &pkt->data, &pkt->size,
//                            cur_ptr, cur_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)) < 0) {
//                        fprintf(stderr, "av_parser_parse2 failed.\n");
//                        //goto ret8;
//                        return;
//                    }
                    
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

                    //Some Info from AVCodecParserContext

                    //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                   // BasicFrame        basicframe;
                    basicframe.copyFromAVPacket(pkt);
                    basicframe.codec_id = AV_CODEC_ID_H264;
                    

                    // unsigned target_size=frameSize+numTruncatedBytes;
                    // mstimestamp=presentationTime.tv_sec*1000+presentationTime.tv_usec/1000;
                    // std::cout << "afterGettingFrame: mstimestamp=" << mstimestamp <<std::endl;
                    basicframe.mstimestamp = startTime + 10.4*framecount;
                    basicframe.fillPars();
                    
//                    if( !framecount &&  basicframe.h264_pars.slice_type == H264SliceType::aud) //AUD Delimiter
//                    {
//                          continue;
//                    }
                     
                    framecount++;
                    
//                    if(framecount == 200 )
//                        break;
                    // std::cout << "afterGettingFrame: " << basicframe << std::endl;

                  //  basicframe.payload.resize(pkt->size); // set correct frame size .. now information about the packet length goes into the filter chain
                    
                    info.run(&basicframe);
                   
                    fragmp4_muxer.run(&basicframe);
                     

                    basicframe.payload.resize(basicframe.payload.capacity());
                    
                    std::this_thread::sleep_for(std::chrono::microseconds(10000));
//
                    int x = 0;
                    // decode(cdc_ctx, frame, pkt, fp_out);
                }
            }

        
            free(in_buffer);
            fclose(fp_in);
//            av_frame_free(&frame);
            av_packet_free(&pkt);
 //           avcodec_close(cdc_ctx);
//            avcodec_free_context(&cdc_ctx);

        }


    }

}




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
