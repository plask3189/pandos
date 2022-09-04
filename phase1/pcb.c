/************************* pcb.c *********************************************
* pcb.c is a Queue Manager with a few parts:
*
*       1)  Allocate and deallocate pcbs
*           pcbFree List is a singly linked stack that holds free/unused pcbs.
*           The head is pointed to by pcbFree_h.
*
*       2) Process Queue Maintenance
*          Performs manipulations on the process queues (the ones in use currently)
*          These process queues are double, circularly linked lists with a tail pointer tp
*
*       3) Process Tree Maintenance
*
*       4) Active Semaphore List
*
*/
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"

HIDDEN pcb_PTR pcbFree_h; // declaration for private global variable that points
                          // to the head of the pcbFree list (pandos page 15)

//------------2.1: The Allocation and Deallocation of pcbs--------------------

/* Insert the element pointed to by p onto the pcbFree list.
*  Pushing a node pointed to by p onto the pcbFree list stack. */
void freePcb(pcb_t *p){
	p -> p_next = pcbFree_h; // load the address of the current head into the next
	                         // of the new node which is being pointed to by p
	pcbFree_h = p; // set the address of the new node to the address that the head
	             // holds. Now the head points to the new node that is on top
							 // of the stack holding the free pcbs.
}

/* Return NULL if the pcbFree list is empty. Otherwise, remove
an element from the pcbFree list, provide initial values for ALL
of the pcb's ﬁelds (i.e. NULL and/or 0) and then return a
pointer to the removed element. pcbs get reused, so it is
important that no previous value persist in a pcb's when it
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

	// -----------Provide initial values for ALL of the pcbs' ﬁelds (i.e. NULL and/or 0)
	p_PTR p_pcbTemp; // initialize another pointer to point to values for pcbs' fields.
	// these fields are from page 8 of pandos
  /* process queue fields */
	p_pcbTemp -> p_next = NULL;
	p_pcbTemp -> p_prev = NULL;
	/* process tree fields */
	p_pcbTemp -> p_prnt = NULL;
	p_pcbTemp -> p_child = NULL;
	p_pcbTemp -> p_sib = NULL;
	/* process status information */
	p_pcbTemp -> p_s = NULL;
	p_pcbTemp -> p_time = 0;
	p_pcbTemp -> p_semAdd =NULL;
	/* support layer information */
	p_pcbTemp -> p_supportScruct = NULL;

	//------------ Then return a pointer to the removed element.------------
	return p_temp;
}

/* Initialize the pcbFree list to contain all the elements of the static array of
MAXPROC pcbs. This method will be called only once during data structure initialization. */
void initPcbs() {
  pcbFree_h = NULL; // initialize the head pointer to null.
	static pcb_t pcbArray[MAXPROC]; // create an array that holds pcbs with a size of MAXPROC. Set to 20 in const.h
	// add each pcb in MAXPROC, add it to the freeList
	for(int i = 0; i < MAXPROC; i++){
			addressOfPcbArrayElement = &pcbArray[i]); // the & means get the address of that element of the array
			                                         // we need to get an address because freePcb() takes a pointer
																							 // and a pointer is an address
			freePcb(addressOfPcbArrayElement); // addressOfPcbArrayElement will be the pointer that freePcb takes as a parameter
		}
}


//------------2.2: Process Queue Maintenance-----------------------------------
/* Generic queue manipulation methods. The queues that will be manipulated are
 double, circularly linked lists. */

/* This method is used to initialize a variable to be tail pointer to a
process queue. Return a pointer to the tail of an empty process queue; i.e. NULL. */
pcb_t *mkEmptyProcQ() {
	return(NULL);
}

/* Return TRUE if the queue whose tail is pointed to by tp is empty.
Return FALSE otherwise. */
int emptyProcQ (pcb_t *tp){ //*tp is the node that tp points to.
	return (tp == NULL);
  /* if (tp == NULL){
    return true;
  } else {
    return false;
  }	*/
}

/* Insert the pcb pointed to by p into the process queue whose
tail-pointer is pointed to by tp. Note the double indirection through
tp to allow for the possible updating of the tail pointer as well. */
void insertProcQ(pcb_t **tp, pcb_t *p){ /* **tp is the pointer to the pointer of tp
	                                       *p is the node that p points to */
	if (*tp == NULL){ // if the queue is empty
		p -> p_prev = p;
		p -> p_next = p;
	}
	else{
		// to enqueue a new node to the "front" when the queue is not empty

		//----Lines involving the head----
		//set the new node's prev to the tail node's prev. (which is the head. Want the actual head. Not just the pointer to the nead)
		p->p_prev = (*tp)->p_prev;
		// reset the tail's next address (which is the head) and get its next. change it to the address of the new node
		(*tp)->p_next = p;
		//--------------------------

		// reset the tail's prev to be the address of the new node.
		// now the tail's prev pointer points to the new node.
		tp->p_prev = p;

		//set the new node's next to be tail node's address
		p->p_next= tp;

		// set the new node's next to be the tail's address
		p->p_next = tp;

	}

}

