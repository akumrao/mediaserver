/*
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

#include <stdint.h>
#include <atomic>

extern "C" {
#include "cpu.h"
#include "cpu_internal.h"
#include "config.h"
#include "opt.h"
#include "common.h"

#if HAVE_SCHED_GETAFFINITY
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <sched.h>
#endif
#if HAVE_GETPROCESSAFFINITYMASK || HAVE_WINRT
#include <windows.h>
#endif
#if HAVE_SYSCTL
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
using namespace std;

static atomic_int cpu_flags = ATOMIC_VAR_INIT(-1);

static int get_cpu_flags(void)
{
    if (ARCH_AARCH64)
        return ff_get_cpu_flags_aarch64();
    if (ARCH_ARM)
        return ff_get_cpu_flags_arm();
    if (ARCH_PPC)
        return ff_get_cpu_flags_ppc();
    //if (ARCH_X86)
    //    return ff_get_cpu_flags_x86();
    return 0;
}

void av_force_cpu_flags(int arg){
    if (   (arg & ( AV_CPU_FLAG_3DNOW    |
                    AV_CPU_FLAG_3DNOWEXT |
                    AV_CPU_FLAG_MMXEXT   |
                    AV_CPU_FLAG_SSE      |
                    AV_CPU_FLAG_SSE2     |
                    AV_CPU_FLAG_SSE2SLOW |
                    AV_CPU_FLAG_SSE3     |
                    AV_CPU_FLAG_SSE3SLOW |
                    AV_CPU_FLAG_SSSE3    |
                    AV_CPU_FLAG_SSE4     |
                    AV_CPU_FLAG_SSE42    |
                    AV_CPU_FLAG_AVX      |
                    AV_CPU_FLAG_AVXSLOW  |
                    AV_CPU_FLAG_XOP      |
                    AV_CPU_FLAG_FMA3     |
                    AV_CPU_FLAG_FMA4     |
                    AV_CPU_FLAG_AVX2     ))
        && !(arg & AV_CPU_FLAG_MMX)) {
        av_log(NULL, AV_LOG_WARNING, "MMX implied by specified flags\n");
        arg |= AV_CPU_FLAG_MMX;
    }

    atomic_store_explicit(&cpu_flags, arg, memory_order_relaxed);
}

int av_get_cpu_flags(void)
{
    int flags = atomic_load_explicit(&cpu_flags, memory_order_relaxed);
    if (flags == -1) {
        flags = get_cpu_flags();
        atomic_store_explicit(&cpu_flags, flags, memory_order_relaxed);
    }
    return flags;
}

void av_set_cpu_flags_mask(int mask)
{
    atomic_store_explicit(&cpu_flags, get_cpu_flags() & mask,
                          memory_order_relaxed);
}

int av_parse_cpu_flags(const char *s)
{
#define CPUFLAG_MMXEXT   (AV_CPU_FLAG_MMX      | AV_CPU_FLAG_MMXEXT | AV_CPU_FLAG_CMOV)
#define CPUFLAG_3DNOW    (AV_CPU_FLAG_3DNOW    | AV_CPU_FLAG_MMX)
#define CPUFLAG_3DNOWEXT (AV_CPU_FLAG_3DNOWEXT | CPUFLAG_3DNOW)
#define CPUFLAG_SSE      (AV_CPU_FLAG_SSE      | CPUFLAG_MMXEXT)
#define CPUFLAG_SSE2     (AV_CPU_FLAG_SSE2     | CPUFLAG_SSE)
#define CPUFLAG_SSE2SLOW (AV_CPU_FLAG_SSE2SLOW | CPUFLAG_SSE2)
#define CPUFLAG_SSE3     (AV_CPU_FLAG_SSE3     | CPUFLAG_SSE2)
#define CPUFLAG_SSE3SLOW (AV_CPU_FLAG_SSE3SLOW | CPUFLAG_SSE3)
#define CPUFLAG_SSSE3    (AV_CPU_FLAG_SSSE3    | CPUFLAG_SSE3)
#define CPUFLAG_SSE4     (AV_CPU_FLAG_SSE4     | CPUFLAG_SSSE3)
#define CPUFLAG_SSE42    (AV_CPU_FLAG_SSE42    | CPUFLAG_SSE4)
#define CPUFLAG_AVX      (AV_CPU_FLAG_AVX      | CPUFLAG_SSE42)
#define CPUFLAG_AVXSLOW  (AV_CPU_FLAG_AVXSLOW  | CPUFLAG_AVX)
#define CPUFLAG_XOP      (AV_CPU_FLAG_XOP      | CPUFLAG_AVX)
#define CPUFLAG_FMA3     (AV_CPU_FLAG_FMA3     | CPUFLAG_AVX)
#define CPUFLAG_FMA4     (AV_CPU_FLAG_FMA4     | CPUFLAG_AVX)
#define CPUFLAG_AVX2     (AV_CPU_FLAG_AVX2     | CPUFLAG_AVX)
#define CPUFLAG_BMI2     (AV_CPU_FLAG_BMI2     | AV_CPU_FLAG_BMI1)
#define CPUFLAG_AESNI    (AV_CPU_FLAG_AESNI    | CPUFLAG_SSE42)
    static const AVOption cpuflags_opts[] = {
        { .name="flags" ,  	.help=NULL, 	.offset=0, 	.type=AV_OPT_TYPE_FLAGS, 	{.i64 = 0 }, 												 .min=INT64_MIN, 	.max=(double)INT64_MAX, 	.flags=0, 	.unit = "flags" },
#if   ARCH_PPC
       { .name="altivec" ,  	.help=NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_ALTIVEC  },	.min=INT64_MIN, 	.max=(double)INT64_MAX, 	.flags=0 ,	.unit = "flags" },
#elif ARCH_X86
       {.name="mmx"     , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_MMX}, 			.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="mmxext"  , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_MMXEXT},				.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse"     , 		.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE},						.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse2"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE2}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse2slow", .help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE2SLOW}, 		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse3"    ,		.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE3}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse3slow", .help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE3SLOW }, 		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="ssse3"   , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSSE3}, 				.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="atom"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_ATOM}, 		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse4.1"  , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE4}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="sse4.2"  , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_SSE42}, 				.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="avx"     , 		.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_AVX}, 						.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="avxslow" , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_AVXSLOW }, 		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="xop"     , 		.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_XOP}, 						.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="fma3"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_FMA3}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="fma4"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_FMA4}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="avx2"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_AVX2}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="bmi1"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_BMI1}, 			.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="bmi2"    , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_BMI2}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="3dnow"   , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_3DNOW}, 				.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="3dnowext",.help=  NULL,	.offset= 0,	.type= AV_OPT_TYPE_CONST,	 { .i64 = CPUFLAG_3DNOWEXT}, 		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="cmov",     	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_CMOV}, 		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       {.name="aesni"   , 	.help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = CPUFLAG_AESNI}, 					.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
#elif ARCH_ARM                                                                                                                                                                                            .
       { .name="armv5te",  .help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_ARMV5TE},	.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="armv6",    	.help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_ARMV6},		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="armv6t2",  .help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_ARMV6T2},	.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="vfp",      		.help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_VFP},			.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="vfp_vm",   	.help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_VFP_VM},	.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="vfpv3",    	.help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_VFPV3},		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="neon",     	.help= NULL, 	.offset=0,	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_NEON },		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
#elif ARCH_AARCH64                                                                                                                                                                                  
       { .name="armv8",     .help= NULL, 	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_ARMV8},		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="neon",    	.help= NULL,	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_NEON},		.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
       { .name="vfp",      		.help= NULL,	.offset=0, 	.type=AV_OPT_TYPE_CONST, 	{ .i64 = AV_CPU_FLAG_VFP},			.min=INT64_MIN, 	.max=(double)INT64_MAX,		.flags=0, 	.unit = "flags" },
#endif
       { NULL },
    };
    static const AVClass class_av = {
        .class_name = "cpuflags",
        .item_name  = av_default_item_name,
        .option     = cpuflags_opts,
        .version    = 1,
        .log_level_offset_offset=0,
        .parent_log_context_offset=0,
        .child_next=NULL,
        .child_class_next=NULL,
        .category=AV_CLASS_CATEGORY_NA
    };

    int flags = 0, ret;
    const AVClass *pclass = &class_av;

    if ((ret = av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, &flags)) < 0)
        return ret;

    return flags & INT_MAX;
}

int av_parse_cpu_caps(unsigned *flags, const char *s)
{
		static const AVOption cpuflags_opts[] = {
		{ .name="flags"	  ,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_FLAGS, { .i64 = 0  }, 														.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
#if	  ARCH_PPC
	   {.name= "altivec" ,		.help= NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ALTIVEC	 },	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
#elif ARCH_X86
	   {.name="mmx"		,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MMX		},	 		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="mmx2"	,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MMX2		},	 		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="mmxext"	,	  	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MMX2	  },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse"		,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE	   },	 			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse2"	,			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE2	 },	  	 	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse2slow",		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE2SLOW },	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse3"	,			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE3	 },	  		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse3slow",		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE3SLOW },	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="ssse3"	,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSSE3	},	  		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="atom"	,		 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ATOM	 },	   		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse4.1"	,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE4		},	  		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="sse4.2"	,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SSE42	 },	   	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="avx"		,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_AVX		},	  			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="avxslow" ,	   .help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_AVXSLOW  },	 	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="xop"		,		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_XOP	   },				.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="fma3"	,		  	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_FMA3	  },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="fma4"	,		  	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_FMA4	  },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="avx2"	,		 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_AVX2	   },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="bmi1"	,		 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_BMI1	   },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="bmi2"	,		 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_BMI2	   },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="3dnow"	,	   	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_3DNOW	 },	   	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="3dnowext",	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_3DNOWEXT },	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="cmov",			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_CMOV	   },	 		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="aesni",			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_AESNI	},	  			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },

#define CPU_FLAG_P2 AV_CPU_FLAG_CMOV | AV_CPU_FLAG_MMX
#define CPU_FLAG_P3 CPU_FLAG_P2 | AV_CPU_FLAG_MMX2 | AV_CPU_FLAG_SSE
#define CPU_FLAG_P4 CPU_FLAG_P3| AV_CPU_FLAG_SSE2
	   {.name="pentium2", 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_P2			},	  				.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="pentium3", 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_P3			},	  				.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="pentium4", 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_P4			},	  				.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },

#define CPU_FLAG_K62 AV_CPU_FLAG_MMX | AV_CPU_FLAG_3DNOW
#define CPU_FLAG_ATHLON	  CPU_FLAG_K62 | AV_CPU_FLAG_CMOV | AV_CPU_FLAG_3DNOWEXT | AV_CPU_FLAG_MMX2
#define CPU_FLAG_ATHLONXP CPU_FLAG_ATHLON | AV_CPU_FLAG_SSE
#define CPU_FLAG_K8	 CPU_FLAG_ATHLONXP | AV_CPU_FLAG_SSE2
	   { .name="k6",	   			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MMX		 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="k62",	   			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_K62		 },	   				.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="athlon",   		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_ATHLON		 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="athlonxp", 	.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_ATHLONXP	 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="k8",	   			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_K8			 },	   			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
#elif ARCH_ARM                                                                                                                                                                                             
	   { .name="armv5te", 		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV5TE	 },	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="armv6",	   		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV6	 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="armv6t2", 		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV6T2	 },	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="vfp",	   			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFP		 },			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="vfp_vm",   		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFP_VM	 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="vfpv3",	   		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFPV3	 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="neon",	   		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_NEON	 },	   		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   { .name="setend",   		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_SETEND	 },	  	.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
#elif ARCH_AARCH64
	   {.name="armv8",	  		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV8	},	  		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="neon",	  		.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_NEON		},	  		.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
	   {.name="vfp",	  			.help=NULL, .offset=0, 	.type=AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFP		},	  			.min=INT64_MIN, .max=(double)INT64_MAX, 		.flags=0, .unit = "flags" },
#endif
	   { NULL },
	};
    static const AVClass class_av = {
        .class_name = "cpuflags",
        .item_name  = av_default_item_name,
        .option     = cpuflags_opts,
        .version    = 1,
    };
    const AVClass *pclass = &class_av;

    return av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, (int*)flags);
}

int av_cpu_count(void)
{
    static volatile int printed;

    int nb_cpus = 1;
#if HAVE_WINRT
    SYSTEM_INFO sysinfo;
#endif
#if HAVE_SCHED_GETAFFINITY && defined(CPU_COUNT)
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    if (!sched_getaffinity(0, sizeof(cpuset), &cpuset))
        nb_cpus = CPU_COUNT(&cpuset);
#elif HAVE_GETPROCESSAFFINITYMASK
    DWORD_PTR proc_aff, sys_aff;
    if (GetProcessAffinityMask(GetCurrentProcess(), &proc_aff, &sys_aff))
        nb_cpus = av_popcount64(proc_aff);
#elif HAVE_SYSCTL && defined(HW_NCPU)
    int mib[2] = { CTL_HW, HW_NCPU };
    size_t len = sizeof(nb_cpus);

    if (sysctl(mib, 2, &nb_cpus, &len, NULL, 0) == -1)
        nb_cpus = 0;
#elif HAVE_SYSCONF && defined(_SC_NPROC_ONLN)
    nb_cpus = sysconf(_SC_NPROC_ONLN);
#elif HAVE_SYSCONF && defined(_SC_NPROCESSORS_ONLN)
    nb_cpus = sysconf(_SC_NPROCESSORS_ONLN);
#elif HAVE_WINRT
    GetNativeSystemInfo(&sysinfo);
    nb_cpus = sysinfo.dwNumberOfProcessors;
#endif

    if (!printed) {
        av_log(NULL, AV_LOG_DEBUG, "detected %d logical cores\n", nb_cpus);
        printed = 1;
    }

    return nb_cpus;
}
}