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

#include "amd64.h"

void amd64_regfile_pp(uint64_t *current_register)
{
	
	printf("\tRegisters:\n");
	char *gpr_string[] = {"rdi",
	                     "rsi",
	                     "rbp",
	                     "rsp",
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

void amd64_pass_reg(uint64_t reg_value, uint64_t reg_id, uint64_t reg_rip, uint64_t (*current_register)[18]){
	(*current_register)[PEEKABOO_RIP] = reg_rip;
		switch (reg_id){
			case PEEKABOO_RDI:
				(*current_register)[PEEKABOO_RDI] = reg_value;
				break;
			case PEEKABOO_RSI:
				(*current_register)[PEEKABOO_RSI] = reg_value;
				break;
			case PEEKABOO_RBP:
				(*current_register)[PEEKABOO_RBP] = reg_value;
				break;
			case PEEKABOO_RSP:
				(*current_register)[PEEKABOO_RSP] = reg_value;
				break;
			case PEEKABOO_RBX:
				(*current_register)[PEEKABOO_RBX] = reg_value;
				break;
			case PEEKABOO_RDX:
				(*current_register)[PEEKABOO_RDX] = reg_value;
				break;
			case PEEKABOO_RCX:
				(*current_register)[PEEKABOO_RCX] = reg_value;
				break;
			case PEEKABOO_RAX:
				(*current_register)[PEEKABOO_RAX] = reg_value;
				break;
			case PEEKABOO_R8:
				(*current_register)[PEEKABOO_R8] = reg_value;
				break;
			case PEEKABOO_R9:
				(*current_register)[PEEKABOO_R9] = reg_value;
				break;
			case PEEKABOO_R10:
				(*current_register)[PEEKABOO_R10] = reg_value;
				break;
			case PEEKABOO_R11:
				(*current_register)[PEEKABOO_R11] = reg_value;
				break;
			case PEEKABOO_R12:
				(*current_register)[PEEKABOO_R12] = reg_value;
				break;
			case PEEKABOO_R13:
				(*current_register)[PEEKABOO_R13] = reg_value;
				break;
			case PEEKABOO_R14:
				(*current_register)[PEEKABOO_R14] = reg_value;
				break;
			case PEEKABOO_R15:
				(*current_register)[PEEKABOO_R15] = reg_value;
				break;
			case PEEKABOO_RFLAGS:
				(*current_register)[PEEKABOO_RFLAGS] = reg_value;
				break;
	}
}

void amd64_pass_reg_bt(uint64_t reg_value, uint64_t reg_id, uint64_t *current_register){
		switch (reg_id){
			case PEEKABOO_RDI:
				current_register[PEEKABOO_RDI] = reg_value;
				break;
			case PEEKABOO_RSI:
				current_register[PEEKABOO_RSI] = reg_value;
				break;
			case PEEKABOO_RBP:
				current_register[PEEKABOO_RBP] = reg_value;
				break;
			case PEEKABOO_RSP:
				current_register[PEEKABOO_RSP] = reg_value;
				break;
			case PEEKABOO_RBX:
				current_register[PEEKABOO_RBX] = reg_value;
				break;
			case PEEKABOO_RDX:
				current_register[PEEKABOO_RDX] = reg_value;
				break;
			case PEEKABOO_RCX:
				current_register[PEEKABOO_RCX] = reg_value;
				break;
			case PEEKABOO_RAX:
				current_register[PEEKABOO_RAX] = reg_value;
				break;
			case PEEKABOO_R8:
				current_register[PEEKABOO_R8] = reg_value;
				break;
			case PEEKABOO_R9:
				current_register[PEEKABOO_R9] = reg_value;
				break;
			case PEEKABOO_R10:
				current_register[PEEKABOO_R10] = reg_value;
				break;
			case PEEKABOO_R11:
				current_register[PEEKABOO_R11] = reg_value;
				break;
			case PEEKABOO_R12:
				current_register[PEEKABOO_R12] = reg_value;
				break;
			case PEEKABOO_R13:
				current_register[PEEKABOO_R13] = reg_value;
				break;
			case PEEKABOO_R14:
				current_register[PEEKABOO_R14] = reg_value;
				break;
			case PEEKABOO_R15:
				current_register[PEEKABOO_R15] = reg_value;
				break;
			case PEEKABOO_RFLAGS:
				current_register[PEEKABOO_RFLAGS] = reg_value;
				break;
	}
}