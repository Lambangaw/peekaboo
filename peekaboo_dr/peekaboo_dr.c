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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h> /* for offsetof */
#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h> /* getopt */

#include "dr_api.h"
#include "drmgr.h"
#include "drreg.h"
#include "drutil.h"
#include "drx.h"
#include "dr_defines.h"

#include "libpeekaboo.h"

static int offset_regfile_index;
static int num_dst_buf;
static uint32_t not_store_reginfo;
static uint32_t not_store_meminfo;


#define MAX_NUM_INS_REFS 8192
#define INSN_REF_SIZE (sizeof(insn_ref_t) * MAX_NUM_INS_REFS)

#define MAX_NUM_REG_REFS 8192
#define REG_BUF_SIZE (sizeof(regfile_t) * MAX_NUM_REG_REFS)

#define MAX_NUM_MEM_REFS 8192
#define MEM_REFS_SIZE (sizeof(memref_t) * MAX_NUM_MEM_REFS)

#define MAX_NUM_MEM_REFS 8192
#define MEMFILE_SIZE (sizeof(memfile_t) * MAX_NUM_MEM_REFS)

#define MAX_NUM_BYTES_MAP 128
#define MAX_BYTES_MAP_SIZE (sizeof(bytes_map_t) * MAX_NUM_BYTES_MAP)

#define MAX_NUM_OFFSET_REGFILE 8192
#define OFFSET_REGFILE_SIZE (sizeof(offset_regfile_t) * MAX_NUM_OFFSET_REGFILE)

#define MAX_NUM_NONGPR_REGFILE 8192
#define NONGPR_REGFILE_SIZE (sizeof(regfile_nongpr_t) * MAX_NUM_NONGPR_REGFILE)



typedef struct {
	peekaboo_trace_t *peek_trace;
	uint64_t num_refs;
	int num_reg_change;
	uint64_t reg_flags;
	uint64_t reg_rsp;
	int ftr_reg_id[17];
	int cur_reg_id[17];
} per_thread_t;

static client_id_t client_id;
static void *mutex;     /* for multithread support */
static uint64 num_refs; /* keep a global instruction reference count */
static int num_reg_change;
static process_id_t root_pid; /* root process pid */
static FILE *bytes_map_file;
static char trace_dir[256];
static int tls_idx;


static drx_buf_t *insn_ref_buf;
static drx_buf_t *bytes_map_buf;
static drx_buf_t *regfile_buf;
static drx_buf_t *memrefs_buf;
static drx_buf_t *memfile_buf;
static drx_buf_t *offset_regfile_buf;
static drx_buf_t *nongpr_regfile_buf;


