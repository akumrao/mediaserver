/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   H264Framer.cpp
 * Author: root
 * 
 * Created on November 22, 2021, 7:55 PM
 */

#include "H264Framer.h"
#include <cmath>

H264Framer::H264Framer() {
}

H264Framer::~H264Framer() {
}

void H264Framer::profile_tier_level(BitVector& bv, unsigned max_sub_layers_minus1) {
    bv.skipBits(96);

    unsigned i;
    Boolean sub_layer_profile_present_flag[7], sub_layer_level_present_flag[7];
    for (i = 0; i < max_sub_layers_minus1; ++i) {
        sub_layer_profile_present_flag[i] = bv.get1BitBoolean();
        sub_layer_level_present_flag[i] = bv.get1BitBoolean();
    }
    if (max_sub_layers_minus1 > 0) {
        bv.skipBits(2 * (8 - max_sub_layers_minus1)); // reserved_zero_2bits
    }
    for (i = 0; i < max_sub_layers_minus1; ++i) {
        if (sub_layer_profile_present_flag[i]) {
            bv.skipBits(88);
        }
        if (sub_layer_level_present_flag[i]) {
            bv.skipBits(8); // sub_layer_level_idc[i]
        }
    }
}

unsigned H264Framer::removeH264or5EmulationBytes(uint8_t* to, unsigned toMaxSize,
        uint8_t const* from, unsigned fromSize) {
    unsigned toSize = 0;
    unsigned i = 0;
    while (i < fromSize && toSize + 1 < toMaxSize) {
        if (i + 2 < fromSize && from[i] == 0 && from[i + 1] == 0 && from[i + 2] == 3) {
            to[toSize] = to[toSize + 1] = 0;
            toSize += 2;
            i += 3;
        } else {
            to[toSize] = from[i];
            toSize += 1;
            i += 1;
        }
    }

    return toSize;
}

void H264Framer::removeEmulationBytes(uint8_t const* nalUnitOrig, unsigned const numBytesInNALunit, uint8_t* nalUnitCopy, unsigned maxSize, unsigned& nalUnitCopySize) {

    nalUnitCopySize = removeH264or5EmulationBytes(nalUnitCopy, maxSize, nalUnitOrig, numBytesInNALunit);
}

void H264Framer::analyze_video_parameter_set_data(uint8_t const* nalUnitOrig, unsigned const numBytesInNALunit, unsigned& num_units_in_tick, unsigned& time_scale) {
    num_units_in_tick = time_scale = 0; // default values

    // Begin by making a copy of the NAL unit data, removing any 'emulation prevention' bytes:
    uint8_t vps[VPS_MAX_SIZE];
    unsigned vpsSize;
    removeEmulationBytes(nalUnitOrig, numBytesInNALunit, vps, sizeof vps, vpsSize);

    BitVector bv(vps, 0, 8 * vpsSize);

    // Assert: fHNumber == 265 (because this function is called only when parsing H.265)
    unsigned i;

    bv.skipBits(28); // nal_unit_header, vps_video_parameter_set_id, vps_reserved_three_2bits, vps_max_layers_minus1
    unsigned vps_max_sub_layers_minus1 = bv.getBits(3);
    DEBUG_PRINT(vps_max_sub_layers_minus1);
    bv.skipBits(17); // vps_temporal_id_nesting_flag, vps_reserved_0xffff_16bits
    profile_tier_level(bv, vps_max_sub_layers_minus1);
    Boolean vps_sub_layer_ordering_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vps_sub_layer_ordering_info_present_flag);
    for (i = vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1;
            i <= vps_max_sub_layers_minus1; ++i) {
        (void) bv.get_expGolomb(); // vps_max_dec_pic_buffering_minus1[i]
        (void) bv.get_expGolomb(); // vps_max_num_reorder_pics[i]
        (void) bv.get_expGolomb(); // vps_max_latency_increase_plus1[i]
    }
    unsigned vps_max_layer_id = bv.getBits(6);
    DEBUG_PRINT(vps_max_layer_id);
    unsigned vps_num_layer_sets_minus1 = bv.get_expGolomb();
    DEBUG_PRINT(vps_num_layer_sets_minus1);
    for (i = 1; i <= vps_num_layer_sets_minus1; ++i) {
        bv.skipBits(vps_max_layer_id + 1); // layer_id_included_flag[i][0..vps_max_layer_id]
    }
    Boolean vps_timing_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vps_timing_info_present_flag);
    if (vps_timing_info_present_flag) {
        DEBUG_TAB;
        num_units_in_tick = bv.getBits(32);
        DEBUG_PRINT(num_units_in_tick);
        time_scale = bv.getBits(32);
        DEBUG_PRINT(time_scale);
        Boolean vps_poc_proportional_to_timing_flag = bv.get1BitBoolean();
        DEBUG_PRINT(vps_poc_proportional_to_timing_flag);
        if (vps_poc_proportional_to_timing_flag) {
            unsigned vps_num_ticks_poc_diff_one_minus1 = bv.get_expGolomb();
            DEBUG_PRINT(vps_num_ticks_poc_diff_one_minus1);
        }
    }
    Boolean vps_extension_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vps_extension_flag);
}

