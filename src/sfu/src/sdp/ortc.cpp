
#include "sdp/ortc.h"
#include "LoggerTag.h"
#include "base/error.h"
//#include <media/base/h264_profile_level_id.h>
#include <algorithm> // std::find_if
#include <regex>
#include <stdexcept>
#include <string>
#include "Utils.h"
#include "sdp/Utils.h"
#include "sdp/scalabilityMode.h"



typedef std::map<std::string, std::string> CodecParameterMap;


const char kProfileLevelId[] = "profile-level-id";
const char kLevelAsymmetryAllowed[] = "level-asymmetry-allowed";
const uint8_t kConstraintSet3Flag = 0x10;



enum Profile {
  kProfileConstrainedBaseline,
  kProfileBaseline,
  kProfileMain,
  kProfileConstrainedHigh,
  kProfileHigh,
  kProfile_Invalid=-1
};

enum H264Level {
  kLevel_Invalid=-1,
  kLevel1_b = 0,
  kLevel1 = 10,
  kLevel1_1 = 11,
  kLevel1_2 = 12,
  kLevel1_3 = 13,
  kLevel2 = 20,
  kLevel2_1 = 21,
  kLevel2_2 = 22,
  kLevel3 = 30,
  kLevel3_1 = 31,
  kLevel3_2 = 32,
  kLevel4 = 40,
  kLevel4_1 = 41,
  kLevel4_2 = 42,
  kLevel5 = 50,
  kLevel5_1 = 51,
  kLevel5_2 = 52
};

// kProfilePatterns is statically initialized.
constexpr uint8_t ByteMaskString(char c, const char (&str)[9]) {
  return (str[0] == c) << 7 | (str[1] == c) << 6 | (str[2] == c) << 5 |
         (str[3] == c) << 4 | (str[4] == c) << 3 | (str[5] == c) << 2 |
         (str[6] == c) << 1 | (str[7] == c) << 0;
}


class BitPattern {
 public:
  explicit constexpr BitPattern(const char (&str)[9])
      : mask_(~ByteMaskString('x', str)),
        masked_value_(ByteMaskString('1', str)) {}

  bool IsMatch(uint8_t value) const { return masked_value_ == (value & mask_); }

 private:
  const uint8_t mask_;
  const uint8_t masked_value_;
};

struct ProfilePattern {
  const uint8_t profile_idc;
  const BitPattern profile_iop;
  const Profile profile;
};

struct ProfileLevelId {
  constexpr ProfileLevelId(Profile profile, H264Level level)
      : profile(profile), level(level) {}
  Profile profile;
  H264Level level;
};


constexpr ProfilePattern kProfilePatterns[] = {
    {0x42, BitPattern("x1xx0000"), kProfileConstrainedBaseline},
    {0x4D, BitPattern("1xxx0000"), kProfileConstrainedBaseline},
    {0x58, BitPattern("11xx0000"), kProfileConstrainedBaseline},
    {0x42, BitPattern("x0xx0000"), kProfileBaseline},
    {0x58, BitPattern("10xx0000"), kProfileBaseline},
    {0x4D, BitPattern("0x0x0000"), kProfileMain},
    {0x64, BitPattern("00000000"), kProfileHigh},
    {0x64, BitPattern("00001100"), kProfileConstrainedHigh}};

// Parse profile level id that is represented as a string of 3 hex bytes.
// Nothing will be returned if the string is not a recognized H264
// profile level id.
ProfileLevelId ParseProfileLevelId(const char* str) {
  // The string should consist of 3 bytes in hexadecimal format.
   ProfileLevelId nullopt{kProfile_Invalid,kLevel_Invalid};
           
  if (strlen(str) != 6u)
    return  nullopt;
  const uint32_t profile_level_id_numeric = strtol(str, nullptr, 16);
  if (profile_level_id_numeric == 0)
    return nullopt;

  // Separate into three bytes.
  const uint8_t level_idc =
      static_cast<uint8_t>(profile_level_id_numeric & 0xFF);
  const uint8_t profile_iop =
      static_cast<uint8_t>((profile_level_id_numeric >> 8) & 0xFF);
  const uint8_t profile_idc =
      static_cast<uint8_t>((profile_level_id_numeric >> 16) & 0xFF);

  // Parse level based on level_idc and constraint set 3 flag.
  H264Level level;
  switch (level_idc) {
    case kLevel1_1:
      level = (profile_iop & kConstraintSet3Flag) != 0 ? kLevel1_b : kLevel1_1;
      break;
    case kLevel1:
    case kLevel1_2:
    case kLevel1_3:
    case kLevel2:
    case kLevel2_1:
    case kLevel2_2:
    case kLevel3:
    case kLevel3_1:
    case kLevel3_2:
    case kLevel4:
    case kLevel4_1:
    case kLevel4_2:
    case kLevel5:
    case kLevel5_1:
    case kLevel5_2:
      level = static_cast<H264Level>(level_idc);
      break;
    default:
      // Unrecognized level_idc.
      return nullopt;
  }

  // Parse profile_idc/profile_iop into a Profile enum.
  for (const ProfilePattern& pattern : kProfilePatterns) {
    if (profile_idc == pattern.profile_idc &&
        pattern.profile_iop.IsMatch(profile_iop)) {
      return ProfileLevelId(pattern.profile, level);
    }
  }

  // Unrecognized profile_idc/profile_iop combination.
  return nullopt;
}

ProfileLevelId ParseSdpProfileLevelId(
    const CodecParameterMap& params) {
  // TODO(magjed): The default should really be kProfileBaseline and kLevel1
  // according to the spec: https://tools.ietf.org/html/rfc6184#section-8.1. In
  // order to not break backwards compatibility with older versions of WebRTC
  // where external codecs don't have any parameters, use
  // kProfileConstrainedBaseline kLevel3_1 instead. This workaround will only be
  // done in an interim period to allow external clients to update their code.
  // http://crbug/webrtc/6337.
  static const ProfileLevelId kDefaultProfileLevelId(
      kProfileConstrainedBaseline, kLevel3_1);

  const auto profile_level_id_it = params.find(kProfileLevelId);
  return (profile_level_id_it == params.end())
             ? kDefaultProfileLevelId
             : ParseProfileLevelId(profile_level_id_it->second.c_str());
}


static bool IsSameH264Profile(const CodecParameterMap& params1,
                              const CodecParameterMap& params2) {
  const ProfileLevelId profile_level_id =
     ParseSdpProfileLevelId(params1);
  const ProfileLevelId other_profile_level_id =
     ParseSdpProfileLevelId(params2);
  // Compare H264 profiles, but not levels.
  return profile_level_id.profile > -1  && other_profile_level_id.profile > -1 &&
         profile_level_id.profile == other_profile_level_id.profile;
}

std::string ProfileLevelIdToString(
    const ProfileLevelId& profile_level_id) {
  // Handle special case level == 1b.
  if (profile_level_id.level == kLevel1_b) {
    switch (profile_level_id.profile) {
      case kProfileConstrainedBaseline:
        return {"42f00b"};
      case kProfileBaseline:
        return {"42100b"};
      case kProfileMain:
        return {"4d100b"};
      // Level 1b is not allowed for other profiles.
      default:
        return "error please fix profile id";
       }
  }

  const char* profile_idc_iop_string;
  switch (profile_level_id.profile) {
    case kProfileConstrainedBaseline:
      profile_idc_iop_string = "42e0";
      break;
    case kProfileBaseline:
      profile_idc_iop_string = "4200";
      break;
    case kProfileMain:
      profile_idc_iop_string = "4d00";
      break;
    case kProfileConstrainedHigh:
      profile_idc_iop_string = "640c";
      break;
    case kProfileHigh:
      profile_idc_iop_string = "6400";
      break;
    // Unrecognized profile.
    default:
       return "error please fix profile id";
  }

  char str[7];
  snprintf(str, 7u, "%s%02x", profile_idc_iop_string, profile_level_id.level);
  return {str};
}


bool IsLevelAsymmetryAllowed(const CodecParameterMap& params) {
  const auto it = params.find(kLevelAsymmetryAllowed);
  return it != params.end() && strcmp(it->second.c_str(), "1") == 0;
}

void GenerateProfileLevelIdForAnswer(
    const CodecParameterMap& local_supported_params,
    const CodecParameterMap& remote_offered_params,
    CodecParameterMap* answer_params) {
  // If both local and remote haven't set profile-level-id, they are both using
  // the default profile. In this case, don't set profile-level-id in answer
  // either.
  if (!local_supported_params.count(kProfileLevelId) &&
      !remote_offered_params.count(kProfileLevelId)) {
    return;
  }

  // Parse profile-level-ids.
  const ProfileLevelId local_profile_level_id =
      ParseSdpProfileLevelId(local_supported_params);
  const ProfileLevelId remote_profile_level_id =
      ParseSdpProfileLevelId(remote_offered_params);


  // Parse level information.
  const bool level_asymmetry_allowed =
      IsLevelAsymmetryAllowed(local_supported_params) &&
      IsLevelAsymmetryAllowed(remote_offered_params);
  const H264Level local_level = local_profile_level_id.level;
  const H264Level remote_level = remote_profile_level_id.level;
  const H264Level min_level = std::min(local_level, remote_level);

  // Determine answer level. When level asymmetry is not allowed, level upgrade
  // is not allowed, i.e., the level in the answer must be equal to or lower
  // than the level in the offer.
  const H264Level answer_level = level_asymmetry_allowed ? local_level : min_level;

  // Set the resulting profile-level-id in the answer parameters.
  (*answer_params)[kProfileLevelId] = ProfileLevelIdToString(
      ProfileLevelId(local_profile_level_id.profile, answer_level));
}





