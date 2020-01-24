

#ifndef BASE_AV_ICapture_H
#define BASE_AV_ICapture_H


#include "base/packetstream.h"

#include <list>


namespace base {
namespace ff {
struct Format;


class  ICapture 
{
public:
    ICapture()
        
    {
    }
    virtual ~ICapture() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    /// Sets the input format for encoding with this capture device.
    virtual void getEncoderFormat(Format& iformat) = 0;



  //  PacketSignal emitter;
};


} // namespace ff
} // namespace base


#endif // BASE_AV_ICapture_H



