
#include "net/netInterface.h"
//#include "http/websocket.h"
#include "http/client.h"
#include "net/IP.h"
#include "base/util.h"
#include "base/application.h"
#include "base/platform.h"
#include "http/websocket.h"
using std::endl;


namespace base {
    namespace net {

        ProgressSignal::ProgressSignal()
        : //sender(nullptr)
         current(0)
        , total(0)
        , latency(0)
        , totalTimeDiff(0) {
            LTrace("ProgressSignal");
        }

        void ProgressSignal::start() {
            start_time = base::Application::GetTime();
        }
           
        void ProgressSignal::update(int nread, ClientConnecton* conn) {
            assert(current <= total);
            current += nread;
            //emit(progress());

            double timeDiff = (double) ((base::Application::GetTime()) - end_time);
            latency = (latency + timeDiff) / 2;
            end_time = base::Application::GetTime();

            LTrace("cur ", current, "total ", total)
                    
            totalTimeDiff += timeDiff;
            if (current >= total) {
                RTrace("\"totalsize\":", double( current / 1000), ", \"time_s\":", double(end_time - start_time) / 1000.00);
            }

            if (totalTimeDiff >= 1000 || current >= total || ((int( progress()) % 10 == 0) && (totalTimeDiff > 10))) {
                totalTimeDiff = 0;
                RTrace("\"Speed_MBS\":", double( current  / ((end_time - start_time)*1000.00)), ", \"latency_ms\":", latency)

                std::ostringstream ss;
                if( conn->fnFormClose) //upload speed
                    ss << "{UplaodSpeed_MBS:" << double( current / ((end_time - start_time)*1000.00)) << ", latency_ms:" << latency << "}";
                else
                    ss << "{DownloadSpeed_MBS:" << double( current / ((end_time - start_time)*1000.00)) << ", latency_ms:" << latency << "}";

                if(conn->fnUpdateProgess)
                conn->fnUpdateProgess(ss.str());

            }

        }

        //
        // Client Connection
        //

        ClientConnecton::ClientConnecton(http_parser_type type) :  HttpBase(type) {
        }

        ClientConnecton::~ClientConnecton() {
        }

        void ClientConnecton::on_payload(const char* data, size_t len) {

            LTrace("ClientConnecton::on_payload")
        }



        /************************************************************************************************************************/

    } // namespace net
} // namespace base
