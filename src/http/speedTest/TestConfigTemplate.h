/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef SPEEDTEST_TESTCONFIGTEMPLATE_H
#define SPEEDTEST_TESTCONFIGTEMPLATE_H

#include "SpeedTest.h"

const TestConfig preflightConfigDownload = {
         600000, // start_size
        2000000, // max_size
         125000, // inc_size
           4096, // buff_size
          10000, // min_test_time_ms
              2, // Concurrency
        "Preflight check"
};

const TestConfig slowConfigDownload = {
         100000, // start_size
         500000, // max_size
          10000, // inc_size
           1024, // buff_size
          20000, // min_test_time_ms
              2, // Concurrency
         "Very-slow-line line type detected: profile selected slowband"
};

const TestConfig slowConfigUpload = {
          50000, // start_size
          80000, // max_size
           1000, // inc_size
           1024, // buff_size
          20000, // min_test_time_ms
              2, // Concurrency
          "Very-slow-line line type detected: profile selected slowband"
};


const TestConfig narrowConfigDownload = {
          1000000, // start_size
        100000000, // max_size
           750000, // inc_size
            4096, // buff_size
            20000, // min_test_time_ms
                2, // Concurrency
          "Buffering-lover line type detected: profile selected narrowband"
};

const TestConfig narrowConfigUpload = {
        1000000, // start_size
        100000000, // max_size
        550000, // inc_size
        4096, // buff_size
        20000, // min_test_time_ms
        2, // Concurrency
        "Buffering-lover line type detected: profile selected narrowband"
};

const TestConfig broadbandConfigDownload = {
        1000000,   // start_size
        100000000, // max_size
        750000,    // inc_size
        65536,     // buff_size
        20000,     // min_test_time_ms
        32,        // concurrency
        "Broadband line type detected: profile selected broadband"

};

const TestConfig broadbandConfigUpload = {
        1000000,  // start_size
        70000000, // max_size
        250000,   // inc_size
        65536,    // buff_size
        20000,    // min_test_time_ms
        8,        // concurrency
        "Broadband line type detected: profile selected broadband"
};

const TestConfig fiberConfigDownload = {
        5000000,   // start_size
        120000000, // max_size
        950000,    // inc_size
        65536,     // buff_size
        20000,     // min_test_time_ms
        32,        // concurrency
        "Fiber / Lan line type detected: profile selected fiber"
};

const TestConfig fiberConfigUpload = {
        1000000,  // start_size
        70000000, // max_size
        250000,   // inc_size
        65536,    // buff_size
        20000,    // min_test_time_ms
        12,       // concurrency
        "Fiber / Lan line type detected: profile selected fiber"
};

void testConfigSelector(const double preSpeed, TestConfig& uploadConfig, TestConfig& downloadConfig){
    uploadConfig   = slowConfigUpload;
    downloadConfig = slowConfigDownload;


    if (preSpeed > 4 && preSpeed <= 30){
        downloadConfig = narrowConfigDownload;
        uploadConfig   = narrowConfigUpload;
    } else if (preSpeed > 30 && preSpeed < 150) {
        downloadConfig = broadbandConfigDownload;
        uploadConfig   = broadbandConfigUpload;
    } else if (preSpeed >= 150) {
        downloadConfig = fiberConfigDownload;
        uploadConfig   = fiberConfigUpload;
    }

}

#endif //SPEEDTEST_TESTCONFIGTEMPLATE_H
