#include "Settings.h"
#include "base/error.h"
#include "base/logger.h"


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
        
        if (cnfg.find("rtsp") != cnfg.end()) {
            Settings::configuration.rtsp = cnfg["rtsp"];
        }
        
      
            
        if (cnfg.find("logLevel") != cnfg.end()) {   // trace, debug, info, warn
            //TBD // Move logger setting from main to here
                // Initialize the Logger.
            
            std::string loglevel = cnfg["logLevel"].get<std::string>();
            
            base::Level ld = base::getLevelFromString(loglevel.c_str());
            
            base::Logger::instance().add(new base::ConsoleChannel("webrtcserver", ld));
            //base::Logger::instance().add(new base::FileChannel("webrtcserver","/var/log/webrtcserver", ld));
           // base::Logger::instance().setWriter(new base::AsyncLogWriter);
            
        }
        
   
            
        if (cnfg.find("dtlsCertificateFile") != cnfg.end()) {
          Settings::configuration.dtlsCertificateFile = cnfg["dtlsCertificateFile"].get<std::string>();
        }
            
            
        if (cnfg.find("dtlsPrivateKeyFile") != cnfg.end()) {
           Settings::configuration.dtlsPrivateKeyFile = cnfg["dtlsPrivateKeyFile"].get<std::string>();
        }

        if (cnfg.find("listenIps") != cnfg.end()) {
           Settings::configuration.listenIps = cnfg["listenIps"];
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
	

	 if (!logTags.empty())
	 {
	 	std::copy(
	 	  logTags.begin(), logTags.end() - 1, std::ostream_iterator<std::string>(logTagsStream, ","));
	 	logTagsStream << logTags.back();
	 }

//	MS_DEBUG_TAG(info, "<configuration>");
//
//	MS_DEBUG_TAG(info, "  logTags             : ", logTagsStream.str().c_str());
//	MS_DEBUG_TAG(info, "  rtcMinPort          : ", Settings::configuration.rtcMinPort);
//	MS_DEBUG_TAG(info, "  rtcMaxPort          : ", Settings::configuration.rtcMaxPort);
//	if (!Settings::configuration.dtlsCertificateFile.empty())
//	{
//		MS_DEBUG_TAG(
//		  info, "  dtlsCertificateFile : ", Settings::configuration.dtlsCertificateFile.c_str());
//		MS_DEBUG_TAG(
//		  info, "  dtlsPrivateKeyFile  : ", Settings::configuration.dtlsPrivateKeyFile.c_str());
//	}
//
//	MS_DEBUG_TAG(info, "</configuration>");
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
		//Utils::File::CheckFile(dtlsCertificateFile.c_str());
	}
	catch (const std::exception& error)
	{
		base::uv::throwError("dtlsCertificateFile: " +  std::string(error.what()));
	}

	try
	{
		//Utils::File::CheckFile(dtlsPrivateKeyFile.c_str());
	}
	catch (const std::exception& error)
	{
		base::uv::throwError("dtlsPrivateKeyFile: " + std::string( error.what()));
	}

	Settings::configuration.dtlsCertificateFile = dtlsCertificateFile;
	Settings::configuration.dtlsPrivateKeyFile  = dtlsPrivateKeyFile;
}
