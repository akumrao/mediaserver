
#ifndef Net_SSLAdapter_H
#define Net_SSLAdapter_H


#include "crypto/crypto.h"
//#include "net/address.h"
//#include "net/net.h"
//#include "base/handle.h"

#include <string>
#include <vector>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>


namespace base {
namespace net {



/// A wrapper for the OpenSSL SSL connection context
///
/// TODO: Decouple from SSLSocket implementation
class  SslConnection;
class  SSLAdapter
{
public:
    SSLAdapter(SslConnection* socket);
    ~SSLAdapter();

    /// Initializes the SSL context as a client.
    void initClient();

    /// Initializes the SSL context as a server.
    void initServer();

    /// Returns true when SSL context has been initialized.
    bool initialized() const;

    /// Returns true when the handshake is complete.
    bool ready() const;

    /// Start/continue the SSL handshake process.
    void handshake();

    /// Returns the number of bytes available in
    /// the SSL buffer for immediate reading.
    int available() const;

    /// Issues an orderly SSL shutdown.
    void shutdown();

    /// Flushes the SSL read/write buffers.
    void flush();

    void addIncomingData(const char* data, size_t len);
    void addOutgoingData(const std::string& data);
    void addOutgoingData(const char* data, size_t len);

protected:
    void handleError(int rc);

    void flushReadBIO();
    void flushWriteBIO();

protected:
    friend class SslConnection;

    SslConnection* _socket;
    
   
     
    SSL* _ssl;
    BIO* _readBIO;  ///< The incoming buffer we write encrypted SSL data into
    BIO* _writeBIO; ///<  The outgoing buffer we write to the socket
    std::vector<char> _bufferOut; ///<  The outgoing payload to be encrypted and sent
};


} // namespace net
} // namespace base


#endif // Net_SSLAdapter_H
