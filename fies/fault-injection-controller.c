/*
 * fault-injection-controller.c
 *
 */
#include "fault-injection-controller.h"
#include "hmp.h"
#include "monitor/monitor.h"
#include "fault-injection-injector.h"
#include "fault-injection-library.h"
#include "profiler.h"
#include "qemu/error-report.h"


/**
 * Maybe useless
 */
static hwaddr address_in_use = UINT64_MAX;


/**
 * Iterates through the linked list and checks if an entry or element belongs to
 * a fault in the address decoder of the main memory (RAM) and if the accessed
 * address is defined in the XML-file.
 *
 * @param[in] addr - the address or the buffer, where the fault is injected.
 */
static void fault_injection_controller_memory_address(CPUArchState *env, hwaddr *addr)
{
//    FaultList *fault = 0;
    int element = 0;
    //FaultInjectionInfo fi_info = {0, 0, 0, 0, 0, 0, 0};

    for (element = 0; element < getNumFaultListElements(); element++)
    {
//    	fault = getFaultListElement(element);
//
// 		#if defined(DEBUG_FAULT_CONTROLLER_TLB_FLUSH)
// 			error_printf("flushing tlb address %x\n", (int)*addr);
// 		#endif
// 		tlb_flush_page(env, (target_ulong)*addr);
//
//
//
// 		/*
// 		 * accessed address is not the defined fault address or the trigger is set to
// 		 * time- or pc-triggering.
// 		 */
//     	if ( fault->params.address != (int)*addr || strcmp(fault->trigger, "ACCESS") )
//     		continue;
//
//     	if (!strcmp(fault->component, "RAM") && !strcmp(fault->target, "ADDRESS DECODER"))
//     	{
//     		/*
//     		 * set/reset values
//     		 */
//     		fi_info.access_triggered_content_fault = 1;
//     		fi_info.new_value = 0;
//     		fi_info.bit_flip = 0;
//     		fi_info.fault_on_address = 1;
//     		fi_info.fault_on_register = 0;
//
// #if defined(DEBUG_FAULT_CONTROLLER)
// 	error_printf("memory address before: 0x%08x\n", (uint32_t) *addr);
// #endif
//
//     		if (!strcmp(fault->mode, "BIT-FLIP"))
//     			fault_injection_controller_bf(env, addr, fault, fi_info, 0);
//     		else if (!strcmp(fault->mode, "NEW VALUE"))
//     			fault_injection_controller_new_value(env, addr, fault, fi_info, 0);
//     		else if (!strcmp(fault->mode, "SF"))
//     			fault_injection_controller_rs(env, addr, fault, fi_info, 0);
//
// #if defined(DEBUG_FAULT_CONTROLLER)
// 	error_printf("memory address after: 0x%08x\n", (uint32_t) *addr);
// #endif
//     	}

// DO not uncomment
//		tlb_flush_page(env, (target_ulong)fault->params.address);
//		tlb_flush_page(env, (target_ulong)fault->params.cf_address);
    }
}

/**
 * Implements the interface to the appropriate controller functions.
 *
 * @param[in] env - Reference to the information of the CPU state.
 * @param[in] addr - the address of the accessed cell.
 * @param[in] value -  the value or buffer, which should be written to register or memory.
 * @param[in] injection_mode - defines the location, where the function is called from.
 * @param[in] access_type - if the access-operation is a write, read or execute.
 *
 */
void fault_injection_controller_init(CPUArchState *env, hwaddr *addr,
												uint32_t *value, InjectionMode injection_mode,
												AccessType access_type)
{
    //FaultList *fault;
    //int element = 0;

    profiler_log(addr, value, access_type);

	  if (*addr == address_in_use)
		  return;

	  if (injection_mode == FI_MEMORY_ADDR)
	  {
		    fault_injection_controller_memory_address(env, addr);
	  }
	// else if (injection_mode == FI_MEMORY_CONTENT)
	// {
	// 	if (env)
	// 	{
	// 		fault_injection_controller_memory_content(env, addr, value, access_type);
	// 		return;
	// 	}
  //
	// 	/**
	// 	 * get the CPUArchState of the current CPU (if not defined)
	// 	 */
	// 	if (next_cpu == NULL)
	// 		next_cpu = first_cpu;
  //
	// 	for (; next_cpu != NULL && !exit_request; next_cpu = CPU_NEXT(next_cpu))
	// 	{
	// 		CPUState *cpu = next_cpu;
	// 		CPUArchState *env = cpu->env_ptr;
  //
	// 		fault_injection_controller_memory_content(env, addr, value, access_type);
	// 	}
  //
	// }
	// else if (injection_mode == FI_INSN)
	// {
	// 	fault_injection_controller_insn(env, addr);
	// }
	// else if (injection_mode == FI_REGISTER_ADDR)
	// {
	// 	fault_injection_controller_register_address(env, addr);
	// }
	// else if (injection_mode == FI_REGISTER_CONTENT)
	// {
	// 	fault_injection_controller_register_content(env, addr, value, access_type);
	// }
	// else if (injection_mode == FI_TIME)
	// {
	//     for (element = 0; element < getNumFaultListElements(); element++)
	//     {
	//     	fault = getFaultListElement(element);
  //
  //   		#if defined(DEBUG_FAULT_CONTROLLER_TLB_FLUSH)
	//     		error_printf("flushing tlb address %x\n", fault->params.address);
	// 		#endif
  //   		tlb_flush_page(env, (target_ulong)fault->params.address);
  //   		tlb_flush_page(env, (target_ulong)fault->params.cf_address);
	//     }
  //
	// 	fault_injection_controller_time(env, addr, access_type);
	// }
	else
		error_printf("Unknown fault injection target!\n");
}

void start_automatic_test_process(CPUArchState *env)
{
	static int already_set = 0, shutting_down = 0;

	if (!already_set)
	{
		already_set = 1;
		hmp_fault_reload(cur_mon, NULL);
	}

	//if (sbst_cycle_count_value > SBST_CYCLES_BEFORE_EXIT && !shutting_down)
	if (false && !shutting_down)
	{
    // This Code does not make sense to me
		shutting_down = 1;
		fclose(outfile);

		hmp_info_faults(cur_mon, NULL);

		if (env)
		{
      CPUState *cpu;
			cpu = ENV_GET_CPU(env);
			cpu_dump_state(cpu, (FILE *)cur_mon, (fprintf_function) monitor_printf, CPU_DUMP_FPU);
		}

	 //   qmp_quit(NULL);
	}
}