void H264Framer::analyze_hrd_parameters(BitVector& bv) {
    DEBUG_TAB;
    unsigned cpb_cnt_minus1 = bv.get_expGolomb();
    DEBUG_PRINT(cpb_cnt_minus1);
    unsigned bit_rate_scale = bv.getBits(4);
    DEBUG_PRINT(bit_rate_scale);
    unsigned cpb_size_scale = bv.getBits(4);
    DEBUG_PRINT(cpb_size_scale);
    for (unsigned SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; ++SchedSelIdx) {
        DEBUG_TAB;
        DEBUG_PRINT(SchedSelIdx);
        unsigned bit_rate_value_minus1 = bv.get_expGolomb();
        DEBUG_PRINT(bit_rate_value_minus1);
        unsigned cpb_size_value_minus1 = bv.get_expGolomb();
        DEBUG_PRINT(cpb_size_value_minus1);
        Boolean cbr_flag = bv.get1BitBoolean();
        DEBUG_PRINT(cbr_flag);
    }
    unsigned initial_cpb_removal_delay_length_minus1 = bv.getBits(5);
    DEBUG_PRINT(initial_cpb_removal_delay_length_minus1);
    cpb_removal_delay_length_minus1 = bv.getBits(5);
    DEBUG_PRINT(cpb_removal_delay_length_minus1);
    dpb_output_delay_length_minus1 = bv.getBits(5);
    DEBUG_PRINT(dpb_output_delay_length_minus1);
    unsigned time_offset_length = bv.getBits(5);
    DEBUG_PRINT(time_offset_length);
}

void H264Framer::analyze_vui_parameters(BitVector& bv,
        unsigned& num_units_in_tick, unsigned& time_scale) {
    Boolean aspect_ratio_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(aspect_ratio_info_present_flag);
    if (aspect_ratio_info_present_flag) {
        DEBUG_TAB;
        unsigned aspect_ratio_idc = bv.getBits(8);
        DEBUG_PRINT(aspect_ratio_idc);
        if (aspect_ratio_idc == 255/*Extended_SAR*/) {
            bv.skipBits(32); // sar_width; sar_height
        }
    }
    Boolean overscan_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(overscan_info_present_flag);
    if (overscan_info_present_flag) {
        bv.skipBits(1); // overscan_appropriate_flag
    }
    Boolean video_signal_type_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(video_signal_type_present_flag);
    if (video_signal_type_present_flag) {
        DEBUG_TAB;
        bv.skipBits(4); // video_format; video_full_range_flag
        Boolean colour_description_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(colour_description_present_flag);
        if (colour_description_present_flag) {
            bv.skipBits(24); // colour_primaries; transfer_characteristics; matrix_coefficients
        }
    }
    Boolean chroma_loc_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(chroma_loc_info_present_flag);
    if (chroma_loc_info_present_flag) {
        (void) bv.get_expGolomb(); // chroma_sample_loc_type_top_field
        (void) bv.get_expGolomb(); // chroma_sample_loc_type_bottom_field
    }
    if (fHNumber == 265) {
        
        DeltaTfiDivisor = 1;
        
        bv.skipBits(2); // neutral_chroma_indication_flag, field_seq_flag
        Boolean frame_field_info_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(frame_field_info_present_flag);
        pic_struct_present_flag = frame_field_info_present_flag; // hack to make H.265 like H.264
        Boolean default_display_window_flag = bv.get1BitBoolean();
        DEBUG_PRINT(default_display_window_flag);
        if (default_display_window_flag) {
            (void) bv.get_expGolomb(); // def_disp_win_left_offset
            (void) bv.get_expGolomb(); // def_disp_win_right_offset
            (void) bv.get_expGolomb(); // def_disp_win_top_offset
            (void) bv.get_expGolomb(); // def_disp_win_bottom_offset
        }
    }
    Boolean timing_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(timing_info_present_flag);
    if (timing_info_present_flag) {
        DEBUG_TAB;
        num_units_in_tick = bv.getBits(32);
        DEBUG_PRINT(num_units_in_tick);
        
	#if FPSONE
	  unsigned tmpCur1 =  bv.curBitIndex();
        #endif
     
       time_scale = bv.getBits(32);
        
       #if FPSONE     
        bv.set_curBitIndex(tmpCur1);
        bv.putBits(2, 32);
       #endif
                
        DEBUG_PRINT(time_scale);
        if (fHNumber == 264) {
            Boolean fixed_frame_rate_flag = bv.get1BitBoolean();
            DEBUG_PRINT(fixed_frame_rate_flag);
        } else { // 265
            Boolean vui_poc_proportional_to_timing_flag = bv.get1BitBoolean();
            DEBUG_PRINT(vui_poc_proportional_to_timing_flag);
            if (vui_poc_proportional_to_timing_flag) {
                unsigned vui_num_ticks_poc_diff_one_minus1 = bv.get_expGolomb();
                DEBUG_PRINT(vui_num_ticks_poc_diff_one_minus1);
            }
            return; // For H.265, don't bother parsing any more of this #####
        }
    }
    // The following is H.264 only: #####
    Boolean nal_hrd_parameters_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(nal_hrd_parameters_present_flag);
    if (nal_hrd_parameters_present_flag) analyze_hrd_parameters(bv);
    Boolean vcl_hrd_parameters_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vcl_hrd_parameters_present_flag);
    if (vcl_hrd_parameters_present_flag) analyze_hrd_parameters(bv);
    CpbDpbDelaysPresentFlag = nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag;
    if (CpbDpbDelaysPresentFlag) {
        bv.skipBits(1); // low_delay_hrd_flag
    }
    pic_struct_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(pic_struct_present_flag);
}

