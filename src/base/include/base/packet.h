
#ifndef BASE_Packet_H
#define BASE_Packet_H

#include "base/logger.h"
#include <cstdint>
#include <cstring> // memcpy
#include <vector>
#include <memory>
namespace base {

struct Bitwise
{
    unsigned data; // storage integer

    Bitwise(unsigned flags = 0)
        : data(flags)
    {
    }

    virtual void reset() { data = 0; };
    virtual void set(unsigned flag)
    {
        if (!has(flag))
            data |= flag;
    }

    virtual void add(unsigned flag) { data |= flag; };
    virtual void remove(unsigned flag) { data &= ~flag; };
    virtual void toggle(unsigned flag) { data ^= flag; };
    virtual bool has(unsigned flag) const { return (data & flag) == flag; };
};
    
    
    
struct IPacket{
 virtual IPacket* clone() const = 0;
 

 
    IPacket( unsigned flags = 0)
        : flags(flags)
    {
    }

    IPacket(const IPacket& r)
        :  flags(r.flags)
    {
    }

    IPacket& operator=(const IPacket& r)
    {

        flags = r.flags;
        return *this;
    }
    
    
     virtual size_t size() const { return 0; };

    /// The packet data pointer for buffered packets.
    virtual char* data() const { return nullptr; }
    
    
        /// Read/parse to the packet from the given input buffer.
    /// The number of bytes read is returned.
    virtual size_t read(const std::string&) = 0;

    /// Copy/generate to the packet given output buffer.
    /// The number of bytes written can be obtained from the buffer.
    ///
    /// Todo: It may be prefferable to use our pod types here
    /// instead of buffer input, but the current codebase requires
    /// that the buffer be dynamically resizable for some protocols...
    ///
    virtual void write(std::vector<char>&) const = 0;
    
    
    Bitwise flags;
    virtual const char* className() const { return "IPacket"; }
};


/// Packet for sending bitwise flags along the packet stream.
class FlagPacket : public IPacket
{
public:
    FlagPacket(unsigned flags = 0)
        : IPacket(flags)
    {
    }

    virtual IPacket* clone() const override
    {
        return new FlagPacket(*this);
    }

    FlagPacket(const FlagPacket& that)
        : IPacket(that)
    {
    }

    virtual ~FlagPacket() = default;

    virtual size_t read(const std::string&) override { return true;}

    virtual void write(std::vector<char>&) const override {}

    virtual const char* className() const override { return "FlagPacket"; }
};

// RawPacket is the default data packet type which consists
/// of an optionally managed char pointer and a size value.
class RawPacket : public IPacket
{
public:
    RawPacket(char* data = nullptr, size_t size = 0, unsigned flags = 0)
        : IPacket( flags)
        , _data(data)
        , _size(size)
        , _free(false)
    {
    }

    RawPacket(const char* data, size_t size = 0, unsigned flags = 0)
        : IPacket( flags)
        , _data(nullptr)
        , _size(size)
        , _free(true)
    {
        copyData(data, size); // copy const data
    }

    RawPacket(const RawPacket& that)
        : IPacket(that)
        , _data(nullptr)
        , _size(0)
        , _free(true)
    {
        // Copy assigned data and set the free flag
        // Todo: Use a simple reference counted buffer wrapper
        // so we don't need to force memcpy here.
        copyData(that._data, that._size);
    }

    virtual ~RawPacket()
    {
        if (_data && _free)
        {
              LTrace("free Cloning");
            delete[] _data;
        }
    }

    virtual IPacket* clone() const override
    {
        return new RawPacket(*this);
    }


    virtual void copyData(const void* data, size_t size)
    {
          LTrace("RawPacket Cloning: ", size );
        // assert(_free);
        if (data && size > 0) {
            if (_data && _free)
                delete[] _data;
            _size = size;
            _data = new char[size];
            _free = true;
            LTrace(_data)
            std::memcpy(_data, data, size);
            LTrace(_data)
            
        }
    }

  
    virtual size_t read(const std::string& buf) override
    {
        copyData((const char*)(buf.c_str()), buf.size());
        return buf.size();
    }

    virtual void write(std::vector<char>& buf) const override
    {
        // buf.insert(a.end(), b.begin(), b.end());
        // buf.append(_data, _size);
        buf.insert(buf.end(), _data, _data + _size);
    }
    

    virtual char* data() const override { return _data; }

    // virtual char* cdata() const { return static_cast<char*>(_data); }

    virtual size_t size() const override { return _size; }

    virtual const char* className() const override { return "RawPacket"; }

    bool ownsBuffer() const { return _free; }

    void assignDataOwnership() { _free = true; }

protected:
    char* _data;
    size_t _size;
    bool _free;
};



struct MediaPacket : public RawPacket
{
    int64_t time; // microseconds

    MediaPacket(uint8_t* data = nullptr, size_t size = 0, int64_t time = 0)
        : RawPacket(reinterpret_cast<char*>(data), size)
        , time(time)
    {
    }

    MediaPacket(const MediaPacket& r)
        : RawPacket(r)
        , time(r.time)
    {
    }

    virtual ~MediaPacket() = default;

    virtual IPacket* clone() const override { 
       return new MediaPacket(*this); 
    }

    virtual const char* className() const override { return "MediaPacket"; }
};






} // namespace base


#endif // BASE_Packet_H



