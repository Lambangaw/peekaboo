/* 
 * Copyright 2019 Chua Zheng Leong
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https:tware
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "amd64.h"
#include <stdlib.h>
#include <stdio.h>

void amd64_regfile_pp(uint64_t *current_register)
{
	
	printf("\tRegisters:\n");
	char *gpr_string[] = {"rdi",
	                     "rsi",
	                     "rsp",
	                     "rbp",
	                     "rbx",
	                     "rdx",
	                     "rcx",
	                     "rax",
	                     "r8 ",
	                     "r9 ",
	                     "r10",
	                     "r11",
	                     "r12",
	                     "r13",
	                     "r14",
	                     "r15",
	                     "rflags",
	                     "rip"};
	for (int x=0; x<18; x++)
	{
		printf("\t  %s: %" PRIx64 "\n", gpr_string[x], current_register[x]);
		
	}
	printf("\n");
}

void amd64_pass_reg(amd64_cur_regfile_t *cur_regfile,uint32_t offset_y, uint64_t reg_rip, uint64_t *current_register){
	current_register[17] = reg_rip;
		switch (cur_regfile->reg_id){
			case peekaboo_rdi:
				current_register[0] = cur_regfile->reg_value;
				break;
			case peekaboo_rsi:
				current_register[1] = cur_regfile->reg_value;
				break;
			case peekaboo_rsp:
				current_register[2] = cur_regfile->reg_value;
				break;
			case peekaboo_rbp:
				current_register[3] = cur_regfile->reg_value;
				break;
			case peekaboo_rbx:
				current_register[4] = cur_regfile->reg_value;
				break;
			case peekaboo_rdx:
				current_register[5] = cur_regfile->reg_value;
				break;
			case peekaboo_rcx:
				current_register[6] = cur_regfile->reg_value;
				break;
			case peekaboo_rax:
				current_register[7] = cur_regfile->reg_value;
				break;
			case peekaboo_r8:
				current_register[8] = cur_regfile->reg_value;
				break;
			case peekaboo_r9:
				current_register[9] = cur_regfile->reg_value;
				break;
			case peekaboo_r10:
				current_register[10] = cur_regfile->reg_value;
				break;
			case peekaboo_r11:
				current_register[12] = cur_regfile->reg_value;
				break;
			case peekaboo_r12:
				current_register[13] = cur_regfile->reg_value;
				break;
			case peekaboo_r13:
				current_register[14] = cur_regfile->reg_value;
				break;
			case peekaboo_r14:
				current_register[15] = cur_regfile->reg_value;
				break;
			case peekaboo_r15:
				current_register[16] = cur_regfile->reg_value;
				break;
			case peekaboo_rflags:
				current_register[17] = cur_regfile->reg_value;
				break;
	}
}