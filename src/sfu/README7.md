Keep in mind  PTS/DTS are for mpeg or mp4 nothing to do with Nalu ( sps and pps)


1 Where did SPS and PPS come from?
2 What role does each parameter in SPS and PPS play?
3 How to parse the SPS and PPS strings of H.264 contained in SDP?


When doing client video decoding, Wireshark packet capture tool is generally used to analyze the received H264 code stream, as shown below:

Here we can see SPS  and PPS that play a key role in decoding video.

SPS ( FPS, resolution)

PPS( Entropy of frame( CBACL or VLC, slice ))



Start Dumping SPS:
PROFILE_IDC = 66
Constrained_set0_flag = 1
Constrained_set1_flag = 1
Constrained_set2_flag = 1
Constrained_set3_flag = 0
LEVEL_IDC = 20
seq_parameter_set_id = 0
CHROMA_FORMAT_IDC = 1
BIT_DEPTH_LUMA_MINUS8 = 0
BIT_DEPTH_CHROMA_MINUS8 = 0
Seq_scaling_matrix_present_flag = 0
LOG2_MAX_FRAME_NUM_MINUS4 = 0
Pic_order_cnt_type = 2
LOG2_MAX_PIC_ORDER_CNT_LSB_MINUS4 = 0
Delta_pic_order_always_zero_flag = 0
Offset_for_non_ref_pic = 0
Offset_for_top_to_bottom_field = 0
num_ref_frames_in_pic_order_cnt_cycle = 0
Num_ref_frames = 1
Gaps_in_frame_num_value_allowed_flag = 0
PIC_WIDTH_IN_MBS_MINUS1 = 21
Pic_height_in_mbs_minus1 = 17
Frame_mbs_only_flag = 1
Mb_adaptive_frame_field_flag = 0
Direct_8x8_interence_flag = 0
Frame_cropping_flag = 0
Frame_cropping_rect_left_offset = 0
Frame_cropping_rect_right_offset = 0
Frame_cropping_rect_top_offset = 0
Frame_cropping_rect_bottom_offset = 0
Vui_parameters_present_flag = 0

Start Dumping PPS:
pic_parameter_set_id = 0
seq_parameter_set_id = 0
Entropy_coding_mode_flag = 0
Pic_order_present_flag = 0
NUM_SLICE_GROUPS_MINUS1 = 0
Slice_group_map_type = 0
NUM_REF_IDX_L0_ACTIVE_MINUS1 = 0
NUM_REF_IDX_L1_ACTIVE_MINUS1 = 0
Weighted_pref_flag = 0
WEIGHTED_BIPRED_IDC = 0
PIC_INIT_QP_MINUS26 = 0
PIC_INIT_QS_MINUS26 = 0
Chroma_qp_index_offset = 10
Deblocking_filter_control_present_flag = 1
Constrained_intra_pred_flag = 0
Redundant_pic_cnt_present_flag = 0
Transform_8x8_mode_flag = 0
Pic_scaling_matrix_present_flag = 0
Second_chroma_qp_index_offset = 10

SPS
/////////////////////////////////////////////////////////////////////////////////////////////////
These two parameters need to be specifically mentioned here
PIC_WIDTH_IN_MBS_MINUS1 = 21
Pic_height_in_mbs_minus1 = 17
Represents the width and height of the image, minus 1 in the macro block (16x16) value, respectively
Therefore, the actual width is (21+1) *16 = 352


SPS Nal unit.

Some streams I found
timing_info_present_flag =1
num_units_in_tick =2002
time_scale = 60000
fixed_frame_rate_flag=0

which gives the framerate 60000/2*2002 = 15fps
In general, fps =  time_scale / (2 * num_units_in_tick)




#2 Detailed explanation of SPS PPS#

2.1 SPS syntax elements and their meanings

A variety of different NAL Unit types are specified in the H.264 standard protocol. Type 7 indicates that the data stored in the NAL Unit is Sequence Paramater Set. Among the various syntax elements of H.264, the information in SPS is very important. If the data is lost or there is an error, the decoding process is likely to fail. SPS and the image parameter set PPS that will be described later in the video processing framework of some platforms (such as iOS VideoToolBox, etc.) are also usually used as the initialization information of the decoder instance.

SPS stands for Sequence Paramater Set, also known as sequence parameter set. A set of global parameters of a coded video sequence (Coded video sequence) is saved in the SPS. The so-called coded video sequence refers to a sequence composed of a frame of pixel data of the original video after being encoded. The parameters on which the encoded data of each frame depend are stored in the image parameter set. In general, the NAL Unit of SPS and PPS is usually located at the beginning of the entire code stream. However, in some special cases, these two structures may also appear in the middle of the code stream. The main reasons may be:

The decoder needs to start decoding in the middle of the code stream;
The encoder changes the parameters of the code stream (such as image resolution, etc.) during the encoding process;
When making a video player, in order for the subsequent decoding process to use the parameters contained in the SPS, the data must be parsed. The SPS format specified in the H.264 standard protocol is located in the section 7.3.2.1.1 of the document, as shown in the following figure:

