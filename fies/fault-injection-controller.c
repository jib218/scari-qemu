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
#include "exec/exec-all.h"
#include <math.h>
#include "fault-injection-data-analyzer.h"

/**
 * Maybe useless
 */
static hwaddr address_in_use = UINT64_MAX;

/**
 * Normalizes the timer value to a uniform value (ns).
 *
 * @param[in] fault - pointer to the linked list entry
 * @param[in] start_time - the start time of the fault injection
 *                                       experiment as int64
 * @param[in] stop_time - the stop time of the fault injection
 *                                       experiment as int64
 * @param[in] interval - the interval time of the fault injection
 *                                   experiment as int64 (could be zero in
 *                                   case of no usage).
 */
static void time_normalization(FaultList *fault, int64_t *start_time,
														int64_t *stop_time, int64_t *interval)
{
	*start_time = (int64_t) timer_to_int(fault->timer);
	*stop_time =  (int64_t) timer_to_int(fault->duration);

	if (interval != NULL)
		*interval = (int64_t) timer_to_int(fault->interval);

	if (fault->timer && ends_with(fault->timer, "MS"))
	{
		*start_time *= SCALE_MS;
		*stop_time *=  SCALE_MS;
		if (interval != 0)
			*interval *=  SCALE_MS;
	}
	else if (fault->timer && ends_with(fault->timer, "US"))
	{
		*start_time *= SCALE_US;
		*stop_time *= SCALE_US;
		if (interval != 0)
			*interval *=  SCALE_MS;
	}
	else if (fault->timer && ends_with(fault->timer, "NS"))
	{
		*start_time *= SCALE_NS;
		*stop_time *= SCALE_NS;
		if (interval != 0)
			*interval *=  SCALE_MS;
	}
	else
	{
		return;
	}
}

/**
 * Sets bit-flip faults active for the different triggering-methods, extract the necessary
 * information (e.g. set bits in the fault mask), calls the appropriate functions in the
 * fault-injector module and increments the counter for the single fault types in the
 * analyzer-module.
 *
 * @param[in] env - Reference to the information of the CPU state.
 * @param[in] addr - the address or the buffer, where the fault is injected.
 * @param[in] fault - pointer to the linked list entry.
 * @param[in] fi_info - information for performing faults.
 * @param[in] pc - pc-value, when a fault should be triggered for pc-triggered faults
 *                          (could be zero in case of no usage).
 */
