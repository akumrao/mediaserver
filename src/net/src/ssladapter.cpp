/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "net/ssladapter.h"
#include "base/logger.h"
//#include "net/sslmanager.h"
#include "net/netInterface.h"
#include "net/SslConnection.h"
#include <assert.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

using namespace std;


namespace base {
    namespace net {


        static SSL_CTX *ctx = nullptr;

        SSL_CTX* InitCTX(bool server) {
            const SSL_METHOD *method;
            SSL_CTX *ctx;

            char CertFile[] = "/var/tmp/key/certificate.crt";
            char KeyFile[] = "/var/tmp/key/private_key.pem";

            SSL_library_init();


            OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
            SSL_load_error_strings(); /* load all error messages */

            if (server)
                method = TLSv1_2_server_method();
            else
                method = TLSv1_2_client_method();
            ctx = SSL_CTX_new(method); /* create new context from method */
            if (ctx == NULL) {
                ERR_print_errors_fp(stderr);
                abort();
            }

            SSL_CTX_set_cipher_list(ctx, "ALL:eNULL");

             //Arvind TBD 
            //SSL_CTX_set_session_cache_mode (ctx, SSL_SESS_CACHE_BOTH);
            //SSL_CTX_set_timeout (ctx, 300);  client side check code before enable it
        

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if (SSL_CTX_load_verify_locations(ctx, CertFile, nullptr) != 1)
                    ERR_print_errors_fp(stderr);

            if (SSL_CTX_set_default_verify_paths(ctx) != 1)
                    ERR_print_errors_fp(stderr);
                
                    /* set the local certificate from CertFile */
            if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
                ERR_print_errors_fp(stderr);
                abort();
            }
            
            
//                

           if (server) {
                //New lines //for server side only 

                
                SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) "12345678");
                
