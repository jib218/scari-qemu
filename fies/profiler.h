/*
 * profiler.h
 *
 *  Created on: 18.12.2015
 *      Author: Andrea Hoeller
 */

#ifndef PROFILER_H_
#define PROFILER_H_

#include "qemu/osdep.h"
#include "fault-injection-config.h"
#include "exec/hwaddr.h"

#define OUTPUT_FILE_NAME_ACCESSED_MEMORY_ADDRESSES "profiling_memory.txt"
#define OUTPUT_FILE_NAME_ACCESSED_REGS "profiling_registers.txt"
#define OUTPUT_FILE_NAME_CONDITION_FLAGS "condition_flags.txt"
#define OUTPUT_FILE_NAME_GENERIC "profiling_generic.txt"

extern unsigned int profile_ram_addresses;
extern unsigned int profile_log_generic;
extern unsigned int profile_pc_status;
extern unsigned int profile_registers;
extern unsigned int profile_condition_flags;


void profiler_log_memory_access(hwaddr *addr, uint32_t *value, AccessType access_type);
void profiler_flush_files(void);
void profiler_close_files(void);
void set_profile_ram_addresses(int flag);
void profiler_log(hwaddr *addr, uint32_t *value, AccessType access_type);
void profiler_log_register_access(hwaddr *addr, uint32_t *value, AccessType access_type);
void profiler_log_generic(const char *fmt, ...);

#endif /* PROFILER_H_ */
