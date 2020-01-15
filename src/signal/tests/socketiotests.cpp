#include "socketio/client.h"
//#include "socketio/transaction.h"
//#include "net/sslmanager.h"
#include "base/application.h"
#include "base/util.h"


using namespace std;
using namespace base;
using namespace base::net;
using namespace base::util;


/*
// Detect Memory Leaks
#ifdef _DEBUG
#include "MemLeakDetect/MemLeakDetect.h"
#include "MemLeakDetect/MemLeakDetect.cpp"
CMemLeakDetect memLeakDetect;
#endif
*/


namespace base {
namespace sockio {

	
#define SERVER_HOST "arvindubuntu"
#define SERVER_PORT 3000 //443
#define USE_SSL     0 //1


// ----------------------------------------------------------------------------
// SocketIO Client Test
//	
class Tests
{
        Client *client; 
	
public:
	Tests()
	{
            LTrace("Tests Begin" )
                    
            client = new Client(SERVER_HOST , SERVER_PORT);
            client->connect();
            client->cbConnected = [&](socket* soc)
            {
                     socket* soc1 = client->io("/my-namespace");

                     soc1->on("hi", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                       // LTrace("data ", data);
                        
                        soc1->emit("new message", "newmessage1");
                        
                     }));

                
                      soc->on("login", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){

                      int participants = data["numUsers"];
                      LTrace("participants ", participants)

                      soc->off("login");
                }));
                
                soc->on("ferret", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){

                    LTrace("ferret name ", data)
                    
                    auto array = json::array();
                    array.push_back("tobi");
    
                     if(isAck)
                     ack_resp = array;
                    
                    //  soc->off("login");
                }));
                
                soc->emit("add user", "arvind");
                
                soc->emit("add user", "arvind");
                
                 soc->emit("add user1", "arvind");
                
                soc->emit("ferret", "tobi", [&](json const& data) { // args are sent in order to acknowledgement function
                    LTrace("ack ", data)
                            
                    //console.log(data); // data will be 'tobi says woot'
                    
                });
    
                
                LTrace("client->cbConnected" )
                
            };
             LTrace("Tests over" )
	}		

	/*void onClientStateChange(void* sender, sockio::ClientState& state, const sockio::ClientState& oldState) 
	{
		sockio::Client* client = reinterpret_cast<sockio::Client*>(sender);	
		//DebugL << "Connection state changed: " << state.toString() << ": " << client->socket().address() << endl;
		
		switch (state.id()) {
		case sockio::ClientState::Connecting:
			break;
		case sockio::ClientState::Connected: 
			DebugL << "Connected on " << client->socket().address() << endl;
			break;
		case sockio::ClientState::Online: 
			// TODO: Send message
			break;
		case sockio::ClientState::Disconnected: 
			break;
		}
	}*/
        
        void run()
        {
        }
        
};


} } // namespace sockio


int main(int argc, char** argv) 
{
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    Application app;

    sockio::Tests run;


    app.run();


    return 0;
}



