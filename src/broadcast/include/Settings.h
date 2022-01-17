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
		std::string dtlsCertificateFile;
		std::string dtlsPrivateKeyFile;
                //json rtsp;
                
                json root;
                json listenIps;
                
	};

public:
	static void SetConfiguration(json &config);
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
};

#endif
