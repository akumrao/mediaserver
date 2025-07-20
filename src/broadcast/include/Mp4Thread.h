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

#ifndef FMPP4Parse_H
#define FMPP4Parse_H

#include "base/thread.h"
#include <string>


namespace base {
namespace web_rtc {

class RestApi;
    
class Mp4Thread : public Thread
{

 public:
  
    
     Mp4Thread(  RestApi *mp4this,  std::string &cam, std::string &date, std::string &hr, std::string &time );
     
     ~Mp4Thread( );
     

    bool playfile( std::string file, int fps);
     
   //virtual void start() override
   // virtual void stop() override;
    void run() override;
     
    private: 
    
    FILE *fmp4;
   
    RestApi *mp4this;
    std::string cam;
    std::string date;
    std::string hr;
    std::string time;

 };
 
}
}

#endif /* Mp4Thread_H */