#ifdef X86
	#ifdef X64
		char *arch_str = "AMD64";

		enum ARCH arch = ARCH_AMD64;
		typedef regfile_amd64_t regfile_t;
		typedef regfile_nongpr_amd64_t regfile_nongpr_t;
		const storage_option_amd64_t storage_option;
		void save_register(void *drcontext, uint64_t reg_id, uint64_t value);
		void save_offset_register(void *drcontext, offset_regfile_t *offset_regfile, per_thread_t *data)
		{
			offset_regfile_index = offset_regfile_index + num_dst_buf;
			num_dst_buf = data->num_reg_change;
			offset_regfile->offset_idx = offset_regfile_index;
			offset_regfile->num_register_change = data->num_reg_change;
		}

		void save_regfile(void *drcontext, dr_mcontext_t *mc, per_thread_t *data){
			if(data->reg_rsp != 0){
				for(int i=0;i<=15;i++){
					if(data->cur_reg_id[i]){
						save_register(drcontext,i,((uint64_t *)&(mc))[i+18]);
					}
				}
				if(data->reg_flags != mc->xflags){
					data->reg_flags = mc->xflags;
					save_register(drcontext,16,mc->xflags);
					data->cur_reg_id[PEEKABOO_RFLAGS] = 1;
				}
			}else{
				data->reg_rsp = mc->rsp;
				save_register(drcontext,3,mc->rsp);
				data->cur_reg_id[PEEKABOO_RSP] = 1;
			}
			data->num_reg_change = 0;
			for(int i=0;i<17;i++){
				data->num_reg_change += data->cur_reg_id[i];
				}
			memset(&data->cur_reg_id,0,sizeof(data->cur_reg_id));
			memcpy(&data->cur_reg_id,&data->ftr_reg_id,sizeof(data->ftr_reg_id) );
			data->cur_reg_id[0] = data->ftr_reg_id[0];
			
		}

		
		
		void storing_nongpr(regfile_nongpr_t *regfile_nongpr, dr_mcontext_t *mc){
			#if defined _STORE_SIMD 
			memcpy(&regfile_nongpr->simd, mc->ymm, sizeof(regfile_nongpr->simd.ymm0)*MCXT_NUM_SIMD_SLOTS);
			#endif
			#ifdef _STORE_FXSAVE
			proc_save_fpstate((byte *)&regfile_nongpr->fxsave);
			#endif
		}

	#else
		char *arch_str = "X86";
		enum ARCH arch = ARCH_X86;
		typedef regfile_x86_t regfile_t;
		void copy_regfile(regfile_t *regfile_ptr, dr_mcontext_t *mc)
		{
			regfile_ptr->gpr.reg_eax = mc->eax;
			regfile_ptr->gpr.reg_ecx = mc->ecx;
			regfile_ptr->gpr.reg_edx = mc->edx;
			regfile_ptr->gpr.reg_ebx = mc->ebx;
			regfile_ptr->gpr.reg_esp = mc->esp;
			regfile_ptr->gpr.reg_ebp = mc->ebp;
			regfile_ptr->gpr.reg_esi = mc->esi;
			regfile_ptr->gpr.reg_edi = mc->edi;
		}
	#endif
#else
	#ifdef X64
		char *arch_str = "AArch64";
		enum ARCH arch = ARCH_AARCH64;
		typedef regfile_aarch64_t regfile_t;
		void copy_regfile(regfile_t *regfile_ptr, dr_mcontext_t *mc)
		{
			memcpy(&regfile_ptr->gpr, &mc->r0, 33*8 + 3*4);
			memcpy(&regfile_ptr->v, &mc->simd, MCXT_NUM_SIMD_SLOTS*sizeof(regfile_ptr->v[0]));
		}
	#else
		char *arch_str = "AArch32";
		// TODO: Implement ARM stuff here
	#endif
#endif

static struct option const long_options[] =
{
  {"noreginfo", no_argument, NULL, 'r'},
  {"nomeminfo", no_argument, NULL, 'm'},
  {NULL, 0, NULL, 0}
};


static void flush_insnrefs(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(insn_ref_t);
	DR_ASSERT(size % sizeof(insn_ref_t) == 0);
	fwrite(buf_base, sizeof(insn_ref_t), count, data->peek_trace->insn_trace);
	data->num_refs += count;
}

static void flush_regfile(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(regfile_t);
	DR_ASSERT(size % sizeof(regfile_t) == 0);
	// fwrite(buf_base, sizeof(regfile_t), count, data->peek_trace->regfile);
}

static void flush_memrefs(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(memref_t);
	DR_ASSERT(size % sizeof(memref_t) == 0);
	fwrite(buf_base, sizeof(memref_t), count, data->peek_trace->memrefs);
}

static void flush_memfile(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(memfile_t);
	DR_ASSERT(size % sizeof(memfile_t) == 0);
	fwrite(buf_base, sizeof(memfile_t), count, data->peek_trace->memfile);
}

static void flush_offset_regfile(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(offset_regfile_t);
	DR_ASSERT(size % sizeof(offset_regfile_t) == 0);
	fwrite(buf_base, sizeof(offset_regfile_t), count, data->peek_trace->offset_regfile);
	
}

#if defined _STORE_SIMD || defined _STORE_FXSAVE
static void flush_nongpr_regfile(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(regfile_nongpr_t);
	DR_ASSERT(size % sizeof(regfile_nongpr_t) == 0);
	fwrite(buf_base, sizeof(regfile_nongpr_t), count, data->peek_trace->nongpr_regfile);
}
#endif
/*

static void flush_map(void *drcontext, void *buf_base, size_t size)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	size_t count = size / sizeof(bytes_map_t);
	DR_ASSERT(size % sizeof(bytes_map_t) == 0);
	fwrite(buf_base, sizeof(bytes_map_t), count, data->peek_trace->bytes_map);
}

*/


