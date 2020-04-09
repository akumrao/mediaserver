#define MS_CLASS "DepLibWebRTC"
// #define MS_LOG_DEV_LEVEL 3

#include "DepLibWebRTC.h"
#include "LoggerTag.h"
#include "system_wrappers/source/field_trial.h" // webrtc::field_trial

/* Static. */

// TODO: Look for a proper value.
static const char FieldTrials[]{ "WebRTC-Pacer-MinPacketLimitMs/Enabled,100/" };

/* Static methods. */

void DepLibWebRTC::ClassInit()
{
    STrace << "DepLibWebRTC::ClassInit()";
    webrtc::field_trial::InitFieldTrialsFromString(FieldTrials);
}

void DepLibWebRTC::ClassDestroy()
{
    STrace << "DepLibWebRTC::ClassDestroy()";
}
