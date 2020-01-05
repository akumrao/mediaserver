
#include "net/netInterface.h"
#include "http/parser.h"
//#include "crypto/crypto.h"
//#include "base/connection.h"
#include "base/util.h"
#include "base/logger.h"
#include <stdexcept>


using std::endl;


namespace base {
    namespace net {

        Parser::Parser(Response* response)
        : _observer(nullptr)
        , _request(nullptr)
        , _response(response)
        , _type(HTTP_RESPONSE) {
            init();
            reset();
        }

        Parser::Parser(Request* request)
        : _observer(nullptr)
        , _request(request)
        , _response(nullptr)
        , _type(HTTP_REQUEST) {
            init();
            reset();
        }

        Parser::Parser(http_parser_type type)
        : _observer(nullptr)
        , _request(nullptr)
        , _response(nullptr)
        , _type(type) {
            init();
            reset();
        }

        Parser::~Parser() {
        }

        void Parser::init() {
            // LTrace("Init: ", _type)

            ::http_parser_init(&_parser, _type);
            _parser.data = this;
            _settings.on_message_begin = on_message_begin;
            _settings.on_url = on_url;
            _settings.on_status = on_status;
            _settings.on_header_field = on_header_field;
            _settings.on_header_value = on_header_value;
            _settings.on_headers_complete = on_headers_complete;
            _settings.on_body = on_body;
            _settings.on_message_complete = on_message_complete;

            reset();
        }

        size_t Parser::parse(const char* data, size_t len) {
            // LTrace("Parse: ", len)

            if (_complete) {
                throw std::runtime_error("HTTP parser already complete");
            }

            size_t nparsed = ::http_parser_execute(&_parser, &_settings, data, len);

            if (_parser.upgrade) {
                // The parser has only parsed the HTTP headers, there
                // may still be unread data from the request body in the buffer.
            } else if (nparsed != len) { // parser.http_errno == HPE_OK && !parser.upgrade
                LWarn("HTTP parse failed: ", len, " != ", nparsed)

                // Handle error. Usually just close the connection.
                onError(_parser.http_errno);
                //assert(0);
            }

            return nparsed;
        }

        void Parser::reset() {
            _complete = false;
            _upgrade = false;
            //_shouldKeepAlive = false;
            _error.reset();
        }

        void Parser::setRequest(Request* request) {
            assert(!_request);
            assert(!_response);
            assert(_type == HTTP_REQUEST);
            _request = request;
        }

        void Parser::setResponse(Response* response) {
            assert(!_request);
           // assert(!_response); // arvind
            assert(_type == HTTP_RESPONSE);
            _response = response;
        }

        void Parser::setObserver(ParserObserver* observer) {
            _observer = observer;
        }

        Message* Parser::message() {
            return _request ? reinterpret_cast<Message*> (_request)
                    : _response ? reinterpret_cast<Message*> (_response)
                    : nullptr;
        }

        ParserObserver* Parser::observer() const {
            return _observer;
        }

        bool Parser::complete() const {
            return _complete;
        }

        bool Parser::upgrade() const {
            return _upgrade; // parser.upgrade > 0;
        }


        //
        // Callbacks

        void Parser::onURL(const std::string& value) {
            // LTrace("onURL: ", value)

            if (_request)
                _request->setURI(value);
        }

        void Parser::onHeader(const std::string& name, const std::string& value) {
            // LTrace("On header: ",  name,  ":", value)

            if (message())
                message()->add(name, value);
            if (_observer)
                _observer->onParserHeader(name, value);
        }

        void Parser::onHeadersEnd() {
            _upgrade = _parser.upgrade > 0;

            if (_observer)
                _observer->onParserHeadersEnd(_upgrade);
        }

        void Parser::onBody(const char* buf, size_t len) {
            // LTrace("On body")
            if (_observer)
                _observer->onParserChunk(buf, len);
        }

        void Parser::onMessageEnd() {
            // LTrace("On message end")
            _complete = true;
            if (_observer)
                _observer->onParserEnd();
        }

        void Parser::onError(unsigned errorno, const std::string& message) {
            assert(errorno != HPE_OK);
            SDebug << "Parse error: "
                    << ::http_errno_name((::http_errno)errorno) << ": "
                    << ::http_errno_description((::http_errno)errorno) << endl;

            _complete = true;
            _error.err = (http_errno) errorno; // HTTP_PARSER_ERRNO((http_errno)errno);
            _error.message = message.empty() ? http_errno_name((::http_errno)errorno) : message;
            if (_observer)
                _observer->onParserError(_error);
        }


        //
        // http_parser callbacks
        //

        int Parser::on_message_begin(http_parser* parser) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            self->reset();
            return 0;
        }

