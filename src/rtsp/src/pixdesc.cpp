/*
 * pixel format descriptor
 * Copyright (c) 2009 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <string.h>
extern "C"  {
#include "avassert.h"
#include "avstring.h"
#include "common.h"
#include "pixfmt.h"
#include "pixdesc.h"
#include "internal_codec.h"
#include "intreadwrite.h"
//#include "version.h"

void av_read_image_line(uint16_t *dst,
                        const uint8_t *data[4], const int linesize[4],
                        const AVPixFmtDescriptor *desc,
                        int x, int y, int c, int w,
                        int read_pal_component)
{
    AVComponentDescriptor comp = desc->comp[c];
    int plane = comp.plane;
    int depth = comp.depth;
    int mask  = (1 << depth) - 1;
    int shift = comp.shift;
    int step  = comp.step;
    int flags = desc->flags;

    if (flags & AV_PIX_FMT_FLAG_BITSTREAM) {
        int skip = x * step + comp.offset;
        const uint8_t *p = data[plane] + y * linesize[plane] + (skip >> 3);
        int shift = 8 - depth - (skip & 7);

        while (w--) {
            int val = (*p >> shift) & mask;
            if (read_pal_component)
                val = data[1][4*val + c];
            shift -= step;
            p -= shift >> 3;
            shift &= 7;
            *dst++ = val;
        }
    } else {
        const uint8_t *p = data[plane] + y * linesize[plane] +
                           x * step + comp.offset;
        int is_8bit = shift + depth <= 8;

        if (is_8bit)
            p += !!(flags & AV_PIX_FMT_FLAG_BE);

        while (w--) {
            int val = is_8bit ? *p :
                flags & AV_PIX_FMT_FLAG_BE ? AV_RB16(p) : AV_RL16(p);
            val = (val >> shift) & mask;
            if (read_pal_component)
                val = data[1][4 * val + c];
            p += step;
            *dst++ = val;
        }
    }
}

void av_write_image_line(const uint16_t *src,
                         uint8_t *data[4], const int linesize[4],
                         const AVPixFmtDescriptor *desc,
                         int x, int y, int c, int w)
{
    AVComponentDescriptor comp = desc->comp[c];
    int plane = comp.plane;
    int depth = comp.depth;
    int step  = comp.step;
    int flags = desc->flags;

    if (flags & AV_PIX_FMT_FLAG_BITSTREAM) {
        int skip = x * step + comp.offset;
        uint8_t *p = data[plane] + y * linesize[plane] + (skip >> 3);
        int shift = 8 - depth - (skip & 7);

        while (w--) {
            *p |= *src++ << shift;
            shift -= step;
            p -= shift >> 3;
            shift &= 7;
        }
    } else {
        int shift = comp.shift;
        uint8_t *p = data[plane] + y * linesize[plane] +
                     x * step + comp.offset;

        if (shift + depth <= 8) {
            p += !!(flags & AV_PIX_FMT_FLAG_BE);
            while (w--) {
                *p |= (*src++ << shift);
                p += step;
            }
        } else {
            while (w--) {
                if (flags & AV_PIX_FMT_FLAG_BE) {
                    uint16_t val = AV_RB16(p) | (*src++ << shift);
                    AV_WB16(p, val);
                } else {
                    uint16_t val = AV_RL16(p) | (*src++ << shift);
                    AV_WL16(p, val);
                }
                p += step;
            }
        }
    }
}

#if FF_API_PLUS1_MINUS1
FF_DISABLE_DEPRECATION_WARNINGS
#endif
static const AVPixFmtDescriptor av_pix_fmt_descriptors[AV_PIX_FMT_NB] = {
    [AV_PIX_FMT_YUV420P] = {
        .name = "yuv420p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,        
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUYV422] = {
        .name = "yuyv422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = 0,
        .comp = {
            { 0, 2, 0, 0, 8, /*1, 7, 1*/ },        /* Y */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* U */
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* V */
        },
    },
    [AV_PIX_FMT_RGB24] = {
        .name = "rgb24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 3, 0, 0, 8, /*2, 7, 1*/ },        /* R */
            { 0, 3, 1, 0, 8, /*2, 7, 2*/ },        /* G */
            { 0, 3, 2, 0, 8, /*2, 7, 3*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_BGR24] = {
        .name = "bgr24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 3, 2, 0, 8, /*2, 7, 3*/ },        /* R */
            { 0, 3, 1, 0, 8, /*2, 7, 2*/ },        /* G */
            { 0, 3, 0, 0, 8, /*2, 7, 1*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_YUV422P] = {
        .name = "yuv422p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUV444P] = {
        .name = "yuv444p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUV410P] = {
        .name = "yuv410p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 2,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUV411P] = {
        .name = "yuv411p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },  
    [AV_PIX_FMT_GRAY8] = {
        .name = "gray",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PSEUDOPAL,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
        },        
        .alias = "gray8,y8",
    },
    [AV_PIX_FMT_MONOWHITE] = {
        .name = "monow",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BITSTREAM,
        .comp = {
            { 0, 1, 0, 0, 1, /*0, 0, 1*/ },        /* Y */
        },
        
    },
    [AV_PIX_FMT_MONOBLACK] = {
        .name = "monob",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BITSTREAM,
        .comp = {
            { 0, 1, 0, 7, 1, /*0, 0, 1*/ },        /* Y */
        },
        
    },
    [AV_PIX_FMT_PAL8] = {
        .name = "pal8",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PAL,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },
        },
        
    },
    [AV_PIX_FMT_YUVJ420P] = {
        .name = "yuvj420p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUVJ422P] = {
        .name = "yuvj422p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUVJ444P] = {
        .name = "yuvj444p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },
        
    },
#if FF_API_XVMC
    [AV_PIX_FMT_XVMC_MPEG2_MC] = {
        .name = "xvmcmc",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_XVMC_MPEG2_IDCT] = {
        .name = "xvmcidct",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#endif /* FF_API_XVMC */
    [AV_PIX_FMT_UYVY422] = {
        .name = "uyvy422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags         = 0,
        .comp = {
            { 0, 2, 1, 0, 8, /*1, 7, 2*/ },        /* Y */
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* U */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* V */
        },
    },
    [AV_PIX_FMT_UYYVYY411] = {
        .name = "uyyvyy411",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .flags         = 0,
        .comp = {
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* Y */
            { 0, 6, 0, 0, 8, /*5, 7, 1*/ },        /* U */
            { 0, 6, 3, 0, 8, /*5, 7, 4*/ },        /* V */
        },
    },
    [AV_PIX_FMT_BGR8] = {
        .name = "bgr8",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PSEUDOPAL,
        .comp = {
            { 0, 1, 0, 0, 3, /*0, 2, 1*/ },        /* R */
            { 0, 1, 0, 3, 3, /*0, 2, 1*/ },        /* G */
            { 0, 1, 0, 6, 2, /*0, 1, 1*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_BGR4] = {
        .name = "bgr4",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 4, 3, 0, 1, /*3, 0, 4*/ },        /* R */
            { 0, 4, 1, 0, 2, /*3, 1, 2*/ },        /* G */
            { 0, 4, 0, 0, 1, /*3, 0, 1*/ },        /* B */
        },
        
    },
    [AV_PIX_FMT_BGR4_BYTE] = {
        .name = "bgr4_byte",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PSEUDOPAL,
        .comp = {
            { 0, 1, 0, 0, 1, /*0, 0, 1*/ },        /* R */
            { 0, 1, 0, 1, 2, /*0, 1, 1*/ },        /* G */
            { 0, 1, 0, 3, 1, /*0, 0, 1*/ },        /* B */
        },
        
    },
    [AV_PIX_FMT_RGB8] = {
        .name = "rgb8",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PSEUDOPAL,
        .comp = {
            { 0, 1, 0, 6, 2, /*0, 1, 1*/ },        /* R */
            { 0, 1, 0, 3, 3, /*0, 2, 1*/ },        /* G */
            { 0, 1, 0, 0, 3, /*0, 2, 1*/ },        /* B */
        },
        
    },
    [AV_PIX_FMT_RGB4] = {
        .name = "rgb4",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 4, 0, 0, 1, /*3, 0, 1*/ },        /* R */
            { 0, 4, 1, 0, 2, /*3, 1, 2*/ },        /* G */
            { 0, 4, 3, 0, 1, /*3, 0, 4*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_RGB4_BYTE] = {
        .name = "rgb4_byte",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PSEUDOPAL,
        .comp = {
            { 0, 1, 0, 3, 1, /*0, 0, 1*/ },        /* R */
            { 0, 1, 0, 1, 2, /*0, 1, 1*/ },        /* G */
            { 0, 1, 0, 0, 1, /*0, 0, 1*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_NV12] = {
        .name = "nv12",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 2, 0, 0, 8, /*1, 7, 1*/ },        /* U */
            { 1, 2, 1, 0, 8, /*1, 7, 2*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_NV21] = {
        .name = "nv21",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 2, 1, 0, 8, /*1, 7, 2*/ },        /* U */
            { 1, 2, 0, 0, 8, /*1, 7, 1*/ },        /* V */
        },
        
    },
    [AV_PIX_FMT_ARGB] = {
        .name = "argb",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* R */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* G */
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* B */
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* A */
        },
        
    },
    [AV_PIX_FMT_RGBA] = {
        .name = "rgba",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* R */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* G */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* B */
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* A */
        },
        
    },
    [AV_PIX_FMT_ABGR] = {
        .name = "abgr",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* R */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* G */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* B */
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* A */
        },
        
    },
    [AV_PIX_FMT_BGRA] = {
        .name = "bgra",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* R */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* G */
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* B */
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* A */
        },       
    },

    [AV_PIX_FMT_GRAY16BE] = {
        .name = "gray16be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* Y */
        },
        
        .alias = "y16be",
    },
    [AV_PIX_FMT_GRAY16LE] = {
        .name = "gray16le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags         = 0,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* Y */
        },
        .alias = "y16le",
    },
    [AV_PIX_FMT_YUV440P] = {
        .name = "yuv440p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUVJ440P] = {
        .name = "yuvj440p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUVA420P] = {
        .name = "yuva420p",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
            { 3, 1, 0, 0, 8, /*0, 7, 1*/ },        /* A */
        },
    },
