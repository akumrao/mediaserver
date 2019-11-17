
#include "http/response.h"
#include "base/datetime.h"
#include "http/util.h"


using std::endl;


namespace base
{
    namespace net
    {

        Response::Response()
        : _status(StatusCode::OK)
        , _reason(getStatusCodeReason(StatusCode::OK)){LOG_CALL;
        }

        Response::Response(StatusCode status, const std::string& reason)
        : _status(status)
        , _reason(reason){LOG_CALL;
        }

        Response::Response(const std::string& version, StatusCode status,
                const std::string& reason)
        : Message(version)
        , _status(status)
        , _reason(reason){LOG_CALL;
        }

        Response::Response(StatusCode status)
        : _status(status)
        , _reason(getStatusCodeReason(status)){LOG_CALL;
        }

        Response::Response(const std::string& version, StatusCode status)
        : Message(version)
        , _status(status)
        , _reason(getStatusCodeReason(status)){LOG_CALL;
        }

        Response::~Response(){LOG_CALL;
        }

        void Response::setStatus(StatusCode status){LOG_CALL;
            _status = status;
            _reason = getStatusCodeReason(status);
        }

        void Response::setReason(const std::string& reason){LOG_CALL;
            _reason = reason;
        }

        void Response::setStatusAndReason(StatusCode status, const std::string& reason){LOG_CALL;
            _status = status;
            _reason = reason;
        }

        void Response::setDate(const Timestamp& dateTime){LOG_CALL;
            set("Date", DateTimeFormatter::format(dateTime, DateTimeFormat::HTTP_FORMAT));
        }

        Timestamp Response::getDate() const {
            const std::string& dateTime = get("Date");
            int tzd;
            return DateTimeParser::parse(dateTime, tzd).timestamp();
        }

        void Response::addCookie(const Cookie& cookie){LOG_CALL;
            add("Set-Cookie", cookie.toString());
        }

        void Response::getCookies(std::vector<Cookie>& cookies) const {
            cookies.clear();
            NVCollection::ConstIterator it = find("Set-Cookie");
            while (it != end() && util::icompare(it->first, "Set-Cookie") == 0)
            {
                NVCollection nvc;
                splitParameters(it->second.begin(), it->second.end(), nvc);
                cookies.push_back(Cookie(nvc));
                ++it;
            }
        }

        void Response::write(std::ostream& ostr) const {
            ostr << _version << " " << static_cast<int> (_status) << " " << _reason << "\r\n";
            Message::write(ostr);
            ostr << "\r\n";
        }

        void Response::write(std::string& str) const {
            str.append(_version);
            str.append(" ");
            str.append(std::to_string(int(_status)));
            str.append(" ");
            str.append(_reason);
            str.append("\r\n");
            Message::write(str);
            str.append("\r\n");
        }

        bool Response::success() const {
            return getStatus() < StatusCode::BadRequest; // < 400
        }

        StatusCode Response::getStatus() const {
            return _status;
        }

        const std::string& Response::getReason() const {
            return _reason;
        }

        const char* getStatusCodeReason(StatusCode status){LOG_CALL;
            switch (status)
            {
                case StatusCode::Continue:
                    return "Continue";
                case StatusCode::SwitchingProtocols:
                    return "Switching Protocols";

                case StatusCode::OK:
                    return "OK";
                case StatusCode::Created:
                    return "Created";
                case StatusCode::Accepted:
                    return "Accepted";
                case StatusCode::NonAuthoritative:
                    return "Non-Authoritative Information";
                case StatusCode::NoContent:
                    return "No Content";
                case StatusCode::ResetContent:
                    return "Reset Content";
                case StatusCode::PartialContent:
                    return "Partial Content";

                    // 300 range: redirects
                case StatusCode::MultipleChoices:
                    return "Multiple Choices";
                case StatusCode::MovedPermanently:
                    return "Moved Permanently";
                case StatusCode::Found:
                    return "Found";
                case StatusCode::SeeOther:
                    return "See Other";
                case StatusCode::NotModified:
                    return "Not Modified";
                case StatusCode::UseProxy:
                    return "Use Proxy";
                case StatusCode::TemporaryRedirect:
                    return "OK";

                    // 400 range: client errors
                case StatusCode::BadRequest:
                    return "Bad Request";
                case StatusCode::Unauthorized:
                    return "Unauthorized";
                case StatusCode::PaymentRequired:
                    return "Payment Required";
                case StatusCode::Forbidden:
                    return "Forbidden";
                case StatusCode::NotFound:
                    return "Not Found";
                case StatusCode::MethodNotAllowed:
                    return "Method Not Allowed";
                case StatusCode::NotAcceptable:
                    return "Not Acceptable";
                case StatusCode::ProxyAuthRequired:
                    return "Proxy Authentication Required";
                case StatusCode::RequestTimeout:
                    return "Request Time-out";
                case StatusCode::Conflict:
                    return "Conflict";
                case StatusCode::Gone:
                    return "Gone";
                case StatusCode::LengthRequired:
                    return "Length Required";
                case StatusCode::PreconditionFailed:
                    return "Precondition Failed";
                case StatusCode::EntityTooLarge:
                    return "Request Entity Too Large";
                case StatusCode::UriTooLong:
                    return "Request-URI Too Large";
                case StatusCode::UnsupportedMediaType:
                    return "Unsupported Media Type";
                case StatusCode::RangeNotSatisfiable:
                    return "Requested range not satisfiable";
                case StatusCode::ExpectationFailed:
                    return "Expectation Failed";
                case StatusCode::UnprocessableEntity:
                    return "Unprocessable Entity";
                case StatusCode::UpgradeRequired:
                    return "Upgrade Required";

                    // 500 range: server errors
                case StatusCode::InternalServerError:
                    return "Internal Server Error";
                case StatusCode::NotImplemented:
                    return "Not Implemented";
                case StatusCode::BadGateway:
                    return "Bad Gateway";
                case StatusCode::Unavailable:
                    return "Service Unavailable";
                case StatusCode::GatewayTimeout:
                    return "Gateway Time-out";
                case StatusCode::VersionNotSupported:
                    return "Version Not Supported";
            }
            assert(0);
            return "Unknown";
        }


    } // namespace net
} // namespace base