        int Parser::on_url(http_parser* parser, const char* at, size_t len) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);
            assert(at && len);

            self->onURL(std::string(at, len));
            return 0;
        }

        int Parser::on_status(http_parser* parser, const char* at, size_t length) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            // Handle response status line
            if (self->_response)
                self->_response->setStatus((net::StatusCode)parser->status_code);

            return 0;
        }

        int Parser::on_header_field(http_parser* parser, const char* at, size_t len) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            if (self->_wasHeaderValue) {
                if (!self->_lastHeaderField.empty()) {
                    self->onHeader(self->_lastHeaderField, self->_lastHeaderValue);
                    self->_lastHeaderValue.clear();
                }
                self->_lastHeaderField = std::string(at, len);
                self->_wasHeaderValue = false;
            } else {
                self->_lastHeaderField += std::string(at, len);
            }

            return 0;
        }

        int Parser::on_header_value(http_parser* parser, const char* at, size_t len) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            if (!self->_wasHeaderValue) {
                self->_lastHeaderValue = std::string(at, len);
                self->_wasHeaderValue = true;
            } else {
                self->_lastHeaderValue += std::string(at, len);
            }

            return 0;
        }

        int Parser::on_headers_complete(http_parser* parser) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            // Add last entry if any
            if (!self->_lastHeaderField.empty()) {
                self->onHeader(self->_lastHeaderField, self->_lastHeaderValue);
            }

            // HTTP version
            // start_line_.version(parser_.http_major, parser_.http_minor);

            // KeepAlive
            // self->_message->setKeepAlive(http_should_keep_alive(&parser) > 0);
            // self->_shouldKeepAlive = http_should_keep_alive(&parser) > 0;

            // Request HTTP method
            if (self->_request)
                self->_request->setMethod(http_method_str(static_cast<http_method> (parser->method)));

            self->onHeadersEnd();
            return 0;
        }

        int Parser::on_body(http_parser* parser, const char* at, size_t len) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            self->onBody(at, len);
            return 0;
        }

        int Parser::on_message_complete(http_parser* parser) {
            auto self = reinterpret_cast<Parser*> (parser->data);
            assert(self);

            // Signal message complete when the http_parser
            // has finished receiving the message.
            self->onMessageEnd();
            return 0;
        }

        /**********************************************************************************************************************/
        HttpBase::HttpBase(http_parser_type type)
        : _parser(type),_type(type), _responder(nullptr), _shouldSendHeader(true) {

            _parser.setObserver(this);
            if (type == HTTP_REQUEST)
                _parser.setRequest(&_request);
            else
                _parser.setResponse(&_response);

        }

        HttpBase::~HttpBase() {

            LTrace("~HttpBase()")
        }

        void HttpBase::onParserHeader(const std::string& /* name */,
                const std::string& /* value */) {
        }

        void HttpBase::onParserHeadersEnd(bool upgrade) {
            LTrace("On headers end: ", _parser.upgrade())


            //this->listener->onHeaders(this);
            onHeaders();

            // Set the position to the end of the headers once
            // they have been handled. Subsequent body chunks will
            // now start at the correct position.
            // _connection.incomingBuffer().position(_parser._parser.nread);
        }

        void HttpBase::onParserChunk(const char* buf, size_t len) {
            LTrace("On parser chunk: ", len)
            //abort();

            // Dispatch the payload
            on_payload(buf, len);

        }

        void HttpBase::onParserEnd() {
            LTrace("On parser end")

            onComplete();
        }

        void HttpBase::onParserError(const base::Error & err) {
            LWarn("On parser error: ", err.message)

#if 0
                    // HACK: Handle those peski flash policy requests here
                    auto base = dynamic_cast<net::TCPSocket*> (_connection.socket().get());
            if (base && std::string(base->buffer().data(), 22) == "<policy-file-request/>") {

                // Send an all access policy file by default
                // TODO: User specified flash policy
                std::string policy;

                // Add the following headers for HTTP policy response
                // policy += "HTTP/1.1 200 OK\r\nContent-Type: text/x-cross-domain-policy\r\nX-Permitted-Cross-Domain-Policies: all\r\n\r\n";
                policy += "<?xml version=\"1.0\"?><cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>";
                base->send(policy.c_str(), policy.length() + 1);
            }
#endif


            // _connection->setError(err.message);

          //  Close(); // do we want to force this? // Arvind
        }


        bool HttpBase::shouldSendHeader() const {
            return _shouldSendHeader;
        }

        void HttpBase::shouldSendHeader(bool flag) {
            _shouldSendHeader = flag;
        }

        Message * HttpBase::incomingHeader() {
            return reinterpret_cast<Message*> (&_request);
        }

        Message * HttpBase::outgoingHeader() {

            return reinterpret_cast<Message*> (&_response);
        }

        /************************************************************************************************************************/


    } // namespace net
} // namespace base
