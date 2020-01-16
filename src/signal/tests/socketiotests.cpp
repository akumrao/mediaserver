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
	/*Tests()
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
                       LTrace(cnfg::stringify(data));
                      int participants = data["numUsers"];
                      LTrace("participants ", participants)

                      soc->off("login");
                }));
                
                soc->on("ferret", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){

                     LTrace(cnfg::stringify(data));
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
*/
    
   Tests()
	{
            LTrace("Tests Begin" )
                    
            client = new Client(SERVER_HOST , SERVER_PORT);
            client->connect();
            client->cbConnected = [&](socket* soc)
            {
                     socket* soc1 = client->io("/my-namespace");

                     soc1->on("hi", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                       
                          LTrace(cnfg::stringify(data));
                         
                         LTrace("data ", data);
                        
                        soc1->emit("new message", "newmessage1");
                        
                     }));

                
   
                
                LTrace("client->cbConnected" )
                
            };
             LTrace("Tests over" )
	}		

    
    /*Tests()
	{
            LTrace("Tests Begin" )
                    
            client = new Client(SERVER_HOST , 8080);
            client->connect();
            client->cbConnected = [&](socket* socket)
            {
                
                socket->on("ipaddr", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
               
                    LTrace("Server IP address is: " , data);
                // updateRoomURL(ipaddr);
              }));

              socket->on("created", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                
                 LTrace(cnfg::stringify(data));
                LTrace("Created room", data[0], "- my client ID is", data[1]);
                isInitiator = true;
                //grabWebCamVideo();
              }));
              
              
              socket->on("bye", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
                //  LTrace("Peer leaving room {" "room" }.`);
               // sendBtn.disabled = true;
                //snapAndSendBtn.disabled = true;
                // If peer did not create the room, re-enter to be creator.
                //if (!isInitiator) {
                 // window.location.reload();
                //}
              }));

              
              

              socket->on("joined", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
                //console.log('This peer has joined room', room, 'with client ID', clientId);
               // isInitiator = false;
               // createPeerConnection(isInitiator, configuration);
               // grabWebCamVideo();
              }));

              socket->on("full", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
               // alert('Room ' + room + ' is full. We will create a new room for you.');
               // window.location.hash = '';
               // window.location.reload();
              }));

              socket->on("ready", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
               // console.log('Socket is ready');
               // createPeerConnection(isInitiator, configuration);
              }));

              socket->on("log", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
                //console.log.apply(console, array);
              }));

              socket->on("message", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
               // console.log('Client received message:', message);
               // signalingMessageCallback(message);
              }));

      

              // Leaving rooms and disconnecting from peers.
              socket->on("disconnect", socket::event_listener_aux([&](string const& name, json const& data, bool isAck,json &ack_resp){
                LTrace(cnfg::stringify(data));
                //console.log(`Disconnected: ${reason}.`);
               // sendBtn.disabled = true;
               // snapAndSendBtn.disabled = true;
              }));

      
             // window.addEventListener('unload', function() {
              //  console.log(`Unloading window. Notifying peers in ${room}.`);
               // socket->emit('bye', room);
             // });


  
         
               socket->emit("create or join", room);
                socket->emit("ipaddr");
            };
            
            
            // function sendMessage(message) {
             //   console.log('Client sending message: ', message);
              //  socket->emit('message', message);
             // }
             //
                 
             LTrace("Tests over" )
	}
    */
	
        
        void run()
        {
        }
        
        
        bool isInitiator{false};
        std::string room{"vns"};
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



