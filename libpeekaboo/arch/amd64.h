/* 
 * Copyright 2019 Chua Zheng Leong
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! @file
 *  this file is the x86-64 regfile structure & memfile structure.
 */
#ifndef __LIBPEEKABOO_AMD64_H__
#define __LIBPEEKABOO_AMD64_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#define PEEKABOO_RDI 0
#define PEEKABOO_RSI 1
#define PEEKABOO_RBP 2
#define PEEKABOO_RSP 3
#define PEEKABOO_RBX 4
#define PEEKABOO_RDX 5
#define PEEKABOO_RCX 6
#define PEEKABOO_RAX 7
#define PEEKABOO_R8 8
#define PEEKABOO_R9 9
#define PEEKABOO_R10 10
#define PEEKABOO_R11 11
#define PEEKABOO_R12 12
#define PEEKABOO_R13 13
#define PEEKABOO_R14 14
#define PEEKABOO_R15 15
#define PEEKABOO_RFLAGS 16
#define PEEKABOO_RIP 17

typedef struct storage_option_amd64{
	uint32_t has_simd;
	uint32_t has_fxsave;
} storage_option_amd64_t;

#include "../common.h"
#include "amd64_conf.h"

#define AMD64_NUM_SIMD_SLOTS 16

// enum REGISTER_GPR{
// 	peekaboo_rdi,
// 	peekaboo_rsi,
// 	peekaboo_rbp,
// 	peekaboo_rsp,
// 	peekaboo_rbx,
// 	peekaboo_rdx,
// 	peekaboo_rcx,
// 	peekaboo_rax,
// 	peekaboo_r8,
// 	peekaboo_r9,
// 	peekaboo_r10,
// 	peekaboo_r11,
// 	peekaboo_r12,
// 	peekaboo_r13,
// 	peekaboo_r14,
// 	peekaboo_r15,
// 	peekaboo_rflags
// };

/* Regfile */


typedef struct {
	uint64_t value_chg;
	uint register_number;
} amd64_cpu_gr_t;

typedef struct {
	uint64_t reg_cs;
	uint64_t reg_ss;
	uint64_t reg_ds;
	uint64_t reg_es;
	uint64_t reg_fs;
	uint64_t reg_gs;
} amd64_cpu_seg_t;

typedef struct {
	//  simd: avx2
	uint256_t ymm0;
	uint256_t ymm1;
	uint256_t ymm2;
	uint256_t ymm3;
	uint256_t ymm4;
	uint256_t ymm5;
	uint256_t ymm6;
	uint256_t ymm7;
	uint256_t ymm8;
	uint256_t ymm9;
	uint256_t ymm10;
	uint256_t ymm11;
	uint256_t ymm12;
	uint256_t ymm13;
	uint256_t ymm14;
	uint256_t ymm15;
} amd64_cpu_simd_t;

typedef struct {
	// fp registers
	uint80_t reg_st0;
	uint80_t reg_st1;
	uint80_t reg_st2;
	uint80_t reg_st3;
	uint80_t reg_st4;
	uint80_t reg_st5;
	uint80_t reg_st6;
	uint80_t reg_st7;
} amd64_cpu_st_t;

typedef struct {
	uint16_t fcw;  // FPU control word
	uint16_t fsw;  // FPU status word
	uint8_t ftw;  // Abridged FPU tag word
	uint8_t reserved_1;
	uint16_t fop;  // FPU opcode
	uint32_t fpu_ip;  // FPU instruction pointer offset
	uint16_t fpu_cs;  // FPU instruction pointer segment selector
	uint16_t reserved_2;
	uint32_t fpu_dp;  // FPU data pointer offset
	uint16_t fpu_ds;  // FPU data pointer segment selector
	uint16_t reserved_3;
	uint32_t mxcsr;  // Multimedia extensions status and control register
	uint32_t mxcsr_mask;  // Valid bits in mxcsr
	uint128_t st_mm[8];  // 8 128-bits FP Registers
	uint128_t xmm[16];  // 16 128-bits XMM Regiters
	uint8_t padding[96]; // 416 Bytes are used. The total area should be 512 bytes.
} __attribute__((packed)) fxsave_area_t;


typedef struct regfile_amd64{
	
	amd64_cpu_gr_t gpr;
} regfile_amd64_t;

typedef struct _regfile_nongpr_t{
	amd64_cpu_simd_t simd;
	fxsave_area_t fxsave;
}regfile_nongpr_amd64_t;

void amd64_regfile_pp(uint64_t *regfile);
void amd64_pass_reg(uint64_t reg_value, uint64_t reg_id,uint32_t offset_y, uint64_t reg_rip, uint64_t *current_register);
void amd64_pass_reg_bt(uint64_t reg_value, uint64_t reg_id,uint32_t offset_y, uint64_t *current_register);
/* End of Regfile */

#endif