using json = nlohmann::json;
using namespace SdpParse;

static constexpr uint32_t ProbatorSsrc{ 1234u};

// Static functions declaration.
static bool isRtxCodec(const json& codec);
static bool matchCodecs(json& aCodec, const json& bCodec, bool strict = false, bool modify = false);
static bool matchHeaderExtensions(const json& aExt, const json& bExt);
static json reduceRtcpFeedback(const json& codecA, const json& codecB);
static uint8_t getH264PacketizationMode(const json& codec);
static uint8_t getH264LevelAssimetryAllowed(const json& codec);
static std::string getH264ProfileLevelId(const json& codec);
static std::string getVP9ProfileId(const json& codec);

namespace SdpParse {
    namespace ortc {

        json getConsumerRtpParameters(json &consumableParams, json &caps) {
            
          //  STrace << "consumableParams " << consumableParams.dump(4);
          //  SInfo << "caps " << caps.dump(4);
            
            json consumerParams = {
                {"codecs", json::array()},
                 {"headerExtensions", json::array()},
                {"encodings", json::array()},
                {"rtcp", consumableParams["rtcp"]}
            };
       
            
            for (auto & capCodec : caps["codecs"]) {
                validateRtpCodecCapability(capCodec);
            }
            //const consumableCodecs = utils.clone(consumableParams.codecs);
            
            json consumableCodecs = consumableParams["codecs"];
            
            bool rtxSupported = false;
            for (auto & codec : consumableCodecs) 
            {
                
                json & capCodecs = caps["codecs"];
                
                auto matchedCapCodec =
                std::find_if(capCodecs.begin(), capCodecs.end(), [&codec](json & capCodec) {
                    return matchCodecs(capCodec, codec, /*strict*/ true);
                });
                
                if (matchedCapCodec == capCodecs.end())
                continue;
                
                codec["rtcpFeedback"] = (*matchedCapCodec)["rtcpFeedback"];
                consumerParams["codecs"].push_back(codec);
                if (!rtxSupported && isRtxCodec(codec))
                    rtxSupported = true;
            }
            // Ensure there is at least one media codec.
            if (consumerParams["codecs"].size() ==  0 || isRtxCodec(consumerParams["codecs"].at(0))) {
                SError << "no compatible media codecs";
                throw ("no compatible media codecs");
            }
            
            
            
             STrace << "caps[headerExtensions] " << caps["headerExtensions"].dump(4);
          
            STrace << "consumerParams[headerExtensions] " << consumableParams["headerExtensions"].dump(4);
            
            
            consumerParams["headerExtensions"]= json::array();
                    
            std::copy_if( consumableParams["headerExtensions"].begin(), consumableParams["headerExtensions"].end(),  std::back_inserter( consumerParams["headerExtensions"]), [&caps](json &ext)
            {
                
               return std::any_of( caps["headerExtensions"].begin(), caps["headerExtensions"].end(), [&ext](json &capExt)
                  {
                   return ((capExt["preferredId"] == ext["id"]) && (capExt["uri"] == ext["uri"]));
                  })? true : false;
            
            } 
            
            ); 
            
            STrace << "consumerParams[headerExtensions] " << consumerParams["headerExtensions"].dump(4);
            
            STrace << "codec[rtcpFeedback]" << consumerParams["codecs"].dump(4);
            
            if( std::any_of( consumerParams["headerExtensions"].begin(), consumerParams["headerExtensions"].end(), [](json &ext){ return (ext["uri"] == "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"); }))
            {
                for (auto& codec : consumerParams["codecs"]) 
                {
                    
                     std::copy_if( codec["rtcpFeedback"].begin(), codec["rtcpFeedback"].end(), codec["rtcpFeedback"].begin(), [](json &fb)
                    {
                       return  (fb["type"] != "goog-remb" );
                    } 
                    ); 
                
                }
            }
            else if( std::any_of( consumerParams["headerExtensions"].begin(), consumerParams["headerExtensions"].end(), [](json &ext){ return (ext["uri"] == "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"); }))
            {

                for (auto& codec : consumerParams["codecs"]) 
                {
                    
                     std::copy_if( codec["rtcpFeedback"].begin(), codec["rtcpFeedback"].end(), codec["rtcpFeedback"].begin(), [](json &fb)
                    {
                       return  (fb["type"] != "transport-cc" );
                    } 
                    ); 
                
                }
                
            }
            else 
            {
                for (auto& codec : consumerParams["codecs"]) 
                {
                    ///////
                     std::copy_if( codec["rtcpFeedback"].begin(), codec["rtcpFeedback"].end(), codec["rtcpFeedback"].begin(), [](json &fb)
                    {
                       return  (fb["type"] == "transport-cc" &&  fb["type"] != "goog-remb");
                    } 
                    ); 
                    //////
                    
                }
            }
             STrace << "codec[rtcpFeedback] " << consumerParams["codecs"].dump(4);
//            consumerParams["headerExtensions"] = consumableParams["headerExtensions"]
//                    .filter((ext) = > (caps.headerExtensions
//                    .some((capExt) = > (capExt.preferredId == = ext.id &&
//                    capExt.uri == = ext.uri))));
            
            // Reduce codecs' RTCP feedback. Use Transport-CC if available, REMB otherwise.
//            if (consumerParams.headerExtensions.some((ext) = > (ext.uri == = 'http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01'))) {
//                for (const codec of consumerParams.codecs) {
//                    codec.rtcpFeedback = (codec.rtcpFeedback)
//                            .filter((fb) = > fb.type != = 'goog-remb');
//                }
//            } else if (consumerParams.headerExtensions.some((ext) = > (ext.uri == = 'http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time'))) {
//                for (const codec of consumerParams.codecs) {
//                    
//                    codec.rtcpFeedback = (codec.rtcpFeedback)
//                            .filter((fb) = > fb.type != = 'transport-cc');
//                    
//                }
//            } else {
//                for (const codec of consumerParams.codecs) {
//                    codec.rtcpFeedback = (codec.rtcpFeedback)
//                            .filter((fb) = > (fb.type != = 'transport-cc' &&
//                            fb.type != = 'goog-remb'));
//                }
//            }
            
            
            
            
            json consumerEncoding = {
                {"ssrc", SdpParse::Utils::getRandomInteger(2000000, 2999999)}
            };
            
            if (rtxSupported)
                consumerEncoding["rtx"] = {{"ssrc" , SdpParse::Utils::getRandomInteger(3000000, 3999999)}};
            // If any of the consumableParams.encodings has scalabilityMode, process it
            // (assume all encodings have the same value).
            
            json& encodings =  consumableParams["encodings"];
            
            auto encodingWithScalabilityMode =
                std::find_if(encodings.begin(), encodings.end(), [](json & encoding) {
                    return (encoding.find("scalabilityMode" ) != encoding.end()) ;
                });

                        
            
            
                 // If there is simulast, mangle spatial layers in scalabilityMode.
                if(encodingWithScalabilityMode != encodings.end())
                {
                   
                    std::string scalabilityMode = (*encodingWithScalabilityMode)["scalabilityMode"].get<std::string>();
                          
                    // TBD //arvind
                    // If there is simulast, mangle spatial layers in scalabilityMode.
                    if (consumableParams["encodings"].size() > 1) {
                        
                        auto tempL = parseScalabilityMode(scalabilityMode);
                        
                        auto temporalLayers = tempL["temporalLayers"].get<int>();

                        scalabilityMode = "S" +  std::to_string(consumableParams["encodings"].size())+ "T" + std::to_string(temporalLayers);
                    }
                    if (!scalabilityMode.empty())
                        consumerEncoding["scalabilityMode"] = scalabilityMode;
                    
                }
            // Set a single encoding for the Consumer.
            consumerParams["encodings"].push_back(consumerEncoding);
            // Copy verbatim.
            consumerParams["rtcp"] = consumableParams["rtcp"];
            return consumerParams;
        }

