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

#include "amd64_syscall.h"



static const struct_syscall_info syscall_infos[] = 
{
    [  0] = { 3,	"read"			},
    [  1] = { 3,	"write"			},
    [  2] = { 3,	"open"			},
    [  3] = { 1,	"close"			},
    [  4] = { 2,	"stat"			},
    [  5] = { 2,	"fstat"			},
    [  6] = { 2,	"lstat"			},
    [  7] = { 3,	"poll"			},
    [  8] = { 3,	"lseek"			},
    [  9] = { 6,	"mmap"			},
    [ 10] = { 3,	"mprotect"		},
    [ 11] = { 2,	"munmap"		},
    [ 12] = { 1,	"brk"			},
    [ 13] = { 4,	"rt_sigaction"		},
    [ 14] = { 4,	"rt_sigprocmask"	},
    [ 15] = { 0,	"rt_sigreturn"		},
    [ 16] = { 3,	"ioctl"			},
    [ 17] = { 4,	"pread64"		},
    [ 18] = { 4,	"pwrite64"		},
    [ 19] = { 3,	"readv"			},
    [ 20] = { 3,	"writev"		},
    [ 21] = { 2,	"access"		},
    [ 22] = { 1,	"pipe"			},
    [ 23] = { 5,	"select"		},
    [ 24] = { 0,	"sched_yield"		},
    [ 25] = { 5,	"mremap"		},
    [ 26] = { 3,	"msync"			},
    [ 27] = { 3,	"mincore"		},
    [ 28] = { 3,	"madvise"		},
    [ 29] = { 3,	"shmget"		},
    [ 30] = { 3,	"shmat"			},
    [ 31] = { 3,	"shmctl"		},
    [ 32] = { 1,	"dup"			},
    [ 33] = { 2,	"dup2"			},
    [ 34] = { 0,	"pause"			},
    [ 35] = { 2,	"nanosleep"		},
    [ 36] = { 2,	"getitimer"		},
    [ 37] = { 1,	"alarm"			},
    [ 38] = { 3,	"setitimer"		},
    [ 39] = { 0,	"getpid"		},
    [ 40] = { 4,	"sendfile"		},
    [ 41] = { 3,	"socket"		},
    [ 42] = { 3,	"connect"		},
    [ 43] = { 3,	"accept"		},
    [ 44] = { 6,	"sendto"		},
    [ 45] = { 6,	"recvfrom"		},
    [ 46] = { 3,	"sendmsg"		},
    [ 47] = { 3,	"recvmsg"		},
    [ 48] = { 2,	"shutdown"		},
    [ 49] = { 3,	"bind"			},
    [ 50] = { 2,	"listen"		},
    [ 51] = { 3,	"getsockname"		},
    [ 52] = { 3,	"getpeername"		},
    [ 53] = { 4,	"socketpair"		},
    [ 54] = { 5,	"setsockopt"		},
    [ 55] = { 5,	"getsockopt"		},
    [ 56] = { 5,	"clone"			},
    [ 57] = { 0,	"fork"			},
    [ 58] = { 0,	"vfork"			},
    [ 59] = { 3,	"execve"		},
    [ 60] = { 1,	"exit"			},
    [ 61] = { 4,	"wait4"			},
    [ 62] = { 2,	"kill"			},
    [ 63] = { 1,	"uname"			},
    [ 64] = { 3,	"semget"		},
    [ 65] = { 3,	"semop"			},
    [ 66] = { 4,	"semctl"		},
    [ 67] = { 1,	"shmdt"			},
    [ 68] = { 2,	"msgget"		},
    [ 69] = { 4,	"msgsnd"		},
    [ 70] = { 5,	"msgrcv"		},
    [ 71] = { 3,	"msgctl"		},
    [ 72] = { 3,	"fcntl"			},
    [ 73] = { 2,	"flock"			},
    [ 74] = { 1,	"fsync"			},
    [ 75] = { 1,	"fdatasync"		},
    [ 76] = { 2,	"truncate"		},
    [ 77] = { 2,	"ftruncate"		},
    [ 78] = { 3,	"getdents"		},
    [ 79] = { 2,	"getcwd"		},
    [ 80] = { 1,	"chdir"			},
    [ 81] = { 1,	"fchdir"		},
    [ 82] = { 2,	"rename"		},
    [ 83] = { 2,	"mkdir"			},
    [ 84] = { 1,	"rmdir"			},
    [ 85] = { 2,	"creat"			},
    [ 86] = { 2,	"link"			},
    [ 87] = { 1,	"unlink"		},
    [ 88] = { 2,	"symlink"		},
    [ 89] = { 3,	"readlink"		},
    [ 90] = { 2,	"chmod"			},
    [ 91] = { 2,	"fchmod"		},
    [ 92] = { 3,	"chown"			},
    [ 93] = { 3,	"fchown"		},
    [ 94] = { 3,	"lchown"		},
    [ 95] = { 1,	"umask"			},
    [ 96] = { 2,	"gettimeofday"		},
    [ 97] = { 2,	"getrlimit"		},
    [ 98] = { 2,	"getrusage"		},
    [ 99] = { 1,	"sysinfo"		},
    [100] = { 1,	"times"			},
    [101] = { 4,	"ptrace"		},
    [102] = { 0,	"getuid"		},
    [103] = { 3,	"syslog"		},
    [104] = { 0,	"getgid"		},
    [105] = { 1,	"setuid"		},
    [106] = { 1,	"setgid"		},
    [107] = { 0,	"geteuid"		},
    [108] = { 0,	"getegid"		},
    [109] = { 2,	"setpgid"		},
    [110] = { 0,	"getppid"		},
    [111] = { 0,	"getpgrp"		},
    [112] = { 0,	"setsid"		},
    [113] = { 2,	"setreuid"		},
    [114] = { 2,	"setregid"		},
    [115] = { 2,	"getgroups"		},
    [116] = { 2,	"setgroups"		},
    [117] = { 3,	"setresuid"		},
    [118] = { 3,	"getresuid"		},
    [119] = { 3,	"setresgid"		},
    [120] = { 3,	"getresgid"		},
    [121] = { 1,	"getpgid"		},
    [122] = { 1,	"setfsuid"		},
    [123] = { 1,	"setfsgid"		},
    [124] = { 1,	"getsid"		},
    [125] = { 2,	"capget"		},
    [126] = { 2,	"capset"		},
    [127] = { 2,	"rt_sigpending"		},
    [128] = { 4,	"rt_sigtimedwait"	},
    [129] = { 3,	"rt_sigqueueinfo"	},
    [130] = { 2,	"rt_sigsuspend"		},
    [131] = { 2,	"sigaltstack"		},
    [132] = { 2,	"utime"			},
    [133] = { 3,	"mknod"			},
    [134] = { 1,	"uselib"		},
    [135] = { 1,	"personality"		},
    [136] = { 2,	"ustat"			},
    [137] = { 2,	"statfs"		},
    [138] = { 2,	"fstatfs"		},
    [139] = { 3,	"sysfs"			},
    [140] = { 2,	"getpriority"		},
    [141] = { 3,	"setpriority"		},
    [142] = { 2,	"sched_setparam"	},
    [143] = { 2,	"sched_getparam"	},
    [144] = { 3,	"sched_setscheduler"	},
    [145] = { 1,	"sched_getscheduler"	},
    [146] = { 1,	"sched_get_priority_max"},
    [147] = { 1,	"sched_get_priority_min"},
    [148] = { 2,	"sched_rr_get_interval"	},
    [149] = { 2,	"mlock"			},
    [150] = { 2,	"munlock"		},
    [151] = { 1,	"mlockall"		},
    [152] = { 0,	"munlockall"		},
    [153] = { 0,	"vhangup"		},
    [154] = { 3,	"modify_ldt"		},
    [155] = { 2,	"pivot_root"		},
    [156] = { 1,	"_sysctl"		},
    [157] = { 5,	"prctl"			},
    [158] = { 2,	"arch_prctl"		},
    [159] = { 1,	"adjtimex"		},
    [160] = { 2,	"setrlimit"		},
    [161] = { 1,	"chroot"		},
    [162] = { 0,	"sync"			},
    [163] = { 1,	"acct"			},
    [164] = { 2,	"settimeofday"		},
    [165] = { 5,	"mount"			},
    [166] = { 2,	"umount2"		},
    [167] = { 2,	"swapon"		},
    [168] = { 1,	"swapoff"		},
    [169] = { 4,	"reboot"		},
    [170] = { 2,	"sethostname"		},
    [171] = { 2,	"setdomainname"		},
    [172] = { 1,	"iopl"			},
    [173] = { 3,	"ioperm"		},
    [174] = { 2,	"create_module"		},
    [175] = { 3,	"init_module"		},
    [176] = { 2,	"delete_module"		},
    [177] = { 1,	"get_kernel_syms"	},
    [178] = { 5,	"query_module"		},
    [179] = { 4,	"quotactl"		},
    [180] = { 3,	"nfsservctl"		},
    [181] = { 5,	"getpmsg"		},
    [182] = { 5,	"putpmsg"		},
    [183] = { 5,	"afs_syscall"		},
    [184] = { 3,	"tuxcall"		},
    [185] = { 3,	"security"		},
    [186] = { 0,	"gettid"		},
    [187] = { 3,	"readahead"		},
    [188] = { 5,	"setxattr"		},
    [189] = { 5,	"lsetxattr"		},
    [190] = { 5,	"fsetxattr"		},
    [191] = { 4,	"getxattr"		},
    [192] = { 4,	"lgetxattr"		},
    [193] = { 4,	"fgetxattr"		},
    [194] = { 3,	"listxattr"		},
    [195] = { 3,	"llistxattr"		},
    [196] = { 3,	"flistxattr"		},
    [197] = { 2,	"removexattr"		},
    [198] = { 2,	"lremovexattr"		},
    [199] = { 2,	"fremovexattr"		},
    [200] = { 2,	"tkill"			},
    [201] = { 1,	"time"			},
    [202] = { 6,	"futex"			},
    [203] = { 3,	"sched_setaffinity"	},
    [204] = { 3,	"sched_getaffinity"	},
    [205] = { 1,	"set_thread_area"	},
    [206] = { 2,	"io_setup"		},
    [207] = { 1,	"io_destroy"		},
    [208] = { 5,	"io_getevents"		},
    [209] = { 3,	"io_submit"		},
    [210] = { 3,	"io_cancel"		},
    [211] = { 1,	"get_thread_area"	},
    [212] = { 3,	"lookup_dcookie"	},
    [213] = { 1,	"epoll_create"		},
    [214] = { 4,	"epoll_ctl_old"		},
    [215] = { 4,	"epoll_wait_old"	},
    [216] = { 5,	"remap_file_pages"	},
    [217] = { 3,	"getdents64"		},
    [218] = { 1,	"set_tid_address"	},
    [219] = { 0,	"restart_syscall"	},
    [220] = { 4,	"semtimedop"		},
    [221] = { 4,	"fadvise64"		},
    [222] = { 3,	"timer_create"		},
    [223] = { 4,	"timer_settime"		},
    [224] = { 2,	"timer_gettime"		},
    [225] = { 1,	"timer_getoverrun"	},
    [226] = { 1,	"timer_delete"		},
    [227] = { 2,	"clock_settime"		},
    [228] = { 2,	"clock_gettime"		},
    [229] = { 2,	"clock_getres"		},
    [230] = { 4,	"clock_nanosleep"	},
    [231] = { 1,	"exit_group"		},
    [232] = { 4,	"epoll_wait"		},
    [233] = { 4,	"epoll_ctl"		},
    [234] = { 3,	"tgkill"		},
    [235] = { 2,	"utimes"		},
    [236] = { 5,	"vserver"		},
    [237] = { 6,	"mbind"			},
    [238] = { 3,	"set_mempolicy"		},
    [239] = { 5,	"get_mempolicy"		},
    [240] = { 4,	"mq_open"		},
    [241] = { 1,	"mq_unlink"		},
    [242] = { 5,	"mq_timedsend"		},
    [243] = { 5,	"mq_timedreceive"	},
    [244] = { 2,	"mq_notify"		},
    [245] = { 3,	"mq_getsetattr"		},
    [246] = { 4,	"kexec_load"		},
    [247] = { 5,	"waitid"		},
    [248] = { 5,	"add_key"		},
    [249] = { 4,	"request_key"		},
    [250] = { 5,	"keyctl"		},
    [251] = { 3,	"ioprio_set"		},
    [252] = { 2,	"ioprio_get"		},
    [253] = { 0,	"inotify_init"		},
    [254] = { 3,	"inotify_add_watch"	},
    [255] = { 2,	"inotify_rm_watch"	},
    [256] = { 4,	"migrate_pages"		},
    [257] = { 4,	"openat"		},
    [258] = { 3,	"mkdirat"		},
    [259] = { 4,	"mknodat"		},
    [260] = { 5,	"fchownat"		},
    [261] = { 3,	"futimesat"		},
    [262] = { 4,	"newfstatat"		},
    [263] = { 3,	"unlinkat"		},
    [264] = { 4,	"renameat"		},
    [265] = { 5,	"linkat"		},
    [266] = { 3,	"symlinkat"		},
    [267] = { 4,	"readlinkat"		},
    [268] = { 3,	"fchmodat"		},
    [269] = { 3,	"faccessat"		},
    [270] = { 6,	"pselect6"		},
    [271] = { 5,	"ppoll"			},
    [272] = { 1,	"unshare"		},
    [273] = { 2,	"set_robust_list"	},
    [274] = { 3,	"get_robust_list"	},
    [275] = { 6,	"splice"		},
    [276] = { 4,	"tee"			},
    [277] = { 4,	"sync_file_range"	},
    [278] = { 4,	"vmsplice"		},
    [279] = { 6,	"move_pages"		},
    [280] = { 4,	"utimensat"		},
    [281] = { 6,	"epoll_pwait"		},
    [282] = { 3,	"signalfd"		},
    [283] = { 2,	"timerfd_create"	},
    [284] = { 1,	"eventfd"		},
    [285] = { 4,	"fallocate"		},
    [286] = { 4,	"timerfd_settime"	},
    [287] = { 2,	"timerfd_gettime"	},
    [288] = { 4,	"accept4"		},
    [289] = { 4,	"signalfd4"		},
    [290] = { 2,	"eventfd2"		},
    [291] = { 1,	"epoll_create1"		},
    [292] = { 3,	"dup3"			},
    [293] = { 2,	"pipe2"			},
    [294] = { 1,	"inotify_init1"		},
    [295] = { 4,	"preadv"		},
    [296] = { 4,	"pwritev"		},
    [297] = { 4,	"rt_tgsigqueueinfo"	},
    [298] = { 5,	"perf_event_open"	},
    [299] = { 5,	"recvmmsg"		},
    [300] = { 2,	"fanotify_init"		},
    [301] = { 5,	"fanotify_mark"		},
    [302] = { 4,	"prlimit64"		},
    [303] = { 5,	"name_to_handle_at"	},
    [304] = { 3,	"open_by_handle_at"	},
    [305] = { 2,	"clock_adjtime"		},
    [306] = { 1,	"syncfs"		},
    [307] = { 4,	"sendmmsg"		},
    [308] = { 2,	"setns"			},
    [309] = { 3,	"getcpu"		},
    [310] = { 6,	"process_vm_readv"	},
    [311] = { 6,	"process_vm_writev"	},
    [312] = { 5,	"kcmp"			},
    [313] = { 3,	"finit_module"		},
    [314] = { 3,	"sched_setattr"		},
    [315] = { 4,	"sched_getattr"		},
    [316] = { 5,	"renameat2"		},
    [317] = { 3,	"seccomp"		},
    [318] = { 3,	"getrandom"		},
    [319] = { 2,	"memfd_create"		},
    [320] = { 5,	"kexec_file_load"	},
    [321] = { 3,	"bpf"			},
    [322] = { 5,	"execveat"		},
    [323] = { 1,	"userfaultfd"		},
    [324] = { 3,	"membarrier"		},
    [325] = { 3,	"mlock2"		},
    [326] = { 6,	"copy_file_range"	},
    [327] = { 6,	"preadv2"		},
    [328] = { 6,	"pwritev2"		},
    [329] = { 4,	"pkey_mprotect"		},
    [330] = { 2,	"pkey_alloc"		},
    [331] = { 1,	"pkey_free"		},
    [332] = { 5,	"statx"			},
    [333] = { 6,	"io_pgetevents"		},
    [334] = { 4,	"rseq"			},
    /* [335 ... 423] - reserved to sync up with other architectures */
};

