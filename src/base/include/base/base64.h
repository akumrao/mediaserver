/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */




#ifndef Base64_H
#define Base64_H



#include "base/interface.h"

#include <cstdint>
#include <iostream>
#include <memory>


namespace base {
namespace base64 {


const int BUFFER_SIZE = 16384;
const int LINE_LENGTH = 72;


//
// Base64 Encoder
//


namespace internal {


typedef enum { step_A, step_B, step_C } encodestep;

typedef struct
{
    encodestep step;
    char result;
    int stepcount;
    int linelength;        // added
    int nullptrlterminate; // added
} encodestate;

Base_API void init_encodestate(internal::encodestate* state_in);

Base_API char encode_value(char value_in);

Base_API ssize_t encode_block(const char* readbuf_in, size_t length_in, char* code_out,
                              internal::encodestate* state_in);

Base_API ssize_t encode_blockend(char* code_out, internal::encodestate* state_in);


} // namespace internal


/// Base64 encoder.
struct Encoder : public basic::Encoder
{
    Encoder(int buffersize = BUFFER_SIZE)
        : _buffersize(buffersize)
    {
        internal::init_encodestate(&_state);
    }

    void encode(std::istream& istrm, std::ostream& ostrm)
    {
        const int N = _buffersize;
        char* readbuf = new char[N];
        char* encbuf = new char[2 * N];
        ssize_t nread;
        ssize_t enclen;

        do {
            istrm.read(readbuf, N);
            nread = static_cast<int>(istrm.gcount());
            enclen = encode(readbuf, nread, encbuf);
            ostrm.write(encbuf, enclen);
        } while (istrm.good() && nread > 0);

        enclen = finalize(encbuf);
        ostrm.write(encbuf, enclen);

        internal::init_encodestate(&_state);

        delete[] encbuf;
        delete[] readbuf;
    }

    void encode(const std::string& in, std::string& out)
    {
        char* encbuf = new char[in.length() * 2];
        ssize_t enclen = encode(in.c_str(), in.length(), encbuf);
        out.append(encbuf, enclen);

        enclen = finalize(encbuf);
        out.append(encbuf, enclen);

        internal::init_encodestate(&_state);

        delete[] encbuf;
    }

    ssize_t encode(const char* inbuf, size_t nread, char* outbuf) override
    {
        return internal::encode_block(inbuf, nread, outbuf, &_state);
    }

    ssize_t finalize(char* outbuf) override
    {
        return internal::encode_blockend(outbuf, &_state);
    }

    void setLineLength(int lineLength) { _state.linelength = lineLength; }

    internal::encodestate _state;
    int _buffersize;
};


/// Convert an STL container to Base64.
template <typename T>
inline std::string encode(const T& bytes, int lineLength = LINE_LENGTH)
{
    std::string res;
    res.reserve(bytes.size() * 2);
    std::unique_ptr<char[]> encbuf(new char[bytes.size() * 2]);

    internal::encodestate state;
    internal::init_encodestate(&state);
    state.linelength = lineLength;

    ssize_t enclen = internal::encode_block(reinterpret_cast<const char*>(&bytes[0]),
                                            bytes.size(), encbuf.get(), &state);
    res.append(encbuf.get(), enclen);

    enclen = internal::encode_blockend(encbuf.get(), &state);
    res.append(encbuf.get(), enclen);
    return res;
}


//
// Base64 Decoder
//


namespace internal {


typedef enum { step_a, step_b, step_c, step_d } decodestep;

typedef struct
{
    decodestep step;
    char plainchar;
} decodestate;

Base_API void init_decodestate(internal::decodestate* state_in);

Base_API ssize_t decode_value(char value_in);

Base_API ssize_t decode_block(const char* inbuf, const size_t nread, char* outbuf,
                              internal::decodestate* state_in);


} // namespace internal


/// Base64 decoder.
struct Decoder : public basic::Decoder
{
    Decoder(int buffersize = BUFFER_SIZE)
        : _buffersize(buffersize)
    {
        internal::init_decodestate(&_state);
    }

    ssize_t decode(char value_in) { return internal::decode_value(value_in); }

    ssize_t decode(const char* inbuf, size_t nread, char* outbuf) override
    {
        return internal::decode_block(inbuf, nread, outbuf, &_state);
    }

    void decode(std::istream& istrm, std::ostream& ostrm)
    {
        const int N = _buffersize;
        char* decbuf = new char[N];
        char* readbuf = new char[N];
        size_t declen;
        size_t nread;

        do {
            istrm.read((char*)decbuf, N);
            declen = static_cast<int>(istrm.gcount());
            nread = decode(decbuf, declen, readbuf);
            ostrm.write((const char*)readbuf, nread);
        } while (istrm.good() && declen > 0);

        internal::init_decodestate(&_state);

        delete[] decbuf;
        delete[] readbuf;
    }

    internal::decodestate _state;
    int _buffersize;
};


/// Decode an STL container from Base64.
template <typename T>
inline std::string decode(const T& bytes)
{
    std::string res;
    res.reserve(bytes.size() * 2);
    std::unique_ptr<char[]> encbuf(new char[bytes.size() * 2]);

    internal::decodestate state;
    internal::init_decodestate(&state);

    size_t enclen = internal::decode_block(reinterpret_cast<const char*>(&bytes[0]),
                                           bytes.size(), encbuf.get(), &state);
    res.append(encbuf.get(), enclen);

    return res;
}


} // namespace base64
} // namespace base


#endif // Base64_H