// KH: This function actually messes up the buffer flush. Need to fix it! 
static dr_signal_action_t event_signal(void *drcontext, dr_siginfo_t *info)
{
	/* Flush data in buffers when receiving SIGINT(2), SIGABRT(6), SIGSEGV(11) */
	if ((info->sig == SIGINT) || (info->sig == SIGABRT) || (info->sig ==  SIGSEGV))
	{
		printf("Peekaboo: Signal %d caught.\n", info->sig);
		per_thread_t *data;
		data = drmgr_get_tls_field(drcontext, tls_idx);
		dr_mutex_lock(mutex);

		flush_insnrefs(drcontext, insn_ref_buf, INSN_REF_SIZE);
		if(!not_store_meminfo)
		{
			flush_memfile(drcontext, memfile_buf, MEMFILE_SIZE);
			flush_memrefs(drcontext, memrefs_buf, MEM_REFS_SIZE);
		}
		
		if(!not_store_reginfo)
		{
			flush_offset_regfile(drcontext, offset_regfile_buf, OFFSET_REGFILE_SIZE);
			#if defined _STORE_SIMD || defined _STORE_FXSAVE
			flush_nongpr_regfile(drcontext, nongpr_regfile_buf, NONGPR_REGFILE_SIZE);
			#endif
		}

		fflush(data->peek_trace->insn_trace);
		fflush(data->peek_trace->bytes_map);
		fflush(data->peek_trace->metafile);
		if(!not_store_meminfo)
		{
			fflush(data->peek_trace->memfile);
			fflush(data->peek_trace->memrefs);
		}
		if(!not_store_reginfo)
		{
			fflush(data->peek_trace->regfile);
			fflush(data->peek_trace->offset_regfile);
			#if defined _STORE_SIMD || defined _STORE_FXSAVE
			fflush(data->peek_trace->nongpr_regfile);
			#endif
		}
		dr_mutex_unlock(mutex);
	}

	/* Deliver the signal to app */
	return DR_SIGNAL_DELIVER;
}


void save_register(void *drcontext, uint64_t reg_id, uint64_t value)
{
	regfile_t *regfile_ptr;
	regfile_ptr = (regfile_t *) drx_buf_get_buffer_ptr(drcontext, regfile_buf);
	
		regfile_ptr->gpr.value_chg = value;
		regfile_ptr->gpr.register_number = reg_id;
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	fwrite(regfile_ptr, sizeof(regfile_t), 1, data->peek_trace->regfile);
}


void save_nongpr_register()
{
	void *drcontext = dr_get_current_drcontext();
	regfile_nongpr_t *regfile_nongpr_ptr;
	regfile_nongpr_ptr = (regfile_nongpr_t *) drx_buf_get_buffer_ptr(drcontext, nongpr_regfile_buf);
	dr_mcontext_t mc = {sizeof(mc), DR_MC_ALL};
	dr_get_mcontext(drcontext, &mc);
	storing_nongpr(regfile_nongpr_ptr,&mc);
}

static void trace_offset_register(app_pc pc)
{
	
	void *drcontext = dr_get_current_drcontext();
	per_thread_t *data = drmgr_get_tls_field(drcontext,tls_idx);
	offset_regfile_t *offset_regfile_ptr;
	offset_regfile_ptr = (offset_regfile_t *) drx_buf_get_buffer_ptr(drcontext, offset_regfile_buf);
	instr_t * instr = instr_create(drcontext);
	decode(drcontext, pc, instr);
	save_offset_register(drcontext, offset_regfile_ptr, data );
	
	
}