        json getConsumableRtpParameters(std::string kind, json& params, json& caps, json& rtpMapping) {
            json consumableParams = {
                {"codecs", json::array()},
                {"headerExtensions", json::array()},
                {"encodings", json::array()},
                {"rtcp", json::object()}
            };

           // SInfo  << " params " << params.dump(4);
           // SInfo  << " caps " << caps.dump(4);
           // SInfo  << " rtpMapping " << rtpMapping.dump(4);
            
            for (auto& codec : params["codecs"]) 
            {
                if (isRtxCodec(codec))
                    continue;

                json &entrys = rtpMapping["codecs"];

                auto consumableCodecPtr =
                        std::find_if(entrys.begin(), entrys.end(), [&codec](json & entry) {
                            return entry["payloadType"] == codec["payloadType"];
                        });
                        
                if(consumableCodecPtr == entrys.end()) continue;

                json consumableCodecPt = (*consumableCodecPtr)["mappedPayloadType"];

                json &capCodecs = caps["codecs"];
                
                
                //std::string mimeb = codec["mimeType"].get<std::string>();
                //SInfo  << " codec " << codec.dump(4);

                
                auto matchedCapCodecItr =
                        std::find_if(capCodecs.begin(), capCodecs.end(), [&consumableCodecPt, &codec](json & capCodec) {
                            return capCodec["preferredPayloadType"] == consumableCodecPt &&  capCodec["mimeType"].get<std::string>() == codec["mimeType"].get<std::string>()  ;
                        });
                        
                if(matchedCapCodecItr == capCodecs.end()) 
                { 
                    continue;
                }

                auto matchedCapCodec = *matchedCapCodecItr;
                json consumableCodec = {
                    {"mimeType", matchedCapCodec["mimeType"]},
                    {"payloadType", matchedCapCodec["preferredPayloadType"]},
                    {"clockRate", matchedCapCodec["clockRate"]},
                    {"channels", matchedCapCodec["channels"]},
                    {"parameters", codec["parameters"]},
                    {"rtcpFeedback", matchedCapCodec["rtcpFeedback"]}
                };

                if (consumableCodec["channels"] == nullptr)
                         consumableCodec.erase("channels");
                
                consumableParams["codecs"].push_back(consumableCodec);

                json &capRtxCodecs = caps["codecs"];

                auto consumableCapRtxCodecItr = std::find_if(capRtxCodecs.begin(), capRtxCodecs.end(), [&consumableCodec,  &codec](json & capRtxCodec) {
                    return (isRtxCodec(capRtxCodec) && (capRtxCodec["parameters"]["apt"] == consumableCodec["payloadType"])&&( capRtxCodec["mimeType"].get<std::string>() == codec["mimeType"].get<std::string>())    );
                });

                
                if (consumableCapRtxCodecItr != capRtxCodecs.end()) {
                    auto consumableCapRtxCodec = *consumableCapRtxCodecItr;
                    json consumableRtxCodec = {
                        {"mimeType", consumableCapRtxCodec["mimeType"]},
                        {"payloadType", consumableCapRtxCodec["preferredPayloadType"]},
                        {"clockRate", consumableCapRtxCodec["clockRate"]},
                        {"parameters", consumableCapRtxCodec["parameters"]},
                        {"rtcpFeedback", consumableCapRtxCodec["rtcpFeedback"]}
                    };
                    
                     if (consumableRtxCodec["channels"]  == nullptr )
                         consumableRtxCodec.erase("channels");
                    
                    consumableParams["codecs"].push_back(consumableRtxCodec);
                }
            }
            
            
            
            for (auto& capExt : caps["headerExtensions"]) {
                // Just take RTP header extension that can be used in Consumers.
                if (capExt["kind"] != kind ||
                        (capExt["direction"] != "sendrecv" && capExt["direction"] != "sendonly")) {
                    continue;
                }
                json consumableExt = {
                    {"uri", capExt["uri"]},
                    {"id", capExt["preferredId"]},
                    {"encrypt", capExt["preferredEncrypt"]},
                    {"parameters",{}}
                };
                consumableParams["headerExtensions"].push_back(consumableExt);
            }
            // Clone Producer encodings since we'll mangle them.
            //const consumableEncodings = utils.clone(params.encodings); 
            json consumableEncodings = params["encodings"]; //arvind TBD to check if clone is required

            for (int i = 0; i < consumableEncodings.size(); ++i) {
                auto &consumableEncoding = consumableEncodings.at(i);
               //const { mappedSsrc } = rtpMapping["encodings"].at(i);
                auto mappedSsrc = rtpMapping["encodings"].at(i);
                // Remove useless fields.
                consumableEncoding.erase("rid");
                consumableEncoding.erase("rtx");
                consumableEncoding.erase("codecPayloadType");
                // Set the mapped ssrc.
                 consumableEncoding["ssrc"] =mappedSsrc["mappedSsrc"];
                consumableParams["encodings"].push_back(consumableEncoding);
            }
            consumableParams["rtcp"] ={
                {"cname", params["rtcp"]["cname"]},
                {"reducedSize", true},
                {"mux", true}
            };
            return consumableParams;
        }




        /*rtpParameters*/ /*routerRtpCapabilities*/

        json getProducerRtpParametersMapping(json& params, json& caps) {

            // This may throw.
          //  validateRtpCapabilities(params);
           // validateRtpCapabilities(caps);

            //static const std::regex MimeTypeRegex(
            //  "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            json rtpMapping ={
                { "codecs", json::array()},
                { "encodings", json::array()}
            };

            // Match parameters media codecs to capabilities media codecs.
            std::map< json, json > codecToCapCodec;

            // Match media codecs and keep the order preferred by remoteCaps.
            auto paramsCapsCodecsIt = params.find("codecs");

            for (auto& codec : *paramsCapsCodecsIt) {
                if (isRtxCodec(codec))
                    continue;

                json& remoteCodecs = caps["codecs"];

                auto matchingLocalCodecIt =
                        std::find_if(remoteCodecs.begin(), remoteCodecs.end(), [&codec](json & remoteCodec) {
                            return matchCodecs(remoteCodec, codec, /*strict*/ true, /*modify*/ true);
                        });

                if (matchingLocalCodecIt == remoteCodecs.end()) {

                    //                                    if (!matchedCapCodec) {
                    //                                        throw new errors_1.UnsupportedError(`unsupported codec [mimeType:${codec.mimeType}, payloadType:${}]`);
                    //                                    }

                    SError << "unsupported codec [mimeType: " << codec["mimeType"] << " payloadType:" << codec["payloadType"];
                    throw "unsupported codec";
                    break;
                }

                auto& matchingLocalCodec = *matchingLocalCodecIt;
                codecToCapCodec[codec] = matchingLocalCodec;
            }

            ////////////////////////////
            for (auto& codec : *paramsCapsCodecsIt) {
                if (!isRtxCodec(codec))
                    continue;
                // Search for the associated media codec.

                json& mediaCodecs = params["codecs"];

                auto associatedMediaCodec =
                        std::find_if(mediaCodecs.begin(), mediaCodecs.end(), [&codec](json & mediaCodec) {
                            return mediaCodec["payloadType"] == codec["parameters"]["apt"];
                        });


                //                        const associatedMediaCodec = 
                //                            .find((mediaCodec) => mediaCodec.payloadType === codec.parameters.apt);
                //                        if (!associatedMediaCodec) {
                //                            throw new TypeError(`missing media codec found for RTX PT ${codec.payloadType}`);
                //                        }

                if (associatedMediaCodec == mediaCodecs.end()) {
                    SError << "missing media codec ound for RTX PT " << codec["payloadType"];
                    throw "missing media codec ";
                    break;
                }
                auto capMediaCodec = codecToCapCodec[*associatedMediaCodec];
                // Ensure that the capabilities media codec has a RTX codec.
                //                        const associatedCapRtxCodec = caps.codecs
                //                            .find((capCodec) => (isRtxCodec(capCodec) &&
                //                            capCodec.parameters.apt === capMediaCodec.preferredPayloadType));
                //                        if (!associatedCapRtxCodec) {
                //                            throw new errors_1.UnsupportedError(`no RTX codec for capability codec PT ${capMediaCodec.preferredPayloadType}`);
                //                        }

                json& capCodecs = caps["codecs"];

                auto associatedCapRtxCodec =
                        std::find_if(capCodecs.begin(), capCodecs.end(), [&capMediaCodec](json & capCodec) {
                            return (isRtxCodec(capCodec) && capMediaCodec["preferredPayloadType"] == capCodec["parameters"]["apt"]);
                        });


                if (associatedCapRtxCodec == capCodecs.end()) {
                    SError << "UnsupportedError ound for RTX PT no RTX codec for capability codec PT " << capMediaCodec["preferredPayloadType"];
                    throw "UnsupportedError ";
                    break;
                }


                codecToCapCodec[codec] = *associatedCapRtxCodec;
            }
            ///////////////////////////

            // Generate codecs mapping.
            for (const auto& ctocap : codecToCapCodec) {
                // kmap->first["payloadType"];
                json obj = json::object();
                obj["payloadType"] = ctocap.first["payloadType"];
                obj["mappedPayloadType"] = ctocap.second["preferredPayloadType"];

                rtpMapping["codecs"].push_back(obj);
            }


            // Generate encodings mapping.
            int mappedSsrc = SdpParse::Utils::getRandomInteger(100000, 999999);
            for (auto& encoding : params["encodings"]) {
                json mappedEncoding = {};
                mappedEncoding["mappedSsrc"] = mappedSsrc++;
                if (encoding.find("rid") != encoding.end())
                    mappedEncoding["rid"] = encoding["rid"];
                if (encoding.find("ssrc") != encoding.end())
                    mappedEncoding["ssrc"] = encoding["ssrc"];
                if (encoding.find("scalabilityMode") != encoding.end())
                    mappedEncoding["scalabilityMode"] = encoding["scalabilityMode"];

                rtpMapping["encodings"].push_back(mappedEncoding);
            }
            return rtpMapping;

            ////////////////////////////////////////////////

        }

