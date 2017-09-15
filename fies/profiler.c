#include "profiler.h"
#include "qemu/error-report.h"

int open_generic_file = 0;

FILE *outfile_generic;

void profiler_flush_files(void)
{
    if (open_generic_file)
    {
        error_printf("profiler flushes generic file\n");
        fflush(outfile_generic);
    }
}

void profiler_close_files(void)
{
    if (open_generic_file)
    {
        error_printf("profiler closes generic file\n");
        fclose(outfile_generic);
        open_generic_file = 0;
    }
}



void profiler_log_generic(const char *fmt, ...)
{
    if (profile_log_generic != 1)
        return;

    if (!open_generic_file)
    {
        outfile_generic = fopen(OUTPUT_FILE_NAME_GENERIC, "w+");
        open_generic_file = 1;
    }

    va_list argptr;

    va_start(argptr, fmt);
    vfprintf(outfile_generic, fmt, argptr);
    va_end(argptr);
}