/* Remove the pcb pointed to by p from the process queue whose tail- pointer
is pointed to by tp. Update the process queue’s tail pointer if necessary.
If the desired entry is not in the indicated queue (an error condition),
return NULL; otherwise, return p. Note that p can point to any element of
the process queue */
pcb_t *removeProcQ(pcb_t **tp){ /* Dequeuing */
	pcb_PTR removed;
	/* If queue is empty */
	if(*tp = NULL) {
		return NULL;
	}

	/* If the queue has only one node */
	if (((*tp) -> p_next == (*tp)) | ((*tp) -> p_prev == (*tp))) {
		removed = (*tp);
		(*tp) = mkEmptyProcQ();
		return removed;
	}

	/* If there are multiple nodes in the queue */
	// the tail's next is the head. This is what will be removed.
	removed = (*tp) -> p_next;

	// set the vice president node's prev to point to the tail
	(*tp) -> p_next -> p_next -> p_prev = (*tp);
		//***** Walk through of this crazy line: ***********
		// 1. get the tail's next which is the head address (let' make it up. say 0xpa...)
		// 2. Now we are at the head address. 0xpa...
		// 3. Go to the head's next which is the vice president node. Let's say it is 0xge...)
		// 4. Now we are at the vice president node. 0xge...
		// 5. Get the vice president's prev field. This would be 0xpa, the head.
		// 6. Since we're deleting the head, reset the vice president's prev field to the tail

	// set the tail's next field to hold the address of the vice president
	(*tp) -> p_next = ((*tp) -> p_next -> p_next);
	   //***** Walk through of this crazy line: ***********
		 // 1. get the tail's next field. (An address like 0xpa...)
		 // 2. On the other side of the equals sign, get the tail's next which is the head.
		 // 3. Get the head's next which is the address of the vice president, (with like 0xge...)]
		 // 4. Set the address of the vice president to be held by the next field of the tail
	return removed;
}

/* Remove the pcb pointed to by p from the process queue whose
tail-pointer is pointed to by tp. Update the process queue’s tail
pointer if necessary. If the desired entry is not in the indicated queue
(an error condition), return NULL; otherwise, return p. Note that p
can point to any element of the process queue. */
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
	pcb_PTR removed;

	/* If queue is empty */
	if(*tp = NULL) {
		return NULL;
	} else {
		if ((*tp) == p) { // If the tp points to the same pcb we want to delete.
			// more to come
		} else { // Otherwise, the tp does not point to the same pcb we want to delete.
			// more to come
		}
	}
}

/* Return a pointer to the first pcb from the process queue
whose tail is pointed to by tp. Do not remove this pcbfrom the process queue.
 Return NULL if the process queue is empty. */
pcb_t *headProcQ(pcb_t *tp){
  if (emptyProcQ(tp)) {
	  return NULL;
  }
  return (tp -> p_next);
}


// ---------------2.3 Process Tree Maintenance ----------------------------

/* Return TRUE if the pcb pointed to by p has no children. Return
FALSE otherwise. */
int emptyChild(pcb_t *p){
	//code
}

/* Make the pcb pointed to by p a child of the pcb pointed to by prnt.
*/
void insertChild(pcb t *prnt, pcb t *p)


/* Make the first child of the pcb pointed to by p no longer a child of p.
Return NULL if initially there were no children of p. Otherwise, return a pointer
to this removed first child pcb. */
pcb_t *removeChild (pcb_t *p){
	// code
}
/* Make the pcb pointed to by p no longer the child of its parent. If the pcb pointed
to by p has no parent, return NULL; otherwise, return p. Note that the element pointed
to by p need not be the first child of its parent. */
pcb_t *outChild(pcb_t *p){
	// code
}

// ---------------- 2.4 The Active Semaphore List (ASL) ------------------------

/* Search the ASL for a descriptor of this semaphore. If none is found, return NULL;
otherwise, remove the first (i.e. head) pcb from the process queue of the found semaphore descriptor and
return a pointer to it. If the process queue for this semaphore becomes empty (emptyProcQ(s procq) is TRUE),
remove the semaphore de- scriptor from the ASL and return it to the semdFree list. */
pcb_t *removeBlocked(int *semAdd) {
	//code
}

/* Remove the pcb pointed to by p from the process queue associated with p’s semaphore
(p→ p semAdd) on the ASL. If pcb pointed to by p does not appear in the process queue associated
with p’s semaphore, which is an error condition, return NULL; otherwise, re- turn p. */
pcb_t *outBlocked(pcb t *p) {
	// code
}



/* Return a pointer to the pcb that is at the head of the process queue associated with the semaphore
semAdd. Return NULL if semAdd is not found on the ASL or if the process queue associated with semAdd is empty. */
pcb_t *headBlocked(int *semAdd){
	//code
}


/* Initialize the semdFree list to contain all the elements of the array
static semd t semdTable[MAXPROC] This method will be only called once during data structure initializa- tion. */
void initASL(){
	//code
}
