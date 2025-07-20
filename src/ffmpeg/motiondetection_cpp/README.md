#-DMOTION_VECTOR for disabling  MB decoder


This module detects a frame contains moving objects utilizing motion vector 
information from mpeg4 frame.

# Extracting motion vector



```
AVFrameSideData* sd = av_frame_get_side_data(decframe, AV_FRAME_DATA_MOTION_VECTORS);
AVMotionVector* mvs = (AVMotionVector*)sd->data;
int mvcount = sd->size / sizeof(AVMotionVector);
```

# Detecting Motion
The extracted motion vector is passed to `process_frame()` in `MVDetector`.
Inside the function, the occupancy map is deteremined basd on the motion vectors.
The occupancy_map is 8x8 grid and deemed to be occupied when the motion vector 
originating the grid cell is found. 

After an occupancy map for each frame is found, the average occupancy for last N 
frames is calculated. Which aims to reduce temporarily noise. The default value 
is set to 3 (`MVDetector::DEFAULT_WINDOW_SIZE`). 

At each frame the total number of grid in the occupancy map whose values is above a
a threshold, currently defaulted to 0.6 (`MVDetector::DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD),
is counted. When this number is above a thredhold (DEFAULT_OCCUPANCY_THRESHOLD),
the frame is finally decided to contain a `movement`.

In order to reduce flickering effect where a continus sequence of frame 


to install opencv 
apt install  libopencv-dev libopencv-core-dev python3-opencv



to compile opencv

https://github.com/Itseez/opencv.git 









*H264*

https://blog.csdn.net/leixiaohua1020/article/details/45114453

https://blog.csdn.net/hffgjh/article/details/86888295


MVP
MVD MV deffrential is ony for cabac not for cavlc
MVD Motion vector prediction



https://blog.fearcat.in/a?ID=00700-089c0c4e-9d57-41b8-9d0e-bc67805ab4b7

https://blog.csdn.net/ricky90/article/details/79436422

h->cur_pic.motion_val






NAL

00001| 09 (AUD)| 10 | 000001 | 67(SPS) | xxxx (SPS data)
   xx | 000001 | 68 (PPS)| xxxx (PPS data)
AUD, SPS, PPS is type of NALU (Network Abstraction Layer Units) NALU have about 31 types.

09 AUD mean Access Unit Delimiter.

Access Unit Delimiter (AUD). An AUD is an optional NALU that can be use to delimit frames in an elementary stream. It is not required (unless otherwise stated by the container/protocol, like TS), and is often not included in order to save space, but it can be useful to finds the start of a frame without having to fully parse each NALU.


*SEI*

supplemental enhancement information (SEI)

In audio and video streaming, supplemental enhancement information (SEI) is additional data inserted into the bitstream to convey extra information, which can be received in accurate synchronization with the related audio and video content.

SEI can be inserted during the encoding and transmission of audio and video content. Various types of information can be added into SEI, such as parameters of the camera or encoder; time code; closed captions; lyrics; and copyright info. While standards define the syntax and semantics of how SEI messages should be inserted into the bitstreams, there is no requirement for decoders to be able to process this information. As such, SEI data may be discarded at some point within the content distribution chain if the equipment does not support it.

In addition to conveying technical information related to the bitstream, SEI can also be used for specific applications purposes. As an example, SEI could be used to send questions in live interactive quizzes, with the audience answering on an app to earn points or rewards. The diagram below shows a sample implementation of this workflow. While the host asks questions, the on-site staff sends instructions to the app server to insert several SEI messages (such as the content of the questions, and time remaining to answer) into the live stream. After the app receives the video stream, it analyzes the SEI messages and immediately requests the corresponding content from the app server for display, completing the quizzing process.




SPS ( widht, hight fps )



PPS ( basic cavlc, main caback ), reference cound, 






Nalu 
Key frame or not key frame
( it does not tell you detail of I and B frames ). For that you have to decode frame
namespace H264SliceType {
  static const unsigned none  =0;
  static const unsigned sps   =7;
  static const unsigned pps   =8;
  static const unsigned idr     =5;   // IDR picture. Do not confuse with IDR KEY Frame
  static const unsigned nonidr  =1;  // non idr slice
  static const unsigned aud  =9;   // aud delimeter
};

following is possible after decoding frame. Nal will no give you information about i, p and b
namespace H264SframeType {
  static const unsigned none  =0;   
  static const unsigned i   =3; 
  static const unsigned p   =2;
  static const unsigned b   =1;  
  
};




 The macroblock decoding function (Decode) decodes the compressed data by methods such as intra-frame prediction, inter-frame prediction, and inverse DCT transform. The decoding function is ff_h264_hl_decode_mb(). Among them, different from the macro block type, several different functions will be called, and the most common one is to call hl_decode_mb_simple_8().
 The definition of hl_decode_mb_simple_8() cannot be found directly in the source code, because the function name of its actual code is written using a macro. The source code of hl_decode_mb_simple_8() is actually the source code of the FUNC(hl_decode_mb)() function.
 As can be seen from the function call diagram, FUNC(hl_decode_mb)() performs different processing according to the type of macroblock: if the macroblock is predicted intraframe (INTRA), hl_decode_mb_predict_luma() will be called for intraframe prediction; if it is a frame For inter prediction macroblock (INTER), FUNC(hl_motion_422)() or FUNC(hl_motion_420)() will be called to perform quarter-pixel motion compensation.
 After the intra-frame prediction or the inter-frame prediction step, prediction data is obtained. Then FUNC(hl_decode_mb)() will call several functions such as hl_decode_mb_idct_luma() to inverse DCT transform the residual data, and superimpose the transformed data on the predicted data to form decoded image data.
 Since the decoding of intra-frame prediction macroblocks and inter-frame prediction macroblocks is complicated, it is divided into two articles to record the source codes of these two parts. This paper records the source code of intra-frame prediction macroblock decoding.
 Let's first review the decode_slice() function.




decode_slice()
decode_slice() is used to decode H.264 slices. This function completes the functions of "entropy decoding", "macroblock decoding" and "loop filtering". Its definition is located in libavcodec\h264_slice.c as shown below.
//解码slice//三个主要步骤：//1.熵解码（CAVLC/CABAC）//2.宏块解码//3.环路滤波//此外还包含了错误隐藏代码static int decode_slice(struct AVCodecContext *avctx, void *arg){    H264Context *h = *(void **)arg;    int lf_x_start = h->mb_x;    h->mb_skip_run = -1;    av_assert0(h->block_offset[15] == (4 * ((scan8[15] - scan8[0]) & 7) << h->pixel_shift) + 4 * h->linesize * ((scan8[15] - scan8[0]) >> 3));    h->is_complex = FRAME_MBAFF(h) || h->picture_structure != PICT_FRAME ||                    avctx->codec_id != AV_CODEC_ID_H264 ||                    (CONFIG_GRAY && (h->flags & CODEC_FLAG_GRAY));    if (!(h->avctx->active_thread_type & FF_THREAD_SLICE) && h->picture_structure == PICT_FRAME && h->er.error_status_table) {        const int start_i  = av_clip(h->resync_mb_x + h->resync_mb_y * h->mb_width, 0, h->mb_num - 1);        if (start_i) {            int prev_status = h->er.error_status_table[h->er.mb_index2xy[start_i - 1]];            prev_status &= ~ VP_START;            if (prev_status != (ER_MV_END | ER_DC_END | ER_AC_END))                h->er.error_occurred = 1;        }    }    //CABAC情况    if (h->pps.cabac) {        /* realign */        align_get_bits(&h->gb);        /* init cabac */        //初始化CABAC解码器        ff_init_cabac_decoder(&h->cabac,                              h->gb.buffer + get_bits_count(&h->gb) / 8,                              (get_bits_left(&h->gb) + 7) / 8);        ff_h264_init_cabac_states(h);        //循环处理每个宏块        for (;;) {            // START_TIMER         //解码CABAC数据            int ret = ff_h264_decode_mb_cabac(h);            int eos;            // STOP_TIMER("decode_mb_cabac")            //解码宏块            if (ret >= 0)                ff_h264_hl_decode_mb(h);            // FIXME optimal? or let mb_decode decode 16x32 ?            //宏块级帧场自适应。很少接触            if (ret >= 0 && FRAME_MBAFF(h)) {                h->mb_y++;                ret = ff_h264_decode_mb_cabac(h);                //解码宏块                if (ret >= 0)                    ff_h264_hl_decode_mb(h);                h->mb_y--;            }            eos = get_cabac_terminate(&h->cabac);            if ((h->workaround_bugs & FF_BUG_TRUNCATED) &&                h->cabac.bytestream > h->cabac.bytestream_end + 2) {             //错误隐藏                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x - 1,                             h->mb_y, ER_MB_END);                if (h->mb_x >= lf_x_start)                    loop_filter(h, lf_x_start, h->mb_x + 1);                return 0;            }            if (h->cabac.bytestream > h->cabac.bytestream_end + 2 )                av_log(h->avctx, AV_LOG_DEBUG, "bytestream overread %"PTRDIFF_SPECIFIER"\n", h->cabac.bytestream_end - h->cabac.bytestream);            if (ret < 0 || h->cabac.bytestream > h->cabac.bytestream_end + 4) {                av_log(h->avctx, AV_LOG_ERROR,                       "error while decoding MB %d %d, bytestream %"PTRDIFF_SPECIFIER"\n",                       h->mb_x, h->mb_y,                       h->cabac.bytestream_end - h->cabac.bytestream);                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x,                             h->mb_y, ER_MB_ERROR);                return AVERROR_INVALIDDATA;            }            //mb_x自增            //如果自增后超过了一行的mb个数            if (++h->mb_x >= h->mb_width) {             //环路滤波                loop_filter(h, lf_x_start, h->mb_x);                h->mb_x = lf_x_start = 0;                decode_finish_row(h);                //mb_y自增（处理下一行）                ++h->mb_y;                //宏块级帧场自适应，暂不考虑                if (FIELD_OR_MBAFF_PICTURE(h)) {                    ++h->mb_y;                    if (FRAME_MBAFF(h) && h->mb_y < h->mb_height)                        predict_field_decoding_flag(h);                }            }            //如果mb_y超过了mb的行数            if (eos || h->mb_y >= h->mb_height) {                tprintf(h->avctx, "slice end %d %d\n",                        get_bits_count(&h->gb), h->gb.size_in_bits);                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x - 1,                             h->mb_y, ER_MB_END);                if (h->mb_x > lf_x_start)                    loop_filter(h, lf_x_start, h->mb_x);                return 0;            }        }    } else {     //CAVLC情况     //循环处理每个宏块        for (;;) {         //解码宏块的CAVLC            int ret = ff_h264_decode_mb_cavlc(h);            //解码宏块            if (ret >= 0)                ff_h264_hl_decode_mb(h);            // FIXME optimal? or let mb_decode decode 16x32 ?            if (ret >= 0 && FRAME_MBAFF(h)) {                h->mb_y++;                ret = ff_h264_decode_mb_cavlc(h);                if (ret >= 0)                    ff_h264_hl_decode_mb(h);                h->mb_y--;            }            if (ret < 0) {                av_log(h->avctx, AV_LOG_ERROR,                       "error while decoding MB %d %d\n", h->mb_x, h->mb_y);                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x,                             h->mb_y, ER_MB_ERROR);                return ret;            }            if (++h->mb_x >= h->mb_width) {             //环路滤波                loop_filter(h, lf_x_start, h->mb_x);                h->mb_x = lf_x_start = 0;                decode_finish_row(h);                ++h->mb_y;                if (FIELD_OR_MBAFF_PICTURE(h)) {                    ++h->mb_y;                    if (FRAME_MBAFF(h) && h->mb_y < h->mb_height)                        predict_field_decoding_flag(h);                }                if (h->mb_y >= h->mb_height) {                    tprintf(h->avctx, "slice end %d %d\n",                            get_bits_count(&h->gb), h->gb.size_in_bits);                    if (   get_bits_left(&h->gb) == 0                        || get_bits_left(&h->gb) > 0 && !(h->avctx->err_recognition & AV_EF_AGGRESSIVE)) {                     //错误隐藏                        er_add_slice(h, h->resync_mb_x, h->resync_mb_y,                                     h->mb_x - 1, h->mb_y, ER_MB_END);                        return 0;                    } else {                        er_add_slice(h, h->resync_mb_x, h->resync_mb_y,                                     h->mb_x, h->mb_y, ER_MB_END);                        return AVERROR_INVALIDDATA;                    }                }            }            if (get_bits_left(&h->gb) <= 0 && h->mb_skip_run <= 0) {                tprintf(h->avctx, "slice end %d %d\n",                        get_bits_count(&h->gb), h->gb.size_in_bits);                if (get_bits_left(&h->gb) == 0) {                    er_add_slice(h, h->resync_mb_x, h->resync_mb_y,                                 h->mb_x - 1, h->mb_y, ER_MB_END);                    if (h->mb_x > lf_x_start)                        loop_filter(h, lf_x_start, h->mb_x);                    return 0;                } else {                    er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x,                                 h->mb_y, ER_MB_ERROR);                    return AVERROR_INVALIDDATA;                }            }        }    }}
1

Repeatedly record the process of decode_slice():
(1) Determine whether the H.264 code stream is CABAC encoding or CAVLC encoding, and enter different processing loops.
(2) If it is CABAC encoding, first call ff_init_cabac_decoder() to initialize the CABAC decoder. Then a loop is entered, and each macroblock is processed in turn as follows:
a) Call ff_h264_decode_mb_cabac() for CABAC entropy decoding
b) Call ff_h264_hl_decode_mb() for macroblock decoding
c) After decoding a line of macroblocks, call loop_filter() for loop filtering
d) In addition, it is possible to call er_add_slice() for error concealment processing
(3) If it is CABAC encoding, directly enter a loop, and perform the following processing on each macroblock in turn:
a) Call ff_h264_decode_mb_cavlc() for CAVLC entropy decoding
b) Call ff_h264_hl_decode_mb() for macroblock decoding
c) After decoding a line of macroblocks, call loop_filter() for loop filtering
d) In addition, it is possible to call er_add_slice() for error concealment processing
It can be seen that the macroblock decoding function is ff_h264_hl_decode_mb(). Take a look at this function below.




ff_h264_hl_decode_mb()
ff_h264_hl_decode_mb() completes the macroblock decoding. "Macroblock decoding" is the process of restoring image data according to the macroblock type, motion vector, reference frame, DCT residual data and other information obtained in the previous step "entropy decoding". The definition of this function is located in libavcodec\h264_mb.c as shown below.
//解码宏块void ff_h264_hl_decode_mb(H264Context *h){ //宏块序号 mb_xy = mb_x + mb_y*mb_stride    const int mb_xy   = h->mb_xy;    //宏块类型    const int mb_type = h->cur_pic.mb_type[mb_xy];    //比较少见，PCM类型    int is_complex    = CONFIG_SMALL || h->is_complex ||                        IS_INTRA_PCM(mb_type) || h->qscale == 0;    //YUV444    if (CHROMA444(h)) {        if (is_complex || h->pixel_shift)            hl_decode_mb_444_complex(h);        else            hl_decode_mb_444_simple_8(h);    } else if (is_complex) {        hl_decode_mb_complex(h);   //PCM类型？    } else if (h->pixel_shift) {        hl_decode_mb_simple_16(h); //色彩深度为16    } else        hl_decode_mb_simple_8(h);  //色彩深度为8}
1

It can be seen that the definition of ff_h264_hl_decode_mb() is very simple: determine which function to call as the decoding function by system parameters (such as whether the color bit depth is 8bit, whether the YUV sampling format is 4:4:4, etc.). Since the most common case is to decode 8bit H.264 data in YUV420P format, hl_decode_mb_simple_8() is usually called. One thing to note here: if we directly look up the definition of hl_decode_mb_simple_8(), we will find that this function cannot be found. The definition of this function is actually the FUNC(hl_decode_mb)() function. FUNC(hl_decode_mb)() The macro "FUNC()" in the function name is expanded to hl_decode_mb_simple_8(). So let's take a look at the FUNC(hl_decode_mb)() function.

FUNC(hl_decode_mb)() 
The definition of FUNC(hl_decode_mb)() is located in libavcodec\h264_mb_template.c. Let's take a look at the definition of the FUNC(hl_decode_mb)() function.
PS: It should be noted here that the functions in the C language files containing "_template" in the name of the FFmpeg H.264 decoder are written in a way similar to "FUNC(name)()". The purpose of this is probably for a variety of functions. For example, when processing 16-bit H.264 code stream, FUNC(hl_decode_mb)() can be expanded to hl_decode_mb_simple_16() function; similarly, FUNC(hl_decode_mb)() can also be expanded to hl_decode_mb_complex() function under other conditions.

//hl是什么意思？high level？/* * 注释：雷霄骅 * leixiaohua1020@126.com * http://blog.csdn.net/leixiaohua1020 * * 宏块解码 * 帧内宏块：帧内预测->残差DCT反变换 * 帧间宏块：帧间预测（运动补偿）->残差DCT反变换 * */static av_noinline void FUNC(hl_decode_mb)(H264Context *h){ //序号：x（行）和y（列）    const int mb_x    = h->mb_x;    const int mb_y    = h->mb_y;    //宏块序号 mb_xy = mb_x + mb_y*mb_stride    const int mb_xy   = h->mb_xy;    //宏块类型    const int mb_type = h->cur_pic.mb_type[mb_xy];    //这三个变量存储最后处理完成的像素值    uint8_t *dest_y, *dest_cb, *dest_cr;    int linesize, uvlinesize /*dct_offset*/;    int i, j;    int *block_offset = &h->block_offset[0];    const int transform_bypass = !SIMPLE && (h->qscale == 0 && h->sps.transform_bypass);    /* is_h264 should always be true if SVQ3 is disabled. */    const int is_h264 = !CONFIG_SVQ3_DECODER || SIMPLE || h->avctx->codec_id == AV_CODEC_ID_H264;    void (*idct_add)(uint8_t *dst, int16_t *block, int stride);    const int block_h   = 16 >> h->chroma_y_shift;    const int chroma422 = CHROMA422(h);    //存储Y，U，V像素的位置：dest_y，dest_cb，dest_cr    //分别对应AVFrame的data[0]，data[1]，data[2]    dest_y  = h->cur_pic.f.data[0] + ((mb_x << PIXEL_SHIFT)     + mb_y * h->linesize)  * 16;    dest_cb = h->cur_pic.f.data[1] +  (mb_x << PIXEL_SHIFT) * 8 + mb_y * h->uvlinesize * block_h;    dest_cr = h->cur_pic.f.data[2] +  (mb_x << PIXEL_SHIFT) * 8 + mb_y * h->uvlinesize * block_h;    h->vdsp.prefetch(dest_y  + (h->mb_x & 3) * 4 * h->linesize   + (64 << PIXEL_SHIFT), h->linesize,       4);    h->vdsp.prefetch(dest_cb + (h->mb_x & 7)     * h->uvlinesize + (64 << PIXEL_SHIFT), dest_cr - dest_cb, 2);    h->list_counts[mb_xy] = h->list_count;    //系统中包含了    //#define SIMPLE 1    //不会执行？    if (!SIMPLE && MB_FIELD(h)) {        linesize     = h->mb_linesize = h->linesize * 2;        uvlinesize   = h->mb_uvlinesize = h->uvlinesize * 2;        block_offset = &h->block_offset[48];        if (mb_y & 1) { // FIXME move out of this function?            dest_y  -= h->linesize * 15;            dest_cb -= h->uvlinesize * (block_h - 1);            dest_cr -= h->uvlinesize * (block_h - 1);        }        if (FRAME_MBAFF(h)) {            int list;            for (list = 0; list < h->list_count; list++) {                if (!USES_LIST(mb_type, list))                    continue;                if (IS_16X16(mb_type)) {                    int8_t *ref = &h->ref_cache[list][scan8[0]];                    fill_rectangle(ref, 4, 4, 8, (16 + *ref) ^ (h->mb_y & 1), 1);                } else {                    for (i = 0; i < 16; i += 4) {                        int ref = h->ref_cache[list][scan8[i]];                        if (ref >= 0)                            fill_rectangle(&h->ref_cache[list][scan8[i]], 2, 2,                                           8, (16 + ref) ^ (h->mb_y & 1), 1);                    }                }            }        }    } else {        linesize   = h->mb_linesize   = h->linesize;        uvlinesize = h->mb_uvlinesize = h->uvlinesize;        // dct_offset = s->linesize * 16;    }    //系统中包含了    //#define SIMPLE 1    //不会执行？    if (!SIMPLE && IS_INTRA_PCM(mb_type)) {        const int bit_depth = h->sps.bit_depth_luma;        if (PIXEL_SHIFT) {            int j;            GetBitContext gb;            init_get_bits(&gb, h->intra_pcm_ptr,                          ff_h264_mb_sizes[h->sps.chroma_format_idc] * bit_depth);            for (i = 0; i < 16; i++) {                uint16_t *tmp_y = (uint16_t *)(dest_y + i * linesize);                for (j = 0; j < 16; j++)                    tmp_y[j] = get_bits(&gb, bit_depth);            }            if (SIMPLE || !CONFIG_GRAY || !(h->flags & CODEC_FLAG_GRAY)) {                if (!h->sps.chroma_format_idc) {                    for (i = 0; i < block_h; i++) {                        uint16_t *tmp_cb = (uint16_t *)(dest_cb + i * uvlinesize);                        uint16_t *tmp_cr = (uint16_t *)(dest_cr + i * uvlinesize);                        for (j = 0; j < 8; j++) {                            tmp_cb[j] = tmp_cr[j] = 1 << (bit_depth - 1);                        }                    }                } else {                    for (i = 0; i < block_h; i++) {                        uint16_t *tmp_cb = (uint16_t *)(dest_cb + i * uvlinesize);                        for (j = 0; j < 8; j++)                            tmp_cb[j] = get_bits(&gb, bit_depth);                    }                    for (i = 0; i < block_h; i++) {                        uint16_t *tmp_cr = (uint16_t *)(dest_cr + i * uvlinesize);                        for (j = 0; j < 8; j++)                            tmp_cr[j] = get_bits(&gb, bit_depth);                    }                }            }        } else {            for (i = 0; i < 16; i++)                memcpy(dest_y + i * linesize, h->intra_pcm_ptr + i * 16, 16);            if (SIMPLE || !CONFIG_GRAY || !(h->flags & CODEC_FLAG_GRAY)) {                if (!h->sps.chroma_format_idc) {                    for (i = 0; i < 8; i++) {                        memset(dest_cb + i * uvlinesize, 1 << (bit_depth - 1), 8);                        memset(dest_cr + i * uvlinesize, 1 << (bit_depth - 1), 8);                    }                } else {                    const uint8_t *src_cb = h->intra_pcm_ptr + 256;                    const uint8_t *src_cr = h->intra_pcm_ptr + 256 + block_h * 8;                    for (i = 0; i < block_h; i++) {                        memcpy(dest_cb + i * uvlinesize, src_cb + i * 8, 8);                        memcpy(dest_cr + i * uvlinesize, src_cr + i * 8, 8);                    }                }            }        }    } else {     //Intra类型     //Intra4x4或者Intra16x16        if (IS_INTRA(mb_type)) {            if (h->deblocking_filter)                xchg_mb_border(h, dest_y, dest_cb, dest_cr, linesize,                               uvlinesize, 1, 0, SIMPLE, PIXEL_SHIFT);            if (SIMPLE || !CONFIG_GRAY || !(h->flags & CODEC_FLAG_GRAY)) {                h->hpc.pred8x8[h->chroma_pred_mode](dest_cb, uvlinesize);                h->hpc.pred8x8[h->chroma_pred_mode](dest_cr, uvlinesize);            }            //帧内预测-亮度            hl_decode_mb_predict_luma(h, mb_type, is_h264, SIMPLE,                                      transform_bypass, PIXEL_SHIFT,                                      block_offset, linesize, dest_y, 0);            if (h->deblocking_filter)                xchg_mb_border(h, dest_y, dest_cb, dest_cr, linesize,                               uvlinesize, 0, 0, SIMPLE, PIXEL_SHIFT);        } else if (is_h264) {         //Inter类型            //运动补偿            if (chroma422) {                FUNC(hl_motion_422)(h, dest_y, dest_cb, dest_cr,                              h->qpel_put, h->h264chroma.put_h264_chroma_pixels_tab,                              h->qpel_avg, h->h264chroma.avg_h264_chroma_pixels_tab,                              h->h264dsp.weight_h264_pixels_tab,                              h->h264dsp.biweight_h264_pixels_tab);            } else {             //“*_put”处理单向预测，“*_avg”处理双向预测，“weight”处理加权预测             //h->qpel_put[16]包含了单向预测的四分之一像素运动补偿所有样点处理的函数             //两个像素之间横向的点（内插点和原始的点）有4个，纵向的点有4个，组合起来一共16个             //h->qpel_avg[16]情况也类似                FUNC(hl_motion_420)(h, dest_y, dest_cb, dest_cr,                              h->qpel_put, h->h264chroma.put_h264_chroma_pixels_tab,                              h->qpel_avg, h->h264chroma.avg_h264_chroma_pixels_tab,                              h->h264dsp.weight_h264_pixels_tab,                              h->h264dsp.biweight_h264_pixels_tab);            }        }        //亮度的IDCT        hl_decode_mb_idct_luma(h, mb_type, is_h264, SIMPLE, transform_bypass,                               PIXEL_SHIFT, block_offset, linesize, dest_y, 0);        //色度的IDCT（没有写在一个单独的函数中）        if ((SIMPLE || !CONFIG_GRAY || !(h->flags & CODEC_FLAG_GRAY)) &&            (h->cbp & 0x30)) {            uint8_t *dest[2] = { dest_cb, dest_cr };            //transform_bypass=0，不考虑            if (transform_bypass) {                if (IS_INTRA(mb_type) && h->sps.profile_idc == 244 &&                    (h->chroma_pred_mode == VERT_PRED8x8 ||                     h->chroma_pred_mode == HOR_PRED8x8)) {                    h->hpc.pred8x8_add[h->chroma_pred_mode](dest[0],                                                            block_offset + 16,                                                            h->mb + (16 * 16 * 1 << PIXEL_SHIFT),                                                            uvlinesize);                    h->hpc.pred8x8_add[h->chroma_pred_mode](dest[1],                                                            block_offset + 32,                                                            h->mb + (16 * 16 * 2 << PIXEL_SHIFT),                                                            uvlinesize);                } else {                    idct_add = h->h264dsp.h264_add_pixels4_clear;                    for (j = 1; j < 3; j++) {                        for (i = j * 16; i < j * 16 + 4; i++)                            if (h->non_zero_count_cache[scan8[i]] ||                                dctcoef_get(h->mb, PIXEL_SHIFT, i * 16))                                idct_add(dest[j - 1] + block_offset[i],                                         h->mb + (i * 16 << PIXEL_SHIFT),                                         uvlinesize);                        if (chroma422) {                            for (i = j * 16 + 4; i < j * 16 + 8; i++)                                if (h->non_zero_count_cache[scan8[i + 4]] ||                                    dctcoef_get(h->mb, PIXEL_SHIFT, i * 16))                                    idct_add(dest[j - 1] + block_offset[i + 4],                                             h->mb + (i * 16 << PIXEL_SHIFT),                                             uvlinesize);                        }                    }                }            } else {                if (is_h264) {                    int qp[2];                    if (chroma422) {                        qp[0] = h->chroma_qp[0] + 3;                        qp[1] = h->chroma_qp[1] + 3;                    } else {                        qp[0] = h->chroma_qp[0];                        qp[1] = h->chroma_qp[1];                    }                    //色度的IDCT                    //直流分量的hadamard变换                    if (h->non_zero_count_cache[scan8[CHROMA_DC_BLOCK_INDEX + 0]])                        h->h264dsp.h264_chroma_dc_dequant_idct(h->mb + (16 * 16 * 1 << PIXEL_SHIFT),                                                               h->dequant4_coeff[IS_INTRA(mb_type) ? 1 : 4][qp[0]][0]);                    if (h->non_zero_count_cache[scan8[CHROMA_DC_BLOCK_INDEX + 1]])                        h->h264dsp.h264_chroma_dc_dequant_idct(h->mb + (16 * 16 * 2 << PIXEL_SHIFT),                                                               h->dequant4_coeff[IS_INTRA(mb_type) ? 2 : 5][qp[1]][0]);                    //IDCT                    //最后的“8”代表内部循环处理8次（U,V各4次）                    h->h264dsp.h264_idct_add8(dest, block_offset,                                              h->mb, uvlinesize,                                              h->non_zero_count_cache);                } else if (CONFIG_SVQ3_DECODER) {                    h->h264dsp.h264_chroma_dc_dequant_idct(h->mb + 16 * 16 * 1,                                                           h->dequant4_coeff[IS_INTRA(mb_type) ? 1 : 4][h->chroma_qp[0]][0]);                    h->h264dsp.h264_chroma_dc_dequant_idct(h->mb + 16 * 16 * 2,                                                           h->dequant4_coeff[IS_INTRA(mb_type) ? 2 : 5][h->chroma_qp[1]][0]);                    for (j = 1; j < 3; j++) {                        for (i = j * 16; i < j * 16 + 4; i++)                            if (h->non_zero_count_cache[scan8[i]] || h->mb[i * 16]) {                                uint8_t *const ptr = dest[j - 1] + block_offset[i];                                ff_svq3_add_idct_c(ptr, h->mb + i * 16,                                                   uvlinesize,                                                   ff_h264_chroma_qp[0][h->qscale + 12] - 12, 2);                            }                    }                }            }        }    }}
1

Let's briefly sort out the process of FUNC(hl_decode_mb) (only the decoding of the luminance component is considered here, and the decoding process of the chrominance component is similar):
(1) Prediction
a) If it is an intra-frame prediction macroblock (Intra), call hl_decode_mb_predict_luma() to perform intra-frame prediction to obtain prediction data.
b) If it is not an intra-frame prediction macroblock (Inter), call FUNC(hl_motion_420)() or FUNC(hl_motion_422)() to perform inter-frame prediction (ie motion compensation) to obtain prediction data.
(2) Residual stacking
a) Call hl_decode_mb_idct_luma() to perform DCT inverse transformation on the DCT residual data to obtain residual pixel data and superimpose it on the previously obtained prediction data to obtain the final image data.
PS: There is an important memory pointer dest_y throughout the process, and the decoded luminance data is stored in the memory it points to.



This article will analyze the two parts of intra-frame prediction and residual stacking in the above process. Let's take a look at the intra prediction function hl_decode_mb_predict_luma().

hl_decode_mb_predict_luma()
hl_decode_mb_predict_luma() performs intra prediction on intra macroblocks, its definition is located in libavcodec\h264_mb.c, as shown below.
//帧内预测-亮度//分成2种情况：Intra4x4和Intra16x16static av_always_inline void hl_decode_mb_predict_luma(H264Context *h,                                                       int mb_type, int is_h264,                                                       int simple,                                                       int transform_bypass,                                                       int pixel_shift,                                                       int *block_offset,                                                       int linesize,                                                       uint8_t *dest_y, int p){ //用于DCT反变换    void (*idct_add)(uint8_t *dst, int16_t *block, int stride);    void (*idct_dc_add)(uint8_t *dst, int16_t *block, int stride);    int i;    int qscale = p == 0 ? h->qscale : h->chroma_qp[p - 1];    //外部调用时候p=0    block_offset += 16 * p;    if (IS_INTRA4x4(mb_type)) {     //Intra4x4帧内预测        if (IS_8x8DCT(mb_type)) {         //如果使用了8x8的DCT，先不研究            if (transform_bypass) {                idct_dc_add =                idct_add    = h->h264dsp.h264_add_pixels8_clear;            } else {                idct_dc_add = h->h264dsp.h264_idct8_dc_add;                idct_add    = h->h264dsp.h264_idct8_add;            }            for (i = 0; i < 16; i += 4) {                uint8_t *const ptr = dest_y + block_offset[i];                const int dir      = h->intra4x4_pred_mode_cache[scan8[i]];                if (transform_bypass && h->sps.profile_idc == 244 && dir <= 1) {                    if (h->x264_build != -1) {                        h->hpc.pred8x8l_add[dir](ptr, h->mb + (i * 16 + p * 256 << pixel_shift), linesize);                    } else                        h->hpc.pred8x8l_filter_add[dir](ptr, h->mb + (i * 16 + p * 256 << pixel_shift),                                                        (h-> topleft_samples_available << i) & 0x8000,                                                        (h->topright_samples_available << i) & 0x4000, linesize);                } else {                    const int nnz = h->non_zero_count_cache[scan8[i + p * 16]];                    h->hpc.pred8x8l[dir](ptr, (h->topleft_samples_available << i) & 0x8000,                                         (h->topright_samples_available << i) & 0x4000, linesize);                    if (nnz) {                        if (nnz == 1 && dctcoef_get(h->mb, pixel_shift, i * 16 + p * 256))                            idct_dc_add(ptr, h->mb + (i * 16 + p * 256 << pixel_shift), linesize);                        else                            idct_add(ptr, h->mb + (i * 16 + p * 256 << pixel_shift), linesize);                    }                }            }        } else {         /*       * Intra4x4帧内预测：16x16 宏块被划分为16个4x4子块       *       * +----+----+----+----+       * |    |    |    |    |       * +----+----+----+----+       * |    |    |    |    |       * +----+----+----+----+       * |    |    |    |    |       * +----+----+----+----+       * |    |    |    |    |       * +----+----+----+----+       *       */         //4x4的IDCT         //transform_bypass=0，不考虑            if (transform_bypass) {                idct_dc_add  =                idct_add     = h->h264dsp.h264_add_pixels4_clear;            } else {             //常见情况                idct_dc_add = h->h264dsp.h264_idct_dc_add;                idct_add    = h->h264dsp.h264_idct_add;            }            //循环4x4=16个DCT块            for (i = 0; i < 16; i++) {             //ptr指向输出的像素数据                uint8_t *const ptr = dest_y + block_offset[i];                //dir存储了帧内预测模式                const int dir      = h->intra4x4_pred_mode_cache[scan8[i]];                if (transform_bypass && h->sps.profile_idc == 244 && dir <= 1) {                    h->hpc.pred4x4_add[dir](ptr, h->mb + (i * 16 + p * 256 << pixel_shift), linesize);                } else {                    uint8_t *topright;                    int nnz, tr;                    uint64_t tr_high;                    //这2种模式特殊的处理？                    if (dir == DIAG_DOWN_LEFT_PRED || dir == VERT_LEFT_PRED) {                        const int topright_avail = (h->topright_samples_available << i) & 0x8000;                        av_assert2(h->mb_y || linesize <= block_offset[i]);                        if (!topright_avail) {                            if (pixel_shift) {                                tr_high  = ((uint16_t *)ptr)[3 - linesize / 2] * 0x0001000100010001ULL;                                topright = (uint8_t *)&tr_high;                            } else {                                tr       = ptr[3 - linesize] * 0x01010101u;                                topright = (uint8_t *)&tr;                            }                        } else                            topright = ptr + (4 << pixel_shift) - linesize;                    } else                        topright = NULL;                    //汇编函数：4x4帧内预测（9种方式：Vertical，Horizontal，DC，Plane等等。。。）                    h->hpc.pred4x4[dir](ptr, topright, linesize);                    //每个4x4块的非0系数个数的缓存                    nnz = h->non_zero_count_cache[scan8[i + p * 16]];                    //有非0系数的时候才处理                    //h->mb中存储了DCT系数                    //输出存储在ptr                    if (nnz) {                        if (is_h264) {                            if (nnz == 1 && dctcoef_get(h->mb, pixel_shift, i * 16 + p * 256))                                idct_dc_add(ptr, h->mb + (i * 16 + p * 256 << pixel_shift), linesize);//特殊：AC系数全为0时候调用                            else                                idct_add(ptr, h->mb + (i * 16 + p * 256 << pixel_shift), linesize);//4x4DCT反变换                        } else if (CONFIG_SVQ3_DECODER)                            ff_svq3_add_idct_c(ptr, h->mb + i * 16 + p * 256, linesize, qscale, 0);                    }                }            }        }    } else {     /*      * Intra16x16帧内预测      *      * +--------+--------+      * |                 |      * |                 |      * |                 |      * +        +        +      * |                 |      * |                 |      * |                 |      * +--------+--------+      *      */     //汇编函数：16x16帧内预测（4种方式：Vertical，Horizontal，DC，Plane）        h->hpc.pred16x16[h->intra16x16_pred_mode](dest_y, linesize);        if (is_h264) {            if (h->non_zero_count_cache[scan8[LUMA_DC_BLOCK_INDEX + p]]) {             //有非0系数的时候才处理             //Hadamard反变换                //h->mb中存储了DCT系数             //h->mb_luma_dc中存储了16个DCT的直流分量                if (!transform_bypass)                    h->h264dsp.h264_luma_dc_dequant_idct(h->mb + (p * 256 << pixel_shift),                                                         h->mb_luma_dc[p],                                                         h->dequant4_coeff[p][qscale][0]);                //注：此处仅仅进行了Hadamard反变换，并未进行DCT反变换                //Intra16x16在解码过程中的DCT反变换并不是在这里进行，而是在后面进行                else {                    static const uint8_t dc_mapping[16] = {                         0 * 16,  1 * 16,  4 * 16,  5 * 16,                         2 * 16,  3 * 16,  6 * 16,  7 * 16,                         8 * 16,  9 * 16, 12 * 16, 13 * 16,                        10 * 16, 11 * 16, 14 * 16, 15 * 16                    };                    for (i = 0; i < 16; i++)                        dctcoef_set(h->mb + (p * 256 << pixel_shift),                                    pixel_shift, dc_mapping[i],                                    dctcoef_get(h->mb_luma_dc[p],                                                pixel_shift, i));                }            }        } else if (CONFIG_SVQ3_DECODER)            ff_svq3_luma_dc_dequant_idct_c(h->mb + p * 256,                                           h->mb_luma_dc[p], qscale);    }}
1

Let's sort out the backbone of hl_decode_mb_predict_luma() according to the original code:
(1) If the macroblock is of 4x4 intra prediction type (Intra4x4), do the following:
a) Loop through 16 4x4 blocks and do the following:
i. Read 4x4 intra prediction method from intra4x4_pred_mode_cache
ii. Call the assembly function pred4x4() in H264PredContext for intra-frame prediction according to the intra-frame prediction method
iii. Call the assembly function h264_idct_add() in H264DSPContext to perform 4x4DCT inverse transformation on the DCT residual data; if the DCT coefficients do not contain AC coefficients, call the assembly function h264_idct_dc_add() to perform 4x4DCT inverse transformation on the residual data (faster speed). ).
(2) If the macroblock is of 16x16 intra prediction type (Intra4x4), proceed as follows:
a) Get 16x16 intra prediction method by intra16x16_pred_mode
b) Call the assembly function pred16x16 () in H264PredContext to perform intra-frame prediction according to the intra-frame prediction method
c) Call the assembly function h264_luma_dc_dequant_idct () in H264DSPContext to perform Hadamard inverse transformation on the DC coefficients of 16 small blocks
 It should be noted here that the intra-frame 4x4 macroblock has actually completed the process of "intra-frame prediction + DCT inverse transform" after executing hl_decode_mb_predict_luma() (decoding is completed); while the intra-frame 16x16 macroblock is executed after hl_decode_mb_predict_luma After ( ), only the process of "intra prediction + Hadamard inverse transformation" is completed, but the step of "DCT inverse transformation" is not performed, and this step needs to be completed in the subsequent steps.
 The assembly functions involved in the above process are recorded below (the function of DCT inverse transformation is not recorded here, and will be described later):
4x4 intra prediction assembly function: H264PredContext -> pred4x4[dir]()
16x16 intra prediction assembly function: H264PredContext -> pred16x16[dir]()
Hadamard inverse transformation assembly function: H264DSPContext->h264_luma_dc_dequant_idct()

Intra-frame prediction tips
Intra-frame prediction calculates the pixel values ​​inside the macroblock based on the boundary pixel values ​​on the left and top of the macroblock. The effect of intra-frame prediction is shown in the following figure. The picture on the left is the original picture of the image, and the picture on the right is the picture without the superimposed residual after intra-frame prediction.
  
There are two intra prediction modes in H.264: 16x16 luma intra prediction mode and 4x4 luma intra prediction mode. Among them, there are 4 types of 16x16 intra-frame prediction modes, as shown in the following figure.
  
The 4 modes are listed below.
         
model

describe

Vertical

The corresponding pixel value is derived from the upper pixel

Horizontal

The corresponding pixel value is derived from the left pixel

DC

The corresponding pixel value is derived from the average value of the upper and left pixels

Plane

The corresponding pixel value is derived from the upper and left pixels


There are a total of 9 4x4 intra prediction modes, as shown in the figure below.
  
As can be seen from the figure, the first 4 of these 9 modes are the same as the 16x16 intra prediction method. Several unique orientations have been added later - the arrow is no longer in the "mouth", but in the "day".



Intra prediction assembly function initialization
 The 4x4 intra prediction function pointer in the FFmpeg H.264 decoder is located in the pred4x4[] array of H264PredContext, where each element points to a 4x4 intra prediction mode. The 16x16 intra-frame prediction function pointer is located in the pred16x16[] array of H264PredContext, where each element points to a 16x16 intra-frame prediction mode.
 When the FFmpeg H.264 decoder is initialized, ff_h264_pred_init() will be called to assign these intra prediction function pointers in the H264PredContext according to the system configuration. Let's take a brief look at the definition of ff_h264_pred_init().

ff_h264_pred_init()
The definition of ff_h264_pred_init() is located in libavcodec\h264pred.c as shown below.
/** * Set the intra prediction function pointers. *///初始化帧内预测相关的汇编函数av_cold void ff_h264_pred_init(H264PredContext *h, int codec_id,                               const int bit_depth,                               int chroma_format_idc){#undef FUNC#undef FUNCC#define FUNC(a, depth) a ## _ ## depth#define FUNCC(a, depth) a ## _ ## depth ## _c#define FUNCD(a) a ## _c//好长的宏定义...（这种很长的宏定义在H.264解码器中似乎很普遍！）//该宏用于给帧内预测模块的函数指针赋值//注意参数为颜色位深度#define H264_PRED(depth) \    if(codec_id != AV_CODEC_ID_RV40){\        if (codec_id == AV_CODEC_ID_VP7 || codec_id == AV_CODEC_ID_VP8) {\            h->pred4x4[VERT_PRED       ]= FUNCD(pred4x4_vertical_vp8);\            h->pred4x4[HOR_PRED        ]= FUNCD(pred4x4_horizontal_vp8);\        } else {\            h->pred4x4[VERT_PRED       ]= FUNCC(pred4x4_vertical          , depth);\            h->pred4x4[HOR_PRED        ]= FUNCC(pred4x4_horizontal        , depth);\        }\        h->pred4x4[DC_PRED             ]= FUNCC(pred4x4_dc                , depth);\        if(codec_id == AV_CODEC_ID_SVQ3)\            h->pred4x4[DIAG_DOWN_LEFT_PRED ]= FUNCD(pred4x4_down_left_svq3);\        else\            h->pred4x4[DIAG_DOWN_LEFT_PRED ]= FUNCC(pred4x4_down_left     , depth);\        h->pred4x4[DIAG_DOWN_RIGHT_PRED]= FUNCC(pred4x4_down_right        , depth);\        h->pred4x4[VERT_RIGHT_PRED     ]= FUNCC(pred4x4_vertical_right    , depth);\        h->pred4x4[HOR_DOWN_PRED       ]= FUNCC(pred4x4_horizontal_down   , depth);\        if (codec_id == AV_CODEC_ID_VP7 || codec_id == AV_CODEC_ID_VP8) {\            h->pred4x4[VERT_LEFT_PRED  ]= FUNCD(pred4x4_vertical_left_vp8);\        } else\            h->pred4x4[VERT_LEFT_PRED  ]= FUNCC(pred4x4_vertical_left     , depth);\        h->pred4x4[HOR_UP_PRED         ]= FUNCC(pred4x4_horizontal_up     , depth);\        if (codec_id != AV_CODEC_ID_VP7 && codec_id != AV_CODEC_ID_VP8) {\            h->pred4x4[LEFT_DC_PRED    ]= FUNCC(pred4x4_left_dc           , depth);\            h->pred4x4[TOP_DC_PRED     ]= FUNCC(pred4x4_top_dc            , depth);\        } else {\            h->pred4x4[TM_VP8_PRED     ]= FUNCD(pred4x4_tm_vp8);\            h->pred4x4[DC_127_PRED     ]= FUNCC(pred4x4_127_dc            , depth);\            h->pred4x4[DC_129_PRED     ]= FUNCC(pred4x4_129_dc            , depth);\            h->pred4x4[VERT_VP8_PRED   ]= FUNCC(pred4x4_vertical          , depth);\            h->pred4x4[HOR_VP8_PRED    ]= FUNCC(pred4x4_horizontal        , depth);\        }\        if (codec_id != AV_CODEC_ID_VP8)\            h->pred4x4[DC_128_PRED     ]= FUNCC(pred4x4_128_dc            , depth);\    }else{\        h->pred4x4[VERT_PRED           ]= FUNCC(pred4x4_vertical          , depth);\        h->pred4x4[HOR_PRED            ]= FUNCC(pred4x4_horizontal        , depth);\        h->pred4x4[DC_PRED             ]= FUNCC(pred4x4_dc                , depth);\        h->pred4x4[DIAG_DOWN_LEFT_PRED ]= FUNCD(pred4x4_down_left_rv40);\        h->pred4x4[DIAG_DOWN_RIGHT_PRED]= FUNCC(pred4x4_down_right        , depth);\        h->pred4x4[VERT_RIGHT_PRED     ]= FUNCC(pred4x4_vertical_right    , depth);\        h->pred4x4[HOR_DOWN_PRED       ]= FUNCC(pred4x4_horizontal_down   , depth);\        h->pred4x4[VERT_LEFT_PRED      ]= FUNCD(pred4x4_vertical_left_rv40);\        h->pred4x4[HOR_UP_PRED         ]= FUNCD(pred4x4_horizontal_up_rv40);\        h->pred4x4[LEFT_DC_PRED        ]= FUNCC(pred4x4_left_dc           , depth);\        h->pred4x4[TOP_DC_PRED         ]= FUNCC(pred4x4_top_dc            , depth);\        h->pred4x4[DC_128_PRED         ]= FUNCC(pred4x4_128_dc            , depth);\        h->pred4x4[DIAG_DOWN_LEFT_PRED_RV40_NODOWN]= FUNCD(pred4x4_down_left_rv40_nodown);\        h->pred4x4[HOR_UP_PRED_RV40_NODOWN]= FUNCD(pred4x4_horizontal_up_rv40_nodown);\        h->pred4x4[VERT_LEFT_PRED_RV40_NODOWN]= FUNCD(pred4x4_vertical_left_rv40_nodown);\    }\\    h->pred8x8l[VERT_PRED           ]= FUNCC(pred8x8l_vertical            , depth);\    h->pred8x8l[HOR_PRED            ]= FUNCC(pred8x8l_horizontal          , depth);\    h->pred8x8l[DC_PRED             ]= FUNCC(pred8x8l_dc                  , depth);\    h->pred8x8l[DIAG_DOWN_LEFT_PRED ]= FUNCC(pred8x8l_down_left           , depth);\    h->pred8x8l[DIAG_DOWN_RIGHT_PRED]= FUNCC(pred8x8l_down_right          , depth);\    h->pred8x8l[VERT_RIGHT_PRED     ]= FUNCC(pred8x8l_vertical_right      , depth);\    h->pred8x8l[HOR_DOWN_PRED       ]= FUNCC(pred8x8l_horizontal_down     , depth);\    h->pred8x8l[VERT_LEFT_PRED      ]= FUNCC(pred8x8l_vertical_left       , depth);\    h->pred8x8l[HOR_UP_PRED         ]= FUNCC(pred8x8l_horizontal_up       , depth);\    h->pred8x8l[LEFT_DC_PRED        ]= FUNCC(pred8x8l_left_dc             , depth);\    h->pred8x8l[TOP_DC_PRED         ]= FUNCC(pred8x8l_top_dc              , depth);\    h->pred8x8l[DC_128_PRED         ]= FUNCC(pred8x8l_128_dc              , depth);\\    if (chroma_format_idc <= 1) {\        h->pred8x8[VERT_PRED8x8   ]= FUNCC(pred8x8_vertical               , depth);\        h->pred8x8[HOR_PRED8x8    ]= FUNCC(pred8x8_horizontal             , depth);\    } else {\        h->pred8x8[VERT_PRED8x8   ]= FUNCC(pred8x16_vertical              , depth);\        h->pred8x8[HOR_PRED8x8    ]= FUNCC(pred8x16_horizontal            , depth);\    }\    if (codec_id != AV_CODEC_ID_VP7 && codec_id != AV_CODEC_ID_VP8) {\        if (chroma_format_idc <= 1) {\            h->pred8x8[PLANE_PRED8x8]= FUNCC(pred8x8_plane                , depth);\        } else {\            h->pred8x8[PLANE_PRED8x8]= FUNCC(pred8x16_plane               , depth);\        }\    } else\        h->pred8x8[PLANE_PRED8x8]= FUNCD(pred8x8_tm_vp8);\    if (codec_id != AV_CODEC_ID_RV40 && codec_id != AV_CODEC_ID_VP7 && \        codec_id != AV_CODEC_ID_VP8) {\        if (chroma_format_idc <= 1) {\            h->pred8x8[DC_PRED8x8     ]= FUNCC(pred8x8_dc                     , depth);\            h->pred8x8[LEFT_DC_PRED8x8]= FUNCC(pred8x8_left_dc                , depth);\            h->pred8x8[TOP_DC_PRED8x8 ]= FUNCC(pred8x8_top_dc                 , depth);\            h->pred8x8[ALZHEIMER_DC_L0T_PRED8x8 ]= FUNC(pred8x8_mad_cow_dc_l0t, depth);\            h->pred8x8[ALZHEIMER_DC_0LT_PRED8x8 ]= FUNC(pred8x8_mad_cow_dc_0lt, depth);\            h->pred8x8[ALZHEIMER_DC_L00_PRED8x8 ]= FUNC(pred8x8_mad_cow_dc_l00, depth);\            h->pred8x8[ALZHEIMER_DC_0L0_PRED8x8 ]= FUNC(pred8x8_mad_cow_dc_0l0, depth);\        } else {\            h->pred8x8[DC_PRED8x8     ]= FUNCC(pred8x16_dc                    , depth);\            h->pred8x8[LEFT_DC_PRED8x8]= FUNCC(pred8x16_left_dc               , depth);\            h->pred8x8[TOP_DC_PRED8x8 ]= FUNCC(pred8x16_top_dc                , depth);\            h->pred8x8[ALZHEIMER_DC_L0T_PRED8x8 ]= FUNC(pred8x16_mad_cow_dc_l0t, depth);\            h->pred8x8[ALZHEIMER_DC_0LT_PRED8x8 ]= FUNC(pred8x16_mad_cow_dc_0lt, depth);\            h->pred8x8[ALZHEIMER_DC_L00_PRED8x8 ]= FUNC(pred8x16_mad_cow_dc_l00, depth);\            h->pred8x8[ALZHEIMER_DC_0L0_PRED8x8 ]= FUNC(pred8x16_mad_cow_dc_0l0, depth);\        }\    }else{\        h->pred8x8[DC_PRED8x8     ]= FUNCD(pred8x8_dc_rv40);\        h->pred8x8[LEFT_DC_PRED8x8]= FUNCD(pred8x8_left_dc_rv40);\        h->pred8x8[TOP_DC_PRED8x8 ]= FUNCD(pred8x8_top_dc_rv40);\        if (codec_id == AV_CODEC_ID_VP7 || codec_id == AV_CODEC_ID_VP8) {\            h->pred8x8[DC_127_PRED8x8]= FUNCC(pred8x8_127_dc              , depth);\            h->pred8x8[DC_129_PRED8x8]= FUNCC(pred8x8_129_dc              , depth);\        }\    }\    if (chroma_format_idc <= 1) {\        h->pred8x8[DC_128_PRED8x8 ]= FUNCC(pred8x8_128_dc                 , depth);\    } else {\        h->pred8x8[DC_128_PRED8x8 ]= FUNCC(pred8x16_128_dc                , depth);\    }\\    h->pred16x16[DC_PRED8x8     ]= FUNCC(pred16x16_dc                     , depth);\    h->pred16x16[VERT_PRED8x8   ]= FUNCC(pred16x16_vertical               , depth);\    h->pred16x16[HOR_PRED8x8    ]= FUNCC(pred16x16_horizontal             , depth);\    switch(codec_id){\    case AV_CODEC_ID_SVQ3:\       h->pred16x16[PLANE_PRED8x8  ]= FUNCD(pred16x16_plane_svq3);\       break;\    case AV_CODEC_ID_RV40:\       h->pred16x16[PLANE_PRED8x8  ]= FUNCD(pred16x16_plane_rv40);\       break;\    case AV_CODEC_ID_VP7:\    case AV_CODEC_ID_VP8:\       h->pred16x16[PLANE_PRED8x8  ]= FUNCD(pred16x16_tm_vp8);\       h->pred16x16[DC_127_PRED8x8]= FUNCC(pred16x16_127_dc               , depth);\       h->pred16x16[DC_129_PRED8x8]= FUNCC(pred16x16_129_dc               , depth);\       break;\    default:\       h->pred16x16[PLANE_PRED8x8  ]= FUNCC(pred16x16_plane               , depth);\       break;\    }\    h->pred16x16[LEFT_DC_PRED8x8]= FUNCC(pred16x16_left_dc                , depth);\    h->pred16x16[TOP_DC_PRED8x8 ]= FUNCC(pred16x16_top_dc                 , depth);\    h->pred16x16[DC_128_PRED8x8 ]= FUNCC(pred16x16_128_dc                 , depth);\\    /* special lossless h/v prediction for h264 */ \    h->pred4x4_add  [VERT_PRED   ]= FUNCC(pred4x4_vertical_add            , depth);\    h->pred4x4_add  [ HOR_PRED   ]= FUNCC(pred4x4_horizontal_add          , depth);\    h->pred8x8l_add [VERT_PRED   ]= FUNCC(pred8x8l_vertical_add           , depth);\    h->pred8x8l_add [ HOR_PRED   ]= FUNCC(pred8x8l_horizontal_add         , depth);\    h->pred8x8l_filter_add [VERT_PRED   ]= FUNCC(pred8x8l_vertical_filter_add           , depth);\    h->pred8x8l_filter_add [ HOR_PRED   ]= FUNCC(pred8x8l_horizontal_filter_add         , depth);\    if (chroma_format_idc <= 1) {\    h->pred8x8_add  [VERT_PRED8x8]= FUNCC(pred8x8_vertical_add            , depth);\    h->pred8x8_add  [ HOR_PRED8x8]= FUNCC(pred8x8_horizontal_add          , depth);\    } else {\        h->pred8x8_add  [VERT_PRED8x8]= FUNCC(pred8x16_vertical_add            , depth);\        h->pred8x8_add  [ HOR_PRED8x8]= FUNCC(pred8x16_horizontal_add          , depth);\    }\    h->pred16x16_add[VERT_PRED8x8]= FUNCC(pred16x16_vertical_add          , depth);\    h->pred16x16_add[ HOR_PRED8x8]= FUNCC(pred16x16_horizontal_add        , depth);\    //注意这里使用了前面那个很长的宏定义 //根据颜色位深的不同，初始化不同的函数    //颜色位深默认值为8，所以一般情况下调用H264_PRED(8)    switch (bit_depth) {        case 9:            H264_PRED(9)            break;        case 10:            H264_PRED(10)            break;        case 12:            H264_PRED(12)            break;        case 14:            H264_PRED(14)            break;        default:            av_assert0(bit_depth<=8);            H264_PRED(8)            break;    }    //如果支持汇编优化，则会调用相应的汇编优化函数    //neon这些的    if (ARCH_ARM) ff_h264_pred_init_arm(h, codec_id, bit_depth, chroma_format_idc);    //mmx这些的    if (ARCH_X86) ff_h264_pred_init_x86(h, codec_id, bit_depth, chroma_format_idc);}
1

 As can be seen from the source code, the ff_h264_pred_init() function contains a very long macro definition named "H264_PRED(depth)". The macro definition contains the initialization code of the C language version of the intra prediction function. ff_h264_pred_init() will initialize the corresponding C language version of the intra prediction function according to the color bit depth bit_depth of the system. At the end of the function, the initialization function of the assembly function is included: if the system is based on the ARM architecture, ff_h264_pred_init_arm() will be called to initialize the assembly-optimized intra-frame prediction function under the ARM platform; if the system is based on the X86 architecture, ff_h264_pred_init_x86 will be called () Initialize the assembly-optimized intra prediction function under the X86 platform.
Let's take a look at the C language version of the intra prediction function.

C language version intra prediction function
The "H264_PRED(depth)" macro is used to initialize the C language version of the intra prediction function. where "depth" represents the color bit depth. Taking the most common 8bit bit depth as an example, the code after expanding the "H264_PRED(8)" macro definition is as follows.
if(codec_id != AV_CODEC_ID_RV40){ if (codec_id == AV_CODEC_ID_VP7 || codec_id == AV_CODEC_ID_VP8) {  h->pred4x4[0       ]= pred4x4_vertical_vp8_c;  h->pred4x4[1        ]= pred4x4_horizontal_vp8_c; } else {  //帧内4x4的Vertical预测方式  h->pred4x4[0       ]= pred4x4_vertical_8_c;  //帧内4x4的Horizontal预测方式  h->pred4x4[1        ]= pred4x4_horizontal_8_c; } //帧内4x4的DC预测方式 h->pred4x4[2             ]= pred4x4_dc_8_c; if(codec_id == AV_CODEC_ID_SVQ3)  h->pred4x4[3 ]= pred4x4_down_left_svq3_c; else  h->pred4x4[3 ]= pred4x4_down_left_8_c; h->pred4x4[4]= pred4x4_down_right_8_c; h->pred4x4[5     ]= pred4x4_vertical_right_8_c; h->pred4x4[6       ]= pred4x4_horizontal_down_8_c; if (codec_id == AV_CODEC_ID_VP7 || codec_id == AV_CODEC_ID_VP8) {  h->pred4x4[7  ]= pred4x4_vertical_left_vp8_c; } else  h->pred4x4[7  ]= pred4x4_vertical_left_8_c; h->pred4x4[8         ]= pred4x4_horizontal_up_8_c; if (codec_id != AV_CODEC_ID_VP7 && codec_id != AV_CODEC_ID_VP8) {  h->pred4x4[9    ]= pred4x4_left_dc_8_c;  h->pred4x4[10     ]= pred4x4_top_dc_8_c; } else {  h->pred4x4[9     ]= pred4x4_tm_vp8_c;  h->pred4x4[12     ]= pred4x4_127_dc_8_c;  h->pred4x4[13     ]= pred4x4_129_dc_8_c;  h->pred4x4[10   ]= pred4x4_vertical_8_c;  h->pred4x4[14    ]= pred4x4_horizontal_8_c; } if (codec_id != AV_CODEC_ID_VP8)  h->pred4x4[11     ]= pred4x4_128_dc_8_c;}else{ h->pred4x4[0           ]= pred4x4_vertical_8_c; h->pred4x4[1            ]= pred4x4_horizontal_8_c; h->pred4x4[2             ]= pred4x4_dc_8_c; h->pred4x4[3 ]= pred4x4_down_left_rv40_c; h->pred4x4[4]= pred4x4_down_right_8_c; h->pred4x4[5     ]= pred4x4_vertical_right_8_c; h->pred4x4[6       ]= pred4x4_horizontal_down_8_c; h->pred4x4[7      ]= pred4x4_vertical_left_rv40_c; h->pred4x4[8         ]= pred4x4_horizontal_up_rv40_c; h->pred4x4[9        ]= pred4x4_left_dc_8_c; h->pred4x4[10         ]= pred4x4_top_dc_8_c; h->pred4x4[11         ]= pred4x4_128_dc_8_c; h->pred4x4[12]= pred4x4_down_left_rv40_nodown_c; h->pred4x4[13]= pred4x4_horizontal_up_rv40_nodown_c; h->pred4x4[14]= pred4x4_vertical_left_rv40_nodown_c;}h->pred8x8l[0           ]= pred8x8l_vertical_8_c;h->pred8x8l[1            ]= pred8x8l_horizontal_8_c;h->pred8x8l[2             ]= pred8x8l_dc_8_c;h->pred8x8l[3 ]= pred8x8l_down_left_8_c;h->pred8x8l[4]= pred8x8l_down_right_8_c;h->pred8x8l[5     ]= pred8x8l_vertical_right_8_c;h->pred8x8l[6       ]= pred8x8l_horizontal_down_8_c;h->pred8x8l[7      ]= pred8x8l_vertical_left_8_c;h->pred8x8l[8         ]= pred8x8l_horizontal_up_8_c;h->pred8x8l[9        ]= pred8x8l_left_dc_8_c;h->pred8x8l[10         ]= pred8x8l_top_dc_8_c;h->pred8x8l[11         ]= pred8x8l_128_dc_8_c;if (chroma_format_idc <= 1) { h->pred8x8[2   ]= pred8x8_vertical_8_c; h->pred8x8[1    ]= pred8x8_horizontal_8_c;} else { h->pred8x8[2   ]= pred8x16_vertical_8_c; h->pred8x8[1    ]= pred8x16_horizontal_8_c;}if (codec_id != AV_CODEC_ID_VP7 && codec_id != AV_CODEC_ID_VP8) { if (chroma_format_idc <= 1) {  h->pred8x8[3]= pred8x8_plane_8_c; } else {  h->pred8x8[3]= pred8x16_plane_8_c; }} else h->pred8x8[3]= pred8x8_tm_vp8_c;if (codec_id != AV_CODEC_ID_RV40 && codec_id != AV_CODEC_ID_VP7 &&  codec_id != AV_CODEC_ID_VP8) { if (chroma_format_idc <= 1) {  h->pred8x8[0     ]= pred8x8_dc_8_c;  h->pred8x8[4]= pred8x8_left_dc_8_c;  h->pred8x8[5 ]= pred8x8_top_dc_8_c;  h->pred8x8[7 ]= pred8x8_mad_cow_dc_l0t_8;  h->pred8x8[8 ]= pred8x8_mad_cow_dc_0lt_8;  h->pred8x8[9 ]= pred8x8_mad_cow_dc_l00_8;  h->pred8x8[10 ]= pred8x8_mad_cow_dc_0l0_8; } else {  h->pred8x8[0     ]= pred8x16_dc_8_c;  h->pred8x8[4]= pred8x16_left_dc_8_c;  h->pred8x8[5 ]= pred8x16_top_dc_8_c;  h->pred8x8[7 ]= pred8x16_mad_cow_dc_l0t_8;  h->pred8x8[8 ]= pred8x16_mad_cow_dc_0lt_8;  h->pred8x8[9 ]= pred8x16_mad_cow_dc_l00_8;  h->pred8x8[10 ]= pred8x16_mad_cow_dc_0l0_8; }}else{ h->pred8x8[0     ]= pred8x8_dc_rv40_c; h->pred8x8[4]= pred8x8_left_dc_rv40_c; h->pred8x8[5 ]= pred8x8_top_dc_rv40_c; if (codec_id == AV_CODEC_ID_VP7 || codec_id == AV_CODEC_ID_VP8) {  h->pred8x8[7]= pred8x8_127_dc_8_c;  h->pred8x8[8]= pred8x8_129_dc_8_c; }}if (chroma_format_idc <= 1) { h->pred8x8[6 ]= pred8x8_128_dc_8_c;} else { h->pred8x8[6 ]= pred8x16_128_dc_8_c;}h->pred16x16[0     ]= pred16x16_dc_8_c;h->pred16x16[2   ]= pred16x16_vertical_8_c;h->pred16x16[1    ]= pred16x16_horizontal_8_c;switch(codec_id){case AV_CODEC_ID_SVQ3:   h->pred16x16[3  ]= pred16x16_plane_svq3_c;   break;case AV_CODEC_ID_RV40:   h->pred16x16[3  ]= pred16x16_plane_rv40_c;   break;case AV_CODEC_ID_VP7:case AV_CODEC_ID_VP8:   h->pred16x16[3  ]= pred16x16_tm_vp8_c;   h->pred16x16[7]= pred16x16_127_dc_8_c;   h->pred16x16[8]= pred16x16_129_dc_8_c;   break;default:   h->pred16x16[3  ]= pred16x16_plane_8_c;   break;}h->pred16x16[4]= pred16x16_left_dc_8_c;h->pred16x16[5 ]= pred16x16_top_dc_8_c;h->pred16x16[6 ]= pred16x16_128_dc_8_c;/* special lossless h/v prediction for h264 */ h->pred4x4_add  [0   ]= pred4x4_vertical_add_8_c;h->pred4x4_add  [ 1   ]= pred4x4_horizontal_add_8_c;h->pred8x8l_add [0   ]= pred8x8l_vertical_add_8_c;h->pred8x8l_add [ 1   ]= pred8x8l_horizontal_add_8_c;h->pred8x8l_filter_add [0   ]= pred8x8l_vertical_filter_add_8_c;h->pred8x8l_filter_add [ 1   ]= pred8x8l_horizontal_filter_add_8_c;if (chroma_format_idc <= 1) {h->pred8x8_add  [2]= pred8x8_vertical_add_8_c;h->pred8x8_add  [ 1]= pred8x8_horizontal_add_8_c;} else { h->pred8x8_add  [2]= pred8x16_vertical_add_8_c; h->pred8x8_add  [ 1]= pred8x16_horizontal_add_8_c;}h->pred16x16_add[2]= pred16x16_vertical_add_8_c;h->pred16x16_add[ 1]= pred16x16_horizontal_add_8_c;
1

It can be seen that in the code expanded by H264_PRED(8), the function pointers of the intra prediction module are all assigned the function of xxxx_8_c(). For example pred4x4[0] (intra 4x4 mode 0) is assigned pred4x4_vertical_8_c(); pred4x4[1] (intra 4x4 mode 1) is assigned pred4x4_horizontal_8_c(); pred4x4[2] (intra 4x4 mode 2) ) is assigned to pred4x4_cd_8_c() as shown below.
//帧内4x4的Vertical预测方式h->pred4x4[0]= pred4x4_vertical_8_c;//帧内4x4的Horizontal预测方式h->pred4x4[1]= pred4x4_horizontal_8_c;//帧内4x4的DC预测方式h->pred4x4[2]= pred4x4_dc_8_c;
1

Let's take a look at the code for these 4x4 intra prediction functions.



4x4 intra prediction assembly function: H264PredContext -> pred4x4[dir]()
The 4x4 intra prediction function pointer is located in the pred4x4[] array of the H264PredContext. Each element in the pred4x4[] array points to an intra prediction mode. The intra prediction function implemented by the three intra prediction functions mentioned above is shown in the following figure.

Let's take a look at the three 4x4 intra prediction functions mentioned above.

pred4x4_vertical_8_c()
pred4x4_vertical_8_c() implements intra prediction in 4x4 block Vertical mode. The definition of this function is located in libavcodec\h264pred_template.c, as shown below.
/* 帧内预测 * * 注释：雷霄骅 * leixiaohua1020@126.com * http://blog.csdn.net/leixiaohua1020 * * 参数： * _src：输入数据 * _stride：一行像素的大小 * *///垂直预测//由上边像素推出像素值static void FUNCC(pred4x4_vertical)(uint8_t *_src, const uint8_t *topright,                                    ptrdiff_t _stride){    pixel *src = (pixel*)_src;    int stride = _stride>>(sizeof(pixel)-1);    /*     * Vertical预测方式     *   |X1 X2 X3 X4     * --+-----------     *   |X1 X2 X3 X4     *   |X1 X2 X3 X4     *   |X1 X2 X3 X4     *   |X1 X2 X3 X4     *     */    //pixel4代表4个像素值。1个像素值占用8bit，4个像素值占用32bit。    const pixel4 a= AV_RN4PA(src-stride);    /* 宏定义展开后：     * const uint32_t a=(((const av_alias32*)(src-stride))->u32);     * 注：av_alias32是一个union类型的变量，存储4byte数据。     * -stride代表了上一行对应位置的像素     * 即a取的是上1行像素的值。     */    AV_WN4PA(src+0*stride, a);    AV_WN4PA(src+1*stride, a);    AV_WN4PA(src+2*stride, a);    AV_WN4PA(src+3*stride, a);    /* 宏定义展开后：     * (((av_alias32*)(src+0*stride))->u32 = (a));     * (((av_alias32*)(src+1*stride))->u32 = (a));     * (((av_alias32*)(src+2*stride))->u32 = (a));     * (((av_alias32*)(src+3*stride))->u32 = (a));     * 即把a的值赋给下面4行。     */}
1

As can be seen from the source code, pred4x4_vertical_8_c() first takes the 4 pixels of the previous line of the current 4x4 block and stores them in the a variable, and then assigns the value of the a variable to the 4 lines of the current block respectively. One thing to note here: stride represents the size of a row of pixels, and "src+stride" represents the pixel directly below src.

pred4x4_horizontal_8_c()
pred4x4_horizontal_8_c() implements intra prediction in 4x4 block Horizontal mode, the definition of this function is located in libavcodec\h264pred_template.c, as shown below.
//水平预测//由左边像素推出像素值static void FUNCC(pred4x4_horizontal)(uint8_t *_src, const uint8_t *topright,                                      ptrdiff_t _stride){    pixel *src = (pixel*)_src;    int stride = _stride>>(sizeof(pixel)-1);    /*     * Horizontal预测方式     *   |     * --+-----------     * X5|X5 X5 X5 X5     * X6|X6 X6 X6 X6     * X7|X7 X7 X7 X7     * X8|X8 X8 X8 X8     *     */    AV_WN4PA(src+0*stride, PIXEL_SPLAT_X4(src[-1+0*stride]));    AV_WN4PA(src+1*stride, PIXEL_SPLAT_X4(src[-1+1*stride]));    AV_WN4PA(src+2*stride, PIXEL_SPLAT_X4(src[-1+2*stride]));    AV_WN4PA(src+3*stride, PIXEL_SPLAT_X4(src[-1+3*stride]));    /* 宏定义展开后：     * (((av_alias32*)(src+0*stride))->u32 = (((src[-1+0*stride])*0x01010101U)));     * (((av_alias32*)(src+1*stride))->u32 = (((src[-1+1*stride])*0x01010101U)));     * (((av_alias32*)(src+2*stride))->u32 = (((src[-1+2*stride])*0x01010101U)));     * (((av_alias32*)(src+3*stride))->u32 = (((src[-1+3*stride])*0x01010101U)));     *     * PIXEL_SPLAT_X4()的作用应该是把最后一个像素（最后8位）拷贝给前面3个像素（前24位）     * 即把0x0100009F变成0x9F9F9F9F     * 推导：     * 前提是x占8bit（对应1个像素）     * y=x*0x01010101     *  =x*(0x00000001+0x00000100+0x00010000+0x01000000)     *  =x<<0+x<<8+x<<16+x<<24     *     * 每行把src[-1]中像素值例如0x02赋值给src[0]开始的4个像素中，形成0x02020202     */}
1

As can be seen from the source code, pred4x4_horizontal_8_c() copies one pixel to the left of each row of pixels in the 4x4 block and assigns it to the current row. The definition of the PIXEL_SPLAT_X4() macro is as follows:
#   define PIXEL_SPLAT_X4(x) ((x)*0x01010101U)
1
After research, it is found that this macro is used to copy the last 8 bits of the 32bit data into 3 copies of the original data 8-16 bits, 16-24 bits and 24-32 bits, that is, "copy" 3 copies of the low-order 8-bit data to the high-order bits. superior. The detailed derivation process has been written in the code comments and will not be repeated.

pred4x4_dc_8_c()
pred4x4_dc_8_c() implements intra prediction in 4x4 block DC mode, the definition of this function is located in libavcodec\h264pred_template.c, as shown below.
//DC预测//由左边和上边像素平均值推出像素值static void FUNCC(pred4x4_dc)(uint8_t *_src, const uint8_t *topright,                              ptrdiff_t _stride){    pixel *src = (pixel*)_src;    int stride = _stride>>(sizeof(pixel)-1);    /*     * DC预测方式     *   |X1 X2 X3 X4     * --+-----------     * X5|     * X6|     Y     * X7|     * X8|     *     * Y=(X1+X2+X3+X4+X5+X6+X7+X8)/8     */    const int dc= (  src[-stride] + src[1-stride] + src[2-stride] + src[3-stride]                   + src[-1+0*stride] + src[-1+1*stride] + src[-1+2*stride] + src[-1+
1

————————————————
版权声明：本文为CSDN博主「比较清纯」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/hffgjh/article/details/86888295










*********************************************************************************************************************************************************************************

 As can be seen from the figure, there are two functions in FFmpeg's entropy decoding: ff_h264_decode_mb_cabac() and ff_h264_decode_mb_cavlc(). ff_h264_decode_mb_cabac() is used to decode H.264 data in CABAC encoding, and ff_h264_decode_mb_cavlc() is used to decode H.264 data in CAVLC encoding. This article selects the ff_h264_decode_mb_cavlc() function for analysis.
 ff_h264_decode_mb_cavlc() calls many functions for reading exponential Golomb encoded data, such as get_ue_golomb_long(), get_ue_golomb(), get_se_golomb(), get_ue_golomb_31(), etc. In addition, when decoding the residual data, the decode_residual() function is called, and decode_residual() will call the get_vlc2() function to read the CAVLC encoded data.
 All in all, the role of the "entropy decoding" part is to read the data (macroblock type, motion vector, reference frame, residual, etc.) and assign it to the corresponding H.264 decoder in the FFmpeg H.264 decoder according to the H.264 syntax and semantics. on the variable. It should be noted that the "entropy decoding" part does not use these variables to restore the video data. The function of restoring the video data is done in the next step "macroblock decoding".
 Let's review the decode_slice() function before we start looking at ff_h264_decode_mb_cavlc().


decode_slice()
decode_slice() is used to decode H.264 slices. This function completes the functions of "entropy decoding", "macroblock decoding" and "loop filtering". Its definition is located in libavcodec\h264_slice.c as shown below.
//解码slice
//三个主要步骤：
//1.熵解码（CAVLC/CABAC）
//2.宏块解码
//3.环路滤波
//此外还包含了错误隐藏代码
static int decode_slice(struct AVCodecContext *avctx, void *arg)
{
    H264Context *h = *(void **)arg;
    int lf_x_start = h->mb_x;
 
    h->mb_skip_run = -1;
 
    av_assert0(h->block_offset[15] == (4 * ((scan8[15] - scan8[0]) & 7) << h->pixel_shift) + 4 * h->linesize * ((scan8[15] - scan8[0]) >> 3));
 
    h->is_complex = FRAME_MBAFF(h) || h->picture_structure != PICT_FRAME ||
                    avctx->codec_id != AV_CODEC_ID_H264 ||
                    (CONFIG_GRAY && (h->flags & CODEC_FLAG_GRAY));
 
    if (!(h->avctx->active_thread_type & FF_THREAD_SLICE) && h->picture_structure == PICT_FRAME && h->er.error_status_table) {
        const int start_i  = av_clip(h->resync_mb_x + h->resync_mb_y * h->mb_width, 0, h->mb_num - 1);
        if (start_i) {
            int prev_status = h->er.error_status_table[h->er.mb_index2xy[start_i - 1]];
            prev_status &= ~ VP_START;
            if (prev_status != (ER_MV_END | ER_DC_END | ER_AC_END))
                h->er.error_occurred = 1;
        }
    }
    //CABAC情况
    if (h->pps.cabac) {
        /* realign */
        align_get_bits(&h->gb);
 
        /* init cabac */
        //初始化CABAC解码器
        ff_init_cabac_decoder(&h->cabac,
                              h->gb.buffer + get_bits_count(&h->gb) / 8,
                              (get_bits_left(&h->gb) + 7) / 8);
 
        ff_h264_init_cabac_states(h);
        //循环处理每个宏块
        for (;;) {
            // START_TIMER
        	//解码CABAC数据
            int ret = ff_h264_decode_mb_cabac(h);
            int eos;
            // STOP_TIMER("decode_mb_cabac")
            //解码宏块
            if (ret >= 0)
                ff_h264_hl_decode_mb(h);
 
            // FIXME optimal? or let mb_decode decode 16x32 ?
            //宏块级帧场自适应。很少接触
            if (ret >= 0 && FRAME_MBAFF(h)) {
                h->mb_y++;
 
                ret = ff_h264_decode_mb_cabac(h);
                //解码宏块
                if (ret >= 0)
                    ff_h264_hl_decode_mb(h);
                h->mb_y--;
            }
            eos = get_cabac_terminate(&h->cabac);
 
            if ((h->workaround_bugs & FF_BUG_TRUNCATED) &&
                h->cabac.bytestream > h->cabac.bytestream_end + 2) {
            	//错误隐藏
                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x - 1,
                             h->mb_y, ER_MB_END);
                if (h->mb_x >= lf_x_start)
                    loop_filter(h, lf_x_start, h->mb_x + 1);
                return 0;
            }
            if (h->cabac.bytestream > h->cabac.bytestream_end + 2 )
                av_log(h->avctx, AV_LOG_DEBUG, "bytestream overread %"PTRDIFF_SPECIFIER"\n", h->cabac.bytestream_end - h->cabac.bytestream);
            if (ret < 0 || h->cabac.bytestream > h->cabac.bytestream_end + 4) {
                av_log(h->avctx, AV_LOG_ERROR,
                       "error while decoding MB %d %d, bytestream %"PTRDIFF_SPECIFIER"\n",
                       h->mb_x, h->mb_y,
                       h->cabac.bytestream_end - h->cabac.bytestream);
                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x,
                             h->mb_y, ER_MB_ERROR);
                return AVERROR_INVALIDDATA;
            }
            //mb_x自增
            //如果自增后超过了一行的mb个数
            if (++h->mb_x >= h->mb_width) {
            	//环路滤波
                loop_filter(h, lf_x_start, h->mb_x);
                h->mb_x = lf_x_start = 0;
                decode_finish_row(h);
                //mb_y自增（处理下一行）
                ++h->mb_y;
                //宏块级帧场自适应，暂不考虑
                if (FIELD_OR_MBAFF_PICTURE(h)) {
                    ++h->mb_y;
                    if (FRAME_MBAFF(h) && h->mb_y < h->mb_height)
                        predict_field_decoding_flag(h);
                }
            }
            //如果mb_y超过了mb的行数
            if (eos || h->mb_y >= h->mb_height) {
                tprintf(h->avctx, "slice end %d %d\n",
                        get_bits_count(&h->gb), h->gb.size_in_bits);
                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x - 1,
                             h->mb_y, ER_MB_END);
                if (h->mb_x > lf_x_start)
                    loop_filter(h, lf_x_start, h->mb_x);
                return 0;
            }
        }
    } else {
    	//CAVLC情况
    	//循环处理每个宏块
        for (;;) {
        	//解码宏块的CAVLC
            int ret = ff_h264_decode_mb_cavlc(h);
            //解码宏块
            if (ret >= 0)
                ff_h264_hl_decode_mb(h);
 
            // FIXME optimal? or let mb_decode decode 16x32 ?
            if (ret >= 0 && FRAME_MBAFF(h)) {
                h->mb_y++;
                ret = ff_h264_decode_mb_cavlc(h);
 
                if (ret >= 0)
                    ff_h264_hl_decode_mb(h);
                h->mb_y--;
            }
 
            if (ret < 0) {
                av_log(h->avctx, AV_LOG_ERROR,
                       "error while decoding MB %d %d\n", h->mb_x, h->mb_y);
                er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x,
                             h->mb_y, ER_MB_ERROR);
                return ret;
            }
 
            if (++h->mb_x >= h->mb_width) {
            	//环路滤波
                loop_filter(h, lf_x_start, h->mb_x);
                h->mb_x = lf_x_start = 0;
                decode_finish_row(h);
                ++h->mb_y;
                if (FIELD_OR_MBAFF_PICTURE(h)) {
                    ++h->mb_y;
                    if (FRAME_MBAFF(h) && h->mb_y < h->mb_height)
                        predict_field_decoding_flag(h);
                }
                if (h->mb_y >= h->mb_height) {
                    tprintf(h->avctx, "slice end %d %d\n",
                            get_bits_count(&h->gb), h->gb.size_in_bits);
 
                    if (   get_bits_left(&h->gb) == 0
                        || get_bits_left(&h->gb) > 0 && !(h->avctx->err_recognition & AV_EF_AGGRESSIVE)) {
                    	//错误隐藏
                        er_add_slice(h, h->resync_mb_x, h->resync_mb_y,
                                     h->mb_x - 1, h->mb_y, ER_MB_END);
 
                        return 0;
                    } else {
                        er_add_slice(h, h->resync_mb_x, h->resync_mb_y,
                                     h->mb_x, h->mb_y, ER_MB_END);
 
                        return AVERROR_INVALIDDATA;
                    }
                }
            }
 
            if (get_bits_left(&h->gb) <= 0 && h->mb_skip_run <= 0) {
                tprintf(h->avctx, "slice end %d %d\n",
                        get_bits_count(&h->gb), h->gb.size_in_bits);
 
                if (get_bits_left(&h->gb) == 0) {
                    er_add_slice(h, h->resync_mb_x, h->resync_mb_y,
                                 h->mb_x - 1, h->mb_y, ER_MB_END);
                    if (h->mb_x > lf_x_start)
                        loop_filter(h, lf_x_start, h->mb_x);
 
                    return 0;
                } else {
                    er_add_slice(h, h->resync_mb_x, h->resync_mb_y, h->mb_x,
                                 h->mb_y, ER_MB_ERROR);
 
                    return AVERROR_INVALIDDATA;
                }
            }
        }
    }
}

It can be seen that the process of decode_slice() is as follows:
(1) Determine whether the H.264 code stream is CABAC encoding or CAVLC encoding, and enter different processing loops.

(2) If it is CABAC encoding, first call ff_init_cabac_decoder() to initialize the CABAC decoder. Then a loop is entered, and each macroblock is processed in turn as follows:

a) Call ff_h264_decode_mb_cabac() for CABAC entropy decoding

b) Call ff_h264_hl_decode_mb() for macroblock decoding

c) After decoding a line of macroblocks, call loop_filter() for loop filtering

d) In addition, it is possible to call er_add_slice() for error concealment processing

(3) If it is CABAC encoding, directly enter a loop, and perform the following processing on each macroblock in turn:

a) Call ff_h264_decode_mb_cavlc() for CAVLC entropy decoding

b) Call ff_h264_hl_decode_mb() for macroblock decoding

c) After decoding a line of macroblocks, call loop_filter() for loop filtering

d) In addition, it is possible to call er_add_slice() for error concealment processing

It can be seen that the functions of macroblock decoding and loop filtering are the same except for entropy decoding. Let's take a look at the CAVLC entropy decoding function ff_h264_decode_mb_cavlc() in detail.


ff_h264_decode_mb_cavlc()
ff_h264_decode_mb_cavlc() completes the "entropy decoding" function in the FFmpeg H.264 decoder. The role of the "entropy decoding" part is to read the data (macroblock type, motion vector, reference frame, residual, etc.) and assign it to the corresponding variable in the FFmpeg H.264 decoder according to the H.264 syntax and semantics . Specifically, it completes the function of parsing Slice Data in H.264 code stream. This function is more complex, its definition is located in libavcodec\h264_cavlc.c, as shown below.
/*
 * 注释：雷霄骅
 * leixiaohua1020@126.com
 * http://blog.csdn.net/leixiaohua1020
 *
 * 解码宏块的CAVLC数据
 * 解码Slice Data（注意不包含Slice Header）
 *
 */
int ff_h264_decode_mb_cavlc(H264Context *h){
    int mb_xy;
    int partition_count;
    unsigned int mb_type, cbp;
    int dct8x8_allowed= h->pps.transform_8x8_mode;
    //如果是YUV420或者YUV422，需要处理色度（YUV444中的UV直接当亮度处理）
    int decode_chroma = h->sps.chroma_format_idc == 1 || h->sps.chroma_format_idc == 2;
    const int pixel_shift = h->pixel_shift;
    unsigned local_ref_count[2];
    //mb_xy的计算方法
    mb_xy = h->mb_xy = h->mb_x + h->mb_y*h->mb_stride;
 
    tprintf(h->avctx, "pic:%d mb:%d/%d\n", h->frame_num, h->mb_x, h->mb_y);
    cbp = 0; /* avoid warning. FIXME: find a solution without slowing
                down the code */
    //slice_type_nos意思是SI/SP 被映射为 I/P （即没有SI/SP这种帧）
    //处理Skip宏块-不携带任何数据
	//解码器通过周围已重建的宏块的数据来恢复skip块
    if(h->slice_type_nos != AV_PICTURE_TYPE_I){
    	//熵编码为CAVLC时候特有的字段
        if(h->mb_skip_run==-1)
            h->mb_skip_run= get_ue_golomb_long(&h->gb);
 
        if (h->mb_skip_run--) {
            if(FRAME_MBAFF(h) && (h->mb_y&1) == 0){
                if(h->mb_skip_run==0)
                    h->mb_mbaff = h->mb_field_decoding_flag = get_bits1(&h->gb);
            }
            decode_mb_skip(h);
            return 0;
        }
    }
    if (FRAME_MBAFF(h)) {
        if( (h->mb_y&1) == 0 )
            h->mb_mbaff = h->mb_field_decoding_flag = get_bits1(&h->gb);
    }
 
    h->prev_mb_skipped= 0;
    //获取宏块类型（I,B,P）
    //I片中只允许出现I宏块
    //P片中即可以出现P宏块也可以出现I宏块
    //B片中即可以出现B宏块也可以出现I宏块
    //这个语义含义比较复杂，需要查表
    mb_type= get_ue_golomb(&h->gb);
    //B
    if(h->slice_type_nos == AV_PICTURE_TYPE_B){
    	//b_mb_type_info存储了B宏块的类型
    	//type代表宏块类型
    	//partition_count代表宏块分区数目
        if(mb_type < 23){
            partition_count= b_mb_type_info[mb_type].partition_count;
            mb_type=         b_mb_type_info[mb_type].type;
        }else{
            mb_type -= 23;
            goto decode_intra_mb;
        }
        //P
    }else if(h->slice_type_nos == AV_PICTURE_TYPE_P){
    	//p_mb_type_info存储了P宏块的类型
    	//type代表宏块类型
    	//partition_count代表宏块分区数目（一般为1，2，4）
        if(mb_type < 5){
            partition_count= p_mb_type_info[mb_type].partition_count;
            mb_type=         p_mb_type_info[mb_type].type;
        }else{
            mb_type -= 5;
            goto decode_intra_mb;
        }
    }else{
    	//i_mb_type_info存储了I宏块的类型
    	//注意i_mb_type_info和p_mb_type_info、b_mb_type_info是不一样的：
    	//type：宏块类型。只有MB_TYPE_INTRA4x4，MB_TYPE_INTRA16x16（基本上都是这种），MB_TYPE_INTRA_PCM三种
    	//pred_mode：帧内预测方式（四种：DC，Horizontal，Vertical，Plane）。
    	//cbp：指亮度和色度分量的各小块的残差的编码方案，所谓编码方案有以下几种：
    	//      0) 所有残差（包括 DC、AC）都不编码。
    	//      1) 只对 DC 系数编码。
    	//      2) 所有残差（包括 DC、AC）都编码。
       av_assert2(h->slice_type_nos == AV_PICTURE_TYPE_I);
        if(h->slice_type == AV_PICTURE_TYPE_SI && mb_type)
            mb_type--;
decode_intra_mb:
        if(mb_type > 25){
            av_log(h->avctx, AV_LOG_ERROR, "mb_type %d in %c slice too large at %d %d\n", mb_type, av_get_picture_type_char(h->slice_type), h->mb_x, h->mb_y);
            return -1;
        }
        partition_count=0;
        cbp= i_mb_type_info[mb_type].cbp;
        h->intra16x16_pred_mode= i_mb_type_info[mb_type].pred_mode;
        mb_type= i_mb_type_info[mb_type].type;
    }
    //隔行
    if(MB_FIELD(h))
        mb_type |= MB_TYPE_INTERLACED;
 
    h->slice_table[ mb_xy ]= h->slice_num;
    //I_PCM不常见
    if(IS_INTRA_PCM(mb_type)){
        const int mb_size = ff_h264_mb_sizes[h->sps.chroma_format_idc] *
                            h->sps.bit_depth_luma;
 
        // We assume these blocks are very rare so we do not optimize it.
        h->intra_pcm_ptr = align_get_bits(&h->gb);
        if (get_bits_left(&h->gb) < mb_size) {
            av_log(h->avctx, AV_LOG_ERROR, "Not enough data for an intra PCM block.\n");
            return AVERROR_INVALIDDATA;
        }
        skip_bits_long(&h->gb, mb_size);
 
        // In deblocking, the quantizer is 0
        h->cur_pic.qscale_table[mb_xy] = 0;
        // All coeffs are present
        memset(h->non_zero_count[mb_xy], 16, 48);
        //赋值
        h->cur_pic.mb_type[mb_xy] = mb_type;
        return 0;
    }
 
    //
    local_ref_count[0] = h->ref_count[0] << MB_MBAFF(h);
    local_ref_count[1] = h->ref_count[1] << MB_MBAFF(h);
 
    /* 设置上左，上，上右，左宏块的索引值和宏块类型
     * 这4个宏块在解码过程中会用到
     * 位置如下图所示
     *
	 * +----+----+----+
	 * | UL |  U | UR |
	 * +----+----+----+
	 * | L  |    |
	 * +----+----+
	 */
    fill_decode_neighbors(h, mb_type);
    //填充Cache
    fill_decode_caches(h, mb_type);
 
    /*
     *
     * 关于多次出现的scan8
     *
     * scan8[]是一个表格。表格中存储了一整个宏块的信息，每一个元素代表了一个“4x4块”（H.264中最小的处理单位）。
     * scan8[]中的“8”，意思应该是按照8x8为单元来扫描？
     * 因此可以理解为“按照8x8为单元来扫描4x4的块”？
     *
     * scan8中按照顺序分别存储了Y，U，V的索引值。具体的存储还是在相应的cache中。
     *
     * PS：“4x4”貌似是H.264解码器中最小的“块”单位
     *
     * cache中首先存储Y，然后存储U和V。cache中的存储方式如下所示。
     * 其中数字代表了scan8[]中元素的索引值
     * scan8[]中元素的值则代表了其代表的变量在cache中的索引值
     * +---+---+---+---+---+---+---+---+---+
     * |   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
     * +---+---+---+---+---+---+---+---+---+
     * | 0 | 48|   |   |   |  y|  y|  y|  y|
     * | 1 |   |   |   |  y|  0|  1|  4|  5|
     * | 2 |   |   |   |  y|  2|  3|  6|  7|
     * | 3 |   |   |   |  y|  8|  9| 12| 13|
     * | 4 |   |   |   |  y| 10| 11| 14| 15|
     * | 5 | 49|   |   |   |  u|  u|  u|  u|
     * | 6 |   |   |   |  u| 16| 17| 20| 21|
     * | 7 |   |   |   |  u| 18| 19| 22| 23|
     * | 8 |   |   |   |  u| 24| 25| 28| 29|
     * | 9 |   |   |   |  u| 26| 27| 30| 31|
     * |10 | 50|   |   |   |  v|  v|  v|  v|
     * |11 |   |   |   |  v| 32| 33| 36| 37|
     * |12 |   |   |   |  v| 34| 35| 38| 39|
     * |13 |   |   |   |  v| 40| 41| 44| 45|
     * |14 |   |   |   |  v| 42| 43| 46| 47|
     * |---+---+---+---+---+---+---+---+---+
     * |   |
     *
     */
 
 
    //mb_pred
    //分成3种情况进行预测工作：
    //1.帧内预测
    //2.划分为4个块（此时每个8x8的块可以再次划分为4种类型）
    //3.其他类型（包括16x16,16x8,8x16，这些划分不可再次划分）
    if(IS_INTRA(mb_type)){
        //情况1：帧内宏块
        int pred_mode;
//            init_top_left_availability(h);
        //如果是帧内4x4，帧内预测方式需要特殊处理（9种）
        if(IS_INTRA4x4(mb_type)){
            int i;
            int di = 1;
            //先不考虑这种相对特殊情况，认为di=1
            if(dct8x8_allowed && get_bits1(&h->gb)){
                mb_type |= MB_TYPE_8x8DCT;
                di = 4;
            }
 
//                fill_intra4x4_pred_table(h);
            //对于一个宏块（16x16）来说，包含了4*4=16个4x4帧内预测的块
            //所以循环16次
            /*
			 * 帧内预测：16x16 宏块被划分为16个4x4子块
			 *
			 * +----+----+----+----+
			 * |    |    |    |    |
			 * +----+----+----+----+
			 * |    |    |    |    |
			 * +----+----+----+----+
			 * |    |    |    |    |
			 * +----+----+----+----+
			 * |    |    |    |    |
			 * +----+----+----+----+
			 *
			 */
            for(i=0; i<16; i+=di){
            	//获得对Intra4x4的预测模式的预测值（挺绕口，确实是这样）
            	//这个预测模式由左边和上边块的预测模式（取最小值）推导主来
                int mode= pred_intra_mode(h, i);
                //这1bit是dcPredModePredictedFlag，如果为1，则直接使用推导出来的预测模式
                if(!get_bits1(&h->gb)){
                	//否则就使用读取出来的预测模式
                    const int rem_mode= get_bits(&h->gb, 3);
                    mode = rem_mode + (rem_mode >= mode);
                }
 
                if(di==4)
                    fill_rectangle( &h->intra4x4_pred_mode_cache[ scan8[i] ], 2, 2, 8, mode, 1 );
                else
                    h->intra4x4_pred_mode_cache[ scan8[i] ] = mode;//赋值
                /*
                 * 将mode填充至intra4x4_pred_mode_cache
                 *
				 * 用简单图形表示intra4x4_pred_mode_cache如下。数字代表填充顺序（一共填充16次）
				 *   |
				 * --+-------------------
				 *   | 0 0 0 0  0  0  0  0
				 *   | 0 0 0 0  1  2  5  6
				 *   | 0 0 0 0  3  4  7  8
				 *   | 0 0 0 0  9 10 13 14
				 *   | 0 0 0 0 11 12 15 16
				 *
				 */
 
            }
            //将宏块的Cache中的intra4x4_pred_mode拷贝至整张图片的intra4x4_pred_mode变量中
            write_back_intra_pred_mode(h);
            if( ff_h264_check_intra4x4_pred_mode(h) < 0)
                return -1;
        }else{
        	//帧内16x16的检测：检查宏块上方和左边的数据是否可用
            h->intra16x16_pred_mode= ff_h264_check_intra_pred_mode(h, h->intra16x16_pred_mode, 0);
            if(h->intra16x16_pred_mode < 0)
                return -1;
        }
        if(decode_chroma){
        	//色度帧内预测的检测，和亮度一样
            pred_mode= ff_h264_check_intra_pred_mode(h, get_ue_golomb_31(&h->gb), 1);
            if(pred_mode < 0)
                return -1;
            h->chroma_pred_mode= pred_mode;
        } else {
            h->chroma_pred_mode = DC_128_PRED8x8;
        }
    }else if(partition_count==4){
    	//情况2：宏块划分为4
    	//为什么宏块划分为4的时候要单独处理？因为宏块划分为4的时候，每个8x8的子宏块还可以进一步划分为2个4x8，2个8x4（4x8），或者4个4x4。
    	//而其他方式的宏块划分（例如16x16,16x8,8x16等）是不可以这样再次划分的
    	/*
		 * 16x16 宏块被划分为4个8x8子块
		 *
		 * +--------+--------+
		 * |        |        |
		 * |   0    |   1    |
		 * |        |        |
		 * +--------+--------+
		 * |        |        |
		 * |   2    |   3    |
		 * |        |        |
		 * +--------+--------+
		 *
		 */
        int i, j, sub_partition_count[4], list, ref[2][4];
        //获得8x8子块的宏块类型
        //后续的很多代码都是循环处理4个8x8子块
        //所以很多for()循环的次数都是为4
        if(h->slice_type_nos == AV_PICTURE_TYPE_B){
            //B宏块
        	//4个子块
            for(i=0; i<4; i++){
            	//子宏块的预测类型
                h->sub_mb_type[i]= get_ue_golomb_31(&h->gb);
                if(h->sub_mb_type[i] >=13){
                    av_log(h->avctx, AV_LOG_ERROR, "B sub_mb_type %u out of range at %d %d\n", h->sub_mb_type[i], h->mb_x, h->mb_y);
                    return -1;
                }
                sub_partition_count[i]= b_sub_mb_type_info[ h->sub_mb_type[i] ].partition_count;
                h->sub_mb_type[i]=      b_sub_mb_type_info[ h->sub_mb_type[i] ].type;
            }
            if( IS_DIRECT(h->sub_mb_type[0]|h->sub_mb_type[1]|h->sub_mb_type[2]|h->sub_mb_type[3])) {
                ff_h264_pred_direct_motion(h, &mb_type);
                h->ref_cache[0][scan8[4]] =
                h->ref_cache[1][scan8[4]] =
                h->ref_cache[0][scan8[12]] =
                h->ref_cache[1][scan8[12]] = PART_NOT_AVAILABLE;
            }
        }else{
            av_assert2(h->slice_type_nos == AV_PICTURE_TYPE_P); //FIXME SP correct ?
            //P宏块
            //4个子块
            for(i=0; i<4; i++){
                h->sub_mb_type[i]= get_ue_golomb_31(&h->gb);
                if(h->sub_mb_type[i] >=4){
                    av_log(h->avctx, AV_LOG_ERROR, "P sub_mb_type %u out of range at %d %d\n", h->sub_mb_type[i], h->mb_x, h->mb_y);
                    return -1;
                }
                //p_sub_mb_type_info存储了P子宏块的类型，和前面的p_mb_type_info类似
                //type代表宏块类型
                //partition_count代表宏块分区数目
                sub_partition_count[i]= p_sub_mb_type_info[ h->sub_mb_type[i] ].partition_count;
                h->sub_mb_type[i]=      p_sub_mb_type_info[ h->sub_mb_type[i] ].type;
            }
        }
        //8x8块的子宏块的参考帧序号
        for(list=0; list<h->list_count; list++){
            int ref_count = IS_REF0(mb_type) ? 1 : local_ref_count[list];
            //4个子块
            for(i=0; i<4; i++){
                if(IS_DIRECT(h->sub_mb_type[i])) continue;
                if(IS_DIR(h->sub_mb_type[i], 0, list)){
                    unsigned int tmp;
                    if(ref_count == 1){
                        tmp= 0;
                    }else if(ref_count == 2){
                        tmp= get_bits1(&h->gb)^1;
                    }else{
                    	//参考帧序号
                        tmp= get_ue_golomb_31(&h->gb);
                        if(tmp>=ref_count){
                            av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", tmp);
                            return -1;
                        }
                    }
                    //存储
                    ref[list][i]= tmp;
                }else{
                 //FIXME
                    ref[list][i] = -1;
                }
            }
        }
 
        if(dct8x8_allowed)
            dct8x8_allowed = get_dct8x8_allowed(h);
 
        //8x8块的子宏块的运动矢量
        //依次处理L0和L1
        for(list=0; list<h->list_count; list++){
        	//4个子块
            for(i=0; i<4; i++){
                if(IS_DIRECT(h->sub_mb_type[i])) {
                    h->ref_cache[list][ scan8[4*i] ] = h->ref_cache[list][ scan8[4*i]+1 ];
                    continue;
                }
                h->ref_cache[list][ scan8[4*i]   ]=h->ref_cache[list][ scan8[4*i]+1 ]=
                h->ref_cache[list][ scan8[4*i]+8 ]=h->ref_cache[list][ scan8[4*i]+9 ]= ref[list][i];
 
                if(IS_DIR(h->sub_mb_type[i], 0, list)){
                    const int sub_mb_type= h->sub_mb_type[i];
                    const int block_width= (sub_mb_type & (MB_TYPE_16x16|MB_TYPE_16x8)) ? 2 : 1;
                    //8x8块的子块（可能是8x8,8x4,4x8,4x4）的运动矢量
                    //依次处理，数量为sub_partition_count
                    for(j=0; j<sub_partition_count[i]; j++){
                        int mx, my;
                        //scan8索引
                        const int index= 4*i + block_width*j;
                        int16_t (* mv_cache)[2]= &h->mv_cache[list][ scan8[index] ];
                        //先获取“预测MV”（取中值），结果存入mx，my
                        pred_motion(h, index, block_width, list, h->ref_cache[list][ scan8[index] ], &mx, &my);
                        //获取MVD并且累加至“预测MV”
                        //MV=预测MV+MVD
                        mx += get_se_golomb(&h->gb);
                        my += get_se_golomb(&h->gb);
                        tprintf(h->avctx, "final mv:%d %d\n", mx, my);
 
                        if(IS_SUB_8X8(sub_mb_type)){
                        	//8x8子宏块的宏块划分方式为8x8（等同于没划分）
                        	//则把mv_cache中的4个块对应的值都赋值成一样的
                        	//即：[0]，[1]，[0+8]，[1+8]
                        	//PS：stride（代表一行元素个数）为8（即“+8”代表是下一行）
                        	/*
							 * +----+----+
							 * |         |
							 * +    +    +
							 * |         |
							 * +----+----+
							 *
							 */
                            mv_cache[ 1 ][0]=
                            mv_cache[ 8 ][0]= mv_cache[ 9 ][0]= mx;
                            mv_cache[ 1 ][1]=
                            mv_cache[ 8 ][1]= mv_cache[ 9 ][1]= my;
                        }else if(IS_SUB_8X4(sub_mb_type)){
                        	//如果是8x4子宏块
							//则把mv_cache中的横向的2个块对应的值都赋值成一样的
							//即：[0]，[1]
                        	/*
							 * +----+----+
							 * |         |
							 * +----+----+
							 * |         |
							 * +----+----+
							 *
							 */
                            mv_cache[ 1 ][0]= mx;
                            mv_cache[ 1 ][1]= my;
                        }else if(IS_SUB_4X8(sub_mb_type)){
                        	//如果是4x8子宏块
							//则把mv_cache中纵向的2个块对应的值都赋值成一样的
							//即：[0]，[0+8]
                        	/*
							 * +----+----+
							 * |    |    |
							 * +    +    +
							 * |    |    |
							 * +----+----+
							 *
							 */
                            mv_cache[ 8 ][0]= mx;
                            mv_cache[ 8 ][1]= my;
                        }
                        //赋值
                        //PS：如果是4x4子宏块划分的话，则不会触发上面的if else语句，即分别得到4个4x4块的运动矢量
                        mv_cache[ 0 ][0]= mx;
                        mv_cache[ 0 ][1]= my;
 
						/*
						 * mv_cache赋值方式如下
						 * scan8[0]代表了cache里面亮度Y的起始点，取值12
						 * 如果全部都是4x4划分的话，mv_cache填充顺序即按照scan8中元素中的顺序：
						 * scan8[0],scan8[1],scan8[2],scan8[3],scan8[4],scan8[5]......
						 * 即：
						 *	4 +  1 * 8, 5 +  1 * 8, 4 +  2 * 8, 5 +  2 * 8,
    					 *	6 +  1 * 8, 7 +  1 * 8, 6 +  2 * 8, 7 +  2 * 8,
    					 *	4 +  3 * 8, 5 +  3 * 8, 4 +  4 * 8, 5 +  4 * 8,......
						 * 用简单图形表示mv_cache如下。数字代表填充顺序（一共填充16次）
						 *   |
						 * --+-------------------
						 *   | 0 0 0 0  0  0  0  0
						 *   | 0 0 0 0  1  2  5  6
						 *   | 0 0 0 0  3  4  7  8
						 *   | 0 0 0 0  9 10 13 14
						 *   | 0 0 0 0 11 12 15 16
						 *
						 *  如果全部是8x8划分的话，mv_cache填充顺序即按照scan8中元素中的顺序：
						 *  scan8[0],scan8[4],scan8[8],scan8[16]......
						 *  填充后赋值3个元素
						 *  用简单图形表示mv_cache如下。数字代表填充顺序（一共填充4次）
						 *   |
						 * --+-------------------
						 *   | 0 0 0 0  0  0  0  0
						 *   | 0 0 0 0  1  1  2  2
						 *   | 0 0 0 0  1  1  2  2
						 *   | 0 0 0 0  3  3  4  4
						 *   | 0 0 0 0  3  3  4  4
						 *
						 *	如果全部是8x4划分的话，mv_cache填充顺序即按照scan8中元素中的顺序：
						 *  scan8[0],scan8[2],scan8[4],scan8[6]......
						 *  填充后赋值右边1个元素
						 *  用简单图形表示mv_cache如下。数字代表填充顺序（一共填充8次）
						 *   |
						 * --+-------------------
						 *   | 0 0 0 0  0  0  0  0
						 *   | 0 0 0 0  1  1  3  3
						 *   | 0 0 0 0  2  2  4  4
						 *   | 0 0 0 0  5  5  7  7
						 *   | 0 0 0 0  6  6  8  8
						 *
						 *  如果全部是4x8划分的话，mv_cache填充顺序即按照scan8中元素中的顺序：
						 *  scan8[0],scan8[1],scan8[4],scan8[5],scan8[8],scan8[9]......
						 *  填充后赋值下边1个元素
						 *  用简单图形表示mv_cache如下。数字代表填充顺序（一共填充8次）
						 *   |
						 * --+-------------------
						 *   | 0 0 0 0  0  0  0  0
						 *   | 0 0 0 0  1  2  3  4
						 *   | 0 0 0 0  1  2  3  4
						 *   | 0 0 0 0  5  6  7  8
						 *   | 0 0 0 0  5  6  7  8
						 *
						 *   其他划分的不同组合，可以参考上面的填充顺序
						 */
                    }
                }else{
                    uint32_t *p= (uint32_t *)&h->mv_cache[list][ scan8[4*i] ][0];
                    p[0] = p[1]=
                    p[8] = p[9]= 0;
                }
            }
        }
    }else if(IS_DIRECT(mb_type)){
    	//Direct模式
        ff_h264_pred_direct_motion(h, &mb_type);
        dct8x8_allowed &= h->sps.direct_8x8_inference_flag;
    }else{
    	//情况3：既不是帧内宏块（情况1），宏块划分数目也不为4（情况2）
    	//这种情况下不存在8x8的子宏块再次划分这样的事情
        int list, mx, my, i;
         //FIXME we should set ref_idx_l? to 0 if we use that later ...
        if(IS_16X16(mb_type)){
        	/*
        	 * 16x16 macroblock
        	 *
        	 * +--------+--------+
        	 * |                 |
        	 * |                 |
        	 * |                 |
        	 * +        +        +
        	 * |                 |
        	 * |                 |
        	 * |                 |
        	 * +--------+--------+
        	 *
        	 */
        	//The reference frame corresponding to the motion vector
        	//L0 and L1
            for(list=0; list<h->list_count; list++){
                    unsigned int val;
                    if(IS_DIR(mb_type, 0, list)){
                        if(local_ref_count[list]==1){
                            val= 0;
                        } else if(local_ref_count[list]==2){
                            val= get_bits1(&h->gb)^1;
                        }else{
                        	//参考帧图像序号
                            val= get_ue_golomb_31(&h->gb);
                            if (val >= local_ref_count[list]){
                                av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                return -1;
                            }
                        }
                        //填充ref_cache
                        //fill_rectangle(数据起始点，宽，高，一行数据个数，数据值，每个数据占用的byte)
                        //scan8[0]代表了cache里面亮度Y的起始点
                        /*
                         * 在这里相当于在ref_cache[list]填充了这样的一份数据（val=v）：
                         *   |
                         * --+--------------
                         *   | 0 0 0 0 0 0 0 0
                         *   | 0 0 0 0 v v v v
                         *   | 0 0 0 0 v v v v
                         *   | 0 0 0 0 v v v v
                         *   | 0 0 0 0 v v v v
                         */
                    fill_rectangle(&h->ref_cache[list][ scan8[0] ], 4, 4, 8, val, 1);
                    }
            }
            //运动矢量
            for(list=0; list<h->list_count; list++){
                if(IS_DIR(mb_type, 0, list)){
                	//预测MV（取中值）
                    pred_motion(h, 0, 4, list, h->ref_cache[list][ scan8[0] ], &mx, &my);
                    //MVD从码流中获取
                    //MV=预测MV+MVD
                    mx += get_se_golomb(&h->gb);
                    my += get_se_golomb(&h->gb);
                    tprintf(h->avctx, "final mv:%d %d\n", mx, my);
                    //填充mv_cache
					//fill_rectangle(数据起始点，宽，高，一行数据个数，数据值，每个数据占用的byte)
					//scan8[0]代表了cache里面亮度Y的起始点
					/*
					 * 在这里相当于在mv_cache[list]填充了这样的一份数据（val=v）：
					 *   |
					 * --+--------------
					 *   | 0 0 0 0 0 0 0 0
					 *   | 0 0 0 0 v v v v
					 *   | 0 0 0 0 v v v v
					 *   | 0 0 0 0 v v v v
					 *   | 0 0 0 0 v v v v
					 */
                    fill_rectangle(h->mv_cache[list][ scan8[0] ], 4, 4, 8, pack16to32(mx,my), 4);
                }
            }
        }
        else if(IS_16X8(mb_type)){ //16x8
        	/*
        	 * 16x8 宏块划分
        	 *
        	 * +--------+--------+
        	 * |        |        |
        	 * |        |        |
        	 * |        |        |
        	 * +--------+--------+
        	 *
        	 */
        	//运动矢量对应的参考帧
            for(list=0; list<h->list_count; list++){
            		//横着的2个
                    for(i=0; i<2; i++){
                    	//存储在val
                        unsigned int val;
                        if(IS_DIR(mb_type, i, list)){
                            if(local_ref_count[list] == 1) {
                                val= 0;
                            } else if(local_ref_count[list] == 2) {
                                val= get_bits1(&h->gb)^1;
                            }else{
                                val= get_ue_golomb_31(&h->gb);
                                if (val >= local_ref_count[list]){
                                    av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                    return -1;
                                }
                            }
                        }else
                            val= LIST_NOT_USED&0xFF;
                        //填充ref_cache
						//fill_rectangle(数据起始点，宽，高，一行数据个数，数据值，每个数据占用的byte)
						//scan8[0]代表了cache里面亮度Y的起始点
						/*
						 * 在这里相当于在ref_cache[list]填充了这样的一份数据（第一次循环val=1，第二次循环val=2）：
						 *   |
						 * --+--------------
						 *   | 0 0 0 0 0 0 0 0
						 *   | 0 0 0 0 1 1 1 1
						 *   | 0 0 0 0 1 1 1 1
						 *   | 0 0 0 0 2 2 2 2
						 *   | 0 0 0 0 2 2 2 2
						 */
                        fill_rectangle(&h->ref_cache[list][ scan8[0] + 16*i ], 4, 2, 8, val, 1);
                    }
            }
            //运动矢量
            for(list=0; list<h->list_count; list++){
            	//2个
                for(i=0; i<2; i++){
                	//存储在val
                    unsigned int val;
                    if(IS_DIR(mb_type, i, list)){
                    	//预测MV
                        pred_16x8_motion(h, 8*i, list, h->ref_cache[list][scan8[0] + 16*i], &mx, &my);
                        //MV=预测MV+MVD
                        mx += get_se_golomb(&h->gb);
                        my += get_se_golomb(&h->gb);
                        tprintf(h->avctx, "final mv:%d %d\n", mx, my);
                        //打包？
                        val= pack16to32(mx,my);
                    }else
                        val=0;
                    //填充mv_cache
					//fill_rectangle(数据起始点，宽，高，一行数据个数，数据值，每个数据占用的byte)
					//scan8[0]代表了cache里面亮度Y的起始点
					/*
					 * This is equivalent to filling such a piece of data in ref_cache[list] (the first loop val=1, the second loop val=2)：
					 *   |
					 * --+--------------
					 *   | 0 0 0 0 0 0 0 0
					 *   | 0 0 0 0 1 1 1 1
					 *   | 0 0 0 0 1 1 1 1
					 *   | 0 0 0 0 2 2 2 2
					 *   | 0 0 0 0 2 2 2 2
					 */
                    fill_rectangle(h->mv_cache[list][ scan8[0] + 16*i ], 4, 2, 8, val, 4);
                }
            }
        }else{ //8x16?
        	/*
        	 * 8x16 宏块划分
        	 *
        	 * +--------+
        	 * |        |
        	 * |        |
        	 * |        |
        	 * +--------+
        	 * |        |
        	 * |        |
        	 * |        |
        	 * +--------+
        	 *
        	 */
            av_assert2(IS_8X16(mb_type));
            for(list=0; list<h->list_count; list++){
            	//竖着的2个
                    for(i=0; i<2; i++){
                        unsigned int val;
                        if(IS_DIR(mb_type, i, list)){ //FIXME optimize
                            if(local_ref_count[list]==1){
                                val= 0;
                            } else if(local_ref_count[list]==2){
                                val= get_bits1(&h->gb)^1;
                            }else{
                                val= get_ue_golomb_31(&h->gb);
                                if (val >= local_ref_count[list]){
                                    av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                    return -1;
                                }
                            }
                        }else
                            val= LIST_NOT_USED&0xFF;
                        //填充ref_cache
						//fill_rectangle(数据起始点，宽，高，一行数据个数，数据值，每个数据占用的byte)
						//scan8[0]代表了cache里面亮度Y的起始点
						/*
						 * 在这里相当于在ref_cache[list]填充了这样的一份数据（第一次循环val=1，第二次循环val=2）：
						 *   |
						 * --+--------------
						 *   | 0 0 0 0 0 0 0 0
						 *   | 0 0 0 0 1 1 2 2
						 *   | 0 0 0 0 1 1 2 2
						 *   | 0 0 0 0 1 1 2 2
						 *   | 0 0 0 0 1 1 2 2
						 */
                        fill_rectangle(&h->ref_cache[list][ scan8[0] + 2*i ], 2, 4, 8, val, 1);
                    }
            }
            for(list=0; list<h->list_count; list++){
                for(i=0; i<2; i++){
                    unsigned int val;
                    if(IS_DIR(mb_type, i, list)){
                    	//预测MV
                        pred_8x16_motion(h, i*4, list, h->ref_cache[list][ scan8[0] + 2*i ], &mx, &my);
                        //MV=预测MV+MVD
                        mx += get_se_golomb(&h->gb);
                        my += get_se_golomb(&h->gb);
                        tprintf(h->avctx, "final mv:%d %d\n", mx, my);
 
                        val= pack16to32(mx,my);
                    }else
                        val=0;
                    //填充mv_cache
                    //fill_rectangle(数据起始点，宽，高，一行数据个数，数据值，每个数据占用的byte)
					//scan8[0]代表了cache里面亮度Y的起始点
					/*
					 * 在这里相当于在mv_cache[list]填充了这样的一份数据（第一次循环val=1，第二次循环val=2）：
					 *   |
					 * --+--------------
					 *   | 0 0 0 0 0 0 0 0
					 *   | 0 0 0 0 1 1 2 2
					 *   | 0 0 0 0 1 1 2 2
					 *   | 0 0 0 0 1 1 2 2
					 *   | 0 0 0 0 1 1 2 2
					 */
                    fill_rectangle(h->mv_cache[list][ scan8[0] + 2*i ], 2, 4, 8, val, 4);
                }
            }
        }
    }
    //将宏块的Cache中的MV拷贝至整张图片的motion_val变量中
    if(IS_INTER(mb_type))
        write_back_motion(h, mb_type);
 
    //Intra16x16的CBP位于mb_type中，其他类型的宏块的CBP需要单独读取
    if(!IS_INTRA16x16(mb_type)){
    	//获取CBP
        cbp= get_ue_golomb(&h->gb);
 
        if(decode_chroma){
        	//YUV420,YUV422的情况
            if(cbp > 47){
                av_log(h->avctx, AV_LOG_ERROR, "cbp too large (%u) at %d %d\n", cbp, h->mb_x, h->mb_y);
                return -1;
            }
            //获取CBP
            if(IS_INTRA4x4(mb_type)) cbp= golomb_to_intra4x4_cbp[cbp];
            else                     cbp= golomb_to_inter_cbp   [cbp];
        }else{
            if(cbp > 15){
                av_log(h->avctx, AV_LOG_ERROR, "cbp too large (%u) at %d %d\n", cbp, h->mb_x, h->mb_y);
                return -1;
            }
            if(IS_INTRA4x4(mb_type)) cbp= golomb_to_intra4x4_cbp_gray[cbp];
            else                     cbp= golomb_to_inter_cbp_gray[cbp];
        }
    } else {
        if (!decode_chroma && cbp>15) {
            av_log(h->avctx, AV_LOG_ERROR, "gray chroma\n");
            return AVERROR_INVALIDDATA;
        }
    }
 
    if(dct8x8_allowed && (cbp&15) && !IS_INTRA(mb_type)){
        mb_type |= MB_TYPE_8x8DCT*get_bits1(&h->gb);
    }
    //赋值CBP
    h->cbp=
    h->cbp_table[mb_xy]= cbp;
    //赋值mb_type
    h->cur_pic.mb_type[mb_xy] = mb_type;
 
    /*
     * 亮度cbp取值（只有低4位有意义）：
     * 变量的最低位比特从最低位开始，每1位对应1个子宏块，该位等于1时表明对应子宏块残差系数被传送；
     * 该位等于0时表明对应子宏块残差全部不被传送
     * 色度cbp取值：
     * 0，代表所有残差都不被传送
     * 1，只传送DC
     * 2，传送DC+AC
     */
 
    //cbp不为0，才有残差信息
    if(cbp || IS_INTRA16x16(mb_type)){
        int i4x4, i8x8, chroma_idx;
        int dquant;
        int ret;
        GetBitContext *gb= IS_INTRA(mb_type) ? h->intra_gb_ptr : h->inter_gb_ptr;
        const uint8_t *scan, *scan8x8;
        const int max_qp = 51 + 6*(h->sps.bit_depth_luma-8);
 
        if(IS_INTERLACED(mb_type)){
            scan8x8= h->qscale ? h->field_scan8x8_cavlc : h->field_scan8x8_cavlc_q0;
            scan= h->qscale ? h->field_scan : h->field_scan_q0;
        }else{
            scan8x8= h->qscale ? h->zigzag_scan8x8_cavlc : h->zigzag_scan8x8_cavlc_q0;
            scan= h->qscale ? h->zigzag_scan : h->zigzag_scan_q0;
        }
        //QP量化参数的偏移值
        dquant= get_se_golomb(&h->gb);
        //由前一个宏块的量化参数累加得到本宏块的QP
        h->qscale += dquant;
        //注：slice中第1个宏块的计算方法（不存在前一个宏块了）：
        //QP = 26 + pic_init_qp_minus26 + slice_qp_delta
 
        if(((unsigned)h->qscale) > max_qp){
            if(h->qscale<0) h->qscale+= max_qp+1;
            else            h->qscale-= max_qp+1;
            if(((unsigned)h->qscale) > max_qp){
                av_log(h->avctx, AV_LOG_ERROR, "dquant out of range (%d) at %d %d\n", dquant, h->mb_x, h->mb_y);
                return -1;
            }
        }
 
        h->chroma_qp[0]= get_chroma_qp(h, 0, h->qscale);
        h->chroma_qp[1]= get_chroma_qp(h, 1, h->qscale);
        //解码残差-亮度
        if( (ret = decode_luma_residual(h, gb, scan, scan8x8, pixel_shift, mb_type, cbp, 0)) < 0 ){
            return -1;
        }
        h->cbp_table[mb_xy] |= ret << 12;
 
        if (CHROMA444(h)) {
            //YUV444，把U，V都当成亮度处理
            if( decode_luma_residual(h, gb, scan, scan8x8, pixel_shift, mb_type, cbp, 1) < 0 ){
                return -1;
            }
            if( decode_luma_residual(h, gb, scan, scan8x8, pixel_shift, mb_type, cbp, 2) < 0 ){
                return -1;
            }
        } else {
        	//解码残差-色度
            const int num_c8x8 = h->sps.chroma_format_idc;
            //色度CBP位于高4位
            //0:不传
            //1:只传DC
            //2:DC+AC
            if(cbp&0x30){
            	//如果传了的话
            	//就要解码残差数据
            	//2个分量
                for(chroma_idx=0; chroma_idx<2; chroma_idx++)
                    if (decode_residual(h, gb, h->mb + ((256 + 16*16*chroma_idx) << pixel_shift),
                                        CHROMA_DC_BLOCK_INDEX+chroma_idx,
                                        CHROMA422(h) ? chroma422_dc_scan : chroma_dc_scan,
                                        NULL, 4*num_c8x8) < 0) {
                        return -1;
                    }
            }
            //如果传递了AC系数
            if(cbp&0x20){
            	 //2个分量
                for(chroma_idx=0; chroma_idx<2; chroma_idx++){
                    const uint32_t *qmul = h->dequant4_coeff[chroma_idx+1+(IS_INTRA( mb_type ) ? 0:3)][h->chroma_qp[chroma_idx]];
                    int16_t *mb = h->mb + (16*(16 + 16*chroma_idx) << pixel_shift);
                    for (i8x8 = 0; i8x8<num_c8x8; i8x8++) {
                        for (i4x4 = 0; i4x4 < 4; i4x4++) {
                            const int index = 16 + 16*chroma_idx + 8*i8x8 + i4x4;
                            if (decode_residual(h, gb, mb, index, scan + 1, qmul, 15) < 0)
                                return -1;
                            mb += 16 << pixel_shift;
                        }
                    }
                }
            }else{
            	/*
				 * non_zero_count_cache:
				 * 每个4x4块的非0系数个数的缓存
				 *
				 * 在这里把U，V都填充为0
				 * non_zero_count_cache[]内容如下所示
				 * 图中v=0，上面的块代表Y，中间的块代表U，下面的块代表V
				 *   |
				 * --+--------------
				 *   | 0 0 0 0 0 0 0 0
				 *   | 0 0 0 0 x x x x
				 *   | 0 0 0 0 x x x x
				 *   | 0 0 0 0 x x x x
				 *   | 0 0 0 0 x x x x
				 *   | 0 0 0 0 0 0 0 0
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 0 0 0 0
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 v v v v
				 *   | 0 0 0 0 v v v v
                 */
                fill_rectangle(&h->non_zero_count_cache[scan8[16]], 4, 4, 8, 0, 1);
                fill_rectangle(&h->non_zero_count_cache[scan8[32]], 4, 4, 8, 0, 1);
            }
        }
    }else{
 
    	/*
    	 * non_zero_count_cache:
    	 * 每个4x4块的非0系数个数的缓存
    	 *
    	 * cbp为0时，既不传DC，也不传AC，即全部赋值为0
    	 *
    	 * non_zero_count_cache[]内容如下所示
    	 * 图中v=0，上面的块代表Y，中间的块代表U，下面的块代表V
		 *   |
		 * --+--------------
		 *   | 0 0 0 0 0 0 0 0
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 0 0 0 0
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 0 0 0 0
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
		 *   | 0 0 0 0 v v v v
    	 *
    	 */
        fill_rectangle(&h->non_zero_count_cache[scan8[ 0]], 4, 4, 8, 0, 1);
        fill_rectangle(&h->non_zero_count_cache[scan8[16]], 4, 4, 8, 0, 1);
        fill_rectangle(&h->non_zero_count_cache[scan8[32]], 4, 4, 8, 0, 1);
    }
    //赋值QP
    h->cur_pic.qscale_table[mb_xy] = h->qscale;
 
    //将宏块的non_zero_count_cache拷贝至整张图片的non_zero_count变量中
    write_back_non_zero_count(h);
 
    return 0;
}











The definition of ff_h264_decode_mb_cavlc() has nearly 1000 lines of code, which is a relatively complex function. I've written quite a few comments in it, so I'm not going to do a detailed analysis of the source code. Here is a brief overview of its process:
(1) Parse the Skip type macroblock

(2) Get mb_type

(3) Fill in the information of the left and upper macroblocks of the current macroblock (will be used in subsequent predictions)

(4) According to the difference of mb_type, the prediction work is divided into three cases:

a) Macroblocks are intra-predicted

i. If the macroblock is of Intra4x4 type, the intra prediction mode needs to be parsed separately.

ii. If the macroblock is of type Intra16x16, no more overprocessing is done.

b) The macroblock is divided into 4 blocks (at this time each 8x8 block can be divided into 4 types again)

At this time, each 8x8 block can be divided into several sub-blocks of 8x8, 8x4, 4x8, and 4x4. These small sub-blocks need to be processed separately:

i. The reference frame number of the parsing sub-block

ii. Parse the motion vector of the sub-block

c) Other types (including 16x16, 16x8, 8x16 several divisions, these divisions cannot be divided again)

At this time, it is necessary to determine whether the type of macroblock is 16x16, 16x8 or 8x16, and then do the following processing:

i. Parse the reference frame number of the sub-macroblock

ii. Parse the motion vector of the sub-macroblock

(5) Decoding residual information

(6) Output various information of the macroblock to the corresponding variables of the whole picture


The following briefly summarizes some of the knowledge points involved in ff_h264_decode_mb_cavlc().


mb_type
mb_type is an index of the type of macroblock. The FFmpeg H.264 decoder uses i_mb_type_info[] to store the type information of I macroblocks; uses p_mb_type_info[] to store the type information of P macroblocks; uses b_mb_type_info[] to store the type information of B macroblocks. The information of this type of macroblock can be obtained by using the method of "X_mb_type_info[mb_type]" ("X" can take "i", "p", "b"). For example, the following code can be used to obtain the number of blocks of the B macroblock.
int partition_count= b_mb_type_info[mb_type].partition_count;
Let's take a look at the definitions of these arrays.


i_mb_type_info[]
i_mb_type_info[] stores the type of the I macroblock. The elements are structures of type IMbInfo. The definition of the IMbInfo type structure is as follows.
typedef struct IMbInfo {
    uint16_t type;
    uint8_t pred_mode;//Intra prediction mode
    uint8_t cbp;// Coded Block Pattern, the upper 4 bits are chroma, and the lower 4 bits are luminance
} IMbInfo;
i_mb_type_info[] is defined as follows.
//mb_type of I macroblock
/*
 * Rule:
 * pred_mode is always Vertical->Horizontal->DC->Plane (remember that Vertical is ranked 0th in intra-frame prediction)
 * cbp: The amount of transmitted data is getting larger and larger (the first half does not transmit the brightness residual)
 * Sort by data volume
 *
 * Only Intra_16x16 macroblock type, the value of CBP is not given by the syntax element, but obtained by mb_type.
 *
 * CBP (Coded Block Pattern)
 * Chroma CBP meaning:
 * 0: do not pass residuals
 * 1: Only transmit DC
 *2: transmit DC+AC
 * Brightness CBP (only the lowest 4 bits are defined) meaning:
 * The lowest bit of the variable starts from the lowest bit, and each bit corresponds to a sub-macroblock. When this bit is equal to 1, it indicates that the residual coefficient of the corresponding sub-macroblock is transmitted; this bit is equal to 0
 * indicates that the corresponding sub-macroblock residuals are not transmitted, and the decoder assigns these residual coefficients to 0.
 */
 
static const IMbInfo i_mb_type_info[26] = {
    { MB_TYPE_INTRA4x4, -1, -1 },//pred_mode also needs to be obtained separately
    { MB_TYPE_INTRA16x16, 2, 0 },//cbp:0000+0
    { MB_TYPE_INTRA16x16, 1, 0 },
    { MB_TYPE_INTRA16x16, 0, 0 },
    { MB_TYPE_INTRA16x16, 3, 0 },
    { MB_TYPE_INTRA16x16, 2, 16 },//cbp:0000+1<<4
    { MB_TYPE_INTRA16x16, 1, 16 },
    { MB_TYPE_INTRA16x16, 0, 16 },
    { MB_TYPE_INTRA16x16, 3, 16 },
    { MB_TYPE_INTRA16x16, 2, 32 },//cbp:0000+2<<4
    { MB_TYPE_INTRA16x16, 1, 32 },
    { MB_TYPE_INTRA16x16, 0, 32 },
    { MB_TYPE_INTRA16x16, 3, 32 },
    { MB_TYPE_INTRA16x16, 2, 15 + 0 },//cbp:1111+0<<4
    { MB_TYPE_INTRA16x16, 1, 15 + 0 },
    { MB_TYPE_INTRA16x16, 0, 15 + 0 },
    { MB_TYPE_INTRA16x16, 3, 15 + 0 },
    { MB_TYPE_INTRA16x16, 2, 15 + 16 },//cbp:1111+1<<4
    { MB_TYPE_INTRA16x16, 1, 15 + 16 },
    { MB_TYPE_INTRA16x16, 0, 15 + 16 },
    { MB_TYPE_INTRA16x16, 3, 15 + 16 },
    { MB_TYPE_INTRA16x16, 2, 15 + 32 },//cbp:1111+2<<4
    { MB_TYPE_INTRA16x16, 1, 15 + 32 },
    { MB_TYPE_INTRA16x16, 0, 15 + 32 },
    { MB_TYPE_INTRA16x16, 3, 15 + 32 },
    { MB_TYPE_INTRA_PCM, -1, -1 },//special
};

p_mb_type_info[]
p_mb_type_info[] stores the type of the P macroblock. The elements are structures of type PMbInfo. The definition of PMbInfo type structure is as follows.
typedef struct PMbInfo {
    uint16_t type;//Macro block type
    uint8_t partition_count;//Number of partitions
} PMbInfo;
The definition of p_mb_type_info[] is as follows.
//mb_type of P macroblock
/*
 * Rule:
 * Macroblock division size from large to small (the number of sub-macroblocks gradually increases)
 * First "fat" (16x8), then "thin" (8x16)
 * "X" in MB_TYPE_PXL0 represents the number of partitions of the macroblock, which can only be 0 or 1
 * "X" in MB_TYPE_P0LX represents which List the macroblock refers to. P macroblocks can only refer to list0
 *
 */
static const PMbInfo p_mb_type_info[5] = {
    { MB_TYPE_16x16 | MB_TYPE_P0L0, 1 },//without "P1"
    { MB_TYPE_16x8 | MB_TYPE_P0L0 | MB_TYPE_P1L0, 2 },
    { MB_TYPE_8x16 | MB_TYPE_P0L0 | MB_TYPE_P1L0, 2 },
    { MB_TYPE_8x8 | MB_TYPE_P0L0 | MB_TYPE_P1L0, 4 },
    { MB_TYPE_8x8 | MB_TYPE_P0L0 | MB_TYPE_P1L0 | MB_TYPE_REF0, 4 },
};

b_mb_type_info[]
b_mb_type_info[] stores the type of B macroblock. The elements are structures of type PMbInfo. It should be noted here that the types of elements in p_mb_type_info[] and b_mb_type_info[] are the same, and they are both structures of type PMbInfo.
The definition of b_mb_type_info[] is as follows.
//mb_type of B macroblock
/*
 * Rule:
 * Macroblock division size from large to small (the number of sub-macroblocks gradually increases)
 * First "fat" (16x8), then "thin" (8x16)
 * There are more and more lists referenced by each partition (the opinions are becoming more and more inconsistent)
 *
 * "X" in MB_TYPE_PXL0 represents the number of partitions of the macroblock, which can only be 0 or 1
 * "X" in MB_TYPE_P0LX represents which List the macroblock refers to. B macroblock reference list0 and list1
 *
 */







static const PMbInfo b_mb_type_info[23] = {
    { MB_TYPE_DIRECT2 | MB_TYPE_L0L1,                                              1, },
    { MB_TYPE_16x16   | MB_TYPE_P0L0,                                              1, },//没有“P1”
    { MB_TYPE_16x16   | MB_TYPE_P0L1,                                              1, },
    { MB_TYPE_16x16   | MB_TYPE_P0L0 | MB_TYPE_P0L1,                               1, },
    { MB_TYPE_16x8    | MB_TYPE_P0L0 | MB_TYPE_P1L0,                               2, },//两个分区（每个分区两个参考帧）都参考list0
    { MB_TYPE_8x16    | MB_TYPE_P0L0 | MB_TYPE_P1L0,                               2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L1 | MB_TYPE_P1L1,                               2, },//两个分区（每个分区两个参考帧）都参考list1
    { MB_TYPE_8x16    | MB_TYPE_P0L1 | MB_TYPE_P1L1,                               2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L0 | MB_TYPE_P1L1,                               2, },//0分区（两个参考帧）参考list0,1分区（两个参考帧）参考list1
    { MB_TYPE_8x16    | MB_TYPE_P0L0 | MB_TYPE_P1L1,                               2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L1 | MB_TYPE_P1L0,                               2, },
    { MB_TYPE_8x16    | MB_TYPE_P0L1 | MB_TYPE_P1L0,                               2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L0 | MB_TYPE_P1L0 | MB_TYPE_P1L1,                2, },
    { MB_TYPE_8x16    | MB_TYPE_P0L0 | MB_TYPE_P1L0 | MB_TYPE_P1L1,                2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L1 | MB_TYPE_P1L0 | MB_TYPE_P1L1,                2, },
    { MB_TYPE_8x16    | MB_TYPE_P0L1 | MB_TYPE_P1L0 | MB_TYPE_P1L1,                2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L0,                2, },
    { MB_TYPE_8x16    | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L0,                2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L1,                2, },
    { MB_TYPE_8x16    | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L1,                2, },
    { MB_TYPE_16x8    | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L0 | MB_TYPE_P1L1, 2, },
    { MB_TYPE_8x16    | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L0 | MB_TYPE_P1L1, 2, },
    { MB_TYPE_8x8     | MB_TYPE_P0L0 | MB_TYPE_P0L1 | MB_TYPE_P1L0 | MB_TYPE_P1L1, 4, },
};


Fill in the information of the left and upper macroblocks of the current macroblock
In the macroblock prediction, information about the macroblocks at the left, top left, top and top right positions of the current macroblock needs to be used. Therefore, this information needs to be filled in before prediction. The H.264 decoder calls two functions fill_decode_neighbors() and fill_decode_caches() to fill this information. I haven't looked at the fill_decode_caches() function yet, so here's a brief analysis of the fill_decode_neighbors() function


fill_decode_neighbors()
fill_decode_neighbors() is used to set the index value and macroblock type of the macroblock on the left, top left, top and top right of the current macroblock. The definition is located in libavcodec\h264_mvpred.h, as shown below.
/* Set the index value and macroblock type of the top left, top, top right, and left macroblocks
 * These 4 macroblocks will be used in the decoding process
 * The location is shown in the picture below
 *
 * +----+----+----+
 *|UL|U|UR|
 * +----+----+----+
 * | L | |
 * +----+----+
 */
static void fill_decode_neighbors(H264Context *h, int mb_type)
{
    const int mb_xy = h->mb_xy;
    int topleft_xy, top_xy, topright_xy, left_xy[LEFT_MBS];
    static const uint8_t left_block_options[4][32] = {
        { 0, 1, 2, 3, 7, 10, 8, 11, 3 + 0 * 4, 3 + 1 * 4, 3 + 2 * 4, 3 + 3 * 4, 1 + 4 * 4, 1 + 8 * 4, 1 + 5 * 4, 1 + 9 * 4 },
        { 2, 2, 3, 3, 8, 11, 8, 11, 3 + 2 * 4, 3 + 2 * 4, 3 + 3 * 4, 3 + 3 * 4, 1 + 5 * 4, 1 + 9 * 4, 1 + 5 * 4, 1 + 9 * 4 },
        { 0, 0, 1, 1, 7, 10, 7, 10, 3 + 0 * 4, 3 + 0 * 4, 3 + 1 * 4, 3 + 1 * 4, 1 + 4 * 4, 1 + 8 * 4, 1 + 4 * 4, 1 + 8 * 4 },
        { 0, 2, 0, 2, 7, 10, 7, 10, 3 + 0 * 4, 3 + 2 * 4, 3 + 0 * 4, 3 + 2 * 4, 1 + 4 * 4, 1 + 8 * 4, 1 + 4 * 4, 1 + 8 * 4 }
    };
 
    h->topleft_partition = -1;
    //The macroblock above. current macroblock minus one line
    //top_xy=mb_xy-mb_stride
    top_xy = mb_xy - (h->mb_stride << MB_FIELD(h));
 
    /* Wow, what a mess, why didn't they simplify the interlacing & intra
     * stuff, I can't imagine that these complex rules are worth it. */
    //Top left macroblock. The macroblock above minus 1
    topleft_xy = top_xy - 1;
    //Top right macroblock. Add 1 to the macroblock above
    topright_xy = top_xy + 1;
    // Macroblock on the left. current macroblock minus 1
    left_xy[LBOT] = left_xy[LTOP] = mb_xy - 1;
    h->left_block = left_block_options[0];
 
    if (FRAME_MBAFF(h)) {
        const int left_mb_field_flag = IS_INTERLACED(h->cur_pic.mb_type[mb_xy - 1]);
        const int curr_mb_field_flag = IS_INTERLACED(mb_type);
        if (h->mb_y & 1) {
            if (left_mb_field_flag != curr_mb_field_flag) {
                left_xy[LBOT] = left_xy[LTOP] = mb_xy - h->mb_stride - 1;
                if (curr_mb_field_flag) {
                    left_xy[LBOT] += h->mb_stride;
                    h->left_block = left_block_options[3];
                } else {
                    topleft_xy += h->mb_stride;
                    /* take top left mv from the middle of the mb, as opposed
                     * to all other modes which use the bottom right partition */
                    h->topleft_partition = 0;
                    h->left_block = left_block_options[1];
                }
            }
        } else {
            if (curr_mb_field_flag) {
                topleft_xy += h->mb_stride & (((h->cur_pic.mb_type[top_xy - 1] >> 7) & 1) - 1);
                topright_xy += h->mb_stride & (((h->cur_pic.mb_type[top_xy + 1] >> 7) & 1) - 1);
                top_xy += h->mb_stride & (((h->cur_pic.mb_type[top_xy] >> 7) & 1) - 1);
            }
            if (left_mb_field_flag != curr_mb_field_flag) {
                if (curr_mb_field_flag) {
                    left_xy[LBOT] += h->mb_stride;
                    h->left_block = left_block_options[3];
                } else {
                    h->left_block = left_block_options[2];
                }
            }
        }
    }
    //macroblock index value
    // top left
    h->topleft_mb_xy = topleft_xy;
    //superior
    h->top_mb_xy = top_xy;
    //up right






    h->topright_mb_xy   = topright_xy;
    //left. When scanning progressively left_xy[LTOP]==left_xy[LBOT]
    h->left_mb_xy[LTOP] = left_xy[LTOP];
    h->left_mb_xy[LBOT] = left_xy[LBOT];
    //FIXME do we need all in the context?
 
    //macroblock type
    h->topleft_type    = h->cur_pic.mb_type[topleft_xy];
    h->top_type        = h->cur_pic.mb_type[top_xy];
    h->topright_type   = h->cur_pic.mb_type[topright_xy];
    h->left_type[LTOP] = h->cur_pic.mb_type[left_xy[LTOP]];
    h->left_type[LBOT] = h->cur_pic.mb_type[left_xy[LBOT]];
 
    if (FMO) {
        if (h->slice_table[topleft_xy] != h->slice_num)
            h->topleft_type = 0;
        if (h->slice_table[top_xy] != h->slice_num)
            h->top_type = 0;
        if (h->slice_table[left_xy[LTOP]] != h->slice_num)
            h->left_type[LTOP] = h->left_type[LBOT] = 0;
    } else {
        if (h->slice_table[topleft_xy] != h->slice_num) {
            h->topleft_type = 0;
            if (h->slice_table[top_xy] != h->slice_num)
                h->top_type = 0;
            if (h->slice_table[left_xy[LTOP]] != h->slice_num)
                h->left_type[LTOP] = h->left_type[LBOT] = 0;
        }
    }
    if (h->slice_table[topright_xy] != h->slice_num)
        h->topright_type = 0;
}

As can be seen from the source code, fill_decode_neighbors() sets the following index values:
topleft_mb_xy，top_mb_xy，topright_mb_xy，left_mb_xy[LTOP]和left_mb_xy[LBOT]

The following macroblock types are set:

topleft_type，top_type，topright_type，left_type[LTOP]，left_type[LBOT]

It should be noted that left_xy[LTOP] and left_xy[LBOT] are equal in the case of progressive scan.



Various Cache (cache)
A variety of Caches are included in the H.264 decoder. E.g:
intra4x4_pred_mode_cache: Intra4x4 intra prediction mode cache

non_zero_count_cache: cache for the number of non-zero coefficients per 4x4 block

mv_cache: motion vector cache

ref_cache: Cache of motion vector reference frames

The definitions of these Caches are as follows.

/**
* Cache for Intra4x4 intra prediction mode
* The structure is as follows
* |
* --+--------------
* | 0 0 0 0 0 0 0 0
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
*/
    int8_t intra4x4_pred_mode_cache[5*8];
 
/**
     * non zero coeff count cache.
     * is 64 if not available.
     * Cache for the number of non-zero coefficients per 4x4 block
     */
    uint8_t __attribute__ ((aligned (8))) non_zero_count_cache[15 * 8];
 
/**
     * Motion vector cache.
     * Motion vector buffer[list][data][x,y]
     * list: L0 or L1
     * data: a total of 5x8 elements (note that it is of type int16_t)
     * The structure is as follows
* |
* --+--------------
* | 0 0 0 0 0 0 0 0
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* x, y: the abscissa and ordinate of the motion vector
     */
    int16_t __attribute__ ((aligned (16))) mv_cache[2][5 * 8][2];
 
/**
* Cache of motion vector reference frames, used in conjunction with mv_cache (note that the data is of type int8_t)
* The structure is as follows
* |
* --+--------------
* | 0 0 0 0 0 0 0 0
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
* | 0 0 0 0 Y Y Y Y
*/
    int8_t __attribute__ ((aligned (8))) ref_cache [2][5 * 8];

 By observing the above definition, we will find that Cache is a one-dimensional array containing x*8 elements (x is 15 or 5). According to my own understanding, I think Cache uses one-dimensional arrays to compare and store two-dimensional image information. It can be seen from the above code that the place where valid data is stored in the Cache is a "square area" located in the lower right corner, which actually corresponds to the 12-15, 20-23, 28-31, 36-39 in the one-dimensional array. Elements. This "square area" represents the luminance-related information of a macroblock, which contains a total of 16 elements. Since the luminance data of a macroblock is a 16x16 block, one element in this "square area" actually represents the information of a 4x4 block (the "4x4" luminance block should also be in H.264 compression coding smallest processing unit).
 If we use subscripts in the range of 12-15, 20-23, 28-31, 36-39 to refer to elements in the Cache, it is really inconvenient. This also leads to another key variable in the FFmpeg H.264 decoder - the scan8[] array.

scan8[]
scan8[] stores the serial number value of the cache, which is generally used in conjunction with the aforementioned Cache. The definition of scan8[] is located in libavcodec\h264.h as shown below.
/*
 *scanning method:

 * o-o o-o
 *  / / /
 * o-o o-o
 *  ,---'
 * o-o o-o
 *  / / /
 * o-o o-o
 */
 
/* Scan8 organization:
 *    0 1 2 3 4 5 6 7
 * 0  DY    y y y y y
 * 1        y Y Y Y Y
 * 2        y Y Y Y Y
 * 3        y Y Y Y Y
 * 4        y Y Y Y Y
 * 5  DU    u u u u u
 * 6        u U U U U
 * 7        u U U U U
 * 8        u U U U U
 * 9        u U U U U
 * 10 DV    v v v v v
 * 11       v V V V V
 * 12       v V V V V
 * 13       v V V V V
 * 14       v V V V V
 * DY/DU/DV are for luma/chroma DC.
 */
 
// This table must be here because scan8[constant] must be known at compiletime
//scan8[] is usually used with various caches (mv_cache, ref_cache, etc.)
static const uint8_t scan8[16 * 3 + 3] = {
    4 +  1 * 8, 5 +  1 * 8, 4 +  2 * 8, 5 +  2 * 8,
    6 +  1 * 8, 7 +  1 * 8, 6 +  2 * 8, 7 +  2 * 8,
    4 +  3 * 8, 5 +  3 * 8, 4 +  4 * 8, 5 +  4 * 8,
    6 +  3 * 8, 7 +  3 * 8, 6 +  4 * 8, 7 +  4 * 8,
    4 +  6 * 8, 5 +  6 * 8, 4 +  7 * 8, 5 +  7 * 8,
    6 +  6 * 8, 7 +  6 * 8, 6 +  7 * 8, 7 +  7 * 8,
    4 +  8 * 8, 5 +  8 * 8, 4 +  9 * 8, 5 +  9 * 8,
    6 +  8 * 8, 7 +  8 * 8, 6 +  9 * 8, 7 +  9 * 8,
    4 + 11 * 8, 5 + 11 * 8, 4 + 12 * 8, 5 + 12 * 8,
    6 + 11 * 8, 7 + 11 * 8, 6 + 12 * 8, 7 + 12 * 8,
    4 + 13 * 8, 5 + 13 * 8, 4 + 14 * 8, 5 + 14 * 8,
    6 + 13 * 8, 7 + 13 * 8, 6 + 14 * 8, 7 + 14 * 8,
    0 +  0 * 8, 0 +  5 * 8, 0 + 10 * 8
};

It can be seen that the values ​​of the elements in the scan8[] array are written in the form of "a+b*8". Let's calculate the values ​​of the first 16 elements:
scan8[0]=12

scan8[1]= 13

scan8[2]= 20

scan8[3]= 21

scan8[4]= 14

scan8[5]= 15

scan8[6]= 22

scan8[7]= 23

scan8[8]= 28

scan8[9]= 29

scan8[10]= 36

scan8[11]= 37

scan8[12]= 30

scan8[13]= 31

scan8[14]= 38

scan8[15]= 39


If the values ​​of these elements in the scan8[] array are used as the serial numbers of the Cache (such as mv_cache, ref_cache, etc.), you will find that the positions of the elements represented in the Cache are as shown in the following figure.

  
 The elements with the gray background in the image above are valid elements in the Cache (elements that do not use the blank space on the left may be due to historical reasons). Using the Cache element number directly may feel abstract. The following figure uses the scan8[] array element number to represent the data stored in the Cache, and the result is shown in the following figure.
  
 Each element in the figure represents the information of a 4x4 block, and each "big square" composed of 16 elements represents the information of 1 component of 1 macroblock. The "big square" on the gray background stores the information related to the luminance Y in the macroblock, the "big square" on the blue background stores the information related to the chrominance U in the macroblock, and the "big square" on the pink background stores the Chroma U-related information in a macroblock. PS: I can find a little information on the scan8[] array on the Internet. But after comparing the source code, I found that the information on the Internet is outdated. The storage method of the cache represented by the old version scan8[] is as follows. 



It can be seen that U and V in the old version of scan8[] are the area stored on the left side of Y, and each component has only 4 elements, while in the new version of scan8[], U and V are stored in the area below Y , and each component has 16 elements.

Speculate Intra4x4 intra prediction mode
In Intra4x4 intra-frame coded macroblocks, each 4x4 sub-block has its own intra-frame prediction method. The intra-frame prediction method of each sub-block is not directly stored in the H.264 code stream (which is not conducive to compression). Instead, the intra prediction mode of the current block is inferred preferentially based on the information of the surrounding blocks. The specific method is to obtain the prediction modes of the left block and the upper block, and then take their minimum value as the prediction mode of the current block. The implementation code of this part of the function in the H.264 decoder is located in the pred_intra_mode() function, as shown below.
/**
 * Get the predicted intra4x4 prediction mode.
 */
//Get the predicted value of the prediction mode of Intra4x4 (it's a twist, it's true)
//This prediction mode is derived from the prediction modes of the left and upper blocks (take the minimum value)
static av_always_inline int pred_intra_mode(H264Context *h, int n)
{
     const int index8 = scan8[n];
     //The prediction method of the left block
     const int left = h->intra4x4_pred_mode_cache[index8 - 1];
     //The prediction method of the upper block
     const int top = h->intra4x4_pred_mode_cache[index8 - 8];
     //Get the minimum value of the left and top
     const int min = FFMIN(left, top);
 
     tprintf(h->avctx, "mode:%d %d min:%d\n", left, top, min);
     //return
     if (min < 0)
         return DC_PRED;
     else
         return min;
}

Reference frame number and motion vector acquisition
No matter which type of macroblock is processed, the H.264 decoder first obtains the reference frame number of the macroblock, and then obtains the motion vector of the macroblock. The code for obtaining the reference frame number and motion vector occupies the largest space in ff_h264_decode_mb_cavlc(). Here we look at the simplest example - inter-frame 16x16 macroblock reference frame number and motion vector acquisition. The code for this section is shown below.
if(IS_16X16(mb_type)){
        	/*
        	 * 16x16 macroblock
        	 *
        	 * +--------+--------+
        	 * |                 |
        	 * |                 |
        	 * |                 |
        	 * +        +        +
        	 * |                 |
        	 * |                 |
        	 * |                 |
        	 * +--------+--------+
        	 *
        	 */
//The reference frame corresponding to the motion vector
        //L0 and L1
            for(list=0; list<h->list_count; list++){
                    unsigned int val;
                    if(IS_DIR(mb_type, 0, list)){
                        if(local_ref_count[list]==1){
                            val= 0;
                        } else if(local_ref_count[list]==2){
                            val= get_bits1(&h->gb)^1;
                        }else{
                        //reference frame image serial number
                            val= get_ue_golomb_31(&h->gb);
                            if (val >= local_ref_count[list]){
                                av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                return -1;
                            }
                        }
                        //fill ref_cache
                        //fill_rectangle(data starting point, width, height, number of data in a row, data value, byte occupied by each data)
                        //scan8[0] represents the starting point of brightness Y in the cache
                        /*
                         * Here is equivalent to filling ref_cache[list] with such a piece of data (val=v):
                         * |
                         * --+--------------
                         * | 0 0 0 0 0 0 0 0
                         * | 0 0 0 0 v v v v
                         * | 0 0 0 0 v v v v
                         * | 0 0 0 0 v v v v
                         * | 0 0 0 0 v v v v
                         */
                    fill_rectangle(&h->ref_cache[list][ scan8[0] ], 4, 4, 8, val, 1);
                    }
            }
            //motion vector
            for(list=0; list<h->list_count; list++){
                if(IS_DIR(mb_type, 0, list)){
                //Predict MV (take the median)
                    pred_motion(h, 0, 4, list, h->ref_cache[list][ scan8[0] ], &mx, &my);
                    //MVD is obtained from the code stream
                    //MV=Predicted MV+MVD
                    mx += get_se_golomb(&h->gb);
                    my += get_se_golomb(&h->gb);
                    tprintf(h->avctx, "final mv:%d %d\n", mx, my);
                    //fill mv_cache
//fill_rectangle(data starting point, width, height, number of data in a row, data value, byte occupied by each data)
//scan8[0] represents the starting point of brightness Y in the cache
/*
* Here is equivalent to filling mv_cache[list] with such a piece of data (val=v):
* |
* --+--------------
* | 0 0 0 0 0 0 0 0
* | 0 0 0 0 v v v v
* | 0 0 0 0 v v v v
* | 0 0 0 0 v v v v
* | 0 0 0 0 v v v v
*/
                    fill_rectangle(h->mv_cache[list][ scan8[0] ], 4, 4, 8, pack16to32(mx,my), 4);
                }
            }
        }

As can be seen from the code, the H.264 decoder first reads the reference frame image number (val variable) and stores it in the ref_cache cache, then reads the motion vector (mx, my variable) and stores it in the mv_cache cache middle. When reading the motion vector, there is one thing to pay attention to: the motion vector information is stored in the form of MVD (Motion Vector Difference) in H.264. Therefore, the true motion vector of a macroblock should be calculated using the following formula:
MV=Predicted MV+MVD
The "predicted MV" is predicted from the MVs of the left, top, and top right macroblocks of the current macroblock. The way to predict is to take the median of these 3 blocks (note that it is not the average). For example, in the figure below, the predicted value of the motion vector of E is taken from the median value of the three block MVs of A, B, and C.
  
In the FFmpeg H.264 decoder, the code for the motion vector prediction part is implemented in the pred_motion() function. The function definition is located in libavcodec\h264_mvpred.h as shown below.
/**
 * Get the predicted MV.
 * @param n the block index
 * @param part_width the width of the partition (4, 8,16) -> (1, 2, 4)
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
//Get the predicted MV (take the median), and store the result in mx, my
static av_always_inline void pred_motion(H264Context *const h, int n,
                                         int part_width, int list, int ref,
                                         int *const mx, int *const my)
{
    const int index8       = scan8[n];
    const int top_ref      = h->ref_cache[list][index8 - 8];
    const int left_ref     = h->ref_cache[list][index8 - 1];
    //left MV
     const int16_t *const A = h->mv_cache[list][index8 - 1];
     //Above MV
     const int16_t *const B = h->mv_cache[list][index8 - 8];
     //Top right MV?
    const int16_t *C;
    int diagonal_ref, match_count;
 
    av_assert2(part_width == 1 || part_width == 2 || part_width == 4);
 
/* mv_cache
 * B . . A T T T T
 * U . . L . . , .
 * U . . L . . . .
 * U . . L . . , .
 * . . . L . . . .
 */
 
    diagonal_ref = fetch_diagonal_mv(h, &C, index8, list, part_width);
    match_count  = (diagonal_ref == ref) + (top_ref == ref) + (left_ref == ref);
    tprintf(h->avctx, "pred_motion match_count=%d\n", match_count);
    if (match_count > 1) { //most common
    	//Take the median of A,B,C
        *mx = mid_pred(A[0], B[0], C[0]);
        *my = mid_pred(A[1], B[1], C[1]);
    } else if (match_count == 1) {
    	//take only one of the values
        if (left_ref == ref) {
            *mx = A[0];
            *my = A[1];
        } else if (top_ref == ref) {
            *mx = B[0];
            *my = B[1];
        } else {
            *mx = C[0];
            *my = C[1];
        }
    } else {
        if (top_ref      == PART_NOT_AVAILABLE &&
            diagonal_ref == PART_NOT_AVAILABLE &&
            left_ref     != PART_NOT_AVAILABLE) {
            *mx = A[0];
            *my = A[1];
        } else {
            *mx = mid_pred(A[0], B[0], C[0]);
            *my = mid_pred(A[1], B[1], C[1]);
        }
    }
 
    tprintf(h->avctx,
            "pred_motion (%2d %2d %2d) (%2d %2d %2d) (%2d %2d %2d) -> (%2d %2d %2d) at %2d %2d %d list %d\n",
            top_ref, B[0], B[1], diagonal_ref, C[0], C[1], left_ref,
            A[0], A[1], ref, *mx, *my, h->mb_x, h->mb_y, n, list);
}

Decoding residuals
The H.264 decoder first determines whether the CBP is 0. If CBP is not 0, decode the residual data encoded by CAVLC; if CBP is 0, directly assign all non_zero_count_cache[] to 0.


CBP
The full name of CBP is Coded Block Pattern, which refers to the coding scheme of the residuals of each small block of luminance and chrominance components. The high 4 bits of the cbp variable (a uint8_t type variable) in the H.264 decoder store the chrominance CBP, and the low 4 bits store the luminance CBP. Chroma CBP and Luma CBP have different meanings:
The luminance CBP data starts from the lowest bit, and each bit corresponds to a sub-macroblock. When the bit is equal to 1, it indicates that the residual coefficient of the corresponding sub-macroblock is transmitted. (So ​​luminance CBP data usually needs to be viewed as binary data)

Chroma CBP contains 3 values:

0: means that all residuals are not transmitted

1: Only transmit DC coefficients

2: Transmission of DC coefficients and AC coefficients

(So ​​chrominance CBP data can usually be viewed as decimal data)



decode_luma_residual()
When CBP is not 0, decode_luma_residual() will be called to decode the luminance residual data. In addition, if chroma residual is included, decode_residual() will be called to decode the chroma residual data. The definition of decode_luma_residual() is shown below.
static av_always_inline int decode_luma_residual(H264Context *h, GetBitContext *gb, const uint8_t *scan, const uint8_t *scan8x8, int pixel_shift, int mb_type, int cbp, int p){
    int i4x4, i8x8;
    int qscale = p == 0 ? h->qscale : h->chroma_qp[p-1];
    if(IS_INTRA16x16(mb_type)){
    //Intra16x16 type
        AV_ZERO128(h->mb_luma_dc[p]+0);
        AV_ZERO128(h->mb_luma_dc[p]+8);
        AV_ZERO128(h->mb_luma_dc[p]+16);
        AV_ZERO128(h->mb_luma_dc[p]+24);
        //decoding residuals
        //here are the coefficients after decoding the Hadamard transform?
        if( decode_residual(h, h->intra_gb_ptr, h->mb_luma_dc[p], LUMA_DC_BLOCK_INDEX+p, scan, NULL, 16) < 0){
            return -1; //FIXME continue if partitioned and other return -1 too
        }
 
        av_assert2((cbp&15) == 0 || (cbp&15) == 15);
 
        //cbp=15=1111
        if(cbp&15){
            //If the sub-macroblock luminance residuals are all encoded
            for(i8x8=0; i8x8<4; i8x8++){
                for(i4x4=0; i4x4<4; i4x4++){
                //loop 16 times
                    const int index= i4x4 + 4*i8x8 + p*16;
                    if( decode_residual(h, h->intra_gb_ptr, h->mb + (16*index << pixel_shift),
                        index, scan + 1, h->dequant4_coeff[p][qscale], 15) < 0 ){
                        return -1;
                    }
                }
            }
            return 0xf;
        }else{
        //If the sub-macroblock luminance residual is not encoded
        // Fill in the brightness part of non_zero_count_cache with 0
            fill_rectangle(&h->non_zero_count_cache[scan8[p*16]], 4, 4, 8, 0, 1);
            return 0;
        }
    }else{
        int cqm = (IS_INTRA( mb_type ) ? 0:3)+p;
        /* For CAVLC 4:4:4, we need to keep track of the luma 8x8 CBP for deblocking nnz purposes. */
        int new_cbp = 0;
        for(i8x8=0; i8x8<4; i8x8++){
            if(cbp & (1<<i8x8)){
                if(IS_8x8DCT(mb_type)){
                    int16_t *buf = &h->mb[64*i8x8+256*p << pixel_shift];
                    uint8_t *nnz;
                    for(i4x4=0; i4x4<4; i4x4++){
                        const int index= i4x4 + 4*i8x8 + p*16;
                        if( decode_residual(h, gb, buf, index, scan8x8+16*i4x4,
                                            h->dequant8_coeff[cqm][qscale], 16) < 0 )
                            return -1;
                    }
                    nnz= &h->non_zero_count_cache[ scan8[4*i8x8+p*16] ];
                    nnz[0] += nnz[1] + nnz[8] + nnz[9];
                    new_cbp |= !!nnz[0] << i8x8;
                }else{
                    for(i4x4=0; i4x4<4; i4x4++){
                        const int index= i4x4 + 4*i8x8 + p*16;
                        //decoding residuals
                        if( decode_residual(h, gb, h->mb + (16*index << pixel_shift), index,
                                            scan, h->dequant4_coeff[cqm][qscale], 16) < 0 ){
                            return -1;
                        }
                        new_cbp |= h->non_zero_count_cache[ scan8[index] ] << i8x8;
                    }
                }
            }else{
                uint8_t * const nnz= &h->non_zero_count_cache[ scan8[4*i8x8+p*16] ];
                nnz[0] = nnz[1] = nnz[8] = nnz[9] = 0;
            }
        }
        return new_cbp;
    }
}

As can be seen from the source code, decode_luma_residual() actually calls decode_residual() to decode residual data. decode_residual() internally calls get_vlc2() to parse the CAVLC data. Since the interior of decode_residual() has not been carefully looked at, it will not be analyzed in detail for the time being.

Various information of the macroblock is output to the corresponding memory of the entire picture
ff_h264_decode_mb_cavlc() contains many functions named write_back_{XXX}(). These functions are used to copy the information of the current macroblock in the Cache to the corresponding variables of the whole picture. For example the following functions:
write_back_intra_pred_mode(): Copy the data in intra4x4_pred_mode_cache to intra4x4_pred_mode.

write_back_motion(): Copy the data in mv_cache to motion_val in the cur_pic structure; then copy the data in ref_cache to ref_index in the cur_pic structure.

write_back_non_zero_count(): Copy the data in non_zero_count_cache to non_zero_count.

Here we choose write_back_motion() to see its source code.


write_back_motion()
write_back_motion() can copy the MV in the Cache of the macroblock to the motion_val variable of the whole picture.
//Copy the MV in the Cache of the macroblock to the motion_val variable of the whole picture
static av_always_inline void write_back_motion(H264Context *h, int mb_type)
{
    const int b_stride = h->b_stride;
    const int b_xy = 4 * h->mb_x + 4 * h->mb_y * h->b_stride; // try mb2b(8)_xy
    const int b8_xy = 4 * h->mb_xy;
    //L0: Copy the MV in the Cache of the macroblock to the motion_val variable of the whole picture
    if (USES_LIST(mb_type, 0)) {
        write_back_motion_list(h, b_stride, b_xy, b8_xy, mb_type, 0);
    } else {
        fill_rectangle(&h->cur_pic.ref_index[0][b8_xy],
                       2, 2, 2, (uint8_t)LIST_NOT_USED, 1);
    }
    //L1: Copy the MV in the Cache of the macroblock to the motion_val variable of the entire picture (the last parameter is different)
    if (USES_LIST(mb_type, 1))
        write_back_motion_list(h, b_stride, b_xy, b8_xy, mb_type, 1);
 
    if (h->slice_type_nos == AV_PICTURE_TYPE_B && CABAC(h)) {
        if (IS_8X8(mb_type)) {
            uint8_t *direct_table = &h->direct_table[4 * h->mb_xy];
            direct_table[1] = h->sub_mb_type[1] >> 1;
            direct_table[2] = h->sub_mb_type[2] >> 1;
            direct_table[3] = h->sub_mb_type[3] >> 1;
        }
    }
}




As can be seen from the source code, if List0 is used, the write_back_motion_list() function will be called once (note that the last parameter is "0"); if List1 (two-way prediction) is used, the write_back_motion_list() function will be called again (note that the last parameter is "0") one parameter is "1"). Let's take a look at the write_back_motion_list() function again.

write_back_motion_list()
write_back_motion_list() is an execution function that copies the MV in the Cache of the macroblock to the motion_val variable of the entire picture. The function definition is shown below.
/Copy the MV in the Cache of the macroblock to the motion_val variable of the whole picture - this is the specific execution function
static av_always_inline void write_back_motion_list(H264Context *h,
                                                    int b_stride,
                                                    int b_xy, int b8_xy,
                                                    int mb_type, int list)
{
// Purpose: motion_val of the entire image
    int16_t(*mv_dst)[2] = &h->cur_pic.motion_val[list][b_xy];
    //Source: Cache of macroblocks, starting from scan8[0]
    int16_t(*mv_src)[2] = &h->mv_cache[list][scan8[0]];
    //The coordinates (x or y) of a motion vector occupy an int16_t
    //A macroblock has 4 motion vectors in one row
    //Each motion vector contains 2 coordinates (x and y)
    //The amount of data of a macroblock line motion vector = 16*4*2=128
    //So copy 128bit here
    AV_COPY128(mv_dst + 0 * b_stride, mv_src + 8 * 0);
    //each macroblock has 4 rows and 4 columns of motion vectors (16 in total)
    //So copy 4 lines separately
    //b_stride represents the number of motion vectors in a line of images
    AV_COPY128(mv_dst + 1 * b_stride, mv_src + 8 * 1);
    AV_COPY128(mv_dst + 2 * b_stride, mv_src + 8 * 2);
    AV_COPY128(mv_dst + 3 * b_stride, mv_src + 8 * 3);
    if (CABAC(h)) {
        uint8_t (*mvd_dst)[2] = &h->mvd_table[list][FMO ? 8 * h->mb_xy
                                                        : h->mb2br_xy[h->mb_xy]];
        uint8_t(*mvd_src)[2] = &h->mvd_cache[list][scan8[0]];
        if (IS_SKIP(mb_type)) {
            AV_ZERO128(mvd_dst);
        } else {
            AV_COPY64(mvd_dst, mvd_src + 8 * 3);
            AV_COPY16(mvd_dst + 3 + 3, mvd_src + 3 + 8 * 0);
            AV_COPY16(mvd_dst + 3 + 2, mvd_src + 3 + 8 * 1);
            AV_COPY16(mvd_dst + 3 + 1, mvd_src + 3 + 8 * 2);
        }
    }
 
    {
    //copy the reference frame number
    // Purpose: ref_index of the entire image
        int8_t *ref_index = &h->cur_pic.ref_index[list][b8_xy];
        //Source: Cache of macroblocks, starting from scan8[0]
        int8_t *ref_cache = h->ref_cache[list];
        ref_index[0 + 0 * 2] = ref_cache[scan8[0]];
        ref_index[1 + 0 * 2] = ref_cache[scan8[4]];
        ref_index[0 + 1 * 2] = ref_cache[scan8[8]];
        ref_index[1 + 1 * 2] = ref_cache[scan8[12]];
    }
}

