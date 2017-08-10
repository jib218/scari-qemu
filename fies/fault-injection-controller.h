/*
 * fault-injection-controller.h
 *
 *  Created on: 17.08.2014
 *      Author: Gerhard Schoenfelder
 */

#ifndef FAULT_INJECTION_CONTROLLER_H_
#define FAULT_INJECTION_CONTROLLER_H_

//#include "config.h"
//#include "cpu.h"

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

/**
 * The declaration of the AccessType, which specifies
 * a read-, write- or execution-access
 */
typedef enum
{
	read_access_type,
	write_access_type,
	exec_access_type,
}AccessType;


#endif /* FAULT_INJECTION_CONTROLLER_H_ */
