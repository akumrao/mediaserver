
#include "base/logger.h"
#include "base/application.h"
#include "http/HTTPResponder.h"
#include "Settings.h"
#include "json/configuration.h"

#include "H264Framer.h"


using namespace base;



int main(int argc, char** argv) {

Logger::instance().add(new ConsoleChannel("debug", Level::Info));
    

unsigned num_units_in_tick, time_scale;

bool foundsps{false};
bool foundpps{false};
  
H264Framer obj;  



//unsigned char fReceiveBuffer[]=  {103, 77, 64, 30, 150, 86, 5, 160, 147, 96, 73, 16, 0, 0, 62, 128, 0, 12, 53, 14, 8, 0, 2, 220, 108, 0, 5, 184, 223, 241, 142, 15, 138, 21, 112};

unsigned char fReceiveBuffer[]= {103, 77, 0, 41, 226, 144, 24, 2, 77, 128, 183, 1, 1, 1, 164, 30, 36, 69, 64};


int frameSize  = sizeof(fReceiveBuffer);
obj.analyze_seq_parameter_set_data(fReceiveBuffer , frameSize, num_units_in_tick, time_scale);

    {
       
        SInfo <<  " Got SPS fps "  << obj.fps << " width "  << obj.width  <<  " height " << obj.height ;
    }
    
   

}