static void copy_regfile(app_pc pc)
{
	void *drcontext = dr_get_current_drcontext();
	dr_mcontext_t mc = {sizeof(mc), DR_MC_ALL};
	per_thread_t *data = drmgr_get_tls_field(drcontext,tls_idx);
	dr_get_mcontext(drcontext, &mc);
	instr_t * instr = instr_create(drcontext);
	decode(drcontext, pc, instr);
	memset(&data->ftr_reg_id,0,sizeof(data->ftr_reg_id));
	if(instr_is_syscall(instr)){
		data->ftr_reg_id[PEEKABOO_RAX] = 1;
	}
	int num_dst = instr_num_dsts(instr);
	for(int i=0;i<num_dst;i++)
	{
		opnd_t dst = instr_get_dst(instr,i);
		if(opnd_is_reg(dst)){
			reg_id_t reg_id = opnd_get_reg(dst);
			if(reg_id < 53 || (reg_id > 56 && reg_id <65) ){
				int simp_reg_id = reg_id % 16;
				switch(simp_reg_id){
					case 0:
						data->ftr_reg_id[PEEKABOO_R15] = 1;
						break;
					case 1:
						data->ftr_reg_id[PEEKABOO_RAX] = 1;
						break;
					case 2:
						data->ftr_reg_id[PEEKABOO_RBP] = 1;
						break;
					case 3:
						data->ftr_reg_id[PEEKABOO_RDX] = 1;
						break;
					case 4:
						data->ftr_reg_id[PEEKABOO_RBX] = 1;
						break;
					case 5:
						data->ftr_reg_id[PEEKABOO_RSP] = 1;
						break;
					case 6:
						data->ftr_reg_id[PEEKABOO_RBP] = 1;
						break;
					case 7:
						data->ftr_reg_id[PEEKABOO_RSI] = 1;
						break;
					case 8:
						data->ftr_reg_id[PEEKABOO_RDI] = 1;
						break;
					case 9:
						data->ftr_reg_id[PEEKABOO_R8] = 1;
						break;
					case 10:
						data->ftr_reg_id[PEEKABOO_R9] = 1;
						break;
					case 11:
						data->ftr_reg_id[PEEKABOO_R10] = 1;
						break;
					case 12:
						data->ftr_reg_id[PEEKABOO_R11] = 1;
						break;
					case 13:
						data->ftr_reg_id[PEEKABOO_R12] = 1;
						break;
					case 14:
						data->ftr_reg_id[PEEKABOO_R13] = 1;
						break;
					case 15:
						data->ftr_reg_id[PEEKABOO_R14] = 1;
						break;
				}
			}else{
				switch(reg_id){
					case 53:
						data->ftr_reg_id[PEEKABOO_RAX] = 1;
						break;
					case 54:
						data->ftr_reg_id[PEEKABOO_RCX] = 1;
						break;
					case 55:
						data->ftr_reg_id[PEEKABOO_RDX] = 1;
						break;
					case 56:
						data->ftr_reg_id[PEEKABOO_RBX] = 1;
						break;
					case 65:
						data->ftr_reg_id[PEEKABOO_RSP] = 1;
						break;
					case 66:
						data->ftr_reg_id[PEEKABOO_RBP] = 1;
						break;
					case 67:
						data->ftr_reg_id[PEEKABOO_RSI] = 1;
						break;
					case 68:
						data->ftr_reg_id[PEEKABOO_RDI] = 1;
						break;
				}
			} 
		}
	}
	save_regfile(drcontext,&mc,data);
	
}

static void instrument_mem(void *drcontext, instrlist_t *ilist, instr_t *where, opnd_t ref, bool write)
{
	/* We need two scratch registers */
	reg_id_t reg_ptr, reg_tmp;
	if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_ptr) !=
			DRREG_SUCCESS ||
			drreg_reserve_register(drcontext, ilist, where, NULL, &reg_tmp) !=
			DRREG_SUCCESS) {
		DR_ASSERT(false); /* cannot recover */
		return;
	}

	uint32_t size = drutil_opnd_mem_size_in_bytes(ref, where);
	drutil_insert_get_mem_addr(drcontext, ilist, where, ref, reg_tmp, reg_ptr);

	drx_buf_insert_load_buf_ptr(drcontext, memfile_buf, ilist, where, reg_ptr);
	drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, DR_REG_NULL, opnd_create_reg(reg_tmp), OPSZ_PTR, offsetof(memfile_t, addr)); 
	drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT64(0), OPSZ_8, offsetof(memfile_t, value));
	drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(size), OPSZ_4, offsetof(memfile_t, size));
	drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(write?1:0), OPSZ_4, offsetof(memfile_t, status));
	
	app_pc pc = instr_get_app_pc(where);
	drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT64(pc), OPSZ_8, offsetof(memfile_t, pc));
	
	drx_buf_insert_update_buf_ptr(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, sizeof(memfile_t));

	//printf("sizesize:%d\n", size);
	//disassemble_with_info(drcontext, instr_get_app_pc(where), 0, true, true);

	if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
	    drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS)
		DR_ASSERT(false);
}

