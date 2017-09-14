#ifndef PROFILER_H_
#define PROFILER_H_

#include "qemu/osdep.h"

#define OUTPUT_FILE_NAME_GENERIC "profiling-generic.txt"

extern unsigned int profile_log_generic;

void profiler_flush_files(void);
void profiler_close_files(void);
void profiler_log_generic(const char *fmt, ...);

#endif
