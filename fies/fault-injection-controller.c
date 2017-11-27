#include "fault-injection-controller.h"
#include "fault-injection-library.h"
#include "profiler.h"
#include "qemu/error-report.h"
#include "exec/exec-all.h"

void fic_inject(CPUArchState *env)
{
    CPUState * cpu = 0;
    if (env == 0) {
        cpu = current_cpu;
    } else {
        cpu = ENV_GET_CPU(env);
    }

    StuckAtList *curr = stuckAtHead;
    while(curr) {
        profiler_log_generic("fic_inject\n");

        memset(curr->cache, 0, curr->numofbytes);

        if(cpu_memory_rw_debug(cpu, curr->vaddr, curr->cache, curr->numofbytes, 0) < 0) {
            curr = curr->next;
            continue;
        }

        for(int currByte = 0; currByte < curr->numofbytes; currByte ++)
            curr->cache[currByte] ^= curr->membytes[currByte];

        cpu_memory_rw_debug(cpu, curr->vaddr, curr->cache, curr->numofbytes, 1);

        curr = curr->next;
    }

}

void fic_flush_pages(CPUArchState *env)
{
    CPUState * cpu = 0;
    if (env == 0) {
        cpu = current_cpu;
    } else {
        cpu = ENV_GET_CPU(env);
    }

    StuckAtList *curr = stuckAtHead;
    while(curr) {
        tlb_flush_page(cpu, curr->vaddr);

        curr = curr->next;
    }
}
