#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

struct node
{
    int data;
    struct node* next;
};

struct node *head;

void push(int num)
{
    struct node *temp,*right;
    if(head != NULL){
        temp= (struct node *)malloc(sizeof(struct node));
        temp->data=num;
        right=(struct node *)head;
        while(right->next != NULL)
        right=right->next;
        right->next =temp;
        right=temp;
        right->next=NULL;
    }else{
        temp= (struct node *)malloc(sizeof(struct node));
        temp->data=num;
        temp->next = NULL;
        head = temp;
    }
}

 int pop(){
    struct node *temp = NULL;
    if(head == NULL){
        return 0;
    }else{
        temp = head;
        head = head->next;
        return temp->data;
    }
 }
 
int delete(int num){
    struct node *temp, *prev;
    temp=head;
    while(temp!=NULL)
    {
    if(temp->data==num)
    {
        if(temp==head)
        {
        head=temp->next;
        free(temp);
        return 1;
        }
        else
        {
        prev->next=temp->next;
        free(temp);
        return 1;
        }
    }
    else
    {
        prev=temp;
        temp= temp->next;
    }
    }
    return 0;
}

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
    int res = 0;
    res = pop();
    
 return res;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
    //search for page in queue
    int pageNum = p->frame >> PAGE_SHIFT;
    delete(pageNum);
    push(pageNum);
    
    
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
    head = NULL;
}