void H264Framer::analyze_seq_parameter_set_data(uint8_t const* nalUnitOrig, unsigned const numBytesInNALunit, unsigned& num_units_in_tick, unsigned& time_scale) {
    num_units_in_tick = time_scale = 0; // default values

    // Begin by making a copy of the NAL unit data, removing any 'emulation prevention' bytes:
    uint8_t sps[SPS_MAX_SIZE];
    unsigned spsSize;
    removeEmulationBytes(nalUnitOrig, numBytesInNALunit, sps, sizeof sps, spsSize);

    BitVector bv(sps, 0, 8 * spsSize);

    if (fHNumber == 264) {
        bv.skipBits(8); // forbidden_zero_bit; nal_ref_idc; nal_unit_type
        unsigned profile_idc = bv.getBits(8);
        DEBUG_PRINT(profile_idc);
        unsigned constraint_setN_flag = bv.getBits(8); // also "reserved_zero_2bits" at end
        DEBUG_PRINT(constraint_setN_flag);
        unsigned level_idc = bv.getBits(8);
        DEBUG_PRINT(level_idc);
        unsigned seq_parameter_set_id = bv.get_expGolomb();
        DEBUG_PRINT(seq_parameter_set_id);
        if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 244 || profile_idc == 44 || profile_idc == 83 || profile_idc == 86 || profile_idc == 118 || profile_idc == 128) {
            DEBUG_TAB;
            unsigned chroma_format_idc = bv.get_expGolomb();
            DEBUG_PRINT(chroma_format_idc);
            if (chroma_format_idc == 3) {
                DEBUG_TAB;
                Boolean separate_colour_plane_flag = bv.get1BitBoolean();
                DEBUG_PRINT(separate_colour_plane_flag);
            }
            (void) bv.get_expGolomb(); // bit_depth_luma_minus8
            (void) bv.get_expGolomb(); // bit_depth_chroma_minus8
            bv.skipBits(1); // qpprime_y_zero_transform_bypass_flag
            Boolean seq_scaling_matrix_present_flag = bv.get1BitBoolean();
            DEBUG_PRINT(seq_scaling_matrix_present_flag);
            if (seq_scaling_matrix_present_flag) {
                for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); ++i) {
                    DEBUG_TAB;
                    DEBUG_PRINT(i);
                    Boolean seq_scaling_list_present_flag = bv.get1BitBoolean();
                    DEBUG_PRINT(seq_scaling_list_present_flag);
                    if (seq_scaling_list_present_flag) {
                        DEBUG_TAB;
                        unsigned sizeOfScalingList = i < 6 ? 16 : 64;
                        unsigned lastScale = 8;
                        unsigned nextScale = 8;
                        for (unsigned j = 0; j < sizeOfScalingList; ++j) {
                            DEBUG_TAB;
                            DEBUG_PRINT(j);
                            DEBUG_PRINT(nextScale);
                            if (nextScale != 0) {
                                DEBUG_TAB;
                                int delta_scale = bv.get_expGolombSigned();
                                DEBUG_PRINT(delta_scale);
                                nextScale = (lastScale + delta_scale + 256) % 256;
                            }
                            lastScale = (nextScale == 0) ? lastScale : nextScale;
                            DEBUG_PRINT(lastScale);
                        }
                    }
                }
            }
        }
        unsigned log2_max_frame_num_minus4 = bv.get_expGolomb();
        DEBUG_PRINT(log2_max_frame_num_minus4);
        unsigned pic_order_cnt_type = bv.get_expGolomb();
        DEBUG_PRINT(pic_order_cnt_type);
        if (pic_order_cnt_type == 0) {
            DEBUG_TAB;
            unsigned log2_max_pic_order_cnt_lsb_minus4 = bv.get_expGolomb();
            DEBUG_PRINT(log2_max_pic_order_cnt_lsb_minus4);
        } else if (pic_order_cnt_type == 1) {
            DEBUG_TAB;
            bv.skipBits(1); // delta_pic_order_always_zero_flag
            (void) bv.get_expGolombSigned(); // offset_for_non_ref_pic
            (void) bv.get_expGolombSigned(); // offset_for_top_to_bottom_field
            unsigned num_ref_frames_in_pic_order_cnt_cycle = bv.get_expGolomb();
            DEBUG_PRINT(num_ref_frames_in_pic_order_cnt_cycle);
            for (unsigned i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i) {
                (void) bv.get_expGolombSigned(); // offset_for_ref_frame[i]
            }
        }
        unsigned max_num_ref_frames = bv.get_expGolomb();
        DEBUG_PRINT(max_num_ref_frames);
        Boolean gaps_in_frame_num_value_allowed_flag = bv.get1BitBoolean();
        DEBUG_PRINT(gaps_in_frame_num_value_allowed_flag);
        unsigned pic_width_in_mbs_minus1 = bv.get_expGolomb();
        DEBUG_PRINT(pic_width_in_mbs_minus1);

        width= (pic_width_in_mbs_minus1 + 1) *16;
        //printf("width %d \n", width );
        

        unsigned pic_height_in_map_units_minus1 = bv.get_expGolomb();
        DEBUG_PRINT(pic_height_in_map_units_minus1);
        
        height = (pic_height_in_map_units_minus1 + 1) *16;

        //printf("height %d \n", height);

        Boolean frame_mbs_only_flag = bv.get1BitBoolean();
        DEBUG_PRINT(frame_mbs_only_flag);
        if (!frame_mbs_only_flag) {
            bv.skipBits(1); // mb_adaptive_frame_field_flag
        }
        bv.skipBits(1); // direct_8x8_inference_flag
        Boolean frame_cropping_flag = bv.get1BitBoolean();
        DEBUG_PRINT(frame_cropping_flag);
        if (frame_cropping_flag) {
            (void) bv.get_expGolomb(); // frame_crop_left_offset
            (void) bv.get_expGolomb(); // frame_crop_right_offset
            (void) bv.get_expGolomb(); // frame_crop_top_offset
            (void) bv.get_expGolomb(); // frame_crop_bottom_offset
        }
        Boolean vui_parameters_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(vui_parameters_present_flag);
        if (vui_parameters_present_flag) {
            DEBUG_TAB;
            analyze_vui_parameters(bv, num_units_in_tick, time_scale);
        }
                            
    }
    else 
    { // 265
        unsigned i;

        bv.skipBits(16); // nal_unit_header
        bv.skipBits(4); // sps_video_parameter_set_id
        unsigned sps_max_sub_layers_minus1 = bv.getBits(3);
        DEBUG_PRINT(sps_max_sub_layers_minus1);
        bv.skipBits(1); // sps_temporal_id_nesting_flag
        profile_tier_level(bv, sps_max_sub_layers_minus1);
        (void) bv.get_expGolomb(); // sps_seq_parameter_set_id
        unsigned chroma_format_idc = bv.get_expGolomb();
        DEBUG_PRINT(chroma_format_idc);
        if (chroma_format_idc == 3) bv.skipBits(1); // separate_colour_plane_flag
        unsigned pic_width_in_luma_samples = bv.get_expGolomb();
        DEBUG_PRINT(pic_width_in_luma_samples);
        width = pic_width_in_luma_samples;
        unsigned pic_height_in_luma_samples = bv.get_expGolomb();
        DEBUG_PRINT(pic_height_in_luma_samples);
        height = pic_height_in_luma_samples;
        Boolean conformance_window_flag = bv.get1BitBoolean();
        DEBUG_PRINT(conformance_window_flag);
        if (conformance_window_flag) {
            DEBUG_TAB;
            unsigned conf_win_left_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_left_offset);
            unsigned conf_win_right_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_right_offset);
            unsigned conf_win_top_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_top_offset);
            unsigned conf_win_bottom_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_bottom_offset);
        }
        (void) bv.get_expGolomb(); // bit_depth_luma_minus8
        (void) bv.get_expGolomb(); // bit_depth_chroma_minus8
        unsigned log2_max_pic_order_cnt_lsb_minus4 = bv.get_expGolomb();
        Boolean sps_sub_layer_ordering_info_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(sps_sub_layer_ordering_info_present_flag);
        for (i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1);
                i <= sps_max_sub_layers_minus1; ++i) {
            (void) bv.get_expGolomb(); // sps_max_dec_pic_buffering_minus1[i]
            (void) bv.get_expGolomb(); // sps_max_num_reorder_pics[i]
            (void) bv.get_expGolomb(); // sps_max_latency_increase[i]
        }
        (void) bv.get_expGolomb(); // log2_min_luma_coding_block_size_minus3
        (void) bv.get_expGolomb(); // log2_diff_max_min_luma_coding_block_size
        (void) bv.get_expGolomb(); // log2_min_transform_block_size_minus2
        (void) bv.get_expGolomb(); // log2_diff_max_min_transform_block_size
        (void) bv.get_expGolomb(); // max_transform_hierarchy_depth_inter
        (void) bv.get_expGolomb(); // max_transform_hierarchy_depth_intra
        Boolean scaling_list_enabled_flag = bv.get1BitBoolean();
        DEBUG_PRINT(scaling_list_enabled_flag);
        if (scaling_list_enabled_flag) {
            DEBUG_TAB;
            Boolean sps_scaling_list_data_present_flag = bv.get1BitBoolean();
            DEBUG_PRINT(sps_scaling_list_data_present_flag);
            if (sps_scaling_list_data_present_flag) {
                // scaling_list_data()
                DEBUG_TAB;
                for (unsigned sizeId = 0; sizeId < 4; ++sizeId) {
                    DEBUG_PRINT(sizeId);
                    for (unsigned matrixId = 0; matrixId < (sizeId == 3 ? 2 : 6); ++matrixId) {
                        DEBUG_TAB;
                        DEBUG_PRINT(matrixId);
                        Boolean scaling_list_pred_mode_flag = bv.get1BitBoolean();
                        DEBUG_PRINT(scaling_list_pred_mode_flag);
                        if (!scaling_list_pred_mode_flag) {
                            (void) bv.get_expGolomb(); // scaling_list_pred_matrix_id_delta[sizeId][matrixId]
                        } else {
                            unsigned const c = 1 << (4 + (sizeId << 1));
                            unsigned coefNum = c < 64 ? c : 64;
                            if (sizeId > 1) {
                                (void) bv.get_expGolomb(); // scaling_list_dc_coef_minus8[sizeId][matrixId]
                            }
                            for (i = 0; i < coefNum; ++i) {
                                (void) bv.get_expGolomb(); // scaling_list_delta_coef
                            }
                        }
                    }
                }
            }
        }
        bv.skipBits(2); // amp_enabled_flag, sample_adaptive_offset_enabled_flag
        Boolean pcm_enabled_flag = bv.get1BitBoolean();
        DEBUG_PRINT(pcm_enabled_flag);
        if (pcm_enabled_flag) {
            bv.skipBits(8); // pcm_sample_bit_depth_luma_minus1, pcm_sample_bit_depth_chroma_minus1
            (void) bv.get_expGolomb(); // log2_min_pcm_luma_coding_block_size_minus3
            (void) bv.get_expGolomb(); // log2_diff_max_min_pcm_luma_coding_block_size
            bv.skipBits(1); // pcm_loop_filter_disabled_flag
        }
        unsigned num_short_term_ref_pic_sets = bv.get_expGolomb();
        DEBUG_PRINT(num_short_term_ref_pic_sets);
        unsigned num_negative_pics = 0, prev_num_negative_pics = 0;
        unsigned num_positive_pics = 0, prev_num_positive_pics = 0;
        for (i = 0; i < num_short_term_ref_pic_sets; ++i) {
            // short_term_ref_pic_set(i):
            DEBUG_TAB;
            DEBUG_PRINT(i);
            Boolean inter_ref_pic_set_prediction_flag = False;
            if (i != 0) {
                inter_ref_pic_set_prediction_flag = bv.get1BitBoolean();
            }
            DEBUG_PRINT(inter_ref_pic_set_prediction_flag);
            if (inter_ref_pic_set_prediction_flag) {
                DEBUG_TAB;
                if (i == num_short_term_ref_pic_sets) {
                    // This can't happen here, but it's in the spec, so we include it for completeness
                    (void) bv.get_expGolomb(); // delta_idx_minus1
                }
                bv.skipBits(1); // delta_rps_sign
                (void) bv.get_expGolomb(); // abs_delta_rps_minus1
                unsigned NumDeltaPocs = prev_num_negative_pics + prev_num_positive_pics; // correct???
                for (unsigned j = 0; j < NumDeltaPocs; ++j) {
                    DEBUG_PRINT(j);
                    Boolean used_by_curr_pic_flag = bv.get1BitBoolean();
                    DEBUG_PRINT(used_by_curr_pic_flag);
                    if (!used_by_curr_pic_flag) bv.skipBits(1); // use_delta_flag[j]
                }
            } else {
                prev_num_negative_pics = num_negative_pics;
                num_negative_pics = bv.get_expGolomb();
                DEBUG_PRINT(num_negative_pics);
                prev_num_positive_pics = num_positive_pics;
                num_positive_pics = bv.get_expGolomb();
                DEBUG_PRINT(num_positive_pics);
                unsigned k;
                for (k = 0; k < num_negative_pics; ++k) {
                    (void) bv.get_expGolomb(); // delta_poc_s0_minus1[k]
                    bv.skipBits(1); // used_by_curr_pic_s0_flag[k]
                }
                for (k = 0; k < num_positive_pics; ++k) {
                    (void) bv.get_expGolomb(); // delta_poc_s1_minus1[k]
                    bv.skipBits(1); // used_by_curr_pic_s1_flag[k]
                }
            }
        }
        Boolean long_term_ref_pics_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(long_term_ref_pics_present_flag);
        if (long_term_ref_pics_present_flag) {
            DEBUG_TAB;
            unsigned num_long_term_ref_pics_sps = bv.get_expGolomb();
            DEBUG_PRINT(num_long_term_ref_pics_sps);
            for (i = 0; i < num_long_term_ref_pics_sps; ++i) {
                bv.skipBits(log2_max_pic_order_cnt_lsb_minus4); // lt_ref_pic_poc_lsb_sps[i]
                bv.skipBits(1); // used_by_curr_pic_lt_sps_flag[1]
            }
        }
        bv.skipBits(2); // sps_temporal_mvp_enabled_flag, strong_intra_smoothing_enabled_flag
        Boolean vui_parameters_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(vui_parameters_present_flag);
        if (vui_parameters_present_flag) {
            DEBUG_TAB;
            analyze_vui_parameters(bv, num_units_in_tick, time_scale);
        }
        Boolean sps_extension_flag = bv.get1BitBoolean();
        DEBUG_PRINT(sps_extension_flag);
       
    }

     if (time_scale > 0 && num_units_in_tick > 0) 
     {
        fps= std::round(  float( time_scale) / float(DeltaTfiDivisor * num_units_in_tick));
     }
     else
     {
         fps = 30; 
     }
    
}


/*
  
unsigned num_units_in_tick, time_scale;

H264VideoStreamFramer obj;
obj.analyze_seq_parameter_set_data(pbuf, 35, num_units_in_tick, time_scale);
//  analyze_seq_parameter_set_data(buffer,sz, num_units_in_tick, time_scale);
if (time_scale > 0 && num_units_in_tick > 0) {
    int fFrameRate = time_scale / (obj.DeltaTfiDivisor * num_units_in_tick);

    cout << "SPS processed frame rate " << fFrameRate << endl;


}  
 
 
 
 */
