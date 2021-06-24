/** --------------------------------------------------------------------------
 *  WebSocketServer.cpp
 *
 *  Base class that WebSocket implementations must inherit from.  Handles the
 *  client connections and calls the child class callbacks for connection
 *  events like onConnect, onMessage, and onDisconnect.
 *
 *  Author    : Jason Kruse <jason@jasonkruse.com> or @mnisjk
 *  Copyright : 2014
 *  License   : BSD (see LICENSE)
 *  --------------------------------------------------------------------------
 **/

#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/time.h>
#include <fcntl.h>
#include "libwebsockets.h"

#include "base/logger.h"
//#include "Util.h"
#include "WebSocketServer.h"

using namespace base;
using namespace std;

// 0 for unlimited
#define MAX_BUFFER_SIZE 0

// Nasty hack because certain callbacks are statically defined
WebSocketServer *self;

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) 
    {
        case LWS_CALLBACK_HTTP:
        {

            char *requested_uri = (char *) in;
            printf("requested URI: %s\n", requested_uri);

            if (strcmp(requested_uri, "/") == 0) {
                lws_serve_http_file(wsi, "index.html", "text/html", NULL, 0);

            } else {
                // try to get current working directory
                char cwd[1024];
                char *resource_path;

                if (getcwd(cwd, sizeof (cwd)) != NULL) {
                    // allocate enough memory for the resource path
                    resource_path = (char*) malloc(strlen(cwd)
                            + strlen(requested_uri));

                    // join current working directory to the resource path
                    sprintf(resource_path, "%s%s", cwd, requested_uri);
                    printf("resource path: %s\n", resource_path);

                    char *extension = strrchr(resource_path, '.');
                    std::string mime;

                    // choose mime type based on the file extension
                    if (extension == NULL) {
                        mime = "text/plain";
                    } else if (strcmp(extension, ".png") == 0) {
                        mime = "image/png";
                    } else if (strcmp(extension, ".jpg") == 0) {
                        mime = "image/jpg";
                    } else if (strcmp(extension, ".gif") == 0) {
                        mime = "image/gif";
                    } else if (strcmp(extension, ".html") == 0) {
                        mime = "text/html";
                    } else if (strcmp(extension, ".css") == 0) {
                        mime = "text/css";
                    } else {
                        mime = "text/plain";
                    }
                    lws_serve_http_file(wsi, resource_path, mime.c_str(), NULL, 0);
                }
            }
        }
       break;
       
       default:
         break;

    };
 

    return 0;
}


static int callback_example(   struct lws *wsi,
                            enum lws_callback_reasons reason,
                            void *user,
                            void *in,
                            size_t len )
{
    int fd;
    //unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
    //unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

    switch( reason ) {
        case LWS_CALLBACK_ESTABLISHED:
            self->onConnectWrapper( lws_get_socket_fd( wsi ) );
            lws_callback_on_writable( wsi );
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
//            fd = lws_get_socket_fd( wsi );
//            while( !self->connections[fd]->buffer.empty( ) )
//            {
//                const char * message = self->connections[fd]->buffer.front( );
//                int msgLen = strlen(message);
//                int charsSent = lws_write( wsi, (unsigned char *)message, msgLen, LWS_WRITE_BINARY );
//                if( charsSent < msgLen )
//                    self->onErrorWrapper( fd, string( "Error writing to socket" ) );
//                else
//                    // Only pop the message if it was sent successfully.
//                    self->connections[fd]->buffer.pop_front( );
//            }
//            lws_callback_on_writable( wsi );
            {
                int charsSent = lws_write( wsi,  self->data,  self->msgLen, LWS_WRITE_BINARY );
                if( charsSent < self->msgLen )
                {
                    SError << "could not send complete data";
                }
            }
            
            break;

        case LWS_CALLBACK_RECEIVE:
            self->onMessage( lws_get_socket_fd( wsi ), string( (const char *)in, len ) );
            break;

        case LWS_CALLBACK_CLOSED:
            self->onDisconnectWrapper( lws_get_socket_fd( wsi ) );
            break;

        default:
            break;
    }
    return 0;
}

//static struct lws_protocols protocols[] = {
//    {
//        "/",
//        callback_main,
//        0, // user data struct not used
//        MAX_BUFFER_SIZE,
//    },{ NULL, NULL, 0, 0 } // terminator
//};

