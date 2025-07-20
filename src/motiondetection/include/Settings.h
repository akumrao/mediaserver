#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP


#include <map>
#include <string>
#include <vector>
#include <json/json.hpp>
//#include <mutex>          // std::mutex

#include <uv.h>


using json = nlohmann::json;

class Settings
{
public:
    
    static void init();
    static void exit();
    
	struct LogTags
	{
		bool info{ false };

	};

public:
	// Struct holding the configuration.
	struct Configuration
	{
		//LogLevel logLevel{ LogLevel::LOG_ERROR };
		struct LogTags logTags;
		uint16_t rtcMinPort{ 10000 };
		uint16_t rtcMaxPort{ 59999 };
		uint16_t pre_clip{ 5 };

		uint16_t cam_reconnect{0};

		#if FPSONE
                bool DROP_P_Frame{false};
                #endif
               
		std::string dtlsCertificateFile;
		std::string dtlsPrivateKeyFile;
                std::string storage{"/usr/site/"};
                //json rtsp;
                
		json root;
		json listenIps;
                
	};
        
        struct MDConfig
	{
		//LogLevel logLevel{ LogLevel::LOG_ERROR };

		uint16_t CLIPSIZE{ 5 };
                
		uint16_t DEFAULT_WINDOW_SIZE{ 3 };
		uint16_t DEFAULT_OCCUPANCY_THRESHOLD{ 2 };
		float DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD{ 0.6 };
		float DEFAULT_OCCUPANCY_AVG_THRESHOLD{ 0.6 };
		json masking_coords;
                uint16_t contourArea{0};
                
                struct Soft2minRule
                {
                    uint16_t DEFAULT_WINDOW_SIZE{ 3 };
                    uint16_t contourArea{0};
                }soft2minRule;
                
                struct NonMonitoringHr
                {
                    uint16_t DEFAULT_WINDOW_SIZE{ 0 };
                    uint16_t contourArea{0};
                    uint16_t statrtHr{0};
                    uint16_t endHr{0};
                    uint16_t statrtMin{0};
                    uint16_t endMin{0};
                }nonMonitoringHr;
	};

public:
	static void SetConfiguration();
	static void PrintConfiguration();

private:
	//static void SetLogLevel(std::string& level);
	static void SetLogTags(const std::vector<std::string>& tags);
	static void SetDtlsCertificateAndPrivateKeyFiles();

public:
	static struct Configuration configuration;

private:
	static void  saveFile(const std::string& path, const std::string& dump);
public:
    
    static void postNode(json &node ) ;
    
    static bool deleteNode(json &node, std::vector<std::string> & vec ) ;
    
    static uv_rwlock_t  rwlock_t;
    
    static std::string  getNode();
   
    static bool putNode(json &node, std::vector<std::string> & vec  ) ;
    
    static bool setNodeState(std::string  & id , std::string  status);
    
    static bool getNodeState(std::string id , std::string  key , std::string  &value) ;
    
    static bool getNodeStates(std::string id , std::string  key1 , std::string  &value1 ,  std::string  key2 ,bool  &value2) ;
    
    static void getMDConfig(std::string id , MDConfig &mdConfig);
    
    static void readConfig(std::string file);
    
    static bool getNodeID(std::string id, std::string  &value) ;
    
    static bool setNodeValue(std::string &id , std::string  &key,  std::string  &value );
    
};

#endif