#if FF_API_VDPAU
    [AV_PIX_FMT_VDPAU_H264] = {
        .name = "vdpau_h264",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VDPAU_MPEG1] = {
        .name = "vdpau_mpeg1",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VDPAU_MPEG2] = {
        .name = "vdpau_mpeg2",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VDPAU_WMV3] = {
        .name = "vdpau_wmv3",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VDPAU_VC1] = {
        .name = "vdpau_vc1",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#endif
    [AV_PIX_FMT_RGB48BE] = {
        .name = "rgb48be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 6, 0, 0, 16, /*5, 15, 1*/ },       /* R */
            { 0, 6, 2, 0, 16, /*5, 15, 3*/ },       /* G */
            { 0, 6, 4, 0, 16, /*5, 15, 5*/ },       /* B */
        },
    },
    [AV_PIX_FMT_RGB48LE] = {
        .name = "rgb48le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 6, 0, 0, 16, /*5, 15, 1*/ },       /* R */
            { 0, 6, 2, 0, 16, /*5, 15, 3*/ },       /* G */
            { 0, 6, 4, 0, 16, /*5, 15, 5*/ },       /* B */
        },
    },
    [AV_PIX_FMT_RGB565BE] = {
        .name = "rgb565be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, -1, 3, 5, /*1, 4, 0*/ },        /* R */
            { 0, 2,  0, 5, 6, /*1, 5, 1*/ },        /* G */
            { 0, 2,  0, 0, 5, /*1, 4, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_RGB565LE] = {
        .name = "rgb565le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, 1, 3, 5, /*1, 4, 2*/ },        /* R */
            { 0, 2, 0, 5, 6, /*1, 5, 1*/ },        /* G */
            { 0, 2, 0, 0, 5, /*1, 4, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_RGB555BE] = {
        .name = "rgb555be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, -1, 2, 5, /*1, 4, 0*/ },        /* R */
            { 0, 2,  0, 5, 5, /*1, 4, 1*/ },        /* G */
            { 0, 2,  0, 0, 5, /*1, 4, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_RGB555LE] = {
        .name = "rgb555le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, 1, 2, 5, /*1, 4, 2*/ },        /* R */
            { 0, 2, 0, 5, 5, /*1, 4, 1*/ },        /* G */
            { 0, 2, 0, 0, 5, /*1, 4, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_BGR565BE] = {
        .name = "bgr565be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2,  0, 0, 5, /*1, 4, 1*/ },        /* R */
            { 0, 2,  0, 5, 6, /*1, 5, 1*/ },        /* G */
            { 0, 2, -1, 3, 5, /*1, 4, 0*/ },        /* B */
        },
    },
    [AV_PIX_FMT_BGR565LE] = {
        .name = "bgr565le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, 0, 0, 5, /*1, 4, 1*/ },        /* R */
            { 0, 2, 0, 5, 6, /*1, 5, 1*/ },        /* G */
            { 0, 2, 1, 3, 5, /*1, 4, 2*/ },        /* B */
        },
    },
    [AV_PIX_FMT_BGR555BE] = {
        .name = "bgr555be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2,  0, 0, 5, /*1, 4, 1*/ },       /* R */
            { 0, 2,  0, 5, 5, /*1, 4, 1*/ },       /* G */
            { 0, 2, -1, 2, 5, /*1, 4, 0*/ },       /* B */
        },
     },
    [AV_PIX_FMT_BGR555LE] = {
        .name = "bgr555le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, 0, 0, 5, /*1, 4, 1*/ },        /* R */
            { 0, 2, 0, 5, 5, /*1, 4, 1*/ },        /* G */
            { 0, 2, 1, 2, 5, /*1, 4, 2*/ },        /* B */
        },
    },
#if FF_API_VAAPI
   [AV_PIX_FMT_VAAPI_MOCO] = {
       .name = "vaapi_moco",
       .log2_chroma_w = 1,
       .log2_chroma_h = 1,
       .flags = AV_PIX_FMT_FLAG_HWACCEL,
   },
   [AV_PIX_FMT_VAAPI_IDCT] = {
       .name = "vaapi_idct",
       .log2_chroma_w = 1,
       .log2_chroma_h = 1,
       .flags = AV_PIX_FMT_FLAG_HWACCEL,
   },
   [AV_PIX_FMT_VAAPI_VLD] = {
       .name = "vaapi_vld",
       .log2_chroma_w = 1,
       .log2_chroma_h = 1,
       .flags = AV_PIX_FMT_FLAG_HWACCEL,
   },
#else
   [AV_PIX_FMT_VAAPI] = {
       .name = "vaapi",
       .nb_components = 0,
       .log2_chroma_w = 1,
       .log2_chroma_h = 1,
       .flags = AV_PIX_FMT_FLAG_HWACCEL,
       .comp = { { 0, }, { 0, }, { 0, }, },
   },
