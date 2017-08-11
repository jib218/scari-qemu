/*
 * fault-injection-controller.c
 *
 */
#include "fault-injection-controller.h"

#include "fault-injection-config.h"
#include "fault-injection-library.h"
#include "qemu/timer.h"

char *fault_library_name;
static int64_t timer_value = 0;
/**
 * Array, which stores the previous
 * memory cell operations for
 * dynamic faults.
 */
static int **ops_on_memory_cell;

/**
 * Array, which stores the previous
 * register cell operations for
 * dynamic faults.
 */
static int **ops_on_register_cell;


void fault_injection_controller_initTimer(void)
{
  timer_value = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
}

/**
 * Compares the ending of a string with a given ending.
 *
 * @param[in] string - the whole string
 * @param[in] ending - the string containing the postfix
 * @param[out] - 1 if the string contains the  given ending, 0 otherwise
 */
int ends_with(const char *string, const char *ending)
{
	int string_len = strlen(string);
	int ending_len = strlen(ending);

	if ( ending_len > string_len)
		return 0;

	return !strcmp(&string[string_len - ending_len], ending);
}

/**
 * Extracts the ending of the given string and converts
 * the result to an interger value.
 *
 * @param[in] string - the given string
 * @param[out] - the timer value as integer
 */
int timer_to_int(const char *string)
{
	int string_len = strlen(string);
	char timer_string[string_len-1];

	if ( string_len < 3)
		return 0;

	memset(timer_string, '\0', sizeof(timer_string));
	strncpy(timer_string, string, string_len-2);

	return (int) strtol(timer_string, NULL, 10);
}

/**
 * Allocates and initializes the ops_on_memory_cell- and
 * ops_on_register_cell-array.
 *
 * @param[in] ids - the maximal id number.
 */
void init_ops_on_cell(int ids)
{
	int i = 0, j = 0;

	ops_on_memory_cell = malloc(ids * sizeof(int *));
	ops_on_register_cell = malloc(ids * sizeof(int *));

	for (i = 0; i < ids; i++)
	{
		ops_on_memory_cell[i] = malloc(MEMORY_WIDTH * sizeof(int *));
		ops_on_register_cell[i] = malloc(MEMORY_WIDTH * sizeof(int *));
	}

	for (i = 0; i < ids; i++)
	{
		for (j = 0; j < MEMORY_WIDTH; j++)
		{
			ops_on_memory_cell[i][j] = -1;
			ops_on_register_cell[i][j] = -1;
		}
	}
}

/**
 * Deletes the ops_on_memory_cell- and
 * ops_on_register_cell-array.
 */
void destroy_ops_on_cell(void)
{
	int i = 0;

	for(i = 0; i < getMaxIDInFaultList(); i++)
	{
	    free(ops_on_memory_cell[i]);
	 	free(ops_on_register_cell[i]);
	}

	free(ops_on_memory_cell);
	free(ops_on_register_cell);
}