Each grammatical element and its meaning are as follows:

(1) profile_idc:

Identifies the profile of the current H.264 bitstream. We know that three commonly used profiles are defined in H.264:

Baseline profile: baseline profile;

Main grade: main profile;

Extended profile: extended profile;

In H.264 SPS, the first byte represents profile_idc. According to the value of profile_idc, it can be determined which grade the code stream conforms to. The judgment rule is:

profile_idc = 66 → baseline profile;

profile_idc = 77 → main profile;

profile_idc = 88 → extended profile;

In the new version of the standard, it also includes High, High 10, High 4:2:2, High 4:4:4, High 10 Intra, High 4:2:2 Intra, High 4:4:4 Intra, CAVLC 4 :4:4 Intra, etc., each of which is represented by a different profile_idc.

In addition, constraint_set0_flag ~ constraint_set5_flag are other additional restrictive conditions added to the code stream in terms of the encoding level.

In our experimental code stream, profile_idc = 0x42 = 66, so the grade of the code stream is the baseline profile.

(2) level_idc

Identifies the Level of the current code stream. The encoding level defines parameters such as the maximum video resolution and the maximum video frame rate under certain conditions, and the level followed by the code stream is specified by level_idc.

In the current code stream, level_idc = 0x1e = 30, so the level of the code stream is 3.

(3) seq_parameter_set_id

Represents the id of the current sequence parameter set. Through the id value, the image parameter set pps can refer to the parameters in the sps it represents.

(4) log2_max_frame_num_minus4

Used to calculate the value of MaxFrameNum. The calculation formula is MaxFrameNum = 2^(log2_max_frame_num_minus4 + 4). MaxFrameNum is the upper limit of frame_num, and frame_num is a way of representing the image sequence number, which is often used as a means of reference frame marking in inter-frame coding.

(5) pic_order_cnt_type

Represents the method of decoding picture order count (POC). POC is another way to measure image serial numbers, and it has a different calculation method from frame_num. The value of this syntax element is 0, 1, or 2.

(6) log2_max_pic_order_cnt_lsb_minus4

Used to calculate the value of MaxPicOrderCntLsb, which represents the upper limit of the POC. The calculation method is MaxPicOrderCntLsb = 2^(log2_max_pic_order_cnt_lsb_minus4 + 4).

(7) max_num_ref_frames

Used to indicate the maximum number of reference frames.

(8) gaps_in_frame_num_value_allowed_flag

The flag indicates whether discontinuous values ​​are allowed in frame_num.

(9) pic_width_in_mbs_minus1

Used to calculate the width of the image. The unit is the number of macroblocks, so the actual width of the image is:

frame_width = 16 × (pic\_width\_in\_mbs_minus1 + 1);

(10) pic_height_in_map_units_minus1

Use PicHeightInMapUnits to measure the height of a frame of image in the video. PicHeightInMapUnits is not the explicit height of the image in pixels or macroblocks, but it needs to consider whether the macroblock is frame coded or field coded. The calculation method of PicHeightInMapUnits is:

PicHeightInMapUnits = pic\_height\_in\_map\_units\_minus1 + 1;

(11) frame_mbs_only_flag

The flag indicates the encoding method of the macro block. When the flag is 0, the macro block may be frame coded or field coded; when the flag is 1, all macro blocks use frame coding. According to the value of the flag, the meaning of PicHeightInMapUnits is also different. When it is 0, it means the height of a field of data calculated according to the macroblock, and when it is 1, it means the height of a frame of data calculated according to the macroblock.

The calculation method of the actual height of the image FrameHeightInMbs calculated according to the macro block is:

FrameHeightInMbs = (2 − frame_mbs_only_flag) * PicHeightInMapUnits

(12) mb_adaptive_frame_field_flag

The flag indicates whether the frame-field adaptive coding at the macro block level is used. When the flag is 0, there is no switch between frame coding and field coding; when the flag is 1, the macro block may choose between frame coding and field coding mode.

(13) direct_8x8_inference_flag

Flag bit, used for derivation and calculation of motion vector in B_Skip and B_Direct mode.

(14) frame_cropping_flag

The flag indicates whether the output image frame needs to be cropped.

(15) vui_parameters_present_flag

The flag indicates whether there is VUI information in the SPS.

2.2 PPS syntax elements and their meanings

In addition to the sequence parameter set SPS, another important parameter set in H.264 is the Picture Paramater Set (PPS). Normally, PPS is similar to SPS. It is stored separately in a NAL Unit in the H.264 bare bit stream, but the nal_unit_type value of PPS NAL Unit is 8; and in the encapsulation format, PPS is usually stored together with SPS. In the file header of the video file.

In the H.264 protocol document, the structure of PPS is defined in section 7.3.2.2. The specific structure is shown in the following table:

Each grammatical element and its meaning are as follows:

(1) pic_parameter_set_id

Represents the id of the current PPS. A certain PPS will be referenced by the corresponding slice in the code stream. The way a slice refers to the PPS is to save the id value of the PPS in the slice header. The range of this value is [0,255].

(2) seq_parameter_set_id