        /**
         * Validates RtpCapabilities. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtpCapabilities(json& caps) {


            if (!caps.is_object())
                MS_ABORT("caps is not an object");

            auto codecsIt = caps.find("codecs");
            auto headerExtensionsIt = caps.find("headerExtensions");

            // codecs is optional. If unset, fill with an empty array.
            if (codecsIt != caps.end() && !codecsIt->is_array()) {
                MS_ABORT("caps.codecs is not an array");
            } else if (codecsIt == caps.end()) {
                caps["codecs"] = json::array();
                codecsIt = caps.find("codecs");
            }

            for (auto& codec : *codecsIt) {
                validateRtpCodecCapability(codec);
            }

            // headerExtensions is optional. If unset, fill with an empty array.
            if (headerExtensionsIt != caps.end() && !headerExtensionsIt->is_array()) {
                MS_ABORT("caps.headerExtensions is not an array");
            } else if (headerExtensionsIt == caps.end()) {
                caps["headerExtensions"] = json::array();
                headerExtensionsIt = caps.find("headerExtensions");
            }

            for (auto& ext : *headerExtensionsIt) {
                validateRtpHeaderExtension(ext);
            }
        }

        /**
         * Validates RtpCodecCapability. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtpCodecCapability(json& codec) {


            static const std::regex MimeTypeRegex(
                    "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            if (!codec.is_object())
                MS_ABORT("codec is not an object");

            auto mimeTypeIt = codec.find("mimeType");
            auto preferredPayloadTypeIt = codec.find("preferredPayloadType");
            auto clockRateIt = codec.find("clockRate");
            auto channelsIt = codec.find("channels");
            auto parametersIt = codec.find("parameters");
            auto rtcpFeedbackIt = codec.find("rtcpFeedback");

            // mimeType is mandatory.
            if (mimeTypeIt == codec.end() || !mimeTypeIt->is_string())
                MS_ABORT("missing codec.mimeType");

            std::smatch mimeTypeMatch;
            //std::regex_match(mimeTypeIt->get<std::string>(), mimeTypeMatch, MimeTypeRegex);
            std::regex_match(mimeTypeIt->get_ref<std::string&>(), mimeTypeMatch, MimeTypeRegex);
            if (mimeTypeMatch.empty())
                MS_ABORT("invalid codec.mimeType");

            // Just override kind with media component of mimeType.
            codec["kind"] = mimeTypeMatch[1].str();

            // preferredPayloadType is optional.
            if (preferredPayloadTypeIt != codec.end() && !preferredPayloadTypeIt->is_number_integer())
                MS_ABORT("invalid codec.preferredPayloadType");

            // clockRate is mandatory.
            if (clockRateIt == codec.end() || !clockRateIt->is_number_integer())
                MS_ABORT("missing codec.clockRate");

            // channels is optional. If unset, set it to 1 (just if audio).
            if (codec["kind"] == "audio") {
                if (channelsIt == codec.end() || !channelsIt->is_number_integer())
                    codec["channels"] = 1;
            } else {
                if (channelsIt != codec.end())
                    codec.erase("channels");
            }

            // parameters is optional. If unset, set it to an empty object.
            if (parametersIt == codec.end() || !parametersIt->is_object()) {
                codec["parameters"] = json::object();
                parametersIt = codec.find("parameters");
            }

            for (auto it = parametersIt->begin(); it != parametersIt->end(); ++it) {
                auto& key = it.key();
                auto& value = it.value();

                if (!value.is_string() && !value.is_number() && value != nullptr)
                    MS_ABORT("invalid codec parameter");

                // Specific parameters validation.
                if (key == "apt") {
                    if (!value.is_number_integer())
                        MS_ABORT("invalid codec apt parameter");
                }
            }

            // rtcpFeedback is optional. If unset, set it to an empty array.
            if (rtcpFeedbackIt == codec.end() || !rtcpFeedbackIt->is_array()) {
                codec["rtcpFeedback"] = json::array();
                rtcpFeedbackIt = codec.find("rtcpFeedback");
            }

            for (auto& fb : *rtcpFeedbackIt) {
                validateRtcpFeedback(fb);
            }
        }

        /**
         * Validates RtcpFeedback. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtcpFeedback(json& fb) {


            if (!fb.is_object())
                MS_ABORT("fb is not an object");

            auto typeIt = fb.find("type");
            auto parameterIt = fb.find("parameter");

            // type is mandatory.
            if (typeIt == fb.end() || !typeIt->is_string())
                MS_ABORT("missing fb.type");

            // parameter is optional. If unset set it to an empty string.
            if (parameterIt == fb.end() || !parameterIt->is_string())
                fb["parameter"] = "";
        }

        /**
         * Validates RtpHeaderExtension. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtpHeaderExtension(json& ext) {


            if (!ext.is_object())
                MS_ABORT("ext is not an object");

            auto kindIt = ext.find("kind");
            auto uriIt = ext.find("uri");
            auto preferredIdIt = ext.find("preferredId");
            auto preferredEncryptIt = ext.find("preferredEncrypt");
            auto directionIt = ext.find("direction");

            // kind is optional. If unset set it to an empty string.
            if (kindIt == ext.end() || !kindIt->is_string())
                ext["kind"] = "";

            kindIt = ext.find("kind");
            std::string kind = kindIt->get<std::string>();

            if (kind != "" && kind != "audio" && kind != "video")
                MS_ABORT("invalid ext.kind");

            // uri is mandatory.
            if (uriIt == ext.end() || !uriIt->is_string() || uriIt->get<std::string>().empty())
                MS_ABORT("missing ext.uri");

            // preferredId is mandatory.
            if (preferredIdIt == ext.end() || !preferredIdIt->is_number_integer())
                MS_ABORT("missing ext.preferredId");

            // preferredEncrypt is optional. If unset set it to false.
            if (preferredEncryptIt != ext.end() && !preferredEncryptIt->is_boolean()) {
                MS_ABORT("invalid ext.preferredEncrypt");
            } else if (preferredEncryptIt == ext.end())
                ext["preferredEncrypt"] = false;

            // direction is optional. If unset set it to sendrecv.
            if (directionIt != ext.end() && !directionIt->is_string()) {
                MS_ABORT("invalid ext.direction");
            } else if (directionIt == ext.end())
                ext["direction"] = "sendrecv";
        }

        /**
         * Validates RtpParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtpParameters(json& params) {


            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto midIt = params.find("mid");
            auto codecsIt = params.find("codecs");
            auto headerExtensionsIt = params.find("headerExtensions");
            auto encodingsIt = params.find("encodings");
            auto rtcpIt = params.find("rtcp");

            // mid is optional.
            if (midIt != params.end() && (!midIt->is_string() || midIt->get<std::string>().empty())) {
                MS_ABORT("params.mid is not a string");
            }

            // codecs is mandatory.
            if (codecsIt == params.end() || !codecsIt->is_array())
                MS_ABORT("missing params.codecs");

            for (auto& codec : *codecsIt) {
                validateRtpCodecParameters(codec);
            }

            // headerExtensions is optional. If unset, fill with an empty array.
            if (headerExtensionsIt != params.end() && !headerExtensionsIt->is_array()) {
                MS_ABORT("params.headerExtensions is not an array");
            } else if (headerExtensionsIt == params.end()) {
                params["headerExtensions"] = json::array();
                headerExtensionsIt = params.find("headerExtensions");
            }

            for (auto& ext : *headerExtensionsIt) {
                validateRtpHeaderExtensionParameters(ext);
            }

            // encodings is optional. If unset, fill with an empty array.
            if (encodingsIt != params.end() && !encodingsIt->is_array()) {
                MS_ABORT("params.encodings is not an array");
            } else if (encodingsIt == params.end()) {
                params["encodings"] = json::array();
                encodingsIt = params.find("encodings");
            }

            for (auto& encoding : *encodingsIt) {
                validateRtpEncodingParameters(encoding);
            }

            // rtcp is optional. If unset, fill with an empty object.
            if (rtcpIt != params.end() && !rtcpIt->is_object()) {
                MS_ABORT("params.rtcp is not an object");
            } else if (rtcpIt == params.end()) {
                params["rtcp"] = json::object();
                rtcpIt = params.find("rtcp");
            }

            validateRtcpParameters(*rtcpIt);
        }

        /**
         * Validates RtpCodecParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtpCodecParameters(json& codec) {


            static const std::regex MimeTypeRegex(
                    "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            if (!codec.is_object())
                MS_ABORT("codec is not an object");

            auto mimeTypeIt = codec.find("mimeType");
            auto payloadTypeIt = codec.find("payloadType");
            auto clockRateIt = codec.find("clockRate");
            auto channelsIt = codec.find("channels");
            auto parametersIt = codec.find("parameters");
            auto rtcpFeedbackIt = codec.find("rtcpFeedback");

            // mimeType is mandatory.
            if (mimeTypeIt == codec.end() || !mimeTypeIt->is_string())
                MS_ABORT("missing codec.mimeType");

            std::smatch mimeTypeMatch;
            //std::regex_match(mimeTypeIt->get<std::string>(), mimeTypeMatch, MimeTypeRegex); // arvind
            std::regex_match(mimeTypeIt->get_ref<std::string&>(), mimeTypeMatch, MimeTypeRegex);
            if (mimeTypeMatch.empty())
                MS_ABORT("invalid codec.mimeType");

            // payloadType is mandatory.
            if (payloadTypeIt == codec.end() || !payloadTypeIt->is_number_integer())
                MS_ABORT("missing codec.payloadType");

            // clockRate is mandatory.
            if (clockRateIt == codec.end() || !clockRateIt->is_number_integer())
                MS_ABORT("missing codec.clockRate");

            // Retrieve media kind from mimeType.
            auto kind = mimeTypeMatch[1].str();

            // channels is optional. If unset, set it to 1 (just for audio).
            if (kind == "audio") {
                if (channelsIt == codec.end() || !channelsIt->is_number_integer())
                    codec["channels"] = 1;
            } else {
                if (channelsIt != codec.end())
                    codec.erase("channels");
            }

            // parameters is optional. If unset, set it to an empty object.
            if (parametersIt == codec.end() || !parametersIt->is_object()) {
                codec["parameters"] = json::object();
                parametersIt = codec.find("parameters");
            }

            for (auto it = parametersIt->begin(); it != parametersIt->end(); ++it) {
                auto& key = it.key();
                auto& value = it.value();

                if (!value.is_string() && !value.is_number() && value != nullptr)
                    MS_ABORT("invalid codec parameter");

                // Specific parameters validation.
                if (key == "apt") {
                    if (!value.is_number_integer())
                        MS_ABORT("invalid codec apt parameter");
                }
            }

            // rtcpFeedback is optional. If unset, set it to an empty array.
            if (rtcpFeedbackIt == codec.end() || !rtcpFeedbackIt->is_array()) {
                codec["rtcpFeedback"] = json::array();
                rtcpFeedbackIt = codec.find("rtcpFeedback");
            }

            for (auto& fb : *rtcpFeedbackIt) {
                validateRtcpFeedback(fb);
            }
        }

        /**
         * Validates RtpHeaderExtensionParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtpHeaderExtensionParameters(json& ext) {

            if (!ext.is_object())
                MS_ABORT("ext is not an object");

            auto uriIt = ext.find("uri");
            auto idIt = ext.find("id");
            auto encryptIt = ext.find("encrypt");
            auto parametersIt = ext.find("parameters");

            // uri is mandatory.
            if (uriIt == ext.end() || !uriIt->is_string() || uriIt->get<std::string>().empty()) {
                MS_ABORT("missing ext.uri");
            }

            // id is mandatory.
            if (idIt == ext.end() || !idIt->is_number_integer()) {
                MS_ABORT("missing ext.id");
            }

            // encrypt is optional. If unset set it to false.
            if (encryptIt != ext.end() && !encryptIt->is_boolean()) {
                MS_ABORT("invalid ext.encrypt");
            } else if (encryptIt == ext.end())
                ext["encrypt"] = false;

            // parameters is optional. If unset, set it to an empty object.
            if (parametersIt == ext.end() || !parametersIt->is_object()) {
                ext["parameters"] = json::object();
                parametersIt = ext.find("parameters");
            }

            for (auto it = parametersIt->begin(); it != parametersIt->end(); ++it) {
                auto& value = it.value();

                if (!value.is_string() && !value.is_number()) {
                    MS_ABORT("invalid header extension parameter");
                }
            }
        }

        /**
         * Validates RtpEncodingParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */

        /**
        Only used for an RTCRtpSender whose kind is audio, this property indicates whether or not to use discontinuous transmission (a feature by which a phone is turned off or the microphone muted automatically in the absence of voice activity). The value is taken from the enumerated string type RTCDtxStatus.
         **/
        void validateRtpEncodingParameters(json& encoding) {

            if (!encoding.is_object())
                MS_ABORT("encoding is not an object");

            auto ssrcIt = encoding.find("ssrc");
            auto ridIt = encoding.find("rid");
            auto rtxIt = encoding.find("rtx");
            auto dtxIt = encoding.find("dtx");
            auto scalabilityModeIt = encoding.find("scalabilityMode");

            // ssrc is optional.
            if (ssrcIt != encoding.end() && !ssrcIt->is_number_integer())
                MS_ABORT("invalid encoding.ssrc");

            // rid is optional.
            if (ridIt != encoding.end() && (!ridIt->is_string() || ridIt->get<std::string>().empty())) {
                MS_ABORT("invalid encoding.rid");
            }

            // rtx is optional.
            if (rtxIt != encoding.end() && !rtxIt->is_object()) {
                MS_ABORT("invalid encoding.rtx");
            } else if (rtxIt != encoding.end()) {
                auto rtxSsrcIt = rtxIt->find("ssrc");

                // RTX ssrc is mandatory if rtx is present.
                if (rtxSsrcIt == rtxIt->end() || !rtxSsrcIt->is_number_integer())
                    MS_ABORT("missing encoding.rtx.ssrc");
            }

            // dtx is optional. If unset set it to false.
            if (dtxIt == encoding.end() || !dtxIt->is_boolean())
                encoding["dtx"] = false;

            // scalabilityMode is optional.
            // clang-format off
            if (
                    scalabilityModeIt != encoding.end() &&
                    (!scalabilityModeIt->is_string() || scalabilityModeIt->get<std::string>().empty())
                    )
                // clang-format on
            {
                MS_ABORT("invalid encoding.scalabilityMode");
            }
        }

        /**
         * Validates RtcpParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateRtcpParameters(json& rtcp) {


            if (!rtcp.is_object())
                MS_ABORT("rtcp is not an object");

            auto cnameIt = rtcp.find("cname");
            auto reducedSizeIt = rtcp.find("reducedSize");

            // cname is optional.
            if (cnameIt != rtcp.end() && !cnameIt->is_string())
                MS_ABORT("invalid rtcp.cname");

            // reducedSize is optional. If unset set it to true.
            if (reducedSizeIt == rtcp.end() || !reducedSizeIt->is_boolean())
                rtcp["reducedSize"] = true;
        }

        /**
         * Validates SctpCapabilities. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateSctpCapabilities(json& caps) {


            if (!caps.is_object())
                MS_ABORT("caps is not an object");

            auto numStreamsIt = caps.find("numStreams");

            // numStreams is mandatory.
            if (numStreamsIt == caps.end() || !numStreamsIt->is_object())
                MS_ABORT("missing caps.numStreams");

            ortc::validateNumSctpStreams(*numStreamsIt);
        }

        /**
         * Validates NumSctpStreams. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateNumSctpStreams(json& numStreams) {


            if (!numStreams.is_object())
                MS_ABORT("numStreams is not an object");

            auto osIt = numStreams.find("OS");
            auto misIt = numStreams.find("MIS");

            // OS is mandatory.
            if (osIt == numStreams.end() || !osIt->is_number_integer())
                MS_ABORT("missing numStreams.OS");

            // MIS is mandatory.
            if (misIt == numStreams.end() || !misIt->is_number_integer())
                MS_ABORT("missing numStreams.MIS");
        }

        /**
         * Validates SctpParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateSctpParameters(json& params) {


            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto portIt = params.find("port");
            auto osIt = params.find("OS");
            auto misIt = params.find("MIS");
            auto maxMessageSizeIt = params.find("maxMessageSize");

            // port is mandatory.
            if (portIt == params.end() || !portIt->is_number_integer())
                MS_ABORT("missing params.port");

            // OS is mandatory.
            if (osIt == params.end() || !osIt->is_number_integer())
                MS_ABORT("missing params.OS");

            // MIS is mandatory.
            if (misIt == params.end() || !misIt->is_number_integer())
                MS_ABORT("missing params.MIS");

            // maxMessageSize is mandatory.
            if (maxMessageSizeIt == params.end() || !maxMessageSizeIt->is_number_integer()) {
                MS_ABORT("missing params.maxMessageSize");
            }
        }

        /**
         * Validates SctpStreamParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateSctpStreamParameters(json& params) {


            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto streamIdIt = params.find("streamId");
            auto orderedIt = params.find("ordered");
            auto maxPacketLifeTimeIt = params.find("maxPacketLifeTime");
            auto maxRetransmitsIt = params.find("maxRetransmits");
            auto priorityIt = params.find("priority");
            auto labelIt = params.find("label");
            auto protocolIt = params.find("protocol");

            // streamId is mandatory.
            if (streamIdIt == params.end() || !streamIdIt->is_number_integer())
                MS_ABORT("missing params.streamId");

            // ordered is optional.
            bool orderedGiven = false;

            if (orderedIt != params.end() && orderedIt->is_boolean())
                orderedGiven = true;
            else
                params["ordered"] = true;

            // maxPacketLifeTime is optional. If unset set it to 0.
            if (maxPacketLifeTimeIt == params.end() || !maxPacketLifeTimeIt->is_number_integer()) {
                params["maxPacketLifeTime"] = 0u;
            }

            // maxRetransmits is optional. If unset set it to 0.
            if (maxRetransmitsIt == params.end() || !maxRetransmitsIt->is_number_integer()) {
                params["maxRetransmits"] = 0u;
            }

            if (maxPacketLifeTimeIt != params.end() && maxRetransmitsIt != params.end()) {
                MS_ABORT("cannot provide both maxPacketLifeTime and maxRetransmits");
            }

            // clang-format off
            if (
                    orderedGiven &&
                    params["ordered"] == true &&
                    (maxPacketLifeTimeIt != params.end() || maxRetransmitsIt != params.end())
                    )
                // clang-format on
            {
                MS_ABORT("cannot be ordered with maxPacketLifeTime or maxRetransmits");
            }                // clang-format off
            else if (
                    !orderedGiven &&
                    (maxPacketLifeTimeIt != params.end() || maxRetransmitsIt != params.end())
                    )
                // clang-format on
            {
                params["ordered"] = false;
            }

            // priority is optional. If unset set it to empty string.
            if (priorityIt == params.end() || !priorityIt->is_string())
                params["priority"] = "";

            // label is optional. If unset set it to empty string.
            if (labelIt == params.end() || !labelIt->is_string())
                params["label"] = "";

            // protocol is optional. If unset set it to empty string.
            if (protocolIt == params.end() || !protocolIt->is_string())
                params["protocol"] = "";
        }

        /**
         * Validates IceParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateIceParameters(json& params) {


            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto usernameFragmentIt = params.find("usernameFragment");
            auto passwordIt = params.find("password");
            auto iceLiteIt = params.find("iceLite");

            // usernameFragment is mandatory.
            if (
                    usernameFragmentIt == params.end() ||
                    (!usernameFragmentIt->is_string() || usernameFragmentIt->get<std::string>().empty())) {
                MS_ABORT("missing params.usernameFragment");
            }

            // userFragment is mandatory.
            if (passwordIt == params.end() || (!passwordIt->is_string() || passwordIt->get<std::string>().empty())) {
                MS_ABORT("missing params.password");
            }

            // iceLIte is optional. If unset set it to false.
            if (iceLiteIt == params.end() || !iceLiteIt->is_boolean())
                params["iceLite"] = false;
        }

        /**
         * Validates IceCandidate. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateIceCandidate(json& params) {


            static const std::regex ProtocolRegex(
                    "(udp|tcp)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            static const std::regex TypeRegex(
                    "(host|srflx|prflx|relay)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto foundationIt = params.find("foundation");
            auto priorityIt = params.find("priority");
            auto ipIt = params.find("ip");
            auto protocolIt = params.find("protocol");
            auto portIt = params.find("port");
            auto typeIt = params.find("type");

            // foundation is mandatory.
            if (
                    foundationIt == params.end() ||
                    (!foundationIt->is_string() || foundationIt->get<std::string>().empty())) {
                MS_ABORT("missing params.foundation");
            }

            // priority is mandatory.
            if (priorityIt == params.end() || !priorityIt->is_number_unsigned())
                MS_ABORT("missing params.priority");

            // ip is mandatory.
            if (ipIt == params.end() || (!ipIt->is_string() || ipIt->get<std::string>().empty())) {
                MS_ABORT("missing params.ip");
            }

            // protocol is mandatory.
            if (protocolIt == params.end() || (!protocolIt->is_string() || protocolIt->get<std::string>().empty())) {
                MS_ABORT("missing params.protocol");
            }

            std::smatch protocolMatch;
            std::regex_match(protocolIt->get_ref<std::string&>(), protocolMatch, ProtocolRegex); //arvind

            if (protocolMatch.empty())
                MS_ABORT("invalid params.protocol");

            // port is mandatory.
            if (portIt == params.end() || !portIt->is_number_unsigned())
                MS_ABORT("missing params.port");

            // type is mandatory.
            if (typeIt == params.end() || (!typeIt->is_string() || typeIt->get<std::string>().empty())) {
                MS_ABORT("missing params.type");
            }

            std::smatch typeMatch;
            std::regex_match(typeIt->get_ref<std::string&>(), typeMatch, TypeRegex); //arvind

            if (typeMatch.empty())
                MS_ABORT("invalid params.type");
        }

        /**
         * Validates IceCandidates. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateIceCandidates(json& params) {


            if (!params.is_array())
                MS_ABORT("params is not an array");

            for (auto& iceCandidate : params) {
                validateIceCandidate(iceCandidate);
            }
        }

        /**
         * Validates DtlsFingerprint. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateDtlsFingerprint(json& params) {


            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto algorithmIt = params.find("algorithm");
            auto valueIt = params.find("value");

            // foundation is mandatory.
            if (
                    algorithmIt == params.end() ||
                    (!algorithmIt->is_string() || algorithmIt->get<std::string>().empty())) {
                MS_ABORT("missing params.algorithm");
            }

            // foundation is mandatory.
            if (valueIt == params.end() || (!valueIt->is_string() || valueIt->get<std::string>().empty())) {
                MS_ABORT("missing params.value");
            }
        }

        /**
         * Validates DtlsParameters. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateDtlsParameters(json& params) {


            static const std::regex RoleRegex(
                    "(auto|client|server)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto roleIt = params.find("role");
            auto fingerprintsIt = params.find("fingerprints");

            // role is mandatory.
            if (roleIt == params.end() || (!roleIt->is_string() || roleIt->get<std::string>().empty())) {
                MS_ABORT("missing params.role");
            }

            std::smatch roleMatch;
            std::regex_match(roleIt->get_ref<std::string&>(), roleMatch, RoleRegex); //arvind

            if (roleMatch.empty())
                MS_ABORT("invalid params.role");

            // fingerprints is mandatory.
            if (fingerprintsIt == params.end() || (!fingerprintsIt->is_array() || fingerprintsIt->empty())) {
                MS_ABORT("missing params.fingerprints");
            }

            for (auto& fingerprint : *fingerprintsIt)
                validateDtlsFingerprint(fingerprint);
        }

        /**
         * Validates Producer codec options. It may modify given data by adding missing
         * fields with default values.
         * It throws if invalid.
         */
        void validateProducerCodecOptions(json& params) {


            if (!params.is_object())
                MS_ABORT("params is not an object");

            auto opusStereoIt = params.find("opusStereo");
            auto opusFecIt = params.find("opusFec");
            auto opusDtxIt = params.find("opusDtx");
            auto opusMaxPlaybackRateIt = params.find("opusMaxPlaybackRate");
            auto opusPtimeIt = params.find("opusPtime");
            auto videoGoogleStartBitrateIt = params.find("videoGoogleStartBitrate");
            auto videoGoogleMaxBitrateIt = params.find("videoGoogleMaxBitrate");
            auto videoGoogleMinBitrateIt = params.find("videoGoogleMinBitrate");

            if (opusStereoIt != params.end() && !opusStereoIt->is_boolean()) {
                MS_ABORT("invalid params.opusStereo");
            }

            if (opusFecIt != params.end() && !opusFecIt->is_boolean()) {
                MS_ABORT("invalid params.opusFec");
            }

            if (opusDtxIt != params.end() && !opusDtxIt->is_boolean()) {
                MS_ABORT("invalid params.opusDtx");
            }

            if (opusMaxPlaybackRateIt != params.end() && !opusMaxPlaybackRateIt->is_number_unsigned()) {
                MS_ABORT("invalid params.opusMaxPlaybackRate");
            }

            if (opusPtimeIt != params.end() && !opusPtimeIt->is_number_integer()) {
                MS_ABORT("invalid params.opusPtime");
            }

            if (videoGoogleStartBitrateIt != params.end() && !videoGoogleStartBitrateIt->is_number_integer()) {
                MS_ABORT("invalid params.videoGoogleStartBitrate");
            }

            if (videoGoogleMaxBitrateIt != params.end() && !videoGoogleMaxBitrateIt->is_number_integer()) {
                MS_ABORT("invalid params.videoGoogleMaxBitrate");
            }

            if (videoGoogleMinBitrateIt != params.end() && !videoGoogleMinBitrateIt->is_number_integer()) {
                MS_ABORT("invalid params.videoGoogleMinBitrate");
            }
        }

