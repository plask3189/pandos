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

HIDDEN pcb_PTR pcbFree_h; // declaration for private global variable that points
                          // to the head of the pcbFree list (page 15)

//------------2.1: The Allocation and Deallocation of pcbs--------------------

/* Insert the element pointed to by p onto the pcbFree list.
*  pushing a node pointed to by p (*p) onto the pcbFree list stack. */
void freePcb(pcb_t *p){
	p -> p_next = pcbFree_h; // load the address of the current head into the next
	                         // of the new node which is being pointed to by p
	pcbFree_h = p; // set the address of the new node to the address that the head
	             // holds. Now the head points to the new node that is on top
							 // of the stack holding the free pcbs.
}

/* Return NULL if the pcbFree list is empty. Otherwise, remove
an element from the pcbFree list, provide initial values for ALL
of the ProcBlk’s ﬁelds (i.e. NULL and/or 0) and then return a
pointer to the removed element. ProcBlk’s get reused, so it is
important that no previous value persist in a ProcBlk when it
gets reallocated. */
pcb_t *allocPcb(){
	// -------- Return NULL if the pcbFree list is empty. -----
  if(pcbFree_h == NULL){
    return NULL;
  }
  // -------- Otherwise, remove an element from the pcbFree list ---------
  pcb_PTR p_temp; // initialize p_temp pointer. We need temp pointer so we can keep track of old head.
  p_temp = pcbFree_h; // set address of current head to address of temp so that we
	                  // can return the pointer to the removed element
  pcbFree_h = pcbFree_h -> p_next; // access the head node's next that pcbFree_h
	                                 // points to. Set this next to pcbFree_h so that
																	 // pcbFree_h points to the node below the old head.

	// -----------Provide initial values for ALL of the ProcBlk’s ﬁelds (i.e. NULL and/or 0)
	p_PTR p_procBlkTemp; // initialize another pointer to point to values for ProcBlk's fields.
	p_procBlkTemp -> p_next = NULL;
	p_procBlkTemp -> p_prev = NULL;
	p_procBlkTemp -> p_prnt = NULL;
	p_procBlkTemp -> p_child = NULL;
	p_procBlkTemp -> p_sib = NULL;
	p_procBlkTemp -> p_semAdd= 0;
	p_procBlkTemp -> p_supportScruct = NULL;

	//------------ Then return a pointer to the removed element.
	return p_procBlkTemp;
}

/* Initialize the pcbFree list
Initialize the pcbFree list to contain all the elements of the static array of
MAXPROC pcbs. This method will be called only once during data structure
initialization. */
void initPcbs() {
	// code
}


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
  if (emptyProcQ(tp)) {
	  return Null;
  }
  return (tp -> p_next);
}

/*Removes the child pointed to by parent node p*/
pcb_t *removeChild (pcb_t *p){
	// code
}
/*outChild removes the child pointed to by p*/
pcb_t *outChild(pcb_t *p){
	// code
}
