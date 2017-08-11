/*
 * fault-injection-controller.h
 *
 */

#ifndef FAULT_INJECTION_CONTROLLER_H_
#define FAULT_INJECTION_CONTROLLER_H_

//#include "config.h"
//#include "cpu.h"
#include "qemu/osdep.h" // IMPORTANT FOR INT64_T ...
#include "fault-injection-config.h"



/**
 * The declaration of the InjectionMode, which specifies,
 * if the controller-function is called from softmmu (for
 * access-triggered memory address or content faults)
 * or from register-access-function (for access-triggered
 * register address or content faults) or from
 * decode-cpu-function (for access-triggered instruction
 * faults) or fromtb_find_fast-function for time-triggered
 * faults.
 */
typedef enum
{
	FI_MEMORY_ADDR,
	FI_MEMORY_CONTENT,
	FI_REGISTER_ADDR,
	FI_REGISTER_CONTENT,
   FI_INSN,
   FI_TIME
}InjectionMode;

void start_automatic_test_process(CPUArchState *env);

#endif /* FAULT_INJECTION_CONTROLLER_H_ */
