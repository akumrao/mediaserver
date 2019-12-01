

#ifndef Buffer_H
#define Buffer_H


#include "base/base.h"
#include "base/byteorder.h"
#include <algorithm>
#include <ostream>
#include <cstdint>
#include <string>
#include <vector>


namespace base {

//
// Bit Reader
//
typedef std::vector<char> Buffer;

/// Class for reading binary streams.
class Base_API BitReader
{
public:
    BitReader(const char* bytes, size_t size, ByteOrder order = ByteOrder::Network);
    BitReader(const Buffer& buf, ByteOrder order = ByteOrder::Network);
    BitReader(const std::string & pod, ByteOrder order = ByteOrder::Network);
    ~BitReader();

    /// Reads a value from the BitReader.
    /// Returns false if there isn't enough data left for the specified type.
    /// Throws a std::out_of_range exception if reading past the limit.
    void get(char* val, size_t len);
    void get(std::string& val, size_t len);
    void getU8(uint8_t& val);
    void getU16(uint16_t& val);
    void getU24(uint32_t& val);
    void getU32(uint32_t& val);

    void getU64(uint64_t& val);

    const char peek();
    const uint8_t peekU8();
    const uint16_t peekU16();
    const uint32_t peekU24();
    const uint32_t peekU32();

    /// Peeks data from the BitReader.
    /// -1 is returned if reading past boundary.
    const uint64_t peekU64();

    //
    // String parsing methods.
    //

    size_t skipToChar(char c);
    size_t skipWhitespace();
    size_t skipToNextLine();
    size_t skipNextWord();
    size_t readNextWord(std::string& val);
    size_t readNextNumber(unsigned int& val);
    size_t readLine(std::string& val);
    size_t readToNext(std::string& val, char c);

    /// Set position pointer to absolute position.
    /// Throws a std::out_of_range exception if the value exceeds the limit.
    void seek(size_t val);

    /// Set position pointer to relative position.
    /// Throws a std::out_of_range exception if the value exceeds the limit.
    void skip(size_t size);

    /// Returns the read limit.
    size_t limit() const;

    /// Returns the current read position.
    size_t position() const { return _position; }

    /// Returns the number of elements between the current position and the
    /// limit.
    size_t available() const;

    const char* begin() const { return _bytes; }
    const char* current() const { return _bytes + _position; }

    ByteOrder order() const { return _order; }

    std::string toString();

    friend std::ostream& operator<<(std::ostream& stream, const BitReader& buf)
    {
        return stream.write(buf.current(), buf.position());
    }

protected:
    void init(const char* bytes, size_t size, ByteOrder order); // nocopy

    size_t _position;
    size_t _limit;
    ByteOrder _order;
    const char* _bytes;
};


//
// Bit Writer
//


/// Class for reading/writing binary streams.
///
/// Note that when using the constructor with the Buffer reference
/// as an argument, the writer will dynamically expand the given buffer
/// when writing passed the buffer capacity.
/// All other cases will throw a std::out_of_range error when writing
/// past the buffer capacity.
class Base_API BitWriter
{
public:
    BitWriter(char* bytes, size_t size, ByteOrder order = ByteOrder::Network);
    BitWriter(Buffer& buf, ByteOrder order = ByteOrder::Network);
    BitWriter(std::string& pod, ByteOrder order = ByteOrder::Network);
    virtual ~BitWriter();

    /// Append bytes to the buffer.
    /// Throws a `std::out_of_range` exception if reading past the limit.
    virtual void put(const char* val, size_t len);
    void put(const std::string& val);
    void putU8(uint8_t val);
    void putU16(uint16_t val);
    void putU24(uint32_t val);
    void putU32(uint32_t val);
    void putU64(uint64_t val);

    /// Update a byte range.
    /// Throws a `std::out_of_range` exception if reading past the limit.
    virtual bool update(const char* val, size_t len, size_t pos);
    bool update(const std::string& val, size_t pos);
    bool updateU8(uint8_t val, size_t pos);
    bool updateU16(uint16_t val, size_t pos);
    bool updateU24(uint32_t val, size_t pos);
    bool updateU32(uint32_t val, size_t pos);
    bool updateU64(uint64_t val, size_t pos);

    /// Set position pointer to absolute position.
    /// Throws a `std::out_of_range` exception if the value exceeds the limit.
    void seek(size_t val);

    /// Set position pointer to relative position.
    /// Throws a `std::out_of_range` exception if the value exceeds the limit.
    void skip(size_t size);

    /// Returns the write limit.
    size_t limit() const;

    /// Returns the current write position.
    size_t position() const { return _position; }

    /// Returns the number of elements between the current write position and
    /// the limit.
    size_t available() const;

    char* begin() { return _bytes; }
    char* current() { return _bytes + _position; }

    const char* begin() const { return _bytes; }
    const char* current() const { return _bytes + _position; }

    ByteOrder order() const { return _order; }

    /// Returns written bytes as a string.
    std::string toString();

    friend std::ostream& operator<<(std::ostream& stream, const BitWriter& wr)
    {
        return stream.write(wr.begin(), wr.position());
    }

protected:
    virtual void init(char* bytes, size_t size, ByteOrder order); // nocopy

    size_t _position;
    size_t _limit;
    ByteOrder _order;
    char* _bytes;
};


/// Class for reading/writing dynamically resizable binary streams.
///
/// Note that when using the constructor with the Buffer reference
/// as an argument, the writer will dynamically expand the given buffer
/// when writing passed the buffer capacity.
/// All other cases will throw a std::out_of_range error when writing
/// past the buffer capacity.
class Base_API DynamicBitWriter : public BitWriter
{
public:
    DynamicBitWriter(Buffer& buf, ByteOrder order = ByteOrder::Network);
    DynamicBitWriter(Buffer& buf, Buffer::iterator offset, ByteOrder order = ByteOrder::Network);
    virtual ~DynamicBitWriter();

    /// Append bytes to the buffer.
    /// Throws a `std::out_of_range` exception if reading past the limit.
    virtual void put(const char* val, size_t len);

    /// Update a byte range.
    /// Throws a `std::out_of_range` exception if reading past the limit.
    virtual bool update(const char* val, size_t len, size_t pos);

protected:
    Buffer& _buffer;
    size_t _offset;
};


} // namespace base


#endif // Buffer_H

