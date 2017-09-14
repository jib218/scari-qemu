#ifndef FAULT_INJECTION_LIBRARY_H_
#define FAULT_INJECTION_LIBRARY_H_

#include "qemu/osdep.h"
#include "exec/hwaddr.h"


struct StuckAt {
    hwaddr vaddr;
    uint8_t *membytes;
    int numofbytes;

    struct StuckAt *next;
};

typedef struct StuckAt StuckAtList;

extern StuckAtList *stuckAtHead;

void insert_stuckat_value(hwaddr vaddr, uint8_t *membytes, int numofbytes);

void remove_stuckat_value(hwaddr vaddr);

void delete_stuckat_list(void);

#endif
