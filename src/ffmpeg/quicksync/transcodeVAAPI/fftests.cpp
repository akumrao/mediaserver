//#include "base/define.h"
//#include "base/test.h"
//#include "base/filesystem.h"
//#include "base/application.h"
//#include "base/packetstream.h"
//#include "base/packet.h"
//#include "base/packetqueue.h"
//#include "base/platform.h"

#include "H264_Decoder.h"
#include "H264_Encoder.h"

using namespace std;
// using namespace base;

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

bool playback_initialized = false;

// H264_Decoder decoder(EN_X264,frame_dec_callback, NULL);
// H264_Encoder encoder(EN_X264, NULL,"/tmp/test.264", NULL);

void transcode(int n, en_EncType encType, int scale)
{
    H264_Decoder *decoder{nullptr};
    st_Cam cam;
    cam.camid = std::to_string(n);

    cam.encType = encType;

    cam.scale = scale;


    // cam.scale = 1; // for forward playing

    int x = 0;
    
    H264_Encoder encoder(encType, "/tmp/test" + cam.camid + ".264");

    if (!decoder)
    {
        decoder = new H264_Decoder(cam);

        if (!decoder->load("/var/tmp/./reverse.264")) { ::exit(EXIT_FAILURE); }

        decoder->cb_frame = [&](AVCodecContext *dec_ctx, AVFrame *frame)
        {
            if (!cam.width) cam.width = frame->width;

            if (!cam.height) cam.height = frame->height;

            encoder.encodeFrame(dec_ctx, frame, dec_ctx->width / 2, dec_ctx->height / 2);
            // std::this_thread::sleep_for(std::chrono::microseconds(40000));
        };

        while (1) { 
            
            decoder->readFrame(); 
            
            ++x;
            
            if( x == 60)
                cam.scale = -1;
            else if( x == 120)
                cam.scale = 1;
            
        }
    }
}

#define NVIDIATHREAD 1

int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_TRACE);
 //   AVBufferRef *avcodec_hw_device_ctx{NULL};
    //int ret = av_hwdevice_ctx_create(&avcodec_hw_device_ctx, AV_HWDEVICE_TYPE_QSV, "auto", NULL, 0);

    if (-1 == putenv("LIBVA_DRIVER_NAME=iHD"))
    {
        printf("putenv LIBVA_DRIVER_NAME failed \n");
        return EXIT_FAILURE;
    }

//    if (-1 == putenv("LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib64"))
//    {
//        printf("putenv failed \n");
//        return EXIT_FAILURE;
//    }

    int nEncoderType = 2, scale = 1;

    std::cout << "press key 0 for x264, 1 for nvidia, 2 for qsv and 3 for vaapi" << std::endl;

   // std::cin >> nEncoderType;

    std::cout << "press key 1 for forward encoding, -1 for backword encoding" << std::endl;

   // std::cin >> scale;

    std::thread t[50];

    for (int x = 0; x < NVIDIATHREAD; ++x)
    {
        t[x] = std::thread(transcode, x, (en_EncType) nEncoderType, scale);
    }

    for (int x = 0; x < NVIDIATHREAD; ++x) { t[x].join(); }
}

// encoder2.load(std::string("e:/test9.264"), 25, 1920, 1080);
//

// YUV420P_Player player;

// player_ptr = &player;
//        decoder_ptr = &decoder;
//
//    if(!decoder.load( "/var/tmp/test.264")) {
//      ::exit(EXIT_FAILURE);
//    }

//     if(!decoder.load( "/workspace/live/mediaServer/test4.264", 30.0f)) {
//      ::exit(EXIT_FAILURE);
//    }
//
//
//
//
//
//    while(1)
//    {
//      decoder.readFrame();
//    }
//
//
//#ifdef __linux__
//
//    std::string fileName("/var/tmp/test1.264");
//    if (argc > 1) {
//      fileName = argv[1];
//    }
//
//   // decoder.load(fileName, 25);
//
//    //fileName = "/tmp/test2.264";
//
//   // encoder.load(fileName, 25, 1280, 720);
//#elif _WIN32
//    int w = 800;
//    int h = 600;
//    std::string fileName("e:/test2.264");
//    if (argc > 3) {
//      fileName = argv[1];
//      w = atoi(argv[2]);
//      h = atoi(argv[3]);
//    }
//    else if (argc > 1)
//    {
//      fileName = argv[1];
//
//    }
//    encoder.load(fileName, 25, w, h);
//#else
//     encoder.load( std::string("/tmp/test2.264") , 25,  800, 600);
//#endif
//
//    for (int x = 0; x < 300; ++x) {
//          decoder.readFrame();
//    }
//
//
//   //    decoder.int_dec();
//
////     for (int x = 0; x < 1000; ++x) {
////      encoder.encodeFrame();
////       //encoder2.encodeFrame();
////      // sleep(1);
////    }
//
//
//
//
//}
//
//
// void frame_dec_callback(AVCodecContext *dec_ctx, AVFrame* frame,  void* user)
//{
//    encoder.encodeFrame( dec_ctx, frame, dec_ctx->width, dec_ctx->height );
//
//}

// void initialize_playback(AVFrame* frame, AVPacket* pkt) {
//
//  if(frame->format != AV_PIX_FMT_YUV420P) {
//    printf("This code only support YUV420P data.\n");
//    ::exit(EXIT_FAILURE);
//  }
//
//   encoder.load( std::string("/tmp/test.264") , 25,  frame->width,
//   frame->height);
////  if(!player_ptr) {
////    printf("player_ptr not found.\n");
////    ::exit(EXIT_FAILURE);
////  }
////
////  if(!player_ptr->setup(frame->width, frame->height)) {
////    printf("Cannot setup the yuv420 player.\n");
////    ::exit(EXIT_FAILURE);
////  }
//
//
//
//}