        /**
         * Generate extended RTP capabilities for sending and receiving.
         */
        json getExtendedRtpCapabilities(json& localCaps, json& remoteCaps) {


            // This may throw.
            validateRtpCapabilities(localCaps);
            validateRtpCapabilities(remoteCaps);

            static const std::regex MimeTypeRegex(
                    "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

            // clang-format off
            json extendedRtpCapabilities ={
                { "codecs", json::array()},
                { "headerExtensions", json::array()}
            };
            // clang-format on

            // Match media codecs and keep the order preferred by remoteCaps.
            auto remoteCapsCodecsIt = remoteCaps.find("codecs");

            for (auto& remoteCodec : *remoteCapsCodecsIt) {
                if (isRtxCodec(remoteCodec))
                    continue;

                json& localCodecs = localCaps["codecs"];

                auto matchingLocalCodecIt =
                        std::find_if(localCodecs.begin(), localCodecs.end(), [&remoteCodec](json & localCodec) {
                            return matchCodecs(localCodec, remoteCodec, /*strict*/ true, /*modify*/ true);
                        });

                if (matchingLocalCodecIt == localCodecs.end())
                    continue;

                auto& matchingLocalCodec = *matchingLocalCodecIt;

                // clang-format off
                json extendedCodec ={
                    { "mimeType", matchingLocalCodec["mimeType"]},
                    { "kind", matchingLocalCodec["kind"]},
                    { "clockRate", matchingLocalCodec["clockRate"]},
                    { "localPayloadType", matchingLocalCodec["preferredPayloadType"]},
                    { "localRtxPayloadType", nullptr},
                    { "remotePayloadType", remoteCodec["preferredPayloadType"]},
                    { "remoteRtxPayloadType", nullptr},
                    { "localParameters", matchingLocalCodec["parameters"]},
                    { "remoteParameters", remoteCodec["parameters"]},
                    { "rtcpFeedback", reduceRtcpFeedback(matchingLocalCodec, remoteCodec)}
                };
                // clang-format on

                if (matchingLocalCodec.contains("channels"))
                    extendedCodec["channels"] = matchingLocalCodec["channels"];

                extendedRtpCapabilities["codecs"].push_back(extendedCodec);
            }

            // Match RTX codecs.
            json& extendedCodecs = extendedRtpCapabilities["codecs"];

            for (json& extendedCodec : extendedCodecs) {
                auto& localCodecs = localCaps["codecs"];
                auto localCodecIt = std::find_if(
                        localCodecs.begin(), localCodecs.end(), [&extendedCodec](const json & localCodec) {
                            return isRtxCodec(localCodec) &&
                                    localCodec["parameters"]["apt"] == extendedCodec["localPayloadType"];
                        });

                if (localCodecIt == localCodecs.end())
                    continue;

                auto& matchingLocalRtxCodec = *localCodecIt;
                auto& remoteCodecs = remoteCaps["codecs"];
                auto remoteCodecIt = std::find_if(
                        remoteCodecs.begin(), remoteCodecs.end(), [&extendedCodec](const json & remoteCodec) {
                            return isRtxCodec(remoteCodec) &&
                                    remoteCodec["parameters"]["apt"] == extendedCodec["remotePayloadType"];
                        });

                if (remoteCodecIt == remoteCodecs.end())
                    continue;

                auto& matchingRemoteRtxCodec = *remoteCodecIt;

                extendedCodec["localRtxPayloadType"] = matchingLocalRtxCodec["preferredPayloadType"];
                extendedCodec["remoteRtxPayloadType"] = matchingRemoteRtxCodec["preferredPayloadType"];
            }

            // Match header extensions.
            auto& remoteExts = remoteCaps["headerExtensions"];

            for (auto& remoteExt : remoteExts) {
                auto& localExts = localCaps["headerExtensions"];
                auto localExtIt =
                        std::find_if(localExts.begin(), localExts.end(), [&remoteExt](const json & localExt) {
                            return matchHeaderExtensions(localExt, remoteExt);
                        });

                if (localExtIt == localExts.end())
                    continue;

                auto& matchingLocalExt = *localExtIt;

                // TODO: Must do stuff for encrypted extensions.

                // clang-format off
                json extendedExt ={
                    { "kind", remoteExt["kind"]},
                    { "uri", remoteExt["uri"]},
                    { "sendId", matchingLocalExt["preferredId"]},
                    { "recvId", remoteExt["preferredId"]},
                    { "encrypt", matchingLocalExt["preferredEncrypt"]}
                };
                // clang-format on

                auto remoteExtDirection = remoteExt["direction"].get<std::string>();

                if (remoteExtDirection == "sendrecv")
                    extendedExt["direction"] = "sendrecv";
                else if (remoteExtDirection == "recvonly")
                    extendedExt["direction"] = "sendonly";
                else if (remoteExtDirection == "sendonly")
                    extendedExt["direction"] = "recvonly";
                else if (remoteExtDirection == "inactive")
                    extendedExt["direction"] = "inactive";

                extendedRtpCapabilities["headerExtensions"].push_back(extendedExt);
            }

            return extendedRtpCapabilities;
        }

        /**
         * Generate RTP capabilities for receiving media based on the given extended
         * RTP capabilities.
         */
        json getRecvRtpCapabilities(const json& extendedRtpCapabilities) {


            // clang-format off
            json rtpCapabilities ={
                { "codecs", json::array()},
                { "headerExtensions", json::array()}
            };
            // clang-format on

            for (auto& extendedCodec : extendedRtpCapabilities["codecs"]) {
                // clang-format off
                json codec ={
                    { "mimeType", extendedCodec["mimeType"]},
                    { "kind", extendedCodec["kind"]},
                    { "preferredPayloadType", extendedCodec["remotePayloadType"]},
                    { "clockRate", extendedCodec["clockRate"]},
                    { "parameters", extendedCodec["localParameters"]},
                    { "rtcpFeedback", extendedCodec["rtcpFeedback"]},
                };
                // clang-format on

                if (extendedCodec.contains("channels"))
                    codec["channels"] = extendedCodec["channels"];

                rtpCapabilities["codecs"].push_back(codec);

                // Add RTX codec.
                if (extendedCodec["remoteRtxPayloadType"] == nullptr)
                    continue;

                auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

                // clang-format off
                json rtxCodec ={
                    { "mimeType", mimeType},
                    { "kind", extendedCodec["kind"]},
                    { "preferredPayloadType", extendedCodec["remoteRtxPayloadType"]},
                    { "clockRate", extendedCodec["clockRate"]},
                    {
                        "parameters",
                        {
                            { "apt", extendedCodec["remotePayloadType"].get<uint8_t>()}
                        }
                    },
                    { "rtcpFeedback", json::array()}
                };
                // clang-format on

                rtpCapabilities["codecs"].push_back(rtxCodec);

                // TODO: In the future, we need to add FEC, CN, etc, codecs.
            }

            for (auto& extendedExtension : extendedRtpCapabilities["headerExtensions"]) {
                std::string direction = extendedExtension["direction"].get<std::string>();

                // Ignore RTP extensions not valid for receiving.
                if (direction != "sendrecv" && direction != "recvonly")
                    continue;

                // clang-format off
                json ext ={
                    { "kind", extendedExtension["kind"]},
                    { "uri", extendedExtension["uri"]},
                    { "preferredId", extendedExtension["recvId"]},
                    { "preferredEncrypt", extendedExtension["encrypt"]},
                    { "direction", extendedExtension["direction"]}
                };
                // clang-format on

                rtpCapabilities["headerExtensions"].push_back(ext);
            }

            return rtpCapabilities;
        }

        /**
         * Generate RTP parameters of the given kind for sending media.
         * Just the first media codec per kind is considered.
         * NOTE: mid, encodings and rtcp fields are left empty.
         */
        json getSendingRtpParameters(const std::string& kind, const json& extendedRtpCapabilities) {


            // clang-format off
            json rtpParameters ={
                { "mid", nullptr},
                { "codecs", json::array()},
                { "headerExtensions", json::array()},
                { "encodings", json::array()},
                { "rtcp", json::object()}
            };
            // clang-format on

            for (auto& extendedCodec : extendedRtpCapabilities["codecs"]) {
                if (kind != extendedCodec["kind"].get<std::string>())
                    continue;

                // clang-format off
                json codec ={
                    { "mimeType", extendedCodec["mimeType"]},
                    { "payloadType", extendedCodec["localPayloadType"]},
                    { "clockRate", extendedCodec["clockRate"]},
                    { "parameters", extendedCodec["localParameters"]},
                    { "rtcpFeedback", extendedCodec["rtcpFeedback"]}
                };
                // clang-format on

                if (extendedCodec.contains("channels"))
                    codec["channels"] = extendedCodec["channels"];

                rtpParameters["codecs"].push_back(codec);

                // Add RTX codec.
                if (extendedCodec["localRtxPayloadType"] != nullptr) {
                    auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

                    // clang-format off
                    json rtxCodec ={
                        { "mimeType", mimeType},
                        { "payloadType", extendedCodec["localRtxPayloadType"]},
                        { "clockRate", extendedCodec["clockRate"]},
                        {
                            "parameters",
                            {
                                { "apt", extendedCodec["localPayloadType"].get<uint8_t>()}
                            }
                        },
                        { "rtcpFeedback", json::array()}
                    };
                    // clang-format on

                    rtpParameters["codecs"].push_back(rtxCodec);
                }

                // NOTE: We assume a single media codec plus an optional RTX codec.
               // break;
            }

            for (auto& extendedExtension : extendedRtpCapabilities["headerExtensions"]) {
                if (kind != extendedExtension["kind"].get<std::string>())
                    continue;

                std::string direction = extendedExtension["direction"].get<std::string>();

                // Ignore RTP extensions not valid for sending.
                if (direction != "sendrecv" && direction != "sendonly")
                    continue;

                // clang-format off
                json ext ={
                    { "uri", extendedExtension["uri"]},
                    { "id", extendedExtension["sendId"]},
                    { "encrypt", extendedExtension["encrypt"]},
                    { "parameters", json::object()}
                };
                // clang-format on

                rtpParameters["headerExtensions"].push_back(ext);
            }

            return rtpParameters;
        }

        /**
         * Generate RTP parameters of the given kind for sending media.
         */
        json getSendingRemoteRtpParameters(const std::string& kind, const json& extendedRtpCapabilities) {


            // clang-format off
            json rtpParameters ={
                { "mid", nullptr},
                { "codecs", json::array()},
                { "headerExtensions", json::array()},
                { "encodings", json::array()},
                { "rtcp", json::object()}
            };
            // clang-format on

            for (auto& extendedCodec : extendedRtpCapabilities["codecs"]) {
                if (kind != extendedCodec["kind"].get<std::string>())
                    continue;

                // clang-format off
                json codec ={
                    { "mimeType", extendedCodec["mimeType"]},
                    { "payloadType", extendedCodec["localPayloadType"]},
                    { "clockRate", extendedCodec["clockRate"]},
                    { "parameters", extendedCodec["remoteParameters"]},
                    { "rtcpFeedback", extendedCodec["rtcpFeedback"]}
                };
                // clang-format on

                if (extendedCodec.contains("channels"))
                    codec["channels"] = extendedCodec["channels"];

                rtpParameters["codecs"].push_back(codec);

                // Add RTX codec.
                if (extendedCodec["localRtxPayloadType"] != nullptr) {
                    auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

                    // clang-format off
                    json rtxCodec ={
                        { "mimeType", mimeType},
                        { "payloadType", extendedCodec["localRtxPayloadType"]},
                        { "clockRate", extendedCodec["clockRate"]},
                        {
                            "parameters",
                            {
                                { "apt", extendedCodec["localPayloadType"].get<uint8_t>()}
                            }
                        },
                        { "rtcpFeedback", json::array()}
                    };
                    // clang-format on

                    rtpParameters["codecs"].push_back(rtxCodec);
                }

                // NOTE: We assume a single media codec plus an optional RTX codec.
                break;
            }

            for (auto& extendedExtension : extendedRtpCapabilities["headerExtensions"]) {
                if (kind != extendedExtension["kind"].get<std::string>())
                    continue;

                std::string direction = extendedExtension["direction"].get<std::string>();

                // Ignore RTP extensions not valid for sending.
                if (direction != "sendrecv" && direction != "sendonly")
                    continue;

                // clang-format off
                json ext ={
                    { "uri", extendedExtension["uri"]},
                    { "id", extendedExtension["sendId"]},
                    { "encrypt", extendedExtension["encrypt"]},
                    { "parameters", json::object()}
                };
                // clang-format on

                rtpParameters["headerExtensions"].push_back(ext);
            }

            auto headerExtensionsIt = rtpParameters.find("headerExtensions");

            // Reduce codecs' RTCP feedback. Use Transport-CC if available, REMB otherwise.
            auto headerExtensionIt =
                    std::find_if(headerExtensionsIt->begin(), headerExtensionsIt->end(), [](json & ext) {
                        return ext["uri"].get<std::string>() ==
                                "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01";
                    });

            if (headerExtensionIt != headerExtensionsIt->end()) {
                for (auto& codec : rtpParameters["codecs"]) {
                    auto& rtcpFeedback = codec["rtcpFeedback"];

                    for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();) {
                        auto& fb = *it;
                        auto type = fb["type"].get<std::string>();

                        if (type == "goog-remb")
                            it = rtcpFeedback.erase(it);
                        else
                            ++it;
                    }
                }

                return rtpParameters;
            }

            headerExtensionIt =
                    std::find_if(headerExtensionsIt->begin(), headerExtensionsIt->end(), [](json & ext) {
                        return ext["uri"].get<std::string>() ==
                                "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";
                    });

            if (headerExtensionIt != headerExtensionsIt->end()) {
                for (auto& codec : rtpParameters["codecs"]) {
                    auto& rtcpFeedback = codec["rtcpFeedback"];

                    for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();) {
                        auto& fb = *it;
                        auto type = fb["type"].get<std::string>();

                        if (type == "transport-cc")
                            it = rtcpFeedback.erase(it);
                        else
                            ++it;
                    }
                }

                return rtpParameters;
            }

