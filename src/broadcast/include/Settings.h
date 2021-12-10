#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP


#include <map>
#include <string>
#include <vector>
#include <json/json.hpp>
#include <mutex>          // std::mutex



using json = nlohmann::json;

class Settings
{
public:
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
                json rtsp;
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
	
public:
    
    static void postNode(json &node ) ;
    
    static bool deleteNode(json &node, std::vector<std::string> & vec ) ;
    
    static std::mutex  mutexNode;
    
    static std::string  getNode();
   
    static bool putNode(json &node, std::vector<std::string> & vec  ) ;
};

#endif
