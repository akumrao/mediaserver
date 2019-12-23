
#ifndef PacketStream_H
#define PacketStream_H


//#include "base/packet.h"

#include <cstdint>


namespace base {


class  PacketProcessor
{
public:
    PacketProcessor()
    {
    }

    /// This method performs processing on the given
    /// packet and emits the result.
    ///
    /// Note: If packet processing is async (the packet is not in
    /// the current thread scope) then packet data must be copied.
    /// Copied data can be freed directly aFter the async call to
    /// emit() the outgoing packet.
    virtual void process(std::vector<unsigned char> &packet) = 0;

    /// This method ensures compatibility with the given
    /// packet type. Return false to reject the packet.
    virtual bool accepts(std::vector<unsigned char>*) { return true; };

    /// Stream operator alias for process()
    virtual void operator<<(std::vector<unsigned char>& packet) { process(packet); };
    
    
    virtual void emit( const char * data, int len ){};
     
    virtual void emit( const std::string &str )
      {
          emit( str.c_str() ,str.length() );
      }
};


typedef PacketProcessor IPacketizer;
typedef PacketProcessor IDepacketizer; /// For 0.8.x compatibility


} // namespace base


#endif // PacketStream_H

