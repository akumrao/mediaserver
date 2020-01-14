

#ifndef Client_Connection_H
#define Client_Connection_H


#include "http/parser.h"

//#include "http/HttpsConn.h"
//#include "http/HttpConn.h"
namespace base {
    namespace net {
        
        class ClientConnecton;
           // HTTP progress signal for upload and download progress notifications.
        class  ProgressSignal 
        {
        public:
            //void* sender;
            long current;
            long total;

            ProgressSignal();
                      
            void start();
                 
            double progress() const { return (current / (total * 1.0)) * 100; }

            void update(int nread , ClientConnecton* conn);
        private: 
            int64_t start_time;
            int64_t end_time;
            double latency ;
            double totalTimeDiff;
            
        };

        
        class ClientConnecton : public HttpBase {
          public:
           ClientConnecton(http_parser_type type);
            
            virtual ~ClientConnecton() ;

             

            std::function<void(const std::string&) > fnUpdateProgess; ///< Signals when raw data is received
            std::function<void(const Response&) > fnComplete; ///< Signals when the HTTP transaction is complete

          

            /// for upload form close
            std::function<void(ClientConnecton*) > fnFormClose;
            
            /// for websocket connect
            

            virtual void setReadStream(std::ostream* os) {
            };
            
            virtual std::stringstream* readStream()
            {
                return nullptr;
            }
		// Returns the cast read stream pointer or nullptr.
	
            
            virtual void send(const char* data, size_t len){};
            virtual void send(){};
            virtual void send(Request& req){};
            virtual void send(const std::string &str){};
            

            void Close() {
            };

            virtual void onHeaders() {
            };
            virtual void on_payload(const char* data, size_t len);

            virtual void onComplete() {
            };
    
            
           // ProgressSignal IncomingProgress; ///< Fired on download progress
            ProgressSignal OutgoingProgress; ///< Fired on upload progress
            
        };


    } // namespace net
} // base

#endif

