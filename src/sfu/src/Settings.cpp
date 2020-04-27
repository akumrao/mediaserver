#include "Settings.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include <json.hpp>
#include <cctype> // isprint()
#include <cerrno>
#include <iterator> // std::ostream_iterator
#include <sstream>  // std::ostringstream

/* Class variables. */

struct Settings::Configuration Settings::configuration;



/* Class methods. */

void Settings::SetConfiguration(json &cnfg)
{

	std::string stringValue;
	std::vector<std::string> logTags;
        
        //std::cout << cnfg.dump(4) << std::flush;

        if (cnfg.find("logTags") != cnfg.end()) {
          // there is an entry with key "foo"
            json &j =  cnfg["logTags"];
            if(j.is_array())
            {
                for (json::iterator it = j.begin(); it != j.end(); ++it) {
                    logTags.push_back(it->get<std::string>());
                }
            }
        }
        
        
        if (cnfg.find("rtcMinPort") != cnfg.end()) {
            Settings::configuration.rtcMinPort = cnfg["rtcMinPort"].get<uint16_t>();
        }
            
        if (cnfg.find("rtcMaxPort") != cnfg.end()) {
            Settings::configuration.rtcMaxPort = cnfg["rtcMaxPort"].get<uint16_t>();
        }
            
            
        if (cnfg.find("logLevel") != cnfg.end()) {
            //TBD // Move logger setting from main to here
                // Initialize the Logger.
            
            std::string loglevel = cnfg["logLevel"].get<std::string>();
            
            Level ld = getLevelFromString(loglevel.c_str());
            
            Logger::instance().add(new ConsoleChannel("mediaserver", ld));
           // Logger::instance().add(new FileChannel("mediaserver","/var/log/mediaserver", ld));
            //Logger::instance().setWriter(new AsyncLogWriter);
            
        }
        
   
            
        if (cnfg.find("dtlsCertificateFile") != cnfg.end()) {
          Settings::configuration.dtlsCertificateFile = cnfg["dtlsCertificateFile"].get<std::string>();
        }
            
            
        if (cnfg.find("dtlsPrivateKeyFile") != cnfg.end()) {
           Settings::configuration.dtlsCertificateFile = cnfg["dtlsPrivateKeyFile"].get<std::string>();
        }


        if (cnfg.find("routerCapabilities") != cnfg.end()) {
           Settings::configuration.routerCapabilities = cnfg["routerCapabilities"];
        }
        
        
        if (cnfg.find("createWebRtcTransport") != cnfg.end()) {
           Settings::configuration.createWebRtcTransport = cnfg["createWebRtcTransport"];
        }
        
        
        if (cnfg.find("maxbitrate") != cnfg.end()) {
           Settings::configuration.maxbitrate = cnfg["maxbitrate"];
        }
        
        if (cnfg.find("transport_connect") != cnfg.end()) {
           Settings::configuration.transport_connect = cnfg["transport_connect"];
        }
        
        if (cnfg.find("transport_produce") != cnfg.end()) {
           Settings::configuration.transport_produce = cnfg["transport_produce"];
        }
	
        if (cnfg.find("transport_consume") != cnfg.end()) {
           Settings::configuration.transport_consume = cnfg["transport_consume"];
        }
	
         

	/* Post configuration. */

	// Set logTags.
	if (!logTags.empty())
	 	Settings::SetLogTags(logTags);

	// Validate RTC ports.
	if (Settings::configuration.rtcMaxPort < Settings::configuration.rtcMinPort)
		base::uv::throwError("rtcMinPort cannot be less than than rtcMinPort");

	// Set DTLS certificate files (if provided),
	Settings::SetDtlsCertificateAndPrivateKeyFiles();
}

void Settings::PrintConfiguration()
{
	

	std::vector<std::string> logTags;
	std::ostringstream logTagsStream;

	 if (Settings::configuration.logTags.info)
	 	logTags.emplace_back("info");
	 if (Settings::configuration.logTags.ice)
	 	logTags.emplace_back("ice");
	 if (Settings::configuration.logTags.dtls)
	 	logTags.emplace_back("dtls");
	 if (Settings::configuration.logTags.rtp)
	 	logTags.emplace_back("rtp");
	 if (Settings::configuration.logTags.srtp)
	 	logTags.emplace_back("srtp");
	 if (Settings::configuration.logTags.rtcp)
	 	logTags.emplace_back("rtcp");
	 if (Settings::configuration.logTags.rtx)
	 	logTags.emplace_back("rtx");
	 if (Settings::configuration.logTags.bwe)
	 	logTags.emplace_back("bwe");
	 if (Settings::configuration.logTags.score)
	 	logTags.emplace_back("score");
	 if (Settings::configuration.logTags.simulcast)
	 	logTags.emplace_back("simulcast");
	 if (Settings::configuration.logTags.svc)
	 	logTags.emplace_back("svc");
	 if (Settings::configuration.logTags.sctp)
	 	logTags.emplace_back("sctp");

	 if (!logTags.empty())
	 {
	 	std::copy(
	 	  logTags.begin(), logTags.end() - 1, std::ostream_iterator<std::string>(logTagsStream, ","));
	 	logTagsStream << logTags.back();
	 }

	MS_DEBUG_TAG(info, "<configuration>");

	MS_DEBUG_TAG(info, "  logTags             : ", logTagsStream.str().c_str());
	MS_DEBUG_TAG(info, "  rtcMinPort          : ", Settings::configuration.rtcMinPort);
	MS_DEBUG_TAG(info, "  rtcMaxPort          : ", Settings::configuration.rtcMaxPort);
	if (!Settings::configuration.dtlsCertificateFile.empty())
	{
		MS_DEBUG_TAG(
		  info, "  dtlsCertificateFile : ", Settings::configuration.dtlsCertificateFile.c_str());
		MS_DEBUG_TAG(
		  info, "  dtlsPrivateKeyFile  : ", Settings::configuration.dtlsPrivateKeyFile.c_str());
	}

	MS_DEBUG_TAG(info, "</configuration>");
}