static void fault_injection_controller_bf(CPUArchState *env, hwaddr *addr,
																		FaultList *fault, FaultInjectionInfo fi_info,
																		uint32_t pc)
{
    int64_t current_timer_value = 0, start_time = 0, stop_time = 0;
    int64_t interval = 0;
	  int mask = fault->params.mask, set_bit = 0;

 	  fi_info.bit_flip = 1;

    if (fault->trigger && !strcmp(fault->trigger, "PC"))
	  {
		    if (pc == fault->params.address)
		    {
    		/**
    		 * search the set bits in mask (integer)
    		 */
    		   while (mask)
    		   {
        		 /**
        		 * extract least significant bit of 2s complement
        		 */
    			   set_bit = mask & -mask;

        		 /**
        		  * toggle the bit off
        		  */
    		 	   mask ^= set_bit ;

        		 /**
        		  * determine the position of the set bit
        		  */
    			   fi_info.injected_bit =  log2(set_bit);
        		 do_inject_memory_register(env, addr, fi_info);

      			if (fi_info.fault_on_register)
      				incr_num_injected_faults(fault->id, "reg trans");
      			else
      				incr_num_injected_faults(fault->id, "ram trans");
      		}
      		fault->is_active = 1;
  		}
  		else
  		{
  			fault->is_active = 0;
  		}
  	}
      else if (fault->type && !strcmp(fault->type, "TRANSIENT"))
  	{
  		time_normalization(fault, &start_time, &stop_time, NULL);

  		current_timer_value = fault_injection_controller_getTimer();
  		if (current_timer_value > start_time
  			&& current_timer_value < stop_time)
  		{
      		/**
      		 * search the set bits in mask (integer)
      		 */
      		while (mask)
      		{
          		/**
          		 * extract least significant bit of 2s complement
          		 */
      			set_bit = mask & -mask;

          		/**
          		 * toggle the bit off
          		 */
      			mask ^= set_bit ;

          		/**
          		 * determine the position of the set bit
          		 */
      			fi_info.injected_bit =  log2(set_bit);
          		do_inject_memory_register(env, addr, fi_info);

      			if (fi_info.fault_on_register)
      				incr_num_injected_faults(fault->id, "reg trans");
      			else
      				incr_num_injected_faults(fault->id, "ram trans");
      		}
      		fault->is_active = 1;
  		}
  		else
  		{
  			fault->is_active = 0;
  		}
  	}
  	else if (fault->type && !strcmp(fault->type, "INTERMITTEND"))
  	{
  		time_normalization(fault, &start_time, &stop_time, &interval);

  		current_timer_value = fault_injection_controller_getTimer();
  		if (current_timer_value > start_time
  			&& current_timer_value < stop_time
  			&& (current_timer_value / interval) % 2 == 0 )
  		{
      			/**
      			 * search the set bits in mask (integer)
      			 */
      		while (mask)
      		{
          		/**
          		 * extract least significant bit of 2s complement
          		 */
      			set_bit = mask & -mask;

          		/**
          		 * toggle the bit off
          		 */
      			mask ^= set_bit ;

          		/**
          		 * determine the position of the set bit
          		 */
      			fi_info.injected_bit =  log2(set_bit);
      			do_inject_memory_register(env, addr, fi_info);

      			if (fi_info.fault_on_register)
      				incr_num_injected_faults(fault->id, "reg trans");
      			else
      				incr_num_injected_faults(fault->id, "ram trans");
      		}
      		fault->is_active = 1;
  		}
  		else
  		{
  			fault->is_active = 0;
  		}
  	}
  	else if (fault->type && !strcmp(fault->type, "PERMANENT"))
  	{
  		/**
  		 * search the set bits in mask (integer)
  		 */
  		while (mask)
  		{
      		/**
      		 * extract least significant bit of 2s complement
      		 */
  			set_bit = mask & -mask;

      		/**
      		 * toggle the bit off
      		 */
  			mask ^= set_bit ;

      		/**
      		 * determine the position of the set bit
      		 */
  			fi_info.injected_bit =  log2(set_bit);
  			do_inject_memory_register(env, addr, fi_info);

  			if (fi_info.fault_on_register)
  				incr_num_injected_faults(fault->id, "reg perm");
  			else
  				incr_num_injected_faults(fault->id, "ram perm");
  		}
  		fault->is_active = 1;
  	}
  	else
  	{
  		return;
  	}
}


/**
 * Sets new-value faults active for the different triggering-methods, prepares the necessary
 * information (e.g. copy new-value to bit_value), calls the appropriate functions in the
 * fault-injector module and increments the counter for the single fault types in the
 * analyzer-module.
 *
 * @param[in] env - Reference to the information of the CPU state.
 * @param[in] addr - the address or the buffer, where the fault is injected.
 * @param[in] fault - pointer to the linked list entry.
 * @param[in] fi_info - information for performing faults.
 * @param[in] pc - pc-value, when a fault should be triggered for pc-triggered faults
 *                          (could be zero in case of no usage).
 */
