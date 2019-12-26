
#include "net/netInterface.h"
#include "http/packetizers.h"

#include <string>

static const std::string HEADER = "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Max-Age: 0\r\n"
        "Expires: 0\r\n"
        "Cache-Control: no-cache, private\r\n"
        "Pragma: no-cache\r\n"
        "Content-Type: multipart/x-mixed-replace;boundary=--boundary\r\n\r\n";

/**
 * @brief HEADER
 * HTTP response BODY. To be completed before sending
 */
static const std::string BODY = "--boundary\r\n"
        "Content-Type: image/jpeg\r\n"
        "Content-Length: ";



namespace base {
    namespace net {


        //
        // HTTP Chunked Adapter
        //




        /// Sets HTTP headers for the initial response.
        /// This method must not include the final carriage return.

        void ChunkedAdapter::emitHeader() {
            // Flush connection headers if the connection is set.
           /* if (connection) {
                connection->shouldSendHeader(true);
                connection->_response.setChunkedTransferEncoding(true);
                connection->_response.set("Cache-Control", "no-store, no-cache, max-age=0, must-revalidate");
                connection->_response.set("Cache-Control", "post-check=0, pre-check=0, FALSE");
                connection->_response.set("Access-Control-Allow-Origin", "*");
                connection->_response.set("Transfer-Encoding", "chunked");
                connection->_response.set("Content-Type", contentType);
                connection->_response.set("Connection", "keep-alive");
                connection->_response.set("Pragma", "no-cache");
                connection->_response.set("Expires", "0");
               // connection->sendHeader();
            }// Otherwise make up the response.
            else */{
                std::ostringstream hst;
                hst << "HTTP/1.1 200 OK\r\n"
                        // Note: If Cache-Control: no-store is not used Chrome's
                        // (27.0.1453.110)
                        // memory usage grows exponentially for HTTP streaming:
                        // https://code.google.com/p/chromium/issues/detail?id=28035
                        << "Cache-Control: no-store, no-cache, max-age=0, "
                        "must-revalidate\r\n"
                        << "Cache-Control: post-check=0, pre-check=0, FALSE\r\n"
                        << "Access-Control-Allow-Origin: *\r\n"
                        << "Connection: keep-alive\r\n"
                        << "Pragma: no-cache\r\n"
                        << "Expires: 0\r\n"
                        << "Transfer-Encoding: chunked\r\n"
                        << "Content-Type: " << contentType << "\r\n"
                        << "\r\n";
                emit(hst.str().c_str() ,hst.str().length() );
            }
        }

        void ChunkedAdapter::process(std::vector<unsigned char>& packet) {
            LTrace("Processing:", packet.size());

            if (!packet.size())
                throw std::invalid_argument("Incompatible packet type");

            // Emit HTTP response header
            if (initial) {
                initial = false;
                emitHeader();
            }

            // Get hex stream length
            std::ostringstream ost;
            ost << std::hex << packet.size();

            // Emit separate packets for nocopy
           /* if (nocopy) {
                           emit(ost.str());
                            emit("\r\n", 2);
                            if (!frameSeparator.empty())
                                emit(frameSeparator);
                            emit(packet.data(), packet.size());
                            emit("\r\n", 2);
            }                // Concat pieces for non fragmented
            else */{
                ost << "\r\n";
                if (!frameSeparator.empty())
                    ost << frameSeparator;
                ost << std::string(packet.begin(), packet.end());
                ost << "\r\n";
                emit(ost.str().c_str() , ost.str().length());
            }
        }
        
        
        
         void ChunkedAdapter::emit(const char * data, int len) {
            SDebug << "emit packet: "
                    // assert(!connection().socket()->closed());
                    << len << ": " << "fpsCounter.fps" << std::endl;

            try {
                connection->send(data, len);
                //fpsCounter.tick();
            } catch (std::exception& exc) {
                LError(exc.what())
                connection->Close();
            }

        }
        
/*****************************************************************************************/        

        void MultipartAdapter::emitHeader() {
            
            emit( (const char*)HEADER.c_str(),  HEADER.size());
        }

        /// Sets HTTP header for the current chunk.

 

        void MultipartAdapter::process(std::vector<unsigned char> & packet) {
            std::string buffer;
            //
            // Write the initial HTTP response header
            if (initial) {
                initial = false;
                 emitHeader();

                //buffer = HEADER;
                //emit( (const uint8_t*)buffer.c_str(),   buffer.size());
                //return;
                 return ;
            } 
            else  {
               
                /*
                std::ifstream f("/var/tmp/red1.jpg", std::ios::binary| std::ifstream::in);
                std::vector<uint8_t> v{std::istreambuf_iterator<char>{f},
                    {}};
                std::cout << "Read complete, got " << v.size() << " bytes\n" << std::endl << std::flush;
                std::string image = std::string(v.begin(), v.end());

                */
                LTrace("Processing:", packet.size());

                buffer = BODY + std::to_string(packet.size()) + "\r\n\r\n" + std::string(packet.begin(), packet.end());
                


                // Broadcast the HTTP header separately
                // so we don't need to copy any data.
                //emitChunkHeader(buf.size());

                // Proxy the input packet.
                //emit( (const uint8_t*)buffer.c_str(),   buffer.size());


            }
            
            LTrace("Processing:", buffer.size());
            
             
        //     sleep(20);
            
            emit( (const char *) buffer.c_str(), buffer.size() );

        }

        void MultipartAdapter::emit(const char * data, int len) {
            SDebug << "emit packet: "
                    // assert(!connection().socket()->closed());
                    << len << ": " << "fpsCounter.fps" << std::endl;

            try {
                connection->send(data, len);
                //fpsCounter.tick();
            } catch (std::exception& exc) {
                LError(exc.what())
                connection->Close();
            }

        }


    } // namespace net
} // namespace base





