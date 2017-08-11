/*
 * fault-injection-controller.h
 *
 */

#ifndef FAULT_INJECTION_CONTROLLER_H_
#define FAULT_INJECTION_CONTROLLER_H_

#include "qemu/osdep.h"
#include "cpu.h"
#include "fault-injection-config.h"

void fault_injection_controller_init(CPUArchState *env, hwaddr *addr,
												uint32_t *value, InjectionMode injection_mode,
												AccessType access_type);

void start_automatic_test_process(CPUArchState *env);

#endif /* FAULT_INJECTION_CONTROLLER_H_ */