#endif
   [AV_PIX_FMT_YUV420P16LE] = {
       .name = "yuv420p16le",
       .nb_components = 3,
       .log2_chroma_w = 1,
       .log2_chroma_h = 1,
       .flags = AV_PIX_FMT_FLAG_PLANAR,
       .comp = {
           { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
           { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
           { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
       },
   },
   [AV_PIX_FMT_YUV420P16BE] = {
       .name = "yuv420p16be",
       .nb_components = 3,
       .log2_chroma_w = 1,
       .log2_chroma_h = 1,
       .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
       .comp = {
           { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
           { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
           { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
       },
   },
   [AV_PIX_FMT_YUV422P16LE] = {
       .name = "yuv422p16le",
       .nb_components = 3,
       .log2_chroma_w = 1,
       .log2_chroma_h = 0,
       .flags = AV_PIX_FMT_FLAG_PLANAR,
       .comp = {
           { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
           { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
           { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
       },
   },
   [AV_PIX_FMT_YUV422P16BE] = {
       .name = "yuv422p16be",
       .nb_components = 3,
       .log2_chroma_w = 1,
       .log2_chroma_h = 0,
       .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
       .comp = {
           { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
           { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
           { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
       },
   },
   [AV_PIX_FMT_YUV444P16LE] = {
       .name = "yuv444p16le",
       .nb_components = 3,
       .log2_chroma_w = 0,
       .log2_chroma_h = 0,
       .flags = AV_PIX_FMT_FLAG_PLANAR,
       .comp = {
           { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
           { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
           { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
       },
   },
    [AV_PIX_FMT_YUV444P16BE] = {
        .name = "yuv444p16be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
        },
    },
#if FF_API_VDPAU
    [AV_PIX_FMT_VDPAU_MPEG4] = {
        .name = "vdpau_mpeg4",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#endif
    [AV_PIX_FMT_DXVA2_VLD] = {
        .name = "dxva2_vld",
        .nb_components = 0,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
        .comp = { { 0, }, { 0, }, { 0, }, },
    },
    [AV_PIX_FMT_RGB444LE] = {
        .name = "rgb444le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, 1, 0, 4, /*1, 3, 2*/ },        /* R */
            { 0, 2, 0, 4, 4, /*1, 3, 1*/ },        /* G */
            { 0, 2, 0, 0, 4, /*1, 3, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_RGB444BE] = {
        .name = "rgb444be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, -1, 0, 4,  /*1, 3, 0*/ },        /* R */
            { 0, 2,  0, 4, 4,  /* 1, 3, 1*/ },        /* G */
            { 0, 2,  0, 0, 4,  /* 1, 3, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_BGR444LE] = {
        .name = "bgr444le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2, 0, 0, 4, /*1, 3, 1*/ },        /* R */
            { 0, 2, 0, 4, 4, /*1, 3, 1*/ },        /* G */
            { 0, 2, 1, 0, 4, /*1, 3, 2*/ },        /* B */
        }
    },
    [AV_PIX_FMT_BGR444BE] = {
        .name = "bgr444be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 2,  0, 0, 4, /*1, 3, 1*/ },       /* R */
            { 0, 2,  0, 4, 4, /*1, 3, 1*/ },       /* G */
            { 0, 2, -1, 0, 4, /*1, 3, 0*/ },       /* B */
        },
     },
    [AV_PIX_FMT_YA8] = {
        .name = "ya8",
        .nb_components = 2,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 8, /*1, 7, 1*/ },        /* Y */
            { 0, 2, 1, 0, 8, /*1, 7, 2*/ },        /* A */
        }, 
        .alias = "gray8a",
    },
    [AV_PIX_FMT_BGR48BE] = {
        .name = "bgr48be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 6, 4, 0, 16, /*5, 15, 5*/ },       /* R */
            { 0, 6, 2, 0, 16, /*5, 15, 3*/ },       /* G */
            { 0, 6, 0, 0, 16, /*5, 15, 1*/ },       /* B */
        },
    },
    [AV_PIX_FMT_BGR48LE] = {
        .name = "bgr48le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 6, 4, 0, 16, /*5, 15, 5*/ },       /* R */
            { 0, 6, 2, 0, 16, /*5, 15, 3*/ },       /* G */
            { 0, 6, 0, 0, 16, /*5, 15, 1*/ },       /* B */
        },
    },
    [AV_PIX_FMT_YUV420P9BE] = {
        .name = "yuv420p9be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV420P9LE] = {
        .name = "yuv420p9le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV420P10BE] = {
        .name = "yuv420p10be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV420P10LE] = {
        .name = "yuv420p10le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P10BE] = {
        .name = "yuv422p10be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P10LE] = {
        .name = "yuv422p10le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P9BE] = {
        .name = "yuv444p9be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P9LE] = {
        .name = "yuv444p9le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P10BE] = {
        .name = "yuv444p10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P10LE] = {
        .name = "yuv444p10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P9BE] = {
        .name = "yuv422p9be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P9LE] = {
        .name = "yuv422p9le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
        },
    },    
    [AV_PIX_FMT_VDA_VLD] = {
        .name = "vda_vld",
        .nb_components = 0,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
        .comp = { { 0, }, { 0, }, { 0, }, },
    },
    [AV_PIX_FMT_GBRP] = {
        .name = "gbrp",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* R */
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* G */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP9BE] = {
        .name = "gbrp9be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* R */
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* G */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP9LE] = {
        .name = "gbrp9le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* R */
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* G */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* B */
        },
    },    
    [AV_PIX_FMT_GBRP10BE] = {
        .name = "gbrp10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* R */
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* G */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP10LE] = {
        .name = "gbrp10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* R */
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* G */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP16BE] = {
        .name = "gbrp16be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },       /* R */
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* G */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },       /* B */
        },
    },
    [AV_PIX_FMT_GBRP16LE] = {
        .name = "gbrp16le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },       /* R */
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* G */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },       /* B */
        },
    },
    [AV_PIX_FMT_YUVA422P] = {
        .name = "yuva422p",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
            { 3, 1, 0, 0, 8, /*0, 7, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P] = {
        .name = "yuva444p",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
            { 3, 1, 0, 0, 8, /*0, 7, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA420P9BE] = {
        .name = "yuva420p9be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
            { 3, 2, 0, 0, 9, /*1, 8, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA420P9LE] = {
        .name = "yuva420p9le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
            { 3, 2, 0, 0, 9, /*1, 8, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA422P9BE] = {
        .name = "yuva422p9be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
            { 3, 2, 0, 0, 9, /*1, 8, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA422P9LE] = {
        .name = "yuva422p9le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
            { 3, 2, 0, 0, 9, /*1, 8, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P9BE] = {
        .name = "yuva444p9be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
            { 3, 2, 0, 0, 9, /*1, 8, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P9LE] = {
        .name = "yuva444p9le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 9, /*1, 8, 1*/ },        /* Y */
            { 1, 2, 0, 0, 9, /*1, 8, 1*/ },        /* U */
            { 2, 2, 0, 0, 9, /*1, 8, 1*/ },        /* V */
            { 3, 2, 0, 0, 9, /*1, 8, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA420P10BE] = {
        .name = "yuva420p10be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA420P10LE] = {
        .name = "yuva420p10le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA422P10BE] = {
        .name = "yuva422p10be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA422P10LE] = {
        .name = "yuva422p10le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P10BE] = {
        .name = "yuva444p10be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P10LE] = {
        .name = "yuva444p10le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA420P16BE] = {
        .name = "yuva420p16be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA420P16LE] = {
        .name = "yuva420p16le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA422P16BE] = {
        .name = "yuva422p16be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA422P16LE] = {
        .name = "yuva422p16le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P16BE] = {
        .name = "yuva444p16be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YUVA444P16LE] = {
        .name = "yuva444p16le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },        /* Y */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },        /* U */
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },        /* V */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_VDPAU] = {
        .name = "vdpau",
        .nb_components = 0,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_XYZ12LE] = {
        .name = "xyz12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = 0, 
        .comp = {
            { 0, 6, 0, 4, 12, /*5, 11, 1*/ },       /* X */
            { 0, 6, 2, 4, 12, /*5, 11, 3*/ },       /* Y */
            { 0, 6, 4, 4, 12, /*5, 11, 5*/ },       /* Z */
      },
      /*.flags = -- not used*/
    },
    [AV_PIX_FMT_XYZ12BE] = {
        .name = "xyz12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 6, 0, 4, 12, /*5, 11, 1*/ },       /* X */
            { 0, 6, 2, 4, 12, /*5, 11, 3*/ },       /* Y */
            { 0, 6, 4, 4, 12, /*5, 11, 5*/ },       /* Z */
       },
    },
    [AV_PIX_FMT_NV16] = {
        .name = "nv16",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 2, 0, 0, 8, /*1, 7, 1*/ },        /* U */
            { 1, 2, 1, 0, 8, /*1, 7, 2*/ },        /* V */
        },
    },
    [AV_PIX_FMT_NV20LE] = {
        .name = "nv20le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 4, 0, 0, 10, /*3, 9, 1*/ },        /* U */
            { 1, 4, 2, 0, 10, /*3, 9, 3*/ },        /* V */
        },
    },
    [AV_PIX_FMT_NV20BE] = {
        .name = "nv20be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 4, 0, 0, 10, /*3, 9, 1*/ },        /* U */
            { 1, 4, 2, 0, 10, /*3, 9, 3*/ },        /* V */
        }, 
    },
    [AV_PIX_FMT_RGBA64BE] = {
        .name = "rgba64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 8, 0, 0, 16, /*7, 15, 1*/ },       /* R */
            { 0, 8, 2, 0, 16, /*7, 15, 3*/ },       /* G */
            { 0, 8, 4, 0, 16, /*7, 15, 5*/ },       /* B */
            { 0, 8, 6, 0, 16, /*7, 15, 7*/ },       /* A */
        },
    },
    [AV_PIX_FMT_RGBA64LE] = {
        .name = "rgba64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 8, 0, 0, 16, /*7, 15, 1*/ },       /* R */
            { 0, 8, 2, 0, 16, /*7, 15, 3*/ },       /* G */
            { 0, 8, 4, 0, 16, /*7, 15, 5*/ },       /* B */
            { 0, 8, 6, 0, 16, /*7, 15, 7*/ },       /* A */
        },
    },
    [AV_PIX_FMT_BGRA64BE] = {
        .name = "bgra64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 8, 4, 0, 16, /*7, 15, 5*/ },       /* R */
            { 0, 8, 2, 0, 16, /*7, 15, 3*/ },       /* G */
            { 0, 8, 0, 0, 16, /*7, 15, 1*/ },       /* B */
            { 0, 8, 6, 0, 16, /*7, 15, 7*/ },       /* A */
        },
    },
    [AV_PIX_FMT_BGRA64LE] = {
        .name = "bgra64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 8, 4, 0, 16, /*7, 15, 5*/ },       /* R */
            { 0, 8, 2, 0, 16, /*7, 15, 3*/ },       /* G */
            { 0, 8, 0, 0, 16, /*7, 15, 1*/ },       /* B */
            { 0, 8, 6, 0, 16, /*7, 15, 7*/ },       /* A */
        },
    },
    [AV_PIX_FMT_YVYU422] = {
        .name = "yvyu422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = 0,
        .comp = {
            { 0, 2, 0, 0, 8, /*1, 7, 1*/ },        /* Y */
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* U */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* V */
        },
    },
    [AV_PIX_FMT_VDA] = {
        .name = "vda",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_YA16BE] = {
        .name = "ya16be",
        .nb_components = 2,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 4, 0, 0, 16, /*3, 15, 1*/ },        /* Y */
            { 0, 4, 2, 0, 16, /*3, 15, 3*/ },        /* A */
        },
    },
    [AV_PIX_FMT_YA16LE] = {
        .name = "ya16le",
        .nb_components = 2,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 4, 0, 0, 16, /*3, 15, 1*/ },        /* Y */
            { 0, 4, 2, 0, 16, /*3, 15, 3*/ },        /* A */
        }, 
    }, 
    [AV_PIX_FMT_GBRAP] = {
        .name = "gbrap",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* R */
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* G */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* B */
            { 3, 1, 0, 0, 8, /*0, 7, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_GBRAP16BE] = {
        .name = "gbrap16be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },       /* R */
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* G */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },       /* B */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },       /* A */
        },
    },
    [AV_PIX_FMT_GBRAP16LE] = {
        .name = "gbrap16le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 2, 0, 0, 16, /*1, 15, 1*/ },       /* R */
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* G */
            { 1, 2, 0, 0, 16, /*1, 15, 1*/ },       /* B */
            { 3, 2, 0, 0, 16, /*1, 15, 1*/ },       /* A */
        },
    },
    [AV_PIX_FMT_QSV] = {
        .name = "qsv",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_MMAL] = {
        .name = "mmal",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_D3D11VA_VLD] = {
        .name = "d3d11va_vld",
        .nb_components = 0,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
        .comp = { { 0, }, { 0, }, { 0, }, },
    },
    [AV_PIX_FMT_CUDA] = {
        .name = "cuda",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_CUDA+1] = {},
    [AV_PIX_FMT_CUDA+2] = {},
    [AV_PIX_FMT_CUDA+3] = {},
    [AV_PIX_FMT_CUDA+4] = {},
    [AV_PIX_FMT_CUDA+5] = {},
    [AV_PIX_FMT_CUDA+6] = {},
    [AV_PIX_FMT_CUDA+7] = {},
    [AV_PIX_FMT_CUDA+8] = {},
    [AV_PIX_FMT_CUDA+9] = {},
    [AV_PIX_FMT_CUDA+10] = {},
    [AV_PIX_FMT_CUDA+11] = {},
    [AV_PIX_FMT_CUDA+12] = {},
    [AV_PIX_FMT_CUDA+13] = {},
    [AV_PIX_FMT_CUDA+14] = {},
    [AV_PIX_FMT_CUDA+15] = {},
    [AV_PIX_FMT_CUDA+16] = {},
    [AV_PIX_FMT_CUDA+17] = {},    
    [AV_PIX_FMT_CUDA+18] = {},
    [AV_PIX_FMT_CUDA+19] = {},
    [AV_PIX_FMT_CUDA+20] = {},
    [AV_PIX_FMT_CUDA+21] = {},
    [AV_PIX_FMT_CUDA+22] = {},
    [AV_PIX_FMT_CUDA+23] = {},
    [AV_PIX_FMT_CUDA+24] = {},
    [AV_PIX_FMT_CUDA+25] = {},
    [AV_PIX_FMT_CUDA+26] = {},
    [AV_PIX_FMT_CUDA+27] = {},    
    [AV_PIX_FMT_CUDA+28] = {},
    [AV_PIX_FMT_CUDA+29] = {},
    [AV_PIX_FMT_CUDA+30] = {},
    [AV_PIX_FMT_CUDA+31] = {},
    [AV_PIX_FMT_CUDA+32] = {},
    [AV_PIX_FMT_CUDA+33] = {},
    [AV_PIX_FMT_CUDA+34] = {},
    [AV_PIX_FMT_CUDA+35] = {},
    [AV_PIX_FMT_CUDA+36] = {},
    [AV_PIX_FMT_CUDA+37] = {},    
    [AV_PIX_FMT_CUDA+38] = {},
    [AV_PIX_FMT_CUDA+39] = {},
    [AV_PIX_FMT_CUDA+40] = {},
    [AV_PIX_FMT_CUDA+41] = {},
    [AV_PIX_FMT_CUDA+42] = {},
    [AV_PIX_FMT_CUDA+43] = {},
    [AV_PIX_FMT_CUDA+44] = {},
    [AV_PIX_FMT_CUDA+45] = {},
    [AV_PIX_FMT_CUDA+46] = {},
    [AV_PIX_FMT_CUDA+47] = {},    
    [AV_PIX_FMT_CUDA+48] = {},
    [AV_PIX_FMT_CUDA+49] = {},
    [AV_PIX_FMT_CUDA+50] = {},
    [AV_PIX_FMT_CUDA+51] = {},
    [AV_PIX_FMT_CUDA+52] = {},
    [AV_PIX_FMT_CUDA+53] = {},
    [AV_PIX_FMT_CUDA+54] = {},
    [AV_PIX_FMT_CUDA+55] = {},
    [AV_PIX_FMT_CUDA+56] = {},
    [AV_PIX_FMT_CUDA+57] = {},    
    [AV_PIX_FMT_CUDA+58] = {},
    [AV_PIX_FMT_CUDA+59] = {},
    [AV_PIX_FMT_CUDA+60] = {},
    [AV_PIX_FMT_CUDA+61] = {},
    [AV_PIX_FMT_CUDA+62] = {},
    [AV_PIX_FMT_CUDA+63] = {},
    [AV_PIX_FMT_CUDA+64] = {},
    [AV_PIX_FMT_CUDA+65] = {},
    [AV_PIX_FMT_CUDA+66] = {},
    [AV_PIX_FMT_CUDA+67] = {},    
    [AV_PIX_FMT_CUDA+68] = {},
    [AV_PIX_FMT_CUDA+69] = {},
    [AV_PIX_FMT_CUDA+70] = {},
    [AV_PIX_FMT_CUDA+71] = {},
    [AV_PIX_FMT_CUDA+72] = {},
    [AV_PIX_FMT_CUDA+73] = {},
    [AV_PIX_FMT_CUDA+74] = {},
    [AV_PIX_FMT_CUDA+75] = {},
    [AV_PIX_FMT_CUDA+76] = {},
    [AV_PIX_FMT_CUDA+77] = {},    
    [AV_PIX_FMT_CUDA+78] = {},
    [AV_PIX_FMT_CUDA+79] = {},
    [AV_PIX_FMT_CUDA+80] = {},
    [AV_PIX_FMT_CUDA+81] = {},
    [AV_PIX_FMT_CUDA+82] = {},
    [AV_PIX_FMT_CUDA+83] = {},
    [AV_PIX_FMT_CUDA+84] = {},
    [AV_PIX_FMT_CUDA+85] = {},
    [AV_PIX_FMT_CUDA+86] = {},
    [AV_PIX_FMT_CUDA+87] = {},    
    [AV_PIX_FMT_CUDA+88] = {},
    [AV_PIX_FMT_CUDA+89] = {},
    [AV_PIX_FMT_CUDA+90] = {},
    [AV_PIX_FMT_CUDA+91] = {},
    [AV_PIX_FMT_CUDA+92] = {},
    [AV_PIX_FMT_CUDA+93] = {},
    [AV_PIX_FMT_CUDA+94] = {},
    [AV_PIX_FMT_CUDA+95] = {},
    [AV_PIX_FMT_CUDA+96] = {},
    [AV_PIX_FMT_CUDA+97] = {},    
    [AV_PIX_FMT_CUDA+98] = {},
    [AV_PIX_FMT_CUDA+99] = {},
    [AV_PIX_FMT_CUDA+100] = {},
    [AV_PIX_FMT_CUDA+101] = {},
    [AV_PIX_FMT_CUDA+102] = {},
    [AV_PIX_FMT_CUDA+103] = {},
    [AV_PIX_FMT_CUDA+104] = {},
    [AV_PIX_FMT_CUDA+105] = {},
    [AV_PIX_FMT_CUDA+106] = {},
    [AV_PIX_FMT_CUDA+107] = {},    
    [AV_PIX_FMT_CUDA+108] = {},
    [AV_PIX_FMT_CUDA+109] = {},
    [AV_PIX_FMT_CUDA+110] = {},
    [AV_PIX_FMT_CUDA+111] = {},
    [AV_PIX_FMT_CUDA+112] = {},
    [AV_PIX_FMT_CUDA+113] = {},
    [AV_PIX_FMT_CUDA+114] = {},
    [AV_PIX_FMT_CUDA+115] = {},
    [AV_PIX_FMT_CUDA+116] = {},
    [AV_PIX_FMT_CUDA+117] = {},    
    [AV_PIX_FMT_CUDA+118] = {},
    [AV_PIX_FMT_CUDA+119] = {},
    [AV_PIX_FMT_CUDA+120] = {},
    [AV_PIX_FMT_CUDA+121] = {},
    [AV_PIX_FMT_CUDA+122] = {},
    [AV_PIX_FMT_CUDA+123] = {},
    [AV_PIX_FMT_CUDA+124] = {},
    [AV_PIX_FMT_CUDA+125] = {},
    [AV_PIX_FMT_CUDA+126] = {},
    [AV_PIX_FMT_CUDA+127] = {},    
    [AV_PIX_FMT_CUDA+128] = {},
    [AV_PIX_FMT_CUDA+129] = {},
    [AV_PIX_FMT_CUDA+130] = {},
    [AV_PIX_FMT_CUDA+131] = {},
    [AV_PIX_FMT_CUDA+132] = {},
    [AV_PIX_FMT_CUDA+133] = {},
    [AV_PIX_FMT_CUDA+134] = {},
    [AV_PIX_FMT_CUDA+135] = {},
    [AV_PIX_FMT_CUDA+136] = {},
    [AV_PIX_FMT_CUDA+137] = {},    
    [AV_PIX_FMT_CUDA+138] = {},
    [AV_PIX_FMT_CUDA+139] = {},
    [AV_PIX_FMT_CUDA+140] = {},
    [AV_PIX_FMT_CUDA+141] = {},
    [AV_PIX_FMT_CUDA+142] = {},
    [AV_PIX_FMT_CUDA+143] = {},
    [AV_PIX_FMT_CUDA+144] = {},
    [AV_PIX_FMT_CUDA+145] = {},
    [AV_PIX_FMT_CUDA+146] = {},
    [AV_PIX_FMT_CUDA+147] = {},    
    [AV_PIX_FMT_CUDA+148] = {},
    [AV_PIX_FMT_CUDA+149] = {},
    [AV_PIX_FMT_CUDA+150] = {},
    [AV_PIX_FMT_CUDA+151] = {},
    [AV_PIX_FMT_CUDA+152] = {},
    [AV_PIX_FMT_CUDA+153] = {},
    [AV_PIX_FMT_CUDA+154] = {},
    [AV_PIX_FMT_CUDA+155] = {},
    [AV_PIX_FMT_CUDA+156] = {},
    [AV_PIX_FMT_CUDA+157] = {},    
    [AV_PIX_FMT_CUDA+158] = {},
    [AV_PIX_FMT_CUDA+159] = {},
    [AV_PIX_FMT_CUDA+160] = {},
    [AV_PIX_FMT_CUDA+161] = {},
    [AV_PIX_FMT_CUDA+162] = {},
    [AV_PIX_FMT_CUDA+163] = {},
    [AV_PIX_FMT_CUDA+164] = {},
    [AV_PIX_FMT_CUDA+165] = {},
    [AV_PIX_FMT_CUDA+166] = {},
    [AV_PIX_FMT_CUDA+167] = {},    
    [AV_PIX_FMT_CUDA+168] = {},
    [AV_PIX_FMT_CUDA+169] = {},
    [AV_PIX_FMT_CUDA+170] = {},
    [AV_PIX_FMT_CUDA+171] = {},
    [AV_PIX_FMT_CUDA+172] = {},
    [AV_PIX_FMT_CUDA+173] = {},
    [AV_PIX_FMT_CUDA+174] = {},
    [AV_PIX_FMT_CUDA+175] = {},
	
    [AV_PIX_FMT_0RGB] = {
        .name = "0rgb",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* R */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* G */
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_RGB0] = {
        .name = "rgb0",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* R */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* G */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_0BGR] = {
        .name = "0bgr",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 4, 3, 0, 8, /*3, 7, 4*/ },        /* R */
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* G */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_BGR0] = {
        .name = "bgr0",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .flags = AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 0, 4, 2, 0, 8, /*3, 7, 3*/ },        /* R */
            { 0, 4, 1, 0, 8, /*3, 7, 2*/ },        /* G */
            { 0, 4, 0, 0, 8, /*3, 7, 1*/ },        /* B */
        },        
    },
    [AV_PIX_FMT_YUV420P12BE] = {
        .name = "yuv420p12be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV420P12LE] = {
        .name = "yuv420p12le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV420P14BE] = {
        .name = "yuv420p14be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* Y */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* U */
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV420P14LE] = {
        .name = "yuv420p14le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* Y */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* U */
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P12BE] = {
        .name = "yuv422p12be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P12LE] = {
        .name = "yuv422p12le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P14BE] = {
        .name = "yuv422p14be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* Y */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* U */
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV422P14LE] = {
        .name = "yuv422p14le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* Y */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* U */
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P12BE] = {
        .name = "yuv444p12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P12LE] = {
        .name = "yuv444p12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P14BE] = {
        .name = "yuv444p14be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* Y */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* U */
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV444P14LE] = {
        .name = "yuv444p14le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* Y */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* U */
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_GBRP12BE] = {
        .name = "gbrp12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* R */
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* G */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP12LE] = {
        .name = "gbrp12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* R */
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* G */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP14BE] = {
        .name = "gbrp14be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* R */
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* G */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* B */
        },
    },
    [AV_PIX_FMT_GBRP14LE] = {
        .name = "gbrp14le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
        .comp = {
            { 2, 2, 0, 0, 14, /*1, 13, 1*/ },        /* R */
            { 0, 2, 0, 0, 14, /*1, 13, 1*/ },        /* G */
            { 1, 2, 0, 0, 14, /*1, 13, 1*/ },        /* B */
        },
    },
   [AV_PIX_FMT_YUVJ411P] = {
        .name = "yuvj411p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 1, 0, 0, 8, /*0, 7, 1*/ },        /* Y */
            { 1, 1, 0, 0, 8, /*0, 7, 1*/ },        /* U */
            { 2, 1, 0, 0, 8, /*0, 7, 1*/ },        /* V */
        },
    },