Indicates the id of the active SPS referenced by the current PPS. In this way, the parameters in the corresponding SPS can also be obtained in the PPS. The range of this value is [0,31].

(3) entropy_coding_mode_flag

Entropy coding mode identification, the identification bit indicates the entropy encoding/decoding algorithm selected in the code stream. For some syntax elements, under different encoding configurations, the selected entropy encoding methods are different. For example, in a macro block syntax element, the syntax element descriptor of the macro block type mb_type is "ue(v) | ae(v)". Exponential Columbus coding is used under settings such as baseline profile, and CABAC is used under settings such as main profile. coding.

The role of the flag entropy_coding_mode_flag is to control this algorithm selection. When the value is 0, the algorithm on the left is selected, usually exponential Columbus coding or CAVLC; when the value is 1, the algorithm on the right is selected, usually CABAC.

(4) bottom_field_pic_order_in_frame_present_flag

Flag bit, used to indicate whether the two syntax elements delta_pic_order_cnt_bottom and delta_pic_order_cn in another strip header exist. These two syntax elements represent the calculation method of the POC of the bottom field of a certain frame.

(5) num_slice_groups_minus1

Represents the number of slice groups in a frame. When the value is 0, all slices in a frame belong to a slice group. A slice group is a combination of macroblocks in a frame, which is defined in section 3.141 of the protocol document.

(6) num_ref_idx_l0_default_active_minus1, num_ref_idx_l0_default_active_minus1

Indicates that when the num_ref_idx_active_override_flag flag in the Slice Header is 0, the default values ​​of the syntax elements num_ref_idx_l0_active_minus1 and num_ref_idx_l1_active_minus1 of the P/SP/B slice.

(7) weighted_pred_flag

The flag indicates whether weighted prediction is enabled in the P/SP slice.

(8) weighted_bipred_idc

Represents the method of weighted prediction in B Slice, the value range is [0,2]. 0 means default weighted prediction, 1 means explicit weighted prediction, and 2 means implicit weighted prediction.

(9) pic_init_qp_minus26 and pic_init_qs_minus26

Indicates the initial quantization parameter. The actual quantization parameter is calculated from this parameter and slice_qp_delta/slice_qs_delta in the slice header.

(10) chroma_qp_index_offset

The quantization parameter used to calculate the chrominance component, the value range is [-12,12].

(11) deblocking_filter_control_present_flag

The flag is used to indicate whether there is information for deblocking filter control in the Slice header. When the flag bit is 1, the slice header contains corresponding information for deblocking filtering; when the flag bit is 0, there is no corresponding information in the slice header.

(12) constrained_intra_pred_flag

If the flag is 1, it means that the I macroblock can only use the information from the I and SI type macroblocks when performing intra prediction; if the flag bit is 0, it means that the I macroblock can use the information from the Inter type macroblock.

(13) redundant_pic_cnt_present_flag

The flag is used to indicate whether there is a redundant_pic_cnt syntax element in the Slice header. When the flag bit is 1, the slice header contains redundant_pic_cnt; when the flag bit is 0, there is no corresponding information in the slice header.

3 Analyze the SPS and PPS strings of H.264 contained in SDP

When using RTP to transmit H264, you need to use the sdp protocol description, of which there are two: Sequence Parameter Sets (SPS) and Picture Parameter Set (PPS) need to be used, so where do you get these two? The answer is from the H264 code Obtained from the stream. In the H264 code stream, the start code is "0x00 0x00 0x01" or "0x00 0x00 0x00 0x01". After the start code is found, the lower 5 bits of the first byte after the start code are used to determine Whether it is 7 (sps) or 8 (pps), and data[4] & 0x1f == 7 || data[4] & 0x1f == 8. Then remove the start code of the acquired nal and perform base64 encoding to obtain the information It can be used for sdp.sps and pps need to be separated by commas.

The SPS and PPS strings of H.264 in the SDP contain the information parameters needed to initialize the H.264 decoder, including the profile and level used for encoding, the width and height of the image, and the deblock filter.

Because of the SDP SPS and PPS are BASE64 encoded form, not easy to understand, there is a software tool can parse the SDP in the SPS and PPS, Download: http://download.csdn.net/download/davebobo/9898045.

Usage is to enter in the command line:

spsparser sps.txt pps.txt output.txt

For example, the content in sps.txt is:

Z0LgFNoFglE=

The content in pps.txt is:

aM4wpIA=

The final analysis result is:

Here we need to mention these two parameters in particular

pic_width_in_mbs_minus1 = 21

pic_height_in_mbs_minus1 = 17

Represents the width and height of the image respectively, the value in the unit of macro block (16x16) minus 1

Therefore, the actual width is (21+1)*16 = 352 and the height is (17+1)*16 = 288

At this point, you should know the problems left by the first part of the client-side capture and calculation of image width and height.

Reference:

http://www.cnblogs.com/lidabo/p/6553305.html

http://blog.csdn.net/heanyu/article/details/6205390

http://blog.csdn.net/shaqoneal/article/details/52771030

http://blog.csdn.net/shaqoneal