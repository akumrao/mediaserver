/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   H264Framer.h
 * Author: root
 *
 * Created on November 22, 2021, 7:55 PM
 */

#ifndef H264VIDEOSTREAMFRAMER_H
#define H264VIDEOSTREAMFRAMER_H


#include<iostream>
#include<fstream>
#include<string.h>
#include<cstdio>
#include<stdio.h>

#include "BitVector.h"


///////////////////////////////////////////

#ifdef DEBUG
static unsigned numDebugTabs = 1;
#define DEBUG_PRINT_TABS for (unsigned _i = 0; _i < numDebugTabs; ++_i) fprintf(stderr, "\t")
#define DEBUG_PRINT(x) do { DEBUG_PRINT_TABS; fprintf(stderr, "%s: %d\n", #x, x); } while (0)
#define DEBUG_STR(x) do { DEBUG_PRINT_TABS; fprintf(stderr, "%s\n", x); } while (0)
class DebugTab {
public:
  DebugTab() {++numDebugTabs;}
  ~DebugTab() {--numDebugTabs;}
};
#define DEBUG_TAB DebugTab dummy
#else
#define DEBUG_PRINT(x) do {x = x;} while (0)
    // Note: the "x=x;" statement is intended to eliminate "unused variable" compiler warning messages
#define DEBUG_STR(x) do {} while (0)
#define DEBUG_TAB do {} while (0)
#endif

#define VPS_MAX_SIZE 1000 // larger than the largest possible VPS (Video Parameter Set) NAL unit
#define SPS_MAX_SIZE 1000 // larger than the largest possible SPS (Sequence Parameter Set) NAL unit 


class H264Framer {
public:
    H264Framer();
     virtual ~H264Framer();
     
     void profile_tier_level(BitVector& bv, unsigned max_sub_layers_minus1);
     unsigned removeH264or5EmulationBytes(uint8_t* to, unsigned toMaxSize,
                                     uint8_t const* from, unsigned fromSize);
     
     void removeEmulationBytes( uint8_t const* nalUnitOrig, unsigned const numBytesInNALunit,    uint8_t* nalUnitCopy, unsigned maxSize, unsigned& nalUnitCopySize);
     
     void analyze_video_parameter_set_data(uint8_t const* nalUnitOrig, unsigned const numBytesInNALunit, unsigned& num_units_in_tick, unsigned& time_scale);
     void analyze_hrd_parameters(BitVector& bv);
     
     void analyze_vui_parameters(BitVector& bv,
			 unsigned& num_units_in_tick, unsigned& time_scale);
     
     void analyze_seq_parameter_set_data(uint8_t const* nalUnitOrig, unsigned const numBytesInNALunit, unsigned& num_units_in_tick, unsigned& time_scale) ;
     
private:
    
  public:
    
    int fHNumber {264}; // 264 or 265   
    Boolean CpbDpbDelaysPresentFlag{0};
    Boolean pic_struct_present_flag{0};

    unsigned cpb_removal_delay_length_minus1{23};
    unsigned dpb_output_delay_length_minus1{23};

    double DeltaTfiDivisor{2.0};

    int fps{0};
    int width{0};
    int height{0};
    

};

#endif /* H264VIDEOSTREAMFRAMER_H */

