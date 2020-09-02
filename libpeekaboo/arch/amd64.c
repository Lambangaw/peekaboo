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

void amd64_regfile_pp(regfile_amd64_t *regfile)
{
	char *gpr_string[] = {"rdi",
	                     "rsi",
	                     "rsp",
	                     "rbp",
	                     "rbx",
	                     "rdx",
	                     "rcx",
	                     "rax",
	                     "r8",
	                     "r9",
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
		printf("%s:%" PRIx64 "\n", gpr_string[x], ((uint64_t *)&(regfile->gpr))[x]);
	}
}
