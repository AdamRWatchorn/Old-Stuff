#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile;

// Globals
int num_of_addrs;
int *mem;
int addr_itr = 0;
int *addrs;
int *to_be_used;

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	// init global variables
	to_be_used = malloc(sizeof(int) * memsize);
	mem = malloc(sizeof(int) * memsize);
	//init local variables
	FILE *fp;
	char buffer[100];
	addr_t addr = 0;

	// open the trace file
	fp = fopen(tracefile, "r");
	if(fp == NULL){
		printf("%s\n", "Cannot open the trace file");
		exit(1);
	}
	// get the number of address in the trace file
	while(fgets(buffer, 100, fp) != NULL){
		num_of_addrs = num_of_addrs + 1;
	}
	addrs = malloc(sizeof(int) * num_of_addrs);

	// reset the file pointer to the head of the file
	rewind(fp);
	//get the addresses
	int i = 0;
	char type;
	while(fgets(buffer, 100, fp)!=NULL){
		if(buffer[0] != '='){
			sscanf(buffer, "%c %lx", &type, &addr);
			addrs[i] = addr;
			i++;
		}
	}
}
/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	//populates mem array whenever a page is referenced
	mem[p->frame>>PAGE_SHIFT] = addrs[addr_itr];
	addr_itr = addr_itr + 1;
	return;
}
/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int frame = 0;
	int j;
	int k;
	// get values for the to_be_used array
	for(j=0; j<memsize; j++){
		for(k = addr_itr; k < num_of_addrs; k++){
			// found the address in memory
			if(addrs[k] == mem[j]){
				to_be_used[j] = k;
				break;
			}
		}
		//reached the end of the tracefile
		if(k == num_of_addrs){
			return j;
		}
	}
	int i;
	int better = 0;
	for(i=0;i<memsize;i++){
		if(to_be_used[i] > better){
			frame = i;
			better = to_be_used[i];
		}
	}
	return frame;
}
