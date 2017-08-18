/*
 * profiler.c
 *
 *  Created on: 18.12.2015
 *      Author: Andrea Hoeller
 */

#include "profiler.h"

int open_memory_addresses_file = 0;
int open_register_file = 0;

FILE *outfile_memory;
FILE *outfile_registers;

void profiler_log(hwaddr *addr, uint32_t *value, AccessType access_type)
{
	if (access_type == write_access_type || access_type == read_access_type)
	{
		if (*addr <= (hwaddr) 15) //GP Register
		{
			if (profile_registers)
				profiler_log_register_access(addr, value, access_type);
		}
		else
		{
			if (profile_ram_addresses)
				profiler_log_memory_access(addr, value, access_type);
		}
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

void profiler_close_files(void)
{
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
}
