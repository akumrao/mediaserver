


#ifndef BASE_AV_Packet_H
#define BASE_AV_Packet_H


#include "ff/ff.h"
//#include "base/packet.h"
#include "base/time.h"


namespace base {
namespace ff {

struct IPacket{
 virtual IPacket* clone() const = 0;
 
  uint8_t* _data;
    
    size_t _size;
    
    
    uint8_t* data()
    {
        return _data;
    }
    
    size_t size()
    {
        return _size;
    }
};

struct MediaPacket:IPacket 
{
    int64_t time; // microseconds

    MediaPacket(uint8_t* data = nullptr, size_t size = 0, int64_t time = 0):
         time(time)
    {
    }

    MediaPacket(const MediaPacket& r)
        : time(r.time)
    {
    }

    virtual ~MediaPacket() = default;

    virtual IPacket* clone() const override { return new MediaPacket(*this); }

    virtual const char* className() const { return "MediaPacket"; }
    
   
};


/// Video packet for interleaved formats
struct VideoPacket : public MediaPacket
{
    int width;
    int height;
    bool iframe;

    VideoPacket(uint8_t* data = nullptr, size_t size = 0,
                int width = 0, int height = 0, int64_t time = 0)
        : MediaPacket(data, size, time)
        , width(width)
        , height(height)
        , iframe(false)
    {
    }

    VideoPacket(const VideoPacket& r)
        : MediaPacket(r)
        , width(r.width)
        , height(r.height)
        , iframe(r.iframe)
    {
    }

    virtual ~VideoPacket() = default;

    virtual IPacket* clone() const override { return new VideoPacket(*this); }

    virtual const char* className() const override { return "VideoPacket"; }
};


/// Video packet for planar formats
struct PlanarVideoPacket : public VideoPacket
{
    uint8_t* buffer[4] = { nullptr };
    int linesize[4];
    std::string pixelFmt;

    PlanarVideoPacket(uint8_t* data[4], const int linesize[4], const std::string& pixelFmt = "",
                      int width = 0, int height = 0, int64_t time = 0);
    PlanarVideoPacket(const PlanarVideoPacket& r);
    virtual ~PlanarVideoPacket();

    virtual IPacket* clone() const { return new PlanarVideoPacket(*this); }

    virtual const char* className() const { return "PlanarVideoPacket"; }
};


/// Audio packet for interleaved formats
struct AudioPacket : public MediaPacket
{
    size_t numSamples; // number of samples per channel

    AudioPacket(uint8_t* data = nullptr, size_t size = 0,
                size_t numSamples = 0,
                int64_t time = 0)
        : MediaPacket(data, size, time)
        , numSamples(numSamples)
    {
    }

    virtual ~AudioPacket() = default;

   virtual IPacket* clone() const override { return new AudioPacket(*this); }

    virtual uint8_t* samples() const
    {
        return reinterpret_cast<uint8_t*>(_data);
    }

    virtual const char* className() const  override{ return "AudioPacket"; }
};


/// Audio packet for planar formats
struct PlanarAudioPacket : public AudioPacket
{
    uint8_t* buffer[4] = { nullptr };
    int linesize;
    int channels;
    std::string sampleFmt;

    PlanarAudioPacket(uint8_t* data[4], int channels = 0, size_t numSamples = 0, //, size_t size = 0
                      const std::string& sampleFmt = "", int64_t time = 0);
    PlanarAudioPacket(const PlanarAudioPacket& r);
    virtual ~PlanarAudioPacket();

   virtual IPacket* clone() const override { return new PlanarAudioPacket(*this); }

    virtual const char* className() const override { return "PlanarAudioPacket"; }
};


} // namespace ff
} // namespace base


#endif // AV_Packet_H