            for (auto& codec : rtpParameters["codecs"]) {
                auto& rtcpFeedback = codec["rtcpFeedback"];

                for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();) {
                    auto& fb = *it;
                    auto type = fb["type"].get<std::string>();

                    if (type == "transport-cc" || type == "goog-remb")
                        it = rtcpFeedback.erase(it);
                    else
                        ++it;
                }
            }

            return rtpParameters;
        }

        /**
         * Create RTP parameters for a Consumer for the RTP probator.
         */
        const json generateProbatorRtpParameters(const json& videoRtpParameters) {


            // This may throw.
            json validatedRtpParameters = videoRtpParameters;

            // This may throw.
            ortc::validateRtpParameters(validatedRtpParameters);

            // clang-format off
            json rtpParameters ={
                { "mid", nullptr},
                { "codecs", json::array()},
                { "headerExtensions", json::array()},
                { "encodings", json::array()},
                {
                    "rtcp",
                    {
                        { "cname", "probator"}
                    }
                }
            };
            // clang-format on

            rtpParameters["codecs"].push_back(validatedRtpParameters["codecs"][0]);

            for (auto& ext : validatedRtpParameters["headerExtensions"]) {
                // clang-format off
                if (
                        ext["uri"] == "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time" ||
                        ext["uri"] == "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
                        )
                    // clang-format on
                {
                    rtpParameters["headerExtensions"].push_back(ext);
                }
            }

            json encoding = json::object();

            encoding["ssrc"] = ProbatorSsrc;

            rtpParameters["encodings"].push_back(encoding);

            return rtpParameters;
        }

        /**
         * Whether media can be sent based on the given RTP capabilities.
         */
        bool canSend(const std::string& kind, const json& extendedRtpCapabilities) {


            auto& codecs = extendedRtpCapabilities["codecs"];
            auto codecIt = std::find_if(codecs.begin(), codecs.end(), [&kind](const json & codec) {
                return kind == codec["kind"].get<std::string>();
            });

            return codecIt != codecs.end();
        }

        /**
         * Whether the given RTP parameters can be received with the given RTP
         * capabilities.
         */
        bool canReceive(json& rtpParameters, const json& extendedRtpCapabilities) {


            // This may throw.
            validateRtpParameters(rtpParameters);

            if (rtpParameters["codecs"].empty())
                return false;

            auto& firstMediaCodec = rtpParameters["codecs"][0];
            auto& codecs = extendedRtpCapabilities["codecs"];
            auto codecIt =
                    std::find_if(codecs.begin(), codecs.end(), [&firstMediaCodec](const json & codec) {
                        return codec["remotePayloadType"] == firstMediaCodec["payloadType"];
                    });

            return codecIt != codecs.end();
        }
    } // namespace ortc
} // namespace SdpParse

