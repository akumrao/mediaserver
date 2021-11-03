#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP


#include <map>
#include <string>
#include <vector>
#include <json/json.hpp>
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
                std::string rtsp1;
                std::string rtsp2;
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
	
};

#endif
