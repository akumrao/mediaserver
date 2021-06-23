/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"
#include "fmp4.h"


using namespace base;


int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("debug", Level::Info));
    //test::init();
    
   Application app;
   
    av_register_all();
    // init network
    avformat_network_init();
    avcodec_register_all();

      
    fmp4::ReadMp4 readmp4;
    
    readmp4.websocketConnect();


    app.run();
    
    readmp4.stop();
    

}
