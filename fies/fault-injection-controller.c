/*
 * fault-injection-controller.c
 *
 */
#include "fault-injection-controller.h"

#include "fault-injection-config.h"
#include "fault-injection-library.h"
#include "qemu/timer.h"
#include "hmp.h"


void start_automatic_test_process(CPUArchState *env)
{
	// static int already_set = 0, shutting_down = 0;
	//
	// CPUState *cpu;
	//
	// if (!already_set)
	// {
	// 	already_set = 1;
	// 	hmp_fault_reload(qemu_serial_monitor, NULL);
	// }
	//
	//
	// //if (sbst_cycle_count_value > SBST_CYCLES_BEFORE_EXIT && !shutting_down)
	// if (false && !shutting_down)
	// {
	// 	shutting_down = 1;
	// 	fclose(outfile);
	//
	// 	hmp_info_faults(qemu_serial_monitor, NULL);

		if (env)
		{
			cpu = ENV_GET_CPU(env);
			cpu_dump_state(cpu, (FILE *)qemu_serial_monitor, (fprintf_function) monitor_printf, CPU_DUMP_FPU);
		}
	//
	//     qmp_quit(NULL);
	// }
}
