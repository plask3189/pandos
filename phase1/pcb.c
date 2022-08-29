/************************* pcb.c *********************************************
* pcb.c is a Queue Manager with a few parts:
*
*       1)  Allocate and deallocate pcbs
*           pcbFree List is a singly linked stack that holds free/unused pcbs.
*           The head is pointed to by pcbFree_h.
*
*       2) Process Queue Maintenance
*          Performs manipulations on the process queues (the ones in use currently)
*
*       3) Process Tree Maintenance
*/

// #include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h" //pcb.h includes types.h

pcb_PTR; // pcb_PTR is declared in pcb.h, but I think we have to define it here?

pcb_PTR pcbFree_h; // declaring pcbFree.h

//------------2.1: The Allocation and Deallocation of pcbs--------------------

/* Insert the element pointed to by p onto the pcbFree list. */
void freePcb(pcb_t *p){
  // code
}

/* Return NULL if the pcbFree list is empty. Otherwise, remove
an element from the pcbFree list, provide initial values for ALL
of the ProcBlk’s ﬁelds (i.e. NULL and/or 0) and then return a
pointer to the removed element. ProcBlk’s get reused, so it is
important that no previous value persist in a ProcBlk when it
gets reallocated. */
pcb_t *allocPcb(){
	// code
}

/* Initialize the pcbFree list
Initialize the pcbFree list to contain all the elements of the static array of
MAXPROC pcbs. This method will be called only once during data structure
initialization. */
initPcbs()


//------------2.2: Process Queue Maintenance-----------------------------------

/* This method is used to initialize a variable to be tail pointer to a
process queue.
Return a pointer to the tail of an empty process queue; i.e. NULL. */
pcb_t *mkEmptyProcQ() {
	return(NULL);
}

/* Return TRUE if the queue whose tail is pointed to by tp is empty.
Return FALSE otherwise. */
int emptyProcQ (pcb_t *tp){
	// NOT READABLE. I DON'T LIKE IT:    return (tp == NULL);
  if (tp == NULL){
    return true;
  } else {
    return false;
  }
}

/* Insert the ProcBlk pointed to by p into the process queue whose
tail-pointer is pointed to by tp. Note the double indirection through
tp to allow for the possible updating of the tail pointer as well. */
void insertProcQ(pcb_t **tp, pcb_t *p){
  // code
}

/* removeProcQ takes the pointer to the tail pointer and removes the
ProcBlk associated with it, then reassigns pointers around the removed ProcBlk*/
pcb_t *removeProcQ(pcb_t **tp){
  // code
}

/* Remove the ProcBlk pointed to by p from the process queue whose
tail-pointer is pointed to by tp. Update the process queue’s tail
pointer if necessary. If the desired entry is not in the indicated queue
(an error condition), return NULL; otherwise, return p. Note that p
can point to any element of the process queue. */
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
  // code
}

/* Return a pointer to the ﬁrst ProcBlk from the process queue whose
tail is pointed to by tp. Do not remove this ProcBlkfrom the process
queue. Return NULL if the process queue is empty. */
pcb_t *headProcQ(pcb_t *tp){
  // code
}

/*Removes the child pointed to by parent node p*/
pcb_t *removeChild (pcb_t *p){
	// code
}
/*outChild removes the child pointed to by p*/
pcb_t *outChild(pcb_t *p){
	// code
}