static void fault_injection_controller_new_value(CPUArchState *env, hwaddr *addr,
																					FaultList *fault, FaultInjectionInfo fi_info,
																					uint32_t pc)
{
    int64_t current_timer_value = 0, start_time = 0, stop_time = 0;
    int64_t interval = 0;

	fi_info.bit_flip = 0;
	fi_info.new_value = 1;

    if (fault->trigger && !strcmp(fault->trigger, "PC"))
	{
		if (pc == fault->params.address)
		{
			/**
			 * copy the new value, which is stored in the mask-variable of
			 * the linked list, to the bit_value  variable of the FaultInjectionInfo
			 * struct, which will be written by the appropriate function from
			 * fault-injector module.
			 */
	  		fi_info.bit_value = fault->params.mask;
	   		do_inject_memory_register(env, addr, fi_info);

			if (fi_info.fault_on_register)
				incr_num_injected_faults(fault->id, "reg trans");
			else
				incr_num_injected_faults(fault->id, "ram trans");

			fault->is_active = 1;
		}
		else
		{
			fault->is_active = 0;
		}
	}
    else if (fault->type && !strcmp(fault->type, "TRANSIENT"))
	{
		time_normalization(fault, &start_time, &stop_time, NULL);

		current_timer_value = fault_injection_controller_getTimer();
		if (current_timer_value > start_time
			&& current_timer_value < stop_time)
		{
			/**
			 * copy the new value, which is stored in the mask-variable of
			 * the linked list, to the bit_value  variable of the FaultInjectionInfo
			 * struct, which will be written by the appropriate function from
			 * fault-injector module.
			 */
  			fi_info.bit_value = fault->params.mask;
   			do_inject_memory_register(env, addr, fi_info);

			if (fi_info.fault_on_register)
				incr_num_injected_faults(fault->id, "reg trans");
			else
				incr_num_injected_faults(fault->id, "ram trans");

    		fault->is_active = 1;
		}
		else
		{
			fault->is_active = 0;
		}
	}
	else if (fault->type && !strcmp(fault->type, "INTERMITTEND"))
	{
		time_normalization(fault, &start_time, &stop_time, &interval);

		current_timer_value = fault_injection_controller_getTimer();
		if (current_timer_value > start_time
			&& current_timer_value < stop_time
			&& (current_timer_value / interval) % 2 == 0 )
		{
			/**
			 * copy the new value, which is stored in the mask-variable of
			 * the linked list, to the bit_value  variable of the FaultInjectionInfo
			 * struct, which will be written by the appropriate function from
			 * fault-injector module.
			 */
  			fi_info.bit_value = fault->params.mask;
   			do_inject_memory_register(env, addr, fi_info);

			if (fi_info.fault_on_register)
				incr_num_injected_faults(fault->id, "reg trans");
			else
				incr_num_injected_faults(fault->id, "ram trans");

    		fault->is_active = 1;
		}
		else
		{
			fault->is_active = 0;
		}
	}
	else if (fault->type && !strcmp(fault->type, "PERMANENT"))
	{
		/**
		 * copy the new value, which is stored in the mask-variable of
		 * the linked list, to the bit_value  variable of the FaultInjectionInfo
		 * struct, which will be written by the appropriate function from
		 * fault-injector module.
		 */

		fi_info.bit_value = fault->params.mask;
		do_inject_memory_register(env, addr, fi_info);

		if (fi_info.fault_on_register)
			incr_num_injected_faults(fault->id, "reg perm");
		else
			incr_num_injected_faults(fault->id, "ram perm");

		fault->is_active = 1;
	}
	else
	{
		return;
	}
}

/**
 * Sets State Faults (SFs) active for the different triggering-methods, extracts the necessary
 * information (e.g. single, set bits in the mask), calls the appropriate functions in the
 * fault-injector module and increments the counter for the single fault types in the
 * analyzer-module.
 *
 * @param[in] env - Reference to the information of the CPU state.
 * @param[in] addr - the address or the buffer, where the fault is injected.
 * @param[in] fault - pointer to the linked list entry.
 * @param[in] fi_info - information for performing faults.
 * @param[in] pc - pc-value, when a fault should be triggered for pc-triggered faults
 *                          (could be zero in case of no usage).
 */
