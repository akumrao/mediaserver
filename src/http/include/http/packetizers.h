
#ifndef HTTP_Packetizers_H
#define HTTP_Packetizers_H


#include "http/HttpConn.h"
#include "base/packetstream.h"
#include <sstream>



namespace base {
namespace net {


//
// HTTP Chunked Adapter
//


class  ChunkedAdapter : public IPacketizer
{
public:
    TcpHTTPConnection *connection;
    std::string contentType;
    std::string frameSeparator;
    bool initial;
    bool nocopy;

    ChunkedAdapter(TcpHTTPConnection *connection = nullptr, const std::string& frameSeparator = "", bool nocopy = true)
        : PacketProcessor()
        , connection(connection)
        , contentType(connection->outgoingHeader()->getContentType())
        , initial(true)
    {
    }

    ChunkedAdapter(const std::string& contentType, const std::string& frameSeparator = "", bool nocopy = true)
        : PacketProcessor()
        , connection(nullptr)
        , contentType(contentType)
        , frameSeparator(frameSeparator)
        , initial(true)
        , nocopy(nocopy)
    {
    }

    virtual ~ChunkedAdapter() {}

    /// Sets HTTP headers for the initial response.
    /// This method must not include the final carriage return.
    virtual void emitHeader();
   
   

    virtual void process(std::vector<unsigned char>& packet); 
   

//    PacketSignal emitter;
};


//
// HTTP Multipart Adapter
//


class HTTP_API MultipartAdapter : public IPacketizer
{
public:
    TcpHTTPConnection* connection;
    std::string contentType;
    bool isBase64;
    bool initial;

    MultipartAdapter(TcpHTTPConnection* con, bool base64 = false)
        : IPacketizer()
        , connection(con)
        , contentType(con->outgoingHeader()->getContentType())
        , isBase64(base64)
        , initial(true)
    {
    }

    MultipartAdapter(const std::string& contentType, TcpHTTPConnection* con, bool base64 = false)
        : IPacketizer()
        , connection(con)
        , contentType(contentType)
        , isBase64(base64)
        , initial(true)
    {
    }

    virtual ~MultipartAdapter() {}

    virtual void emitHeader();
   

    /// Sets HTTP header for the current chunk.
    //virtual void emitChunkHeader( int outlen );
   

    virtual void process(std::vector<unsigned char> & packet);

    
      void  emit( const uint8_t * data, int len );

//    PacketSignal emitter;
};


} // namespace http
} // namespace scy


#endif