#define BAYER8_DESC_COMMON \
        .nb_components= 3, \
        .log2_chroma_w= 0, \
        .log2_chroma_h= 0,
        
#define BAYER8_DESC_COMMON_COMP \
        .comp = {          \
            {0,1,0,0,2,/*0,1,1*/},\
            {0,1,0,0,4,/*0,3,1*/},\
            {0,1,0,0,2,/*0,1,1*/},\
        },
 
#define BAYER16_DESC_COMMON \
        .nb_components= 3, \
        .log2_chroma_w= 0, \
        .log2_chroma_h= 0, 

#define BAYER16_DESC_COMMON_COMP  \
        .comp = {                 \
            {0,2,0,0,4,/*1,3,1*/},\
            {0,2,0,0,8,/*1,7,1*/},\
            {0,2,0,0,4,/*1,3,1*/},\
        },

    [AV_PIX_FMT_BAYER_BGGR8] = {
        .name = "bayer_bggr8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER8_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_RGGB8] = {
        .name = "bayer_rggb8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER8_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_GBRG8] = {
        .name = "bayer_gbrg8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER8_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_GRBG8] = {
        .name = "bayer_grbg8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER8_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_BGGR16LE] = {
        .name = "bayer_bggr16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_BGGR16BE] = {
        .name = "bayer_bggr16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_RGGB16LE] = {
        .name = "bayer_rggb16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_RGGB16BE] = {
        .name = "bayer_rggb16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_GBRG16LE] = {
        .name = "bayer_gbrg16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_GBRG16BE] = {
        .name = "bayer_gbrg16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_GRBG16LE] = {
        .name = "bayer_grbg16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
    [AV_PIX_FMT_BAYER_GRBG16BE] = {
        .name = "bayer_grbg16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
        BAYER16_DESC_COMMON_COMP
    },
#if !FF_API_XVMC
    [AV_PIX_FMT_XVMC] = {
        .name = "xvmc",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,        
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#endif /* !FF_API_XVMC */
    [AV_PIX_FMT_YUV440P10LE] = {
        .name = "yuv440p10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },        
    },
    [AV_PIX_FMT_YUV440P10BE] = {
        .name = "yuv440p10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },        /* U */
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV440P12LE] = {
        .name = "yuv440p12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_YUV440P12BE] = {
        .name = "yuv440p12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },        /* Y */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },        /* U */
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },        /* V */
        },
    },
    [AV_PIX_FMT_AYUV64LE] = {
        .name = "ayuv64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 8, 2, 0, 16, /*7, 15, 3*/ },        /* Y */
            { 0, 8, 4, 0, 16, /*7, 15, 5*/ },        /* U */
            { 0, 8, 6, 0, 16, /*7, 15, 7*/ },        /* V */
            { 0, 8, 0, 0, 16, /*7, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_AYUV64BE] = {
        .name = "ayuv64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 0, 8, 2, 0, 16, /*7, 15, 3*/ },        /* Y */
            { 0, 8, 4, 0, 16, /*7, 15, 5*/ },        /* U */
            { 0, 8, 6, 0, 16, /*7, 15, 7*/ },        /* V */
            { 0, 8, 0, 0, 16, /*7, 15, 1*/ },        /* A */
        },
    },
    [AV_PIX_FMT_VIDEOTOOLBOX] = {
        .name = "videotoolbox_vld",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_P010LE] = {
        .name = "p010le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 6, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 4, 0, 6, 10, /*3, 9, 1*/ },        /* U */
            { 1, 4, 2, 6, 10, /*3, 9, 3*/ },        /* V */
        },
    },
    [AV_PIX_FMT_P010BE] = {
        .name = "p010be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 2, 0, 6, 10, /*1, 9, 1*/ },        /* Y */
            { 1, 4, 0, 6, 10, /*3, 9, 1*/ },        /* U */
            { 1, 4, 2, 6, 10, /*3, 9, 3*/ },        /* V */
        }, 
    },
    [AV_PIX_FMT_GBRAP12BE] = {
        .name = "gbrap12be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 2, 0, 0, 12, /*1, 11, 1*/ },       /* R */
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },       /* G */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },       /* B */
            { 3, 2, 0, 0, 12, /*1, 11, 1*/ },       /* A */
        },
    },
    [AV_PIX_FMT_GBRAP12LE] = {
        .name = "gbrap12le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 2, 0, 0, 12, /*, 11, 1*/ },       /* R */
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },       /* G */
            { 1, 2, 0, 0, 12, /*1, 11, 1*/ },       /* B */
            { 3, 2, 0, 0, 12, /*1, 11, 1*/ },       /* A */
        },
    },
    [AV_PIX_FMT_GBRAP10BE] = {
        .name = "gbrap10be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },       /* R */
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },       /* G */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },       /* B */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },       /* A */
        },
    },
    [AV_PIX_FMT_GBRAP10LE] = {
        .name = "gbrap10le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
        .comp = {
            { 2, 2, 0, 0, 10, /*1, 9, 1*/ },       /* R */
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },       /* G */
            { 1, 2, 0, 0, 10, /*1, 9, 1*/ },       /* B */
            { 3, 2, 0, 0, 10, /*1, 9, 1*/ },       /* A */
        },
    },
    [AV_PIX_FMT_MEDIACODEC] = {
        .name = "mediacodec",
        .nb_components = 0,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GRAY12BE] = {
        .name = "gray12be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },       /* Y */
        },        
        .alias = "y12be",
    },
    [AV_PIX_FMT_GRAY12LE] = {
        .name = "gray12le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags         = 0, 
        .comp = {
            { 0, 2, 0, 0, 12, /*1, 11, 1*/ },       /* Y */
        },
        .alias = "y12le",
    },
    [AV_PIX_FMT_GRAY10BE] = {
        .name = "gray10be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags = AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },       /* Y */
        },        
        .alias = "y10be",
    },
    [AV_PIX_FMT_GRAY10LE] = {
        .name = "gray10le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .flags         = 0, 
        .comp = {
            { 0, 2, 0, 0, 10, /*1, 9, 1*/ },       /* Y */
        },
        .alias = "y10le",
    },
    [AV_PIX_FMT_P016LE] = {
        .name = "p016le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/},       /* Y */
            { 1, 4, 0, 0, 16, /*3, 15, 1*/ },       /* U */
            { 1, 4, 2, 0, 16, /*3, 15, 3*/ },       /* V */
        },
    },
    [AV_PIX_FMT_P016BE] = {
        .name = "p016be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
        .comp = {
            { 0, 2, 0, 0, 16, /*1, 15, 1*/ },       /* Y */
            { 1, 4, 0, 0, 16, /*3, 15, 1*/ },       /* U */
            { 1, 4, 2, 0, 16, /*3, 15, 3*/ },       /* V */
        },
    },
};
#if FF_API_PLUS1_MINUS1
FF_ENABLE_DEPRECATION_WARNINGS
#endif