enum protocols
{
	PROTOCOL_HTTP = 0,
	PROTOCOL_EXAMPLE,
	PROTOCOL_COUNT
};

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,             /* No per session data. */
		0,             /* max frame size / rx buffer */
	},
	{
		"stream",
		callback_example,
		0,
		MAX_BUFFER_SIZE,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};


WebSocketServer::WebSocketServer( int port, const string certPath, const string& keyPath )
{
    this->_port     = port;
    this->_certPath = certPath;
    this->_keyPath  = keyPath;

    lws_set_log_level( (1024 + 7) , lwsl_emit_stderr_notimestamp ); // We'll do our own logging, thank you.
    struct lws_context_creation_info info;
    
    memset( &info, 0, sizeof info );
    info.port = this->_port;
    info.iface = NULL;
    info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
    info.extensions = lws_get_internal_extensions( );
#endif

    if( !this->_certPath.empty( ) && !this->_keyPath.empty( ) )
    {
       // Util::log( "Using SSL certPath=" + this->_certPath + ". keyPath=" + this->_keyPath + "." );
        info.ssl_cert_filepath        = this->_certPath.c_str( );
        info.ssl_private_key_filepath = this->_keyPath.c_str( );
    }
    else
    {
       // Util::log( "Not using SSL" );
        info.ssl_cert_filepath        = NULL;
        info.ssl_private_key_filepath = NULL;
    }
    info.gid = -1;
    info.uid = -1;
   // info.options = 0;

    // keep alive
//    info.ka_time = 60; // 60 seconds until connection is suspicious
//    info.ka_probes = 10; // 10 probes after ^ time
//    info.ka_interval = 10; // 10s interval for sending probes
    this->_context = lws_create_context( &info );
    if( !this->_context )
        throw "libwebsocket init failed";
   // Util::log( "Server started on port " + Util::toString( this->_port ) );

    // Some of the libwebsocket stuff is define statically outside the class. This
    // allows us to call instance variables from the outside.  Unfortunately this
    // means some attributes must be public that otherwise would be private.
    self = this;
}

WebSocketServer::~WebSocketServer( )
{
    // Free up some memory
    for( map<int,Connection*>::const_iterator it = this->connections.begin( ); it != this->connections.end( ); ++it )
    {
        Connection* c = it->second;
        this->connections.erase( it->first );
        delete c;
    }
}

void WebSocketServer::onConnectWrapper( int socketID )
{
    Connection* c = new Connection;
    c->createTime = time( 0 );
    this->connections[ socketID ] = c;
    this->onConnect( socketID );
}

void WebSocketServer::onDisconnectWrapper( int socketID )
{
    this->onDisconnect( socketID );
    this->_removeConnection( socketID );
}

void WebSocketServer::onErrorWrapper( int socketID, const string& message )
{
   // Util::log( "Error: " + message + " on socketID '" + Util::toString( socketID ) + "'" );
    this->onError( socketID, message );
    this->_removeConnection( socketID );
}

void WebSocketServer::send( int socketID, string data )
{
    // Push this onto the buffer. It will be written out when the socket is writable.
    this->connections[socketID]->buffer.push_back( data.c_str() );
}

void WebSocketServer::broadcast(const char * data, int msgLen, bool binary )
{
//    for( map<int,Connection*>::const_iterator it = this->connections.begin( ); it != this->connections.end( ); ++it )
//        this->send( it->first, data );
    this->data = (unsigned char *)data;
    this->msgLen = msgLen;
    lws_callback_on_writable_all_protocol( this->_context, protocols );
}

void WebSocketServer::setValue( int socketID, const string& name, const string& value )
{
    this->connections[socketID]->keyValueMap[name] = value;
}

string WebSocketServer::getValue( int socketID, const string& name )
{
    return this->connections[socketID]->keyValueMap[name];
}
int WebSocketServer::getNumberOfConnections( )
{
    return this->connections.size( );
}

void WebSocketServer::runwebsocket( uint64_t timeout )
{
    while( 1 )
    {
        this->wait( timeout );
    }
}

void WebSocketServer::wait( uint64_t timeout )
{
    if( lws_service( this->_context, timeout ) < 0 )
        throw "Error polling for socket activity.";
}

void WebSocketServer::_removeConnection( int socketID )
{
    Connection* c = this->connections[ socketID ];
    this->connections.erase( socketID );
    delete c;
}