static void instrument_insn(void *drcontext, instrlist_t *ilist, instr_t *where, int mem_count)
{
	reg_id_t reg_ptr, reg_tmp;
	if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_ptr) != DRREG_SUCCESS ||
	    drreg_reserve_register(drcontext, ilist, where, NULL, &reg_tmp) != DRREG_SUCCESS)
	{
		DR_ASSERT(false);
		return;
	}

	int insn_len = instr_length(drcontext, where);
	app_pc pc = instr_get_app_pc(where);

	// instrument update to insn_ref, pushes a 32/64-bit pc into the buffer
	drx_buf_insert_load_buf_ptr(drcontext, insn_ref_buf, ilist, where, reg_ptr);
	#ifdef X64
		drx_buf_insert_buf_store(drcontext, insn_ref_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT64(pc), OPSZ_8, 0);
	#else
		drx_buf_insert_buf_store(drcontext, insn_ref_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(pc), OPSZ_4, 0);
	#endif
	drx_buf_insert_update_buf_ptr(drcontext, insn_ref_buf, ilist, where, reg_ptr, DR_REG_NULL, sizeof(insn_ref_t));

	if(!not_store_meminfo)
	{
		// ZL: insert write to store mem_count into memrefs
		drx_buf_insert_load_buf_ptr(drcontext, memrefs_buf, ilist, where, reg_ptr);
		drx_buf_insert_buf_store(drcontext, memrefs_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(mem_count), OPSZ_4, offsetof(memref_t, length));
		drx_buf_insert_update_buf_ptr(drcontext, memrefs_buf, ilist, where, reg_ptr, DR_REG_NULL, sizeof(memref_t));
	}

	
	// num_dst = 0;
	if(!not_store_reginfo)
	{


			drx_buf_insert_load_buf_ptr(drcontext, regfile_buf, ilist, where, reg_ptr);
			drx_buf_insert_buf_store(drcontext, regfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(0), OPSZ_4, 0);	
			dr_insert_clean_call(drcontext, ilist, where, (void *)copy_regfile, false, 1, OPND_CREATE_INT64(pc));
			drx_buf_insert_update_buf_ptr(drcontext, regfile_buf, ilist, where, reg_ptr, DR_REG_NULL, sizeof(regfile_t));
			drx_buf_insert_load_buf_ptr(drcontext, regfile_buf, ilist, where, reg_ptr);

			drx_buf_insert_load_buf_ptr(drcontext, offset_regfile_buf, ilist, where, reg_ptr);
			// ZL: insert a write 0 into the stream using dynamorio sanctioned instruction to trigger the flushing of file from trace buffer.
			drx_buf_insert_buf_store(drcontext, offset_regfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(0), OPSZ_4, 0);
			dr_insert_clean_call(drcontext, ilist, where, (void *)trace_offset_register, false, 1, OPND_CREATE_INT64(pc));

			#ifdef X86
				#ifdef X64
					// KH: We save app_pc+instr_len into reg_rip, though it is not the actually rip.
					drx_buf_insert_buf_store(drcontext, offset_regfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT64(pc+insn_len), OPSZ_8, offsetof(offset_regfile_t, reg_rip) );
				#else
				// We currently don't have eip reg for 32bit X86
				#endif
			#endif
			drx_buf_insert_update_buf_ptr(drcontext, offset_regfile_buf, ilist, where, reg_ptr, DR_REG_NULL, sizeof(offset_regfile_t));
			drx_buf_insert_load_buf_ptr(drcontext, offset_regfile_buf, ilist, where, reg_ptr);

		#if defined _STORE_SIMD || defined _STORE_FXSAVE
			drx_buf_insert_load_buf_ptr(drcontext, nongpr_regfile_buf, ilist, where, reg_ptr);
			drx_buf_insert_buf_store(drcontext, nongpr_regfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(0), OPSZ_4, 0);	
			dr_insert_clean_call(drcontext, ilist, where, (void *)save_nongpr_register, false, 0);
			drx_buf_insert_update_buf_ptr(drcontext, nongpr_regfile_buf, ilist, where, reg_ptr, DR_REG_NULL, sizeof(regfile_nongpr_t));
			drx_buf_insert_load_buf_ptr(drcontext, nongpr_regfile_buf, ilist, where, reg_ptr);
		#endif
	}

	if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
	    drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS)
		DR_ASSERT(false);
}


