/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fmp4.h
 * Author: root
 *
 * Created on June 8, 2021, 10:48 PM
 */

#ifndef FMP4_H
#define FMP4_H

#include "base/thread.h"
#include <string>
#include <vector>

namespace base {
namespace wrtc {
    

class Peer;
 class ReadMp4: public Thread
 {
     
     
 public:
     Peer *pc;
     ReadMp4( Peer *pc);
     
     ~ReadMp4( );
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
 private:
     
   
     
     std::string fileName;

 };
 
}
}

#endif /* FMP4_H */