// Private helpers used in this file.

static bool isRtxCodec(const json& codec) {


    static const std::regex RtxMimeTypeRegex(
            "^(audio|video)/rtx$", std::regex_constants::ECMAScript | std::regex_constants::icase);

    std::smatch match;
    auto mimeType = codec["mimeType"].get<std::string>();

    return std::regex_match(mimeType, match, RtxMimeTypeRegex);
}

static bool matchCodecs(json& aCodec, const json& bCodec, bool strict, bool modify) {


    auto aMimeTypeIt = aCodec.find("mimeType");
    auto bMimeTypeIt = bCodec.find("mimeType");
    auto aMimeType = aMimeTypeIt->get<std::string>();
    auto bMimeType = bMimeTypeIt->get<std::string>();

    std::transform(aMimeType.begin(), aMimeType.end(), aMimeType.begin(), ::tolower);
    std::transform(bMimeType.begin(), bMimeType.end(), bMimeType.begin(), ::tolower);

    if (aMimeType != bMimeType)
        return false;

    if (aCodec["clockRate"] != bCodec["clockRate"])
        return false;

  //  if (aCodec.contains("channels") != bCodec.contains("channels"))
  //      return false;

    if (aCodec.contains("channels") && bCodec.contains("channels") && aCodec["channels"] != bCodec["channels"])
        return false;

    // Match H264 parameters.
    if (aMimeType == "video/h264") {
        auto aPacketizationMode = getH264PacketizationMode(aCodec);
        auto bPacketizationMode = getH264PacketizationMode(bCodec);

        if (aPacketizationMode != bPacketizationMode)
            return false;

        // If strict matching check profile-level-id.  // arvind TBD
        if (strict) {
            
            
            			CodecParameterMap aParameters;
           			CodecParameterMap bParameters;
                                
                               // SInfo << "aCodec " << aCodec.dump(4);
                               // SInfo << "bCodec " << bCodec.dump(4);
                                            
            			aParameters["level-asymmetry-allowed"] = std::to_string(getH264LevelAssimetryAllowed(aCodec));
            			aParameters["packetization-mode"]      = std::to_string(aPacketizationMode);
            			aParameters["profile-level-id"]        = getH264ProfileLevelId(aCodec);
                                
                                bParameters["level-asymmetry-allowed"] = std::to_string(getH264LevelAssimetryAllowed(bCodec));
            			bParameters["packetization-mode"]      = std::to_string(bPacketizationMode);
            			bParameters["profile-level-id"]        = getH264ProfileLevelId(bCodec);
            
            			if (!IsSameH264Profile(aParameters, bParameters))
            				return false;
            
            			CodecParameterMap newParameters;
          
            			GenerateProfileLevelIdForAnswer(aParameters, bParameters, &newParameters);

            			if (modify)
            			{
            				auto profileLevelIdIt = newParameters.find("profile-level-id");
            
            				if (profileLevelIdIt != newParameters.end())
            					aCodec["parameters"]["profile-level-id"] = profileLevelIdIt->second;
           				else
            					aCodec["parameters"].erase("profile-level-id");
            			}
        }
    }        // Match VP9 parameters.
    else if (aMimeType == "video/vp9") {
        // If strict matching check profile-id.
        if (strict) {
            auto aProfileId = getVP9ProfileId(aCodec);
            auto bProfileId = getVP9ProfileId(bCodec);

            if (aProfileId != bProfileId)
                return false;
        }
    }

    return true;
}

static bool matchHeaderExtensions(const json& aExt, const json& bExt) {


    if (aExt["kind"] != bExt["kind"])
        return false;

    return aExt["uri"] == bExt["uri"];
}

static json reduceRtcpFeedback(const json& codecA, const json& codecB) {


    auto reducedRtcpFeedback = json::array();
    auto rtcpFeedbackAIt = codecA.find("rtcpFeedback");
    auto rtcpFeedbackBIt = codecB.find("rtcpFeedback");

    for (auto& aFb : *rtcpFeedbackAIt) {
        auto rtcpFeedbackIt =
                std::find_if(rtcpFeedbackBIt->begin(), rtcpFeedbackBIt->end(), [&aFb](const json & bFb) {
                    return (aFb["type"] == bFb["type"] && aFb["parameter"] == bFb["parameter"]);
                });

        if (rtcpFeedbackIt != rtcpFeedbackBIt->end())
            reducedRtcpFeedback.push_back(*rtcpFeedbackIt);
    }

    return reducedRtcpFeedback;
}

static uint8_t getH264PacketizationMode(const json& codec) {


    auto& parameters = codec["parameters"];
    auto packetizationModeIt = parameters.find("packetization-mode");

    // clang-format off
    if (
            packetizationModeIt == parameters.end() ||
            !packetizationModeIt->is_number_integer()
            )
        // clang-format on
    {
        return 0;
    }

    return packetizationModeIt->get<uint8_t>();
}

static uint8_t getH264LevelAssimetryAllowed(const json& codec) {


    auto& parameters = codec["parameters"];
    auto levelAssimetryAllowedIt = parameters.find("level-asymmetry-allowed");

    // clang-format off
    if (
            levelAssimetryAllowedIt == parameters.end() ||
            !levelAssimetryAllowedIt->is_number_integer()
            )
        // clang-format on
    {
        return 0;
    }

    return levelAssimetryAllowedIt->get<uint8_t>();
}

static std::string getH264ProfileLevelId(const json& codec) {


    auto& parameters = codec["parameters"];
    auto profileLevelIdIt = parameters.find("profile-level-id");

    if (profileLevelIdIt == parameters.end())
        return "";
    else if (profileLevelIdIt->is_number())
        return std::to_string(profileLevelIdIt->get<int32_t>());
    else
        return profileLevelIdIt->get<std::string>();
}

static std::string getVP9ProfileId(const json& codec) {


    auto& parameters = codec["parameters"];
    auto profileIdIt = parameters.find("profile-id");

    if (profileIdIt == parameters.end())
        return "0";

    if (profileIdIt->is_number())
        return std::to_string(profileIdIt->get<int32_t>());
    else
        return profileIdIt->get<std::string>();
}