static const char *color_range_names[] = {
    [AVCOL_RANGE_UNSPECIFIED] = "unknown",
    [AVCOL_RANGE_MPEG] = "tv",
    [AVCOL_RANGE_JPEG] = "pc",
};

static const char *color_primaries_names[AVCOL_PRI_NB] = {
    [AVCOL_PRI_RESERVED0] = "reserved",
    [AVCOL_PRI_BT709] = "bt709",
    [AVCOL_PRI_UNSPECIFIED] = "unknown",
    [AVCOL_PRI_RESERVED] = "reserved",
    [AVCOL_PRI_BT470M] = "bt470m",
    [AVCOL_PRI_BT470BG] = "bt470bg",
    [AVCOL_PRI_SMPTE170M] = "smpte170m",
    [AVCOL_PRI_SMPTE240M] = "smpte240m",
    [AVCOL_PRI_FILM] = "film",
    [AVCOL_PRI_BT2020] = "bt2020",
    [AVCOL_PRI_SMPTE428] = "smpte428",
    [AVCOL_PRI_SMPTE431] = "smpte431",
    [AVCOL_PRI_SMPTE432] = "smpte432",
    [13] = NULL,
    [14] = NULL,
    [15] = NULL,
    [16] = NULL,
    [17] = NULL,
    [18] = NULL,
    [19] = NULL,
    [20] = NULL,
    [21] = NULL,
    [AVCOL_PRI_JEDEC_P22] = "jedec-p22",
};