static dr_emit_flags_t save_bb_rawbytes(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating, void **user_data)
{
	per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
	bytes_map_t bytes_map[MAX_NUM_BYTES_MAP];
	uint idx=0;
	instr_t *insn;

	for (insn = instrlist_first_app(bb), idx=0; insn && idx < MAX_NUM_BYTES_MAP; insn=instr_get_next_app(insn), idx++)
	{
		uint32_t length = instr_length(drcontext, insn);
		DR_ASSERT(length <= 16);
		bytes_map[idx].pc = (uint64_t)instr_get_app_pc(insn);

		bytes_map[idx].size = length;
    	
		int x;
		for (x=0; x<length; x++)
		{
			bytes_map[idx].rawbytes[x] = instr_get_raw_byte(insn, x);
		}
	}
	dr_mutex_lock(mutex);
	fwrite(bytes_map, sizeof(bytes_map_t), idx, data->peek_trace->bytes_map);
	dr_mutex_unlock(mutex);

	return DR_EMIT_DEFAULT;
}

static dr_emit_flags_t per_insn_instrument(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, 
		                             bool for_trace, bool translating, void *user_data)
{
	drmgr_disable_auto_predication(drcontext, bb);
	if (!instr_is_app(instr)) return DR_EMIT_DEFAULT;

	/* insert code to add an entry for each memory reference opnd */
	uint32_t mem_count = 0;
	if(!not_store_meminfo)
	{
		int i;
		for (i = 0; i < instr_num_srcs(instr); i++) {
			if (opnd_is_memory_reference(instr_get_src(instr, i)))
			{
				instrument_mem(drcontext, bb, instr, instr_get_src(instr, i), false);
				mem_count++;
			}
		}

		for (i = 0; i < instr_num_dsts(instr); i++) {
			if (opnd_is_memory_reference(instr_get_dst(instr, i)))
			{
				instrument_mem(drcontext, bb, instr, instr_get_dst(instr, i), true);
				mem_count++;
			}
		}
	}


	// ZL: would instrument the memref count (memfile) inside
	instrument_insn(drcontext, bb, instr, mem_count);


	//if (drmgr_is_first_instr(drcontext, instr) IF_AARCHXX(&& !instr_is_exclusive_store(instr)))
	//	dr_insert_clean_call(drcontext, bb, instr, (void *)save_insn, false, 0);
	return DR_EMIT_DEFAULT;
}

static void init_thread_in_process(void *drcontext)
{
	char buf[256];
	per_thread_t *data = dr_thread_alloc(drcontext, sizeof(per_thread_t));
	DR_ASSERT(data != NULL);
	drmgr_set_tls_field(drcontext, tls_idx, data);

	int pid = dr_get_process_id();
	snprintf(buf, 256, "%s/%d", trace_dir, pid);
	offset_regfile_index = 0;
	num_dst_buf = 0;
	data->num_refs = 0;
	data->peek_trace = create_trace(buf);
	// data->head = dr_thread_alloc(drcontext, REG_LINKED_LIST_SIZE);;
	data->num_reg_change = 0;

	if (data->peek_trace == NULL)
	{
		PEEKABOO_DIE("libpeekaboo: Unable to create directory %s.\n", buf);
	}

	data->peek_trace->bytes_map = bytes_map_file;
	write_metadata(data->peek_trace, arch, LIBPEEKABOO_VER,not_store_meminfo,not_store_reginfo);
	
	char path[512];
	snprintf(path, 512, "%s/proc_map", buf);

	if(access(path, F_OK ) == -1 ) 
	{
		snprintf(path, 512, "cp /proc/%d/maps %s/proc_map", pid, buf);
		system(path);
	}

	printf("Created a new trace for %d\n", pid);
}

