#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clockPointer;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	while (clockPointer <= memsize){
	    clockPointer = (clockPointer + 1) % memsize;

	    if((coremap[clockPointer].pte->frame & PG_REF) == 0){
	        break;
	    }
	    coremap[clockPointer].pte->frame &= ~PG_REF;

	}
	return clockPointer;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
    p->frame |= PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
    clockPointer = 0;
}