static void fault_injection_controller_rs(CPUArchState *env, hwaddr *addr,
																		FaultList *fault, FaultInjectionInfo fi_info,
																		uint32_t pc)
{
    int64_t current_timer_value = 0, start_time = 0, stop_time = 0;
    int64_t interval = 0;
    int mask = fault->params.mask, set_bit = 0;

	fi_info.bit_flip = 0;

    if (fault->trigger && !strcmp(fault->trigger, "PC"))
	{
		if (pc == fault->params.address)
		{
    		/**
    		 * search the set bits in mask (integer)
    		 */
    		while (mask)
    		{
        		/**
        		 * extract least significant bit of 2s complement
        		 */
    			set_bit = mask & -mask;

        		/**
        		 * toggle the bit off
        		 */
    			mask ^= set_bit ;

        		/**
        		 * determine the position of the set bit
        		 */
    			fi_info.injected_bit =  log2(set_bit);

    			/**
    			 * copy the information, if a bit should be set or reset, which is stored
    			 * in the set_bit-variable of the linked list to the bit-value-variable of
    			 * the FaultInjectionInfo struct, if the mask is set at this position.
    			 * The double negation (!!) is used to convert an integer to a single
    			 * logical value (0 or 1).
    			 */
    			fi_info.bit_value = !!(fault->params.set_bit & set_bit);
       			do_inject_memory_register(env, addr, fi_info);

    			if (fi_info.fault_on_register)
    				incr_num_injected_faults(fault->id, "reg trans");
    			else
    				incr_num_injected_faults(fault->id, "ram trans");
    		}
    		fault->is_active = 1;
		}
		else
		{
			fault->is_active = 0;
		}
	}
    else if (fault->type && !strcmp(fault->type, "TRANSIENT"))
	{
		time_normalization(fault, &start_time, &stop_time, NULL);

		current_timer_value = fault_injection_controller_getTimer();
		if (current_timer_value > start_time
			&& current_timer_value < stop_time)
		{
    		/**
    		 * search the set bits in mask (integer)
    		 */
    		while (mask)
    		{
        		/**
        		 * extract least significant bit of 2s complement
        		 */
    			set_bit = mask & -mask;

        		/**
        		 * toggle the bit off
        		 */
    			mask ^= set_bit ;

        		/**
        		 * determine the position of the set bit
        		 */
    			fi_info.injected_bit =  log2(set_bit);

    			/**
    			 * copy the information, if a bit should be set or reset, which is stored
    			 * in the set_bit-variable of the linked list to the bit-value-variable of
    			 * the FaultInjectionInfo struct, if the mask is set at this position.
    			 * The double negation (!!) is used to convert an integer to a single
    			 * logical value (0 or 1).
    			 */
    			fi_info.bit_value = !!(fault->params.set_bit & set_bit);
       			do_inject_memory_register(env, addr, fi_info);

    			if (fi_info.fault_on_register)
    				incr_num_injected_faults(fault->id, "reg trans");
    			else
    				incr_num_injected_faults(fault->id, "ram trans");
    		}
    		fault->is_active = 1;
		}
		else
		{
			fault->is_active = 0;
		}
	}
	else if (fault->type && !strcmp(fault->type, "INTERMITTEND"))
	{
		time_normalization(fault, &start_time, &stop_time, &interval);

		current_timer_value = fault_injection_controller_getTimer();
		if (current_timer_value > start_time
			&& current_timer_value < stop_time
			&& (current_timer_value / interval) % 2 == 0 )
		{
    		/**
    		 * search the set bits in mask (integer)
    		 */
    		while (mask)
    		{
        		/**
        		 * extract least significant bit of 2s complement
        		 */
    			set_bit = mask & -mask;

        		/**
        		 * toggle the bit off
        		 */
    			mask ^= set_bit ;

        		/**
        		 * determine the position of the set bit
        		 */
    			fi_info.injected_bit =  log2(set_bit);

    			/**
    			 * copy the information, if a bit should be set or reset, which is stored
    			 * in the set_bit-variable of the linked list to the bit-value-variable of
    			 * the FaultInjectionInfo struct, if the mask is set at this position.
    			 * The double negation (!!) is used to convert an integer to a single
    			 * logical value (0 or 1).
    			 */
    			fi_info.bit_value = !!(fault->params.set_bit & set_bit);
        		do_inject_memory_register(env, addr, fi_info);

    			if (fi_info.fault_on_register)
    				incr_num_injected_faults(fault->id, "reg trans");
    			else
    				incr_num_injected_faults(fault->id, "ram trans");
    		}
    		fault->is_active = 1;
		}
		else
		{
			fault->is_active = 0;
		}
	}
	else if (fault->type && !strcmp(fault->type, "PERMANENT"))
	{
		/* search the set bits in mask (integer) */
		while (mask)
		{
			set_bit = mask & -mask;  // extract least significant bit of 2s complement
			mask ^= set_bit ;  // toggle the bit off

			fi_info.injected_bit =  (uint32_t) log2(set_bit); // determine the position of the set bit
			fi_info.bit_value =  !!(fault->params.set_bit & set_bit); // determine if bit should be set or reset
			do_inject_memory_register(env, addr, fi_info);

			if (fi_info.fault_on_register)
				incr_num_injected_faults(fault->id, "reg perm");
			else
				incr_num_injected_faults(fault->id, "ram perm");
		}
		fault->is_active = 1;
	}
	else
	{
		return;
	}
}

