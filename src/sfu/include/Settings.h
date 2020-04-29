#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP

#include "common.h"
#include "Channel/Request.h"
#include <map>
#include <string>
#include <vector>

class Settings
{
public:
	struct LogTags
	{
		bool info{ false };
		bool ice{ false };
		bool dtls{ false };
		bool rtp{ false };
		bool srtp{ false };
		bool rtcp{ false };
		bool rtx{ false };
		bool bwe{ false };
		bool score{ false };
		bool simulcast{ false };
		bool svc{ false };
		bool sctp{ false };
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
                json routerCapabilities;
                json createWebRtcTransport;
                json maxbitrate;
                json transport_connect;
                json transport_produce;
                json transport_consume;
                json consumer_resume;
	};

public:
	static void SetConfiguration(json &config);
	static void PrintConfiguration();
	static void HandleRequest(Channel::Request* request);

private:
	//static void SetLogLevel(std::string& level);
	static void SetLogTags(const std::vector<std::string>& tags);
	static void SetDtlsCertificateAndPrivateKeyFiles();

public:
	static struct Configuration configuration;

private:
	
};

#endif
