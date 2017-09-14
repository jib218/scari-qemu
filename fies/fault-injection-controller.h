#ifndef FAULT_INJECTION_CONTROLLER_H_
#define FAULT_INJECTION_CONTROLLER_H_

#include "qemu/osdep.h"
#include "cpu.h"

void fic_inject(CPUArchState *env);
void fic_flush_pages(CPUArchState *env);

#endif
