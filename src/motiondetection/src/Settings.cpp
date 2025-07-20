/*  For testing reset api use Postman
 * https://web.postman.co/workspace/My-Workspace~292b44c7-cae4-44d6-8253-174622f0233e/request/create?requestId=e6995876-3b8c-4b7e-b170-83a733a631db
 */


#include "Settings.h"
#include "base/error.h"
#include "base/logger.h"
#include "json/configuration.h"

#include <cctype> // isprint()
#include <cerrno>
#include <iterator> // std::ostream_iterator
#include <sstream>  // std::ostringstream

#define LOGGING_LOG_TO_FILE 1
/* Class variables. */

struct Settings::Configuration Settings::configuration;


uv_rwlock_t Settings:: rwlock_t;
/* Class methods. */

void Settings::init()
{
    uv_rwlock_init(&rwlock_t);

}

void Settings::exit()
{
   uv_rwlock_destroy(&rwlock_t);
 
}

void Settings::readConfig(std::string file)
{
 
    uv_rwlock_rdlock(&rwlock_t);
       
    base::cnfg::Configuration config;

    config.load(file);

    Settings::configuration.root = config.root;
    
    uv_rwlock_rdunlock(&rwlock_t);
 }

void Settings::SetConfiguration()
{      
        //Settings::configuration.root = Settings::configuration.root;
            
	std::string stringValue;
	std::vector<std::string> logTags;
        
       //  std::cout << Settings::configuration.root.dump(4) << std::flush;
//
//        if (Settings::configuration.root.find("logTags") != Settings::configuration.root.end()) {
//          // there is an entry with key "foo"
//            json &j =  Settings::configuration.root["logTags"];
//            if(j.is_array())
//            {
//                for (json::iterator it = j.begin(); it != j.end(); ++it) {
//                    logTags.push_back(it->get<std::string>());
//                }
//            }
//        }
//        
        
//        if (Settings::configuration.root.find("rtcMinPort") != Settings::configuration.root.end()) {
//            Settings::configuration.rtcMinPort = Settings::configuration.root["rtcMinPort"].get<uint16_t>();
//        }
//            
//        if (Settings::configuration.root.find("rtcMaxPort") != Settings::configuration.root.end()) {
//            Settings::configuration.rtcMaxPort = Settings::configuration.root["rtcMaxPort"].get<uint16_t>();
//        }
//        
        
        if (Settings::configuration.root.find("pre_clip") != Settings::configuration.root.end()) {
            Settings::configuration.pre_clip = Settings::configuration.root["pre_clip"].get<uint16_t>();
        }
        

        if (Settings::configuration.root.find("cam_reconnect") != Settings::configuration.root.end()) {
            Settings::configuration.cam_reconnect = Settings::configuration.root["cam_reconnect"].get<uint16_t>();
        }


        #if FPSONE 
        if (Settings::configuration.root.find("DROP_P_Frame") != Settings::configuration.root.end()) {
            Settings::configuration.DROP_P_Frame = Settings::configuration.root["DROP_P_Frame"].get<bool>();
        }
        #endif  
        
        /*
            
        if (Settings::configuration.root.find("CLIPSIZE") != Settings::configuration.root.end()) {
            Settings::configuration.CLIPSIZE = Settings::configuration.root["CLIPSIZE"].get<uint16_t>();
        }
        
        
        
        
        if (Settings::configuration.root.find("DEFAULT_WINDOW_SIZE") != Settings::configuration.root.end()) {
            Settings::configuration.DEFAULT_WINDOW_SIZE = Settings::configuration.root["DEFAULT_WINDOW_SIZE"].get<uint16_t>();
        }
            
        if (Settings::configuration.root.find("DEFAULT_OCCUPANCY_THRESHOLD") != Settings::configuration.root.end()) {
            Settings::configuration.DEFAULT_OCCUPANCY_THRESHOLD = Settings::configuration.root["DEFAULT_OCCUPANCY_THRESHOLD"].get<uint16_t>();
        }
        
        if (Settings::configuration.root.find("DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD") != Settings::configuration.root.end()) {
            Settings::configuration.DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD = Settings::configuration.root["DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD"].get<float>();
        }
            
        if (Settings::configuration.root.find("DEFAULT_OCCUPANCY_AVG_THRESHOLD") != Settings::configuration.root.end()) {
            Settings::configuration.DEFAULT_OCCUPANCY_AVG_THRESHOLD = Settings::configuration.root["DEFAULT_OCCUPANCY_AVG_THRESHOLD"].get<float>();
        }
         
        */ 
        
            
       // if (Settings::configuration.root.find("rtsp") != Settings::configuration.root.end()) {
       //     Settings::configuration.rtsp = Settings::configuration.root["rtsp"];
       // }
        
      
            
        if (Settings::configuration.root.find("logLevel") != Settings::configuration.root.end()) {   // trace, debug, info, warn
            //TBD // Move logger setting from main to here
                // Initialize the Logger.
            
            std::string loglevel = Settings::configuration.root["logLevel"].get<std::string>();
            
            base::Level ld = base::getLevelFromString(loglevel.c_str());
            
#if	LOGGING_LOG_TO_FILE
            base::Logger::instance().add(new base::RotatingFileChannel("mdserver","/var/log/mdserver", ld));
            base::Logger::instance().setWriter(new base::AsyncLogWriter);
#else
            base::Logger::instance().add(new base::ConsoleChannel("mdserver", ld));
#endif
            
        }
        
   
            
        if (Settings::configuration.root.find("dtlsCertificateFile") != Settings::configuration.root.end()) {
          Settings::configuration.dtlsCertificateFile = Settings::configuration.root["dtlsCertificateFile"].get<std::string>();
        }
            
            
        if (Settings::configuration.root.find("dtlsPrivateKeyFile") != Settings::configuration.root.end()) {
           Settings::configuration.dtlsPrivateKeyFile = Settings::configuration.root["dtlsPrivateKeyFile"].get<std::string>();
        }
        
        
        if (Settings::configuration.root.find("storage") != Settings::configuration.root.end()) {
           Settings::configuration.storage = Settings::configuration.root["storage"].get<std::string>();
        }
        

        if (Settings::configuration.root.find("listenIps") != Settings::configuration.root.end()) {
           Settings::configuration.listenIps = Settings::configuration.root["listenIps"];
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


 void  Settings::saveFile(const std::string& path, const std::string& dump)
{
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    if (!ofs.is_open())
        throw std::runtime_error("Cannot open output file: " + path);


    ofs << dump;
    
    ofs.close();
}


void Settings::postNode(json &node ) // complete json
{
 
    std::string dump;
    uv_rwlock_wrlock(&rwlock_t);
          
    Settings::configuration.root["rtsp"] = node ;
    
    dump =  Settings::configuration.root.dump(4) ;
          
    uv_rwlock_wrunlock(&rwlock_t);
    
    saveFile( "./config.js", dump   );

     
}

bool Settings::putNode(json &node , std::vector<std::string> & vec )  // only one node
{
    bool ret = false;
    std::string dump;
    uv_rwlock_wrlock(&rwlock_t);
      
    json &rtsp =   Settings::configuration.root["rtsp"] ;
    
    for (auto& [key, value] : node.items())
    {
       
       if (rtsp.find(key) == rtsp.end()) 
       {
            rtsp[key] = value;
            vec.push_back(key);
            ret = true;
       }
    }
    dump =  Settings::configuration.root.dump(4) ;
    uv_rwlock_wrunlock(&rwlock_t);
    
    saveFile( "./config.js", dump   );
    
    return ret;
     
}


bool Settings::deleteNode(json &node , std::vector<std::string> & vec  ) 
{
    bool ret = false;
    std::string dump;
    
    
   // if(node.is_object()
    
    uv_rwlock_wrlock(&rwlock_t);      
    json &rtsp =  Settings::configuration.root["rtsp"];
    
     for (json::iterator it = node.begin(); it != node.end(); ++it) 
    //for (auto& [key, value] : node.items())
    {
       std::string key;
       
       if(node.is_object())
          key = it.key();
       else
          key = *it;
       
       if (rtsp.find(key) != rtsp.end()) 
       {
            rtsp.erase(key);
            vec.push_back(key);
            ret = true;
       }
       
    }
    
    dump =  Settings::configuration.root.dump(4) ;
     
    uv_rwlock_wrunlock(&rwlock_t);
    
    saveFile( "./config.js", dump   );
     
    return ret;
     
}

std::string Settings::getNode() 
{
    std::string ret;
    uv_rwlock_rdlock(&rwlock_t);
    
    json &rtsp =  Settings::configuration.root["rtsp"];
    ret = rtsp.dump(4) ;
    uv_rwlock_rdunlock(&rwlock_t);
    return ret;  
}

bool Settings::setNodeState(std::string &id , std::string  status) 
{
    bool ret = false;
    
    uv_rwlock_wrlock(&rwlock_t);     
    json &rtsp =   Settings::configuration.root["rtsp"];
    if (rtsp.find(id) != rtsp.end()) 
    {
        rtsp[id]["state"]= status;   
        ret = true;
    }
    uv_rwlock_wrunlock(&rwlock_t);

    return ret;  
}

bool Settings::setNodeValue(std::string &id , std::string  &key,  std::string  &value ) 
{
    bool ret = false;
    
    uv_rwlock_wrlock(&rwlock_t);     
    json &rtsp =   Settings::configuration.root["rtsp"];
    if (rtsp.find(id) != rtsp.end()) 
    {
        rtsp[id][key]= value;   
        ret = true;
    }
    uv_rwlock_wrunlock(&rwlock_t);

    return ret;  
}


bool Settings::getNodeID(std::string id, std::string  &value) 
{
    
    bool ret = false;
    
    uv_rwlock_rdlock(&rwlock_t);
       
    json &rtsp =   Settings::configuration.root["rtsp"];
    if (rtsp.find(id) != rtsp.end()) 
    {
       json &tmp =  rtsp[id];  
       value = tmp.dump(4) ;
       ret = true;
    }
    
   uv_rwlock_rdunlock(&rwlock_t);
    

    return ret;  
}


bool Settings::getNodeState(std::string id ,  std::string  key ,   std::string  &value) 
{
    
    bool ret = false;
    
     uv_rwlock_rdlock(&rwlock_t);
       
    json &rtsp =   Settings::configuration.root["rtsp"];
    if (rtsp.find(id) != rtsp.end()) 
    {
       value =  rtsp[id][key];   
       ret = true;
    }
    
  uv_rwlock_rdunlock(&rwlock_t);
    

    return ret;  
}

 
 bool Settings::getNodeStates(std::string id , std::string  key1 , std::string  &value1 ,  std::string  key2 , bool  &value2) 
{
    
    bool ret = false;
    
     uv_rwlock_rdlock(&rwlock_t);
       
    json &rtsp =   Settings::configuration.root["rtsp"];
    if (rtsp.find(id) != rtsp.end()) 
    {
       value1 =  rtsp[id][key1];
       if (rtsp[id].find(key2) != rtsp[id].end()) 
          value2 =  rtsp[id][key2].get<bool>();;   
       ret = true;
    }
    
  uv_rwlock_rdunlock(&rwlock_t);
    

    return ret;  
}
    
    


 
  void Settings::getMDConfig(std::string id , MDConfig &mdConfig) 
{
    
    
    
     uv_rwlock_rdlock(&rwlock_t);
       
    json &rtsp =   Settings::configuration.root["rtsp"];
    if (rtsp.find(id) != rtsp.end()) 
    {
        if (rtsp[id].find("CLIPSIZE") != rtsp[id].end()) 
        {
            mdConfig.CLIPSIZE = rtsp[id]["CLIPSIZE"];
        }
        
        if (rtsp[id].find("DEFAULT_WINDOW_SIZE") != rtsp[id].end()) 
        {
            mdConfig.DEFAULT_WINDOW_SIZE = rtsp[id]["DEFAULT_WINDOW_SIZE"];
        }
        
        if (rtsp[id].find("DEFAULT_OCCUPANCY_THRESHOLD") != rtsp[id].end()) 
        {
            mdConfig.DEFAULT_OCCUPANCY_THRESHOLD = rtsp[id]["DEFAULT_OCCUPANCY_THRESHOLD"];
        }
        
        if (rtsp[id].find("DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD") != rtsp[id].end()) 
        {
            mdConfig.DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD = rtsp[id]["DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD"];
        }
        
        if (rtsp[id].find("DEFAULT_OCCUPANCY_AVG_THRESHOLD") != rtsp[id].end()) 
        {
            mdConfig.DEFAULT_OCCUPANCY_AVG_THRESHOLD = rtsp[id]["DEFAULT_OCCUPANCY_AVG_THRESHOLD"];
        }

        if (rtsp[id].find("masking_coords") != rtsp[id].end()) 
        {
            mdConfig.masking_coords = rtsp[id]["masking_coords"];
        }
        
        if (rtsp[id].find("contourArea") != rtsp[id].end()) 
        {
            mdConfig.contourArea = rtsp[id]["contourArea"];
            mdConfig.contourArea =  mdConfig.contourArea > 8 ? mdConfig.contourArea/8:1;
        }
        
        if (rtsp[id].find("nonMonitoringHr") != rtsp[id].end()) 
        {
            if (rtsp[id]["nonMonitoringHr"].find("contourArea") != rtsp[id]["nonMonitoringHr"].end()) 
            {
                mdConfig.nonMonitoringHr.contourArea = rtsp[id]["nonMonitoringHr"]["contourArea"];
                mdConfig.nonMonitoringHr.contourArea =  mdConfig.nonMonitoringHr.contourArea > 8 ? mdConfig.nonMonitoringHr.contourArea/8:1;
            }
            if (rtsp[id]["nonMonitoringHr"].find("DEFAULT_WINDOW_SIZE") != rtsp[id]["nonMonitoringHr"].end()) 
            {
                mdConfig.nonMonitoringHr.DEFAULT_WINDOW_SIZE = rtsp[id]["nonMonitoringHr"]["DEFAULT_WINDOW_SIZE"];
            }
            if (rtsp[id]["nonMonitoringHr"].find("statrtHr") != rtsp[id]["nonMonitoringHr"].end()) 
            {
                mdConfig.nonMonitoringHr.statrtHr = rtsp[id]["nonMonitoringHr"]["statrtHr"];
            }
             if (rtsp[id]["nonMonitoringHr"].find("endHr") != rtsp[id]["nonMonitoringHr"].end()) 
            {
                mdConfig.nonMonitoringHr.endHr = rtsp[id]["nonMonitoringHr"]["endHr"];
            }
             if (rtsp[id]["nonMonitoringHr"].find("statrtMin") != rtsp[id]["nonMonitoringHr"].end()) 
            {
                mdConfig.nonMonitoringHr.statrtMin = rtsp[id]["nonMonitoringHr"]["statrtMin"];
            }
            if (rtsp[id]["nonMonitoringHr"].find("endMin") != rtsp[id]["nonMonitoringHr"].end()) 
            {
                mdConfig.nonMonitoringHr.endMin = rtsp[id]["nonMonitoringHr"]["endMin"];
            }
        }
        
        if (rtsp[id].find("soft2minRule") != rtsp[id].end()) 
        {
            if (rtsp[id]["soft2minRule"].find("contourArea") != rtsp[id]["soft2minRule"].end()) 
            {
                mdConfig.soft2minRule.contourArea = rtsp[id]["soft2minRule"]["contourArea"];
                mdConfig.soft2minRule.contourArea =   mdConfig.soft2minRule.contourArea  > 8 ?  mdConfig.soft2minRule.contourArea /8:1;
            }
            
            if (rtsp[id]["soft2minRule"].find("DEFAULT_WINDOW_SIZE") != rtsp[id]["soft2minRule"].end()) 
            {
                mdConfig.soft2minRule.DEFAULT_WINDOW_SIZE = rtsp[id]["soft2minRule"]["DEFAULT_WINDOW_SIZE"];
            }
        }


       //if (rtsp[id].find(key2) != rtsp[id].end()) 
       //   value2 =  rtsp[id][key2].get<bool>();;   
      // ret = true;
    }
    
   uv_rwlock_rdunlock(&rwlock_t);
    

}
 

#undef LOGGING_LOG_TO_FILE