static const char *color_transfer_names[] = {
    [AVCOL_TRC_RESERVED0] = "reserved",
    [AVCOL_TRC_BT709] = "bt709",
    [AVCOL_TRC_UNSPECIFIED] = "unknown",
    [AVCOL_TRC_RESERVED] = "reserved",
    [AVCOL_TRC_GAMMA22] = "bt470m",
    [AVCOL_TRC_GAMMA28] = "bt470bg",
    [AVCOL_TRC_SMPTE170M] = "smpte170m",
    [AVCOL_TRC_SMPTE240M] = "smpte240m",
    [AVCOL_TRC_LINEAR] = "linear",
    [AVCOL_TRC_LOG] = "log100",
    [AVCOL_TRC_LOG_SQRT] = "log316",
    [AVCOL_TRC_IEC61966_2_4] = "iec61966-2-4",
    [AVCOL_TRC_BT1361_ECG] = "bt1361e",
    [AVCOL_TRC_IEC61966_2_1] = "iec61966-2-1",
    [AVCOL_TRC_BT2020_10] = "bt2020-10",
    [AVCOL_TRC_BT2020_12] = "bt2020-12",
    [AVCOL_TRC_SMPTE2084] = "smpte2084",
    [AVCOL_TRC_SMPTE428] = "smpte428",
    [AVCOL_TRC_ARIB_STD_B67] = "arib-std-b67",
};

static const char *color_space_names[] = {
    [AVCOL_SPC_RGB] = "gbr",
    [AVCOL_SPC_BT709] = "bt709",
    [AVCOL_SPC_UNSPECIFIED] = "unknown",
    [AVCOL_SPC_RESERVED] = "reserved",
    [AVCOL_SPC_FCC] = "fcc",
    [AVCOL_SPC_BT470BG] = "bt470bg",
    [AVCOL_SPC_SMPTE170M] = "smpte170m",
    [AVCOL_SPC_SMPTE240M] = "smpte240m",
    [AVCOL_SPC_YCGCO] = "ycgco",
    [AVCOL_SPC_BT2020_NCL] = "bt2020nc",
    [AVCOL_SPC_BT2020_CL] = "bt2020c",
    [AVCOL_SPC_SMPTE2085] = "smpte2085",
};

static const char *chroma_location_names[] = {
    [AVCHROMA_LOC_UNSPECIFIED] = "unspecified",
    [AVCHROMA_LOC_LEFT] = "left",
    [AVCHROMA_LOC_CENTER] = "center",
    [AVCHROMA_LOC_TOPLEFT] = "topleft",
    [AVCHROMA_LOC_TOP] = "top",
    [AVCHROMA_LOC_BOTTOMLEFT] = "bottomleft",
    [AVCHROMA_LOC_BOTTOM] = "bottom",
};

static enum AVPixelFormat get_pix_fmt_internal(const char *name)
{
    enum AVPixelFormat pix_fmt;