static void event_thread_init(void *drcontext)
{
	root_pid = dr_get_process_id();

	char name[256];
	snprintf(name, 256, "%s-%d", dr_get_application_name(), root_pid);
	if (create_folder(name, trace_dir, 256))	
	{
		// Fail to create trace directory. Try again with timestamp
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC_RAW, &now);
		char name_with_timestamp[256];
		sprintf(name_with_timestamp, "%s-%d_%02d_%02d-%02d_%02d_%02d-%d", name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, now.tv_nsec%1000000);
		if (create_folder(name_with_timestamp, trace_dir, 256))
		{
			PEEKABOO_DIE("libpeekaboo: Unable to create directory %s.\n", name_with_timestamp);
		}
		else
		{
			// Success!
			strncpy(name, name_with_timestamp, 256);
		}
	}

	dr_mutex_lock(mutex);
	create_trace_file(trace_dir, "insn.bytemap", 256, &bytes_map_file);
	snprintf(name, 256, "%s/insn.bytemap", trace_dir);
	chmod(name, S_IRWXU|S_IRWXG|S_IRWXO);

	snprintf(name, 256, "%s/process_tree.txt", trace_dir);
	FILE * fp;
	fp = fopen(name, "w");
	assert(fp!=NULL);
	fprintf(fp, "%d-%d\n", dr_get_parent_id(), root_pid);
	fclose(fp);
	chmod(name, S_IRWXU|S_IRWXG|S_IRWXO);
	dr_mutex_unlock(mutex);

	printf("Peekaboo: Main thread starts. ");
	init_thread_in_process(drcontext);
}

#ifdef UNIX
static void fork_init(void *drcontext)
{	
	char name[256];
	snprintf(name, 256, "%s/process_tree.txt", trace_dir);
	FILE * fp;
	dr_mutex_lock(mutex);
	fp = fopen(name, "a");
	if (fp == NULL) PEEKABOO_DIE("Peekaboo: Cannot append to process tree at %s!", name);
	fprintf(fp, "%d-%d\n", dr_get_parent_id(), dr_get_process_id());
	fclose(fp);
	dr_mutex_unlock(mutex);

	// Recreate buffers to make them clean
	
	drx_buf_free(insn_ref_buf);
	if(!not_store_meminfo)
	{
		drx_buf_free(memrefs_buf);
		drx_buf_free(memfile_buf);
	}

	if(!not_store_reginfo)
	{
		drx_buf_free(regfile_buf);
		drx_buf_free(offset_regfile_buf);
		#if defined _STORE_SIMD || defined _STORE_FXSAVE
		drx_buf_free(nongpr_regfile_buf);
		#endif
	}

	insn_ref_buf = drx_buf_create_trace_buffer(INSN_REF_SIZE, flush_insnrefs);
	if(!not_store_meminfo)
	{
		memfile_buf = drx_buf_create_trace_buffer(MEMFILE_SIZE, flush_memfile);
		memrefs_buf = drx_buf_create_trace_buffer(MEM_REFS_SIZE, flush_memrefs);
	}
	if(!not_store_reginfo)
	{
		regfile_buf = drx_buf_create_trace_buffer(REG_BUF_SIZE, flush_regfile);
		offset_regfile_buf = drx_buf_create_trace_buffer(OFFSET_REGFILE_SIZE,flush_offset_regfile);
		#if defined _STORE_SIMD || defined _STORE_FXSAVE
		nongpr_regfile_buf = drx_buf_create_trace_buffer(NONGPR_REGFILE_SIZE,flush_nongpr_regfile);
		#endif
	}


	printf("Peekaboo: Application process forks. ");
	init_thread_in_process(drcontext);
}
#endif

static void event_thread_exit(void *drcontext)
{
	per_thread_t *data;
	data = drmgr_get_tls_field(drcontext, tls_idx);
	dr_mutex_lock(mutex);
	num_refs += data->num_refs;
	close_trace(data->peek_trace);
	dr_mutex_unlock(mutex);
	dr_thread_free(drcontext, data, sizeof(per_thread_t));
}

