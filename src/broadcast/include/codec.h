/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef codec_HEADER_GUARD
#define codec_HEADER_GUARD

/** Outdated.  We're using FFmpeg/libav parameters instead
 * 
 */
enum class MediaType {
    none,
    video,
    audio
};

/** Outdated.  We're using FFmpeg/libav parameters instead
 * 
 */
enum class Codec {
    none,
    h264,
    yuv,
    rgb,
    pcmu
};

/** Various H264 frame types
 * 
 * @ingroup frames_tag
 */
namespace H264SliceType {
static const unsigned none = 0;
static const unsigned sps = 7;
static const unsigned pps = 8;
static const unsigned idr = 5; // IDR picture. Do not confuse with IDR KEY Frame
static const unsigned nonidr = 1; // non idr slice
static const unsigned aud = 9; // aud delimeter
};

namespace H264SframeType {
static const unsigned none = 0;
static const unsigned i = 3;
static const unsigned p = 2;
static const unsigned b = 1;
};

struct H264Pars { ///< H264 parameters
    H264Pars()
        : slice_type(H264SliceType::none){};
    short unsigned slice_type;
    short unsigned frameType;
};
namespace H265SliceType {
static const unsigned none = 0;
static const unsigned BLA_W_LP = 16;
static const unsigned BLA_W_RADL = 17;
static const unsigned BLA_N_LP = 18;
static const unsigned IDR_W_RADL = 19; // IDR picture. Do not confuse with IDR KEY Frame
static const unsigned IDR_N_LP = 20; // non idr slice
static const unsigned CRA_NUT = 21;

static const unsigned VPS_NUT = 32;
static const unsigned SPS_NUT = 33;
static const unsigned PPS_NUT = 34;
static const unsigned AUD_NUT = 35;
static const unsigned EOS_NUT = 36;
static const unsigned EOB_NUT = 37;
static const unsigned FD_NUT = 38;
static const unsigned PREFIX_SEI_NUT = 39;
static const unsigned SUFFIX_SEI_NUT = 40;
};

//namespace H264SframeType {
//  static const unsigned none  =0;
//  static const unsigned i   =3;
//  static const unsigned p   =2;
//  static const unsigned b   =1;
//
//};

struct H265Pars { ///< H264 parameters
    H265Pars()
        : slice_type(H265SliceType::none){};
    short unsigned slice_type;
    ///short unsigned frameType;
};

inline std::ostream& operator<<(std::ostream& os, H265Pars const& m)
{
    return os << "H265:  slice_type=" << m.slice_type;
}

inline std::ostream& operator<<(std::ostream& os, H264Pars const& m)
{
    return os << "H264: frame_type=" << m.frameType << " slice_type=" << m.slice_type;
}

struct SetupPars { ///< Setup parameters for decoders and muxers (outdated)
    // AVCodecID codec_id; //https://ffmpeg.org/doxygen/3.0/group__lavc__core.html#gaadca229ad2c20e060a14fec08a5cc7ce
    SetupPars()
        : mediatype(MediaType::none)
        , codec(Codec::none){};
    MediaType mediatype;
    Codec codec;
};
inline std::ostream& operator<<(std::ostream& os, SetupPars const& m)
{
    return os << "Setup: mediatype=" << int(m.mediatype) << " codec=" << int(m.codec);
}

#endif