int amd64_syscall_pp(uint64_t *regfile, uint64_t rvalue, bool print_details)
{
    uint64_t args_offset[] =
    {
        [0] = regfile[PEEKABOO_RDI],
        [1] = regfile[PEEKABOO_RSI],
        [2] = regfile[PEEKABOO_RDX],
        [3] = regfile[PEEKABOO_RCX],
        [4] = regfile[PEEKABOO_R8],
        [5] = regfile[PEEKABOO_R9]
    };
	const uint64_t syscall_id = regfile[PEEKABOO_RAX];
	if (syscall_id >= sizeof(syscall_infos)/sizeof(struct_syscall_info))
	{
		// Unrecognized syscall ID. Return.
		printf("syscall %lu", syscall_id);
		return -1;
	}
	const struct_syscall_info *syscall_info = &syscall_infos[syscall_id];

	if (!print_details)
	{
		// Just print what syscall is this. Job done!
		printf("syscall %s (%lu)", syscall_info->sys_name, syscall_id);
		return 0;
	}
	
	// Print arguments and rvalue.
	printf("%s(", syscall_info->sys_name);
	assert(syscall_info->nargs <= sizeof(args_offset)/sizeof(uint64_t));
	unsigned int arg_idx;
	for (arg_idx = 0; arg_idx < syscall_info->nargs; arg_idx++)
	{
		if (arg_idx != 0) printf(", ");
		printf("0x%lx", args_offset[arg_idx]);
	}
	printf(") = 0x%lx", rvalue);
	return 0;
}