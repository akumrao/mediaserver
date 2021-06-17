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

#ifndef FFParse_H
#define FFParse_H

#include "base/thread.h"
#include <string>
#include <vector>
#include "muxer.h"


namespace base {
namespace fmp4 {
    

 class FFParse: public Thread
 {
     
     
 public:
  

     FFParse( );
     
     ~FFParse( );
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
     
    void parseH264(const char *input_file);
  
    ssize_t get_nal_size(uint8_t *buf, ssize_t size,  uint8_t **poutbuf, int *poutbuf_size);
      

        
       
 private:
     
   
    DummyFrameFilter fragmp4_filter;
    FragMP4MuxFrameFilter fragmp4_muxer;
    InfoFrameFilter info;
    
    
    std::string fileName;

 };
 
}
}

#endif /* FFParse_H */