/**
 * Iterates through the linked list and checks if an entry or element belongs to
 * a fault in the address decoder of the main memory (RAM) and if the accessed
 * address is defined in the XML-file.
 *
 * @param[in] addr - the address or the buffer, where the fault is injected.
 */
static void fault_injection_controller_memory_address(CPUArchState *env, hwaddr *addr)
{
    FaultList *fault = 0;
    int element = 0;
    FaultInjectionInfo fi_info = {0, 0, 0, 0, 0, 0, 0};

    for (element = 0; element < getNumFaultListElements(); element++)
    {
    	fault = getFaultListElement(element);

		#if defined(DEBUG_FAULT_CONTROLLER_TLB_FLUSH)
			error_printf("flushing tlb address %x\n", (int)*addr);
		#endif
		  tlb_flush_page(ENV_GET_CPU(env), (target_ulong)*addr);

		/*
		 * accessed address is not the defined fault address or the trigger is set to
		 * time- or pc-triggering.
		 */
    	if ( fault->params.address != (int)*addr || strcmp(fault->trigger, "ACCESS") )
    		continue;

    	if (!strcmp(fault->component, "RAM") && !strcmp(fault->target, "ADDRESS DECODER"))
    	{
    		/*
    		 * set/reset values
    		 */
    		fi_info.access_triggered_content_fault = 1;
    		fi_info.new_value = 0;
    		fi_info.bit_flip = 0;
    		fi_info.fault_on_address = 1;
    		fi_info.fault_on_register = 0;

#if defined(DEBUG_FAULT_CONTROLLER)
	error_printf("memory address before: 0x%08x\n", (uint32_t) *addr);
#endif

    		if (!strcmp(fault->mode, "BIT-FLIP"))
    			fault_injection_controller_bf(env, addr, fault, fi_info, 0);
    		else if (!strcmp(fault->mode, "NEW VALUE"))
    			fault_injection_controller_new_value(env, addr, fault, fi_info, 0);
    		else if (!strcmp(fault->mode, "SF"))
    			fault_injection_controller_rs(env, addr, fault, fi_info, 0);

#if defined(DEBUG_FAULT_CONTROLLER)
	error_printf("memory address after: 0x%08x\n", (uint32_t) *addr);
#endif
    	}

// There seems to be a reason why these two lines are commented
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
