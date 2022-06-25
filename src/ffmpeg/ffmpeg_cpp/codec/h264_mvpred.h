/*
 * H.26L/H.264/AVC/JVT/14496-10/... motion vector prediction
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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

/**
 * @file
 * H.264 / AVC / MPEG-4 part10 motion vector prediction.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#ifndef AVCODEC_H264_MVPRED_H
#define AVCODEC_H264_MVPRED_H

#include "internal_util.h"
#include "avcodec.h"
#include "h264dec.h"
#include "mpegutils.h"
#include "avassert.h"


 av_always_inline int fetch_diagonal_mv(const H264Context *h, H264SliceContext *sl,
                                              const int16_t **C,
                                              int i, int list, int part_width);

/**
 * Get the predicted MV.
 * @param n the block index
 * @param part_width the width of the partition (4, 8,16) -> (1, 2, 4)
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
 
 
 av_always_inline void pred_motion(const H264Context *const h,
                                         H264SliceContext *sl,
                                         int n,
                                         int part_width, int list, int ref,
                                         int *const mx, int *const my);


/**
 * Get the directionally predicted 16x8 MV.
 * @param n the block index
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
 av_always_inline void pred_16x8_motion(const H264Context *const h,
                                              H264SliceContext *sl,
                                              int n, int list, int ref,
                                              int *const mx, int *const my);

/**
 * Get the directionally predicted 8x16 MV.
 * @param n the block index
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
 av_always_inline void pred_8x16_motion(const H264Context *const h,
                                              H264SliceContext *sl,
                                              int n, int list, int ref,
                                              int *const mx, int *const my);



 av_always_inline void pred_pskip_motion(const H264Context *const h,
                                               H264SliceContext *sl);


 void fill_decode_neighbors(const H264Context *h, H264SliceContext *sl, int mb_type);

 void fill_decode_caches(const H264Context *h, H264SliceContext *sl, int mb_type);

/**
 * decodes a P_SKIP or B_SKIP macroblock
 */

 
void av_unused decode_mb_skip(const H264Context *h, H264SliceContext *sl);

#endif /* AVCODEC_H264_MVPRED_H */
