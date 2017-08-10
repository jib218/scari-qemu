/*
 * fault-injection-controller.c
 *
 */
#include "fault-injection-controller.h"

#include "fault-injection-config.h"
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
