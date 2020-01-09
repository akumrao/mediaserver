


#include "http/request.h"
#include "http/response.h"
#include <http_parser.h>
#include <functional> 

#ifndef HTTP_Parser_H
#define HTTP_Parser_H


namespace base {
    namespace net {

        class ServerResponder;
        class ParserObserver {
        public:
            virtual void onParserHeader(const std::string& name, const std::string& value) = 0;
            virtual void onParserHeadersEnd(bool upgrade) = 0;
            virtual void onParserChunk(const char* data, size_t len) = 0;
            virtual void onParserEnd() = 0;

            virtual void onParserError(const Error& err) = 0;
        };

        class Parser {
        public:
            Parser(Response* response);
            Parser(Request* request);
            Parser(http_parser_type type);
            ~Parser();

            /// Parse a HTTP packet.
            ///
            /// Returns true of the message is complete, false if incomplete.
            /// Reset the parser state for a new message
            size_t parse(const char* data, size_t length);

            /// Reset the internal state.
            void reset();

            /// Returns true if parsing is complete, either
            /// in success or error.
            bool complete() const;

            /// Returns true if the connection should be upgraded.
            bool upgrade() const;

            void setRequest(Request* request);
            void setResponse(Response* response);
            void setObserver(ParserObserver* observer);

            Message* message();
            ParserObserver* observer() const;

        protected:
            void init();

            /// Callbacks
            void onURL(const std::string& value);
            void onHeader(const std::string& name, const std::string& value);
            void onHeadersEnd();
            void onBody(const char* buf, size_t len);
            void onMessageEnd();
            void onError(unsigned errnum, const std::string& message = "");

            /// http_parser callbacks
            static int on_message_begin(http_parser* parser);
            static int on_url(http_parser* parser, const char* at, size_t len);
            static int on_status(http_parser* parser, const char* at, size_t len);
            static int on_header_field(http_parser* parser, const char* at, size_t len);
            static int on_header_value(http_parser* parser, const char* at, size_t len);
            static int on_headers_complete(http_parser* parser);
            static int on_body(http_parser* parser, const char* at, size_t len);
            static int on_message_complete(http_parser* parser);

        protected:
            ParserObserver* _observer;
            Request* _request;
            Response* _response;
            Message* _message;

            http_parser _parser;
            http_parser_settings _settings;
            http_parser_type _type;

            bool _wasHeaderValue;
            std::string _lastHeaderField;
            std::string _lastHeaderValue;

            bool _complete;
            bool _upgrade;

            Error _error;
        };

        class HttpBase : public ParserObserver {
        public:

            HttpBase(http_parser_type type);
        public:

            virtual ~HttpBase();

        public:
            virtual void tcpsend(const char* data, size_t len) {};
            virtual void send(const char* data, size_t len) = 0;
            virtual void Close() = 0;


            /* Pure virtual methods inherited from ::HttpConnection. */
        public:
            // void on_read(const char* data, size_t len) override;
            //void on_close() override;

            /// HTTP Parser interface
            virtual void onParserHeader(const std::string& name, const std::string& value);
            virtual void onParserHeadersEnd(bool upgrade);
            virtual void onParserChunk(const char* buf, size_t len);
            virtual void onParserError(const base::Error& err);
            virtual void onParserEnd();

            /// HTTP connection and server interface
            virtual void onHeaders() =0;
            virtual void on_payload(const char* data, size_t len)=0;
            virtual void onComplete()=0;

            std::function<void(HttpBase*, std::string) > fnClose;
            std::function<void(HttpBase*) > fnConnect;
            std::function<void(HttpBase*, const char* data, size_t ) > fnPayload;
            

            virtual Message* incomingHeader();
            virtual Message* outgoingHeader();


        private:


        public:
            Request _request;
            Response _response;
            Parser _parser;
            http_parser_type _type;

        protected:
            
        protected:

            bool _shouldSendHeader;

        public:
            ServerResponder* _responder;
            /// Return true if headers should be automatically sent.
            bool shouldSendHeader() const;
            /// Set true to prevent auto-sending HTTP headers.
            void shouldSendHeader(bool flag);


        };





    } // namespace net
} // namespace base


#endif // HTTP_Parser_H


