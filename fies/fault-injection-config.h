/*
 * fault-injection-config.h
 *
 *  Created on: 01.10.2014
 *      Author: Gerhard Schoenfelder
 */

#ifndef FAULT_INJECTION_CONFIG_H_
#define FAULT_INJECTION_CONFIG_H_

#include "qemu/osdep.h"

/**
 * Uncomment the following define for activating debug output
 */
// #define DEBUG_FAULT_CONTROLLER

/**
 * Uncomment the following define for activating debug output
 */
// #define DEBUG_FAULT_CONTROLLER_TLB_FLUSH

/**
 * Defines the width of the memory interface (in this
 * case 16bit). It is used  for the allocation of the
 * array, which stores the previous cell operation
 * modes for dynamic faults.
 */
#define MEMORY_WIDTH	32

/**
 * Uncomment the following define for activating debug output
 */
// #define DEBUG_FAULT_LIST

/**
 * Memory address of the fault counter variable
 */
#define FAULT_COUNTER_ADDRESS	0x402010c8

/**
 * Memory address of the sbst cycle count variable
 */
#define SBST_CYCLE_COUNT_ADDRESS	0x40200048

/**
 * Defines the name and path of the file, where the data collector writes
 * his information to.
 */
#define DATA_COLLECTOR_FILENAME	"fies.log"

/**
 * Defines the amount of cycles, before the  fault injection experiment is terminated.
 */
//#define SBST_CYCLES_BEFORE_EXIT	1




extern unsigned int fault_counter_address;
extern unsigned int file_input_to_use;
extern unsigned int file_input_to_use_address;
extern char *fault_library_name;


extern FILE *outfile;

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

#endif /* FAULT_INJECTION_CONFIG_H_ */