                if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
                    ERR_print_errors_fp(stderr);
                    abort();
                }
            
                if (!SSL_CTX_check_private_key(ctx)) {
                    fprintf(stderr, "Private key does not match the public certificate\n");
                    abort();
                }

            
            }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




            // if (server) {
            //     //New lines //for server side only 

            //     if (SSL_CTX_load_verify_locations(ctx, CertFile, KeyFile) != 1)
            //         ERR_print_errors_fp(stderr);

            //     if (SSL_CTX_set_default_verify_paths(ctx) != 1)
            //         ERR_print_errors_fp(stderr);
            // }
            // //End new lines

            // /* set the local certificate from CertFile */
            // if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
            //     ERR_print_errors_fp(stderr);
            //     abort();
            // }
            // /* set the private key from KeyFile (may be the same as CertFile) */
            // SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) "12345678");
            // if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
            //     ERR_print_errors_fp(stderr);
            //     abort();
            // }
            // /* verify private key */
            // if (!SSL_CTX_check_private_key(ctx)) {
            //     fprintf(stderr, "Private key does not match the public certificate\n");
            //     abort();
            // }

            //New lines - Force the client-side have a certificate
            //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
            //SSL_CTX_set_verify_depth(ctx, 4);
            //End new lines

            return ctx;
        }

        void ShowCerts(SSL* ssl) {
            X509 *cert;
            char *line;

            cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
            if (cert != NULL) {
                printf("Server certificates:\n");
                line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
                printf("Subject: %s\n", line);
                free(line);
                line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
                printf("Issuer: %s\n", line);
                free(line);
                X509_free(cert);
            } else
                printf("No certificates.\n");
        }

        SSLAdapter::SSLAdapter(SslConnection* socket)
        : _socket(socket)
        , _ssl(nullptr)
        , _readBIO(nullptr)
        , _writeBIO(nullptr) {
            LTrace("SSLAdapter")
        }

        SSLAdapter::~SSLAdapter() {
            // LTrace("Destroy")
            if (_ssl) {
                SSL_free(_ssl);
                _ssl = nullptr;
            }
            // LTrace("Destroy: OK")
        }

        void SSLAdapter::initClient() {
            LTrace("Init client")
                    /*assert(_socket);
                    if (!_socket->context())
                        _socket->useContext(SSLManager::instance().defaultClientContext());
                    assert(!_socket->context()->isForServerUse());
                     */

            if (!ctx) {
                ctx = InitCTX(false); /* initialize SSL */
            }


            _ssl = SSL_new(ctx);

            // TODO: Improve automatic SSL session handling.
            // Maybe add a stored session to the network manager.
//            if (_socket->currentSession())
 //               SSL_set_session(_ssl, _socket->currentSession()->sslSession());

            _readBIO = BIO_new(BIO_s_mem());
            _writeBIO = BIO_new(BIO_s_mem());
            SSL_set_bio(_ssl, _readBIO, _writeBIO);
            SSL_set_connect_state(_ssl);
            SSL_do_handshake(_ssl);
        }

        void SSLAdapter::initServer() //(SSL* ssl)
        {
            LTrace("Init server")
                    /*assert(_socket);
                    if (!_socket->context())
                        _socket->useContext(SSLManager::instance().defaultServerContext());
                    assert(_socket->context()->isForServerUse());*/

            if (!ctx) {
                ctx = InitCTX(true); /* initialize SSL */

            }


            _ssl = SSL_new(ctx);
            _readBIO = BIO_new(BIO_s_mem());
            _writeBIO = BIO_new(BIO_s_mem());
            SSL_set_bio(_ssl, _readBIO, _writeBIO);
            SSL_set_accept_state(_ssl);
            SSL_do_handshake(_ssl);
        }

        void SSLAdapter::shutdown() {
            LTrace("Shutdown")
            if (_ssl) {
                // LTrace("Shutdown SSL")

                // Don't shut down the socket more than once.
                int shutdownState = SSL_get_shutdown(_ssl);
                bool shutdownSent =
                        (shutdownState & SSL_SENT_SHUTDOWN) == SSL_SENT_SHUTDOWN;
                if (!shutdownSent) {
                    // A proper clean shutdown would require us to
                    // retry the shutdown if we get a zero return
                    // value, until SSL_shutdown() returns 1.
                    // However, this will lead to problems with
                    // most web browsers, so we just set the shutdown
                    // flag by calling SSL_shutdown() once and be
                    // done with it.
                    int rc = SSL_shutdown(_ssl);
                    if (rc <= 0)
                        handleError(rc);
                }
            }
        }

        bool SSLAdapter::initialized() const {
            return !!_ssl;
        }

        bool SSLAdapter::ready() const {
            return _ssl && SSL_is_init_finished(_ssl);
        }

        int SSLAdapter::available() const {
            assert(_ssl);
            return SSL_pending(_ssl);
        }

        void SSLAdapter::addIncomingData(const char* data, size_t len) {
            // LTrace("Add incoming data: ", len)
            assert(_readBIO);
            BIO_write(_readBIO, data, (int) len);
            flush();
        }

        void SSLAdapter::addOutgoingData(const std::string& s) {
            addOutgoingData(s.c_str(), s.size());
        }

        void SSLAdapter::addOutgoingData(const char* data, size_t len) {
            std::copy(data, data + len, std::back_inserter(_bufferOut));
        }

        void SSLAdapter::handshake() {
            int r = SSL_do_handshake(_ssl);
            if (r <= 0)
                handleError(r);
        }

        void SSLAdapter::flush() {
            //LTrace("Flushing")

                    // Keep trying to handshake until initialized
            if (!ready())
                return handshake();

            // Read any decrypted remote data from SSL and emit to the app
            flushReadBIO();

            // Write any local data to SSL for excryption
            if (_bufferOut.size() > 0) {
                int r = SSL_write(_ssl, &_bufferOut[0], (int) _bufferOut.size());
                if (r <= 0) {
                    handleError(r);
                }
                _bufferOut.clear();
                // flushWriteBIO();
            }

            // send any encrypted data from SSL to the remote peer
            flushWriteBIO();
        }

        void SSLAdapter::flushReadBIO() {
            size_t npending = BIO_ctrl_pending(_readBIO);
            if (npending > 0) {
                int nread;
                char buffer[npending];
                while ((nread = SSL_read(_ssl, buffer, npending)) > 0) {
                   // LTrace("On Read ", buffer)
                    //  _socket->listener->on_read(_socket, buffer, nread); // arvind
                    _socket->on_read(buffer, nread); // arvind
                }
            }
        }

        void SSLAdapter::flushWriteBIO() {
            size_t npending = BIO_ctrl_pending(_writeBIO);
            if (npending > 0) {
                char buffer[npending];
                int nread = BIO_read(_writeBIO, buffer, npending);
                if (nread > 0) {
                    _socket->Write(buffer, nread); // arvind
                }
            }
        }

        void SSLAdapter::handleError(int rc) {
            if (rc > 0)
                return;
            int error = SSL_get_error(_ssl, rc);
            switch (error) {
                case SSL_ERROR_ZERO_RETURN:
                    LTrace("SSL_ERROR_ZERO_RETURN")
                    return;
                case SSL_ERROR_WANT_READ:
                    LTrace("SSL_ERROR_WANT_READ")
                    flushWriteBIO();
                    break;
                case SSL_ERROR_WANT_WRITE:
                    LTrace("SSL_ERROR_WANT_WRITE")
                    assert(0 && "not implemented");
                    break;
                case SSL_ERROR_WANT_CONNECT:
                case SSL_ERROR_WANT_ACCEPT:
                case SSL_ERROR_WANT_X509_LOOKUP:
                    assert(0 && "should not occur");
                    break;
                default:
                    char buffer[256];
                    ERR_error_string_n(ERR_get_error(), buffer, sizeof (buffer));
                    std::string msg(buffer);
                    LTrace(msg)
                            //throw std::runtime_error("SSL connection failed: " + msg);
                            //_socket->setError("SSL connection failed: " + msg);  //arvind
                    break;
            }
        }


    } // namespace net
} // namespace base

