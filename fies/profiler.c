/*
 * profiler.c
 *
 *  Created on: 18.12.2015
 *      Author: Andrea Hoeller
 */

#include "profiler.h"
#include "qemu/error-report.h"

int open_memory_addresses_file = 0;
int open_register_file = 0;
int open_generic_file = 0;

FILE *outfile_memory;
FILE *outfile_registers;
FILE *outfile_generic;

void profiler_log(hwaddr *addr, uint32_t *value, AccessType access_type)
{
	if (access_type == write_access_type || access_type == read_access_type)
	{
//		if (*addr <= (hwaddr) 15) //GP Register
//		{
//			if (profile_registers)
//				profiler_log_register_access(addr, value, access_type);
//		}
//		else
//		{
  //      error_printf("inside profiler log\n");
			if (profile_ram_addresses)
				profiler_log_memory_access(addr, value, access_type);
//		}
	}
}

void profiler_log_memory_access(hwaddr *addr, uint32_t *value, AccessType access_type)
{
	if (!open_memory_addresses_file)
	{
        outfile_memory = fopen(OUTPUT_FILE_NAME_ACCESSED_MEMORY_ADDRESSES, "w+");
		if (outfile_memory == NULL)
		{
			printf("Error opening file\n");
			perror("Error");
		}
		else
		  open_memory_addresses_file = 1;
	}

	if (access_type == write_access_type)
	{
        error_printf("write access type\n");
        fprintf(outfile_memory, "%lx w 0x%x\n", (hwaddr)*addr, *value); // I do not know wheter value should be dereferenced
	}
	else
	{
		char access_str = (access_type == read_access_type) ? 'r' : 'e';
        fprintf(outfile_memory, "%lx %c \n", (hwaddr)*addr, access_str);
	}
}

void profiler_log_register_access(hwaddr *addr, uint32_t *value, AccessType access_type)
{
	if (!open_register_file)
	{
		outfile_registers = fopen(OUTPUT_FILE_NAME_ACCESSED_REGS, "w+");
		open_register_file = 1;
	}

	if (access_type == write_access_type)
	{
        fprintf(outfile_registers, "%lx w 0x%x\n", (hwaddr)*addr, *value);
	}
	else
	{
		char access_str = (access_type == read_access_type) ? 'r' : 'e';
        fprintf(outfile_registers, "%lx %c \n", (hwaddr)*addr, access_str);
	}
}


void profiler_flush_files(void)
{
    error_printf("profiler flushes files\n");

    if (open_memory_addresses_file)
    {
        fflush(outfile_memory);
    }
    if (open_register_file)
    {
        fflush(outfile_registers);
    }
    if (open_generic_file)
    {
        fflush(outfile_generic);
    }
}

void profiler_close_files(void)
{
    error_printf("profiler closes files\n");
	if (open_memory_addresses_file)
	{
		fclose(outfile_memory);
		open_memory_addresses_file = 0;
	}
	if (open_register_file)
	{
		fclose(outfile_registers);
		open_register_file = 0;
	}

    if (open_generic_file)
    {
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