void Settings::HandleRequest(Channel::Request* request)
{
	

	switch (request->methodId)
	{
		case Channel::Request::MethodId::WORKER_UPDATE_SETTINGS:
		{
			auto jsonLogLevelIt = request->data.find("logLevel");
			auto jsonLogTagsIt  = request->data.find("logTags");

			// Update logLevel if requested.
			if (jsonLogLevelIt != request->data.end() && jsonLogLevelIt->is_string())
			{
				std::string logLevel = *jsonLogLevelIt;

				// This may throw.
//				Settings::SetLogLevel(logLevel);
			}

			// Update logTags if requested.
			if (jsonLogTagsIt != request->data.end() && jsonLogTagsIt->is_array())
			{
				std::vector<std::string> logTags;

				for (const auto& logTag : *jsonLogTagsIt)
				{
					if (logTag.is_string())
						logTags.push_back(logTag);
				}

				Settings::SetLogTags(logTags);
			}

			// Print the new effective configuration.
			Settings::PrintConfiguration();

			request->Accept();

			break;
		}

		default:
		{
			base::uv::throwError("unknown method " +  request->method);
		}
	}
}

// void Settings::SetLogLevel(std::string& level)
// {
// 	

// 	// Lowcase given level.
// 	Utils::String::ToLowerCase(level);

// 	if (Settings::string2LogLevel.find(level) == Settings::string2LogLevel.end())
// 		base::uv::throwError("invalid value '%s' for logLevel", level.c_str());

// 	Settings::configuration.logLevel = Settings::string2LogLevel[level];
// }

 void Settings::SetLogTags(const std::vector<std::string>& tags)
 {
 	

 	// Reset logTags.
 	struct LogTags newLogTags;

 	for (auto& tag : tags)
 	{
 		if (tag == "info")
 			newLogTags.info = true;
 		else if (tag == "ice")
 			newLogTags.ice = true;
 		else if (tag == "dtls")
 			newLogTags.dtls = true;
 		else if (tag == "rtp")
 			newLogTags.rtp = true;
 		else if (tag == "srtp")
 			newLogTags.srtp = true;
 		else if (tag == "rtcp")
 			newLogTags.rtcp = true;
 		else if (tag == "rtx")
 			newLogTags.rtx = true;
 		else if (tag == "bwe")
 			newLogTags.bwe = true;
 		else if (tag == "score")
 			newLogTags.score = true;
 		else if (tag == "simulcast")
 			newLogTags.simulcast = true;
 		else if (tag == "svc")
 			newLogTags.svc = true;
 		else if (tag == "sctp")
 			newLogTags.sctp = true;
 	}

 	Settings::configuration.logTags = newLogTags;
 }

void Settings::SetDtlsCertificateAndPrivateKeyFiles()
{
	

	if (
	  !Settings::configuration.dtlsCertificateFile.empty() &&
	  Settings::configuration.dtlsPrivateKeyFile.empty())
	{
		base::uv::throwError("missing dtlsPrivateKeyFile");
	}
	else if (
	  Settings::configuration.dtlsCertificateFile.empty() &&
	  !Settings::configuration.dtlsPrivateKeyFile.empty())
	{
		base::uv::throwError("missing dtlsCertificateFile");
	}
	else if (
	  Settings::configuration.dtlsCertificateFile.empty() &&
	  Settings::configuration.dtlsPrivateKeyFile.empty())
	{
		return;
	}

	std::string& dtlsCertificateFile = Settings::configuration.dtlsCertificateFile;
	std::string& dtlsPrivateKeyFile  = Settings::configuration.dtlsPrivateKeyFile;

	try
	{
		Utils::File::CheckFile(dtlsCertificateFile.c_str());
	}
	catch (const std::exception& error)
	{
		base::uv::throwError("dtlsCertificateFile: " +  std::string(error.what()));
	}

	try
	{
		Utils::File::CheckFile(dtlsPrivateKeyFile.c_str());
	}
	catch (const std::exception& error)
	{
		base::uv::throwError("dtlsPrivateKeyFile: " + std::string( error.what()));
	}

	Settings::configuration.dtlsCertificateFile = dtlsCertificateFile;
	Settings::configuration.dtlsPrivateKeyFile  = dtlsPrivateKeyFile;
}