    for (pix_fmt = (AVPixelFormat)0; pix_fmt < AV_PIX_FMT_NB; pix_fmt=(AVPixelFormat)(pix_fmt+1))
        if (av_pix_fmt_descriptors[pix_fmt].name &&
            (!strcmp(av_pix_fmt_descriptors[pix_fmt].name, name) ||
             av_match_name(name, av_pix_fmt_descriptors[pix_fmt].alias)))
            return pix_fmt;

    return AV_PIX_FMT_NONE;
}

const char *av_get_pix_fmt_name(enum AVPixelFormat pix_fmt)
{
    return (unsigned)pix_fmt < AV_PIX_FMT_NB ?
        av_pix_fmt_descriptors[pix_fmt].name : NULL;
}

#if HAVE_BIGENDIAN
#   define X_NE(be, le) be
#else
#   define X_NE(be, le) le
#endif

enum AVPixelFormat av_get_pix_fmt(const char *name)
{
    enum AVPixelFormat pix_fmt;

    if (!strcmp(name, "rgb32"))
        name = X_NE("argb", "bgra");
    else if (!strcmp(name, "bgr32"))
        name = X_NE("abgr", "rgba");

    pix_fmt = get_pix_fmt_internal(name);
    if (pix_fmt == AV_PIX_FMT_NONE) {
        char name2[32];

        snprintf(name2, sizeof(name2), "%s%s", name, X_NE("be", "le"));
        pix_fmt = get_pix_fmt_internal(name2);
    }

#if FF_API_VAAPI
    if (pix_fmt == AV_PIX_FMT_NONE && !strcmp(name, "vaapi"))
        pix_fmt = AV_PIX_FMT_VAAPI;
#endif
    return pix_fmt;
}

int av_get_bits_per_pixel(const AVPixFmtDescriptor *pixdesc)
{
    int c, bits = 0;
    int log2_pixels = pixdesc->log2_chroma_w + pixdesc->log2_chroma_h;

    for (c = 0; c < pixdesc->nb_components; c++) {
        int s = c == 1 || c == 2 ? 0 : log2_pixels;
        bits += pixdesc->comp[c].depth << s;
    }

    return bits >> log2_pixels;
}

int av_get_padded_bits_per_pixel(const AVPixFmtDescriptor *pixdesc)
{
    int c, bits = 0;
    int log2_pixels = pixdesc->log2_chroma_w + pixdesc->log2_chroma_h;
    int steps[4] = {0};

    for (c = 0; c < pixdesc->nb_components; c++) {
        const AVComponentDescriptor *comp = &pixdesc->comp[c];
        int s = c == 1 || c == 2 ? 0 : log2_pixels;
        steps[comp->plane] = comp->step << s;
    }
    for (c = 0; c < 4; c++)
        bits += steps[c];

    if(!(pixdesc->flags & AV_PIX_FMT_FLAG_BITSTREAM))
        bits *= 8;

    return bits >> log2_pixels;
}

char *av_get_pix_fmt_string(char *buf, int buf_size,
                            enum AVPixelFormat pix_fmt)
{
    /* print header */
    if (pix_fmt < 0) {
       snprintf (buf, buf_size, "name" " nb_components" " nb_bits");
    } else {
        const AVPixFmtDescriptor *pixdesc = &av_pix_fmt_descriptors[pix_fmt];
        snprintf(buf, buf_size, "%-11s %7d %10d", pixdesc->name,
                 pixdesc->nb_components, av_get_bits_per_pixel(pixdesc));
    }

    return buf;
}

const AVPixFmtDescriptor *av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt)
{
    if (pix_fmt < 0 || pix_fmt >= AV_PIX_FMT_NB)
        return NULL;
    return &av_pix_fmt_descriptors[pix_fmt];
}

const AVPixFmtDescriptor *av_pix_fmt_desc_next(const AVPixFmtDescriptor *prev)
{
    if (!prev)
        return &av_pix_fmt_descriptors[0];
    while (prev - av_pix_fmt_descriptors < FF_ARRAY_ELEMS(av_pix_fmt_descriptors) - 1) {
        prev++;
        if (prev->name)
            return prev;
    }
    return NULL;
}

enum AVPixelFormat av_pix_fmt_desc_get_id(const AVPixFmtDescriptor *desc)
{
    if (desc < av_pix_fmt_descriptors ||
        desc >= av_pix_fmt_descriptors + FF_ARRAY_ELEMS(av_pix_fmt_descriptors))
        return AV_PIX_FMT_NONE;

    return (AVPixelFormat)(desc - av_pix_fmt_descriptors);
}

int av_pix_fmt_get_chroma_sub_sample(enum AVPixelFormat pix_fmt,
                                     int *h_shift, int *v_shift)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    if (!desc)
        return AVERROR(ENOSYS);
    *h_shift = desc->log2_chroma_w;
    *v_shift = desc->log2_chroma_h;

    return 0;
}

int av_pix_fmt_count_planes(enum AVPixelFormat pix_fmt)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    int i, planes[4] = { 0 }, ret = 0;

    if (!desc)
        return AVERROR(EINVAL);

    for (i = 0; i < desc->nb_components; i++)
        planes[desc->comp[i].plane] = 1;
    for (i = 0; i < FF_ARRAY_ELEMS(planes); i++)
        ret += planes[i];
    return ret;
}

void ff_check_pixfmt_descriptors(void){
    int i, j;

    for (i=0; i<FF_ARRAY_ELEMS(av_pix_fmt_descriptors); i++) {
        const AVPixFmtDescriptor *d = &av_pix_fmt_descriptors[i];
        uint8_t fill[4][8+6+3] = {{0}};
        uint8_t *data[4] = {fill[0], fill[1], fill[2], fill[3]};
        int linesize[4] = {0,0,0,0};
        uint16_t tmp[2];

        if (!d->name && !d->nb_components && !d->log2_chroma_w && !d->log2_chroma_h && !d->flags)
            continue;
//         av_log(NULL, AV_LOG_DEBUG, "Checking: %s\n", d->name);
        av_assert0(d->log2_chroma_w <= 3);
        av_assert0(d->log2_chroma_h <= 3);
        av_assert0(d->nb_components <= 4);
        av_assert0(d->name && d->name[0]);
        av_assert0((d->nb_components==4 || d->nb_components==2) == !!(d->flags & AV_PIX_FMT_FLAG_ALPHA));
        av_assert2(av_get_pix_fmt(d->name) == i);

        for (j=0; j<FF_ARRAY_ELEMS(d->comp); j++) {
            const AVComponentDescriptor *c = &d->comp[j];
            if(j>=d->nb_components) {
                av_assert0(!c->plane && !c->step && !c->offset && !c->shift && !c->depth);
                continue;
            }
            if (d->flags & AV_PIX_FMT_FLAG_BITSTREAM) {
                av_assert0(c->step >= c->depth);
            } else {
                av_assert0(8*c->step >= c->depth);
            }
            if (d->flags & AV_PIX_FMT_FLAG_BAYER)
                continue;
            av_read_image_line(tmp, (const uint8_t **)data, linesize, d, 0, 0, j, 2, 0);
            av_assert0(tmp[0] == 0 && tmp[1] == 0);
            tmp[0] = tmp[1] = (1<<c->depth) - 1;
            av_write_image_line(tmp, data, linesize, d, 0, 0, j, 2);
        }
    }
}


enum AVPixelFormat av_pix_fmt_swap_endianness(enum AVPixelFormat pix_fmt)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    char name[16];
    int i;

    if (!desc || strlen(desc->name) < 2)
        return AV_PIX_FMT_NONE;
    av_strlcpy(name, desc->name, sizeof(name));
    i = strlen(name) - 2;
    if (strcmp(name + i, "be") && strcmp(name + i, "le"))
        return AV_PIX_FMT_NONE;

    name[i] ^= 'b' ^ 'l';

    return get_pix_fmt_internal(name);
}

#define FF_COLOR_NA      -1
#define FF_COLOR_RGB      0 /**< RGB color space */
#define FF_COLOR_GRAY     1 /**< gray color space */
#define FF_COLOR_YUV      2 /**< YUV color space. 16 <= Y <= 235, 16 <= U, V <= 240 */
#define FF_COLOR_YUV_JPEG 3 /**< YUV color space. 0 <= Y <= 255, 0 <= U, V <= 255 */
#define FF_COLOR_XYZ      4

#define pixdesc_has_alpha(pixdesc) \
    ((pixdesc)->nb_components == 2 || (pixdesc)->nb_components == 4 || (pixdesc)->flags & AV_PIX_FMT_FLAG_PAL)


