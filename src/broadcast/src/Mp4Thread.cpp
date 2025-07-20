/* This file is part of mediaserver. A webrtc sfu server.
* Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
*/


#include "Mp4Thread.h"
#include <thread>
#include "base/logger.h"
#include "muxer.h"
#include "restApi.h"
#include "tools.h"
#include "Settings.h"

#define IOBUFSIZE 327680
//40960*6

namespace base {
namespace web_rtc {


    // based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html


    Mp4Thread::Mp4Thread( RestApi *mp4this,  std::string &cam, std::string &date, std::string &hr, std::string &time  ): mp4this(mp4this), cam(cam), date(date), hr(hr), time(time) {


    }

    Mp4Thread::~Mp4Thread() {
        SInfo << "~Mp4Thread( )";
        if( fmp4)
        fclose(fmp4);
    }


    void Mp4Thread::run()
    {

        
       std::string basePath =  Settings::configuration.storage + "CAM"+ cam + "/";
       json ret;


       //for (json::iterator itDate = ret.begin(); itDate != ret.end(); ++itDate) 
       {

           std::string path =  basePath + date + "/manifest.js";

           base::cnfg::Configuration encconfig;
           encconfig.load(path);
           
           int fps = encconfig.root["fps"];
           
           if(encconfig.root.find( date) != encconfig.root.end())
           {
               json &nodeHr =  encconfig.root[date];

              // for (json::iterator itHr = nodeHr.begin(); itHr != nodeHr.end(); ++itHr)
               {
                    std::string endTime;

                    json &nodeMin =  nodeHr[hr];
                    for (json::iterator itmin = nodeMin.begin(); itmin != nodeMin.end(); ++itmin)
                    {
                       std::string ltime = itmin.key();
                       
                       int lmin = std::stoi( ltime.substr(3,2));
                       
                       int min = std::stoi( time.substr(3,2));
                       
                       if(  lmin > 0 &&  lmin <= 15 &&  min > 0 &&  min <= 15   )
                       {
                       }
                       else  if( lmin > 15 &&  lmin <= 30 && min > 15 &&  min <= 30  )
                       {
                       }
                       else  if(lmin > 30 &&  lmin <= 45 && min > 30 &&  min <= 45  )
                       {
                       }
                       else  if(lmin > 45 &&  lmin <= 60 && min > 45 &&  min <= 60  )
                       {
                       }
                       else
                       {
                           continue;
                       }
                       
                       std::string file =  basePath + date + "/" + hr + "/" + ltime + ".mp4" ;
                       
                       playfile(file, fps);

                    }

               }

           }
       }
        
        SInfo << "run exit";
     
    }
    
    
    bool Mp4Thread::playfile( std::string file, int fps)
    {
        
        uint8_t buffer[IOBUFSIZE];
        fmp4 = fopen(file.c_str(),"rb");

        if(!fmp4)
        {
            SError << "can't open file! " <<  file;
            return false;
        }

       int sz=0;
       
       char boxname[5] = {'\0'};
       std::string name;
       uint32_t len;
       int fametype=0;        
       while( !stopped())
       { 
            uint64_t currentTime =  CurrentTime_microseconds();
            
            sz = fread(buffer, 1, 8 , fmp4);
            
            
            if(feof(fmp4))
            {
              break;   
            }
                    
            if (sz < 8)
            { 
                SError << "Recording is messed up!. Not a possible state " <<  file;
                break;
            }
            
            getLenName( buffer, len, boxname );
            
            // SInfo << boxname << " size " <<  len;
            
            sz = fread(&buffer[8], 1, len -8 , fmp4);
            
            if (sz < len- 8)
            { 
                SError << "Recording is messed up!. Not a possible state " <<  file;
                break;
            }
            
            if (strncmp(boxname, "ftyp", 4) == 0) 
            {
                fametype = 1;
            } 
            else if (strncmp(boxname, "moov",4) == 0) 
            {
                fametype = 2;
            } 
            else if (strncmp(boxname, "moof", 4) == 0) 
            {
                if(  moofHasFirstSampleFlag(buffer))
                fametype = 3;
                else
                fametype = 4;
                    
            }   

            mp4this->broadcast((const char *)buffer, len , true, fametype , cam  );
            
            uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
            
            std::this_thread::sleep_for(std::chrono::microseconds(  uint64_t(1000000) / uint64_t(fps) - deltaTimeMillis));

        }
       
        fclose(fmp4);
        fmp4 = nullptr;
        
    }

}

}
    