static void event_exit(void)
{
	//dr_log(NULL, DR_LOG_ALL, 1, "'peekaboo': Total number of instructions seen: " SZFMT "\n", num_refs);
	int pid = dr_get_process_id();
	if (pid != root_pid)
		printf("Peekaboo: Child process (PID:%d) exits. Total number of instructions seen: " SZFMT "\n", pid, num_refs);
	else
		printf("Peekaboo: Parent process (PID:%d) exits. Total number of instructions seen: " SZFMT "\n", pid, num_refs);

	if (!drmgr_unregister_tls_field(tls_idx) ||
	    !drmgr_unregister_thread_init_event(event_thread_init) ||
	    !drmgr_unregister_thread_exit_event(event_thread_exit) ||
	    !drmgr_unregister_bb_insertion_event(per_insn_instrument) ||
	    drreg_exit() != DRREG_SUCCESS)
	    DR_ASSERT(false);

#ifdef UNIX
	if (!dr_unregister_fork_init_event(fork_init))
		DR_ASSERT(false);
#endif

	dr_mutex_destroy(mutex);
	drmgr_exit();
	drutil_exit();

	drx_buf_free(insn_ref_buf);

	if(!not_store_meminfo)
	{
		drx_buf_free(memrefs_buf);
		drx_buf_free(memfile_buf);
	}
	if(!not_store_reginfo)
	{
		drx_buf_free(regfile_buf);
		drx_buf_free(offset_regfile_buf);
		#if defined _STORE_SIMD || defined _STORE_FXSAVE
		drx_buf_free(nongpr_regfile_buf);
		#endif
	}

	drx_exit();
}


DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[])
{

    drreg_options_t ops = {sizeof(ops), 4, false};
	dr_set_client_name("peekaboo DynamoRIO tracer", "https://github.com/melynx/peekaboo");
	drreg_init(&ops);
	drmgr_init();
	drutil_init();
	drx_init();

	not_store_reginfo = 0;
	not_store_meminfo = 0;

	int c;
	while((c = getopt_long (argc, argv, "r:m:",long_options, NULL)) != -1){
		switch(c){
			case 'r':
				not_store_reginfo = 1;
				break;
			case 'm':
				not_store_meminfo = 1;
				break;
		}
	}


	// drmgr_register_signal_event(event_signal);

	dr_register_exit_event(event_exit);
#ifdef UNIX
	// KH: dr_register_fork_init_event only support UNIX
    dr_register_fork_init_event(fork_init);
#endif
	drmgr_register_thread_init_event(event_thread_init);
	drmgr_register_thread_exit_event(event_thread_exit);
	drmgr_register_bb_instrumentation_event(save_bb_rawbytes, per_insn_instrument, NULL);

	client_id = id;
	mutex = dr_mutex_create();

	tls_idx = drmgr_register_tls_field();
	DR_ASSERT(tls_idx != -1);

	insn_ref_buf = drx_buf_create_trace_buffer(INSN_REF_SIZE, flush_insnrefs);
	
	if(!not_store_meminfo)
	{
		memfile_buf = drx_buf_create_trace_buffer(MEMFILE_SIZE, flush_memfile);
		memrefs_buf = drx_buf_create_trace_buffer(MEM_REFS_SIZE, flush_memrefs);
	}

	if(!not_store_reginfo){
		regfile_buf = drx_buf_create_trace_buffer(REG_BUF_SIZE, flush_regfile);
		offset_regfile_buf = drx_buf_create_trace_buffer(OFFSET_REGFILE_SIZE, flush_offset_regfile);
		#if defined _STORE_SIMD || defined _STORE_FXSAVE
		nongpr_regfile_buf = drx_buf_create_trace_buffer(NONGPR_REGFILE_SIZE, flush_nongpr_regfile);
		#endif
	}
	//dr_log(NULL, DR_LOG_ALL, 11, "%s - Client 'peekaboo' initializing\n", arch);
	printf("Peekaboo: %s - Client 'peekaboo' initializing\n", arch_str);

	printf("Peekaboo: Binary being traced: %s\n", dr_get_application_name());
	printf("Peekaboo: Number of SIMD slots: %d\n", MCXT_NUM_SIMD_SLOTS);
	printf("Peekaboo: libpeekaboo Version: %d\n", LIBPEEKABOO_VER);

}