static int get_color_type(const AVPixFmtDescriptor *desc) {
    if (desc->flags & AV_PIX_FMT_FLAG_PAL)
        return FF_COLOR_RGB;

    if(desc->nb_components == 1 || desc->nb_components == 2)
        return FF_COLOR_GRAY;

    if(desc->name && !strncmp(desc->name, "yuvj", 4))
        return FF_COLOR_YUV_JPEG;

    if(desc->name && !strncmp(desc->name, "xyz", 3))
        return FF_COLOR_XYZ;

    if(desc->flags & AV_PIX_FMT_FLAG_RGB)
        return  FF_COLOR_RGB;

    if(desc->nb_components == 0)
        return FF_COLOR_NA;

    return FF_COLOR_YUV;
}

static int get_pix_fmt_depth(int *min, int *max, enum AVPixelFormat pix_fmt)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    int i;

    if (!desc || !desc->nb_components) {
        *min = *max = 0;
        return AVERROR(EINVAL);
    }

    *min = INT_MAX, *max = -INT_MAX;
    for (i = 0; i < desc->nb_components; i++) {
        *min = FFMIN(desc->comp[i].depth, *min);
        *max = FFMAX(desc->comp[i].depth, *max);
    }
    return 0;
}

static int get_pix_fmt_score(enum AVPixelFormat dst_pix_fmt,
                              enum AVPixelFormat src_pix_fmt,
                              unsigned *lossp, unsigned consider)
{
    const AVPixFmtDescriptor *src_desc = av_pix_fmt_desc_get(src_pix_fmt);
    const AVPixFmtDescriptor *dst_desc = av_pix_fmt_desc_get(dst_pix_fmt);
    int src_color, dst_color;
    int src_min_depth, src_max_depth, dst_min_depth, dst_max_depth;
    int ret, loss, i, nb_components;
    int score = INT_MAX - 1;

    if (dst_pix_fmt >= AV_PIX_FMT_NB || dst_pix_fmt <= AV_PIX_FMT_NONE)
        return ~0;

    /* compute loss */
    *lossp = loss = 0;

    if (dst_pix_fmt == src_pix_fmt)
        return INT_MAX;

    if ((ret = get_pix_fmt_depth(&src_min_depth, &src_max_depth, src_pix_fmt)) < 0)
        return ret;
    if ((ret = get_pix_fmt_depth(&dst_min_depth, &dst_max_depth, dst_pix_fmt)) < 0)
        return ret;

    src_color = get_color_type(src_desc);
    dst_color = get_color_type(dst_desc);
    if (dst_pix_fmt == AV_PIX_FMT_PAL8)
        nb_components = FFMIN(src_desc->nb_components, 4);
    else
        nb_components = FFMIN(src_desc->nb_components, dst_desc->nb_components);

    for (i = 0; i < nb_components; i++) {
        int depth_minus1 = (dst_pix_fmt == AV_PIX_FMT_PAL8) ? 7/nb_components : (dst_desc->comp[i].depth - 1);
        if (src_desc->comp[i].depth - 1 > depth_minus1 && (consider & FF_LOSS_DEPTH)) {
            loss |= FF_LOSS_DEPTH;
            score -= 65536 >> depth_minus1;
        }
    }

    if (consider & FF_LOSS_RESOLUTION) {
        if (dst_desc->log2_chroma_w > src_desc->log2_chroma_w) {
            loss |= FF_LOSS_RESOLUTION;
            score -= 256 << dst_desc->log2_chroma_w;
        }
        if (dst_desc->log2_chroma_h > src_desc->log2_chroma_h) {
            loss |= FF_LOSS_RESOLUTION;
            score -= 256 << dst_desc->log2_chroma_h;
        }
        // don't favor 422 over 420 if downsampling is needed, because 420 has much better support on the decoder side
        if (dst_desc->log2_chroma_w == 1 && src_desc->log2_chroma_w == 0 &&
            dst_desc->log2_chroma_h == 1 && src_desc->log2_chroma_h == 0 ) {
            score += 512;
        }
    }

    if(consider & FF_LOSS_COLORSPACE)
    switch(dst_color) {
    case FF_COLOR_RGB:
        if (src_color != FF_COLOR_RGB &&
            src_color != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_GRAY:
        if (src_color != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_YUV:
        if (src_color != FF_COLOR_YUV)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_YUV_JPEG:
        if (src_color != FF_COLOR_YUV_JPEG &&
            src_color != FF_COLOR_YUV &&
            src_color != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    default:
        /* fail safe test */
        if (src_color != dst_color)
            loss |= FF_LOSS_COLORSPACE;
        break;
    }
    if(loss & FF_LOSS_COLORSPACE)
        score -= (nb_components * 65536) >> FFMIN(dst_desc->comp[0].depth - 1, src_desc->comp[0].depth - 1);

    if (dst_color == FF_COLOR_GRAY &&
        src_color != FF_COLOR_GRAY && (consider & FF_LOSS_CHROMA)) {
        loss |= FF_LOSS_CHROMA;
        score -= 2 * 65536;
    }
    if (!pixdesc_has_alpha(dst_desc) && (pixdesc_has_alpha(src_desc) && (consider & FF_LOSS_ALPHA))) {
        loss |= FF_LOSS_ALPHA;
        score -= 65536;
    }
    if (dst_pix_fmt == AV_PIX_FMT_PAL8 && (consider & FF_LOSS_COLORQUANT) &&
        (src_pix_fmt != AV_PIX_FMT_PAL8 && (src_color != FF_COLOR_GRAY || (pixdesc_has_alpha(src_desc) && (consider & FF_LOSS_ALPHA))))) {
        loss |= FF_LOSS_COLORQUANT;
        score -= 65536;
    }

    *lossp = loss;
    return score;
}

int av_get_pix_fmt_loss(enum AVPixelFormat dst_pix_fmt,
                            enum AVPixelFormat src_pix_fmt,
                            int has_alpha)
{
    int loss;
    int ret = get_pix_fmt_score(dst_pix_fmt, src_pix_fmt, (unsigned *)&loss, has_alpha ? ~0 : ~FF_LOSS_ALPHA);
    if (ret < 0)
        return ret;
    return loss;
}

enum AVPixelFormat av_find_best_pix_fmt_of_2(enum AVPixelFormat dst_pix_fmt1, enum AVPixelFormat dst_pix_fmt2,
                                             enum AVPixelFormat src_pix_fmt, int has_alpha, int *loss_ptr)
{
    enum AVPixelFormat dst_pix_fmt;
    int loss1, loss2, loss_mask;
    const AVPixFmtDescriptor *desc1 = av_pix_fmt_desc_get(dst_pix_fmt1);
    const AVPixFmtDescriptor *desc2 = av_pix_fmt_desc_get(dst_pix_fmt2);
    int score1, score2;

    loss_mask= loss_ptr?~*loss_ptr:~0; /* use loss mask if provided */
    if(!has_alpha)
        loss_mask &= ~FF_LOSS_ALPHA;

    score1 = get_pix_fmt_score(dst_pix_fmt1, src_pix_fmt, (unsigned int*)&loss1, loss_mask);
    score2 = get_pix_fmt_score(dst_pix_fmt2, src_pix_fmt, (unsigned int*)&loss2, loss_mask);

    if (score1 == score2) {
        if(av_get_padded_bits_per_pixel(desc2) != av_get_padded_bits_per_pixel(desc1)) {
            dst_pix_fmt = av_get_padded_bits_per_pixel(desc2) < av_get_padded_bits_per_pixel(desc1) ? dst_pix_fmt2 : dst_pix_fmt1;
        } else {
            dst_pix_fmt = desc2->nb_components < desc1->nb_components ? dst_pix_fmt2 : dst_pix_fmt1;
        }
    } else {
        dst_pix_fmt = score1 < score2 ? dst_pix_fmt2 : dst_pix_fmt1;
    }

    if (loss_ptr)
        *loss_ptr = av_get_pix_fmt_loss(dst_pix_fmt, src_pix_fmt, has_alpha);
    return dst_pix_fmt;
}

const char *av_color_range_name(enum AVColorRange range)
{
    return (unsigned) range < AVCOL_RANGE_NB ?
        color_range_names[range] : NULL;
}

const char *av_color_primaries_name(enum AVColorPrimaries primaries)
{
    return (unsigned) primaries < AVCOL_PRI_NB ?
        color_primaries_names[primaries] : NULL;
}

const char *av_color_transfer_name(enum AVColorTransferCharacteristic transfer)
{
    return (unsigned) transfer < AVCOL_TRC_NB ?
        color_transfer_names[transfer] : NULL;
}

const char *av_color_space_name(enum AVColorSpace space)
{
    return (unsigned) space < AVCOL_SPC_NB ?
        color_space_names[space] : NULL;
}

const char *av_chroma_location_name(enum AVChromaLocation location)
{
    return (unsigned) location < AVCHROMA_LOC_NB ?
        chroma_location_names[location] : NULL;
}
}