#include "fault-injection-library.h"

StuckAtList *stuckAtHead = 0;

void insert_stuckat_value(hwaddr vaddr, uint8_t *membytes, int numofbytes)
{
    remove_stuckat_value(vaddr);

    StuckAtList *ptr = (StuckAtList*) malloc( sizeof(struct StuckAt));
    ptr->vaddr = vaddr;
    ptr->membytes = membytes;
    ptr->numofbytes = numofbytes;

    if(stuckAtHead == 0) {
        ptr->next = 0;
    } else {
        ptr->next = stuckAtHead;
    }

    stuckAtHead = ptr;
}

void remove_stuckat_value(hwaddr vaddr)
{
    if(!stuckAtHead)
        return;

    StuckAtList *curr = stuckAtHead;

    if(curr->vaddr == vaddr) {
        stuckAtHead = curr->next;
        free(curr->membytes);
        free(curr);
        return;
    }


    while (curr->next) {
        if(curr->next->vaddr == vaddr) {
          StuckAtList *tmp = curr->next;
          curr->next = tmp->next;
          free(tmp->membytes);
          free(tmp);
          break;
        }
        curr = curr->next;
    }
}

void delete_stuckat_list(void)
{
    StuckAtList *ptr;

    while ( (ptr = stuckAtHead) )
    {
        stuckAtHead = ptr->next;
        free(ptr->membytes);
        free(ptr);
    }
}
