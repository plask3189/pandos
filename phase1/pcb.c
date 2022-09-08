/************************* pcb.c *********************************************
* Written by Kate Plas and Travis Wahl
* For CSCI-320 Operating Systems
*
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
*
*/

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"

HIDDEN pcb_PTR pcbFree_h; /* declaration for private global variable that points */
                          /* to the head of the pcbFree list (pandos page 15) */

/*------------2.1: The Allocation and Deallocation of pcbs--------------------

/* Insert the element pointed to by p onto the pcbFree list.
*  Pushing a node pointed to by p onto the pcbFree list stack. */
void freePcb(pcb_t *p){
	p -> p_next = pcbFree_h; /* load the address of the current head into the next */
	                         /* of the new node which is being pointed to by p */
	pcbFree_h = p; /* set the address of the new node to the address that the head */
	             /* holds. Now the head points to the new node that is on top */
							 /* of the stack holding the free pcbs. */
}


/* Return NULL if the pcbFree list is empty. Otherwise, remove
an element from the pcbFree list, provide initial values for ALL
of the pcb's ﬁelds (i.e. NULL and/or 0) and then return a
pointer to the removed element. pcbs get reused, so it is
important that no previous value persist in a pcb's when it
gets reallocated. */
pcb_t *allocPcb(){
	/* -------- Return NULL if the pcbFree list is empty. ----- */
  if (pcbFree_h == NULL){
    return NULL;
  }
  else {
  /* -------- Otherwise, remove an element from the pcbFree list --------- */
  pcb_PTR p_temp; /* initialize p_temp pointer. We need temp pointer so we can keep track of old head. */
  p_temp = pcbFree_h; /* set address of current head to address of temp so that we */
	                  /* can return the pointer to the removed element */
  pcbFree_h = p_temp -> p_next; /* access the head node's next that pcbFree_h */
	                                 /* points to. Set this next to pcbFree_h so that */
								/* pcbFree_h points to the node below the old head. */

	/* -----------Provide initial values for ALL of the pcbs' ﬁelds (i.e. NULL and/or 0) */
	/* these fields are from page 8 of pandos */
  	/* process queue fields */
	p_temp -> p_next = NULL;
	p_temp -> p_prev = NULL;
	/* process tree fields */
	p_temp -> p_prnt = NULL;
	p_temp -> p_child = NULL;
	p_temp -> p_sib = NULL;
	p_temp -> p_sibPrev = NULL;
	/* process status information */
	p_temp -> p_s = NULL;
	p_temp -> p_time = 0;
	p_temp -> p_semAdd =NULL;
	/* support layer information */
	p_temp -> p_supportStruct = NULL;

	/*------------ Then return a pointer to the removed element.------------ */
	return p_temp;
	}
}

/* Initialize the pcbFree list to contain all the elements of the static array of
MAXPROC pcbs. This method will be called only once during data structure initialization. */
void initPcbs() {
  pcbFree_h = NULL; /* initialize the head pointer to null. */
	int i = 0;
	static pcb_t pcbArray[MAXPROC]; /* create an array that holds pcbs with a size of MAXPROC. Set to 20 in const.h */
	/* add each pcb in MAXPROC, add it to the freeList */
	for (i = 0; i < MAXPROC; i++){
			pcb_PTR addressOfPcbArrayElement = &pcbArray[i]; /* the & means get the address of that element of the array */
			                                         /* we need to get an address because freePcb() takes a pointer */
								/* and a pointer is an address */
			freePcb(addressOfPcbArrayElement); /* addressOfPcbArrayElement will be the pointer that freePcb takes as a parameter */
		}
}


/*------------2.2: Process Queue Maintenance-----------------------------------*/
/* Generic queue manipulation methods. The queues that will be manipulated are
 double, circularly linked lists. */

/* This method is used to initialize a variable to be tail pointer to a
process queue. Return a pointer to the tail of an empty process queue; i.e. NULL. */
pcb_t *mkEmptyProcQ() {
	return NULL;
}

/* Return TRUE if the queue whose tail is pointed to by tp is empty.
Return FALSE otherwise. */
int emptyProcQ (pcb_t *tp){ /*tp is the node that tp points to. */
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
void insertProcQ(pcb_t **tp, pcb_t *p){
	/*If the queue is empty */
	if (emptyProcQ(*tp)){
		(*tp) = p; 
		p -> p_prev = p;
		p -> p_next = p;
	}
	/* If there is only one node in the queue*/
	else if ((*tp)->p_next == (*tp)){
		/* Pointer rearrangements to link the old lonely node to the new one */
		p->p_next = (*tp); /* set the new node's next to the tail */
		p->p_prev = (*tp); /* set the new node's pre to the tail */
		(*tp)->p_prev = p; /* set the tail's prev to the new node */
		(*tp)->p_next = p; /* set the tail's next to the new node */
		(*tp) = p; /* the tail pointer is set to the new node */
	}

	/* If there are multiple nodes in the queue*/
	else {
		p->p_next = (*tp)->p_next; /* the tail's next which is the address of the head is set to the next of the new node. */
		p->p_next->p_prev = p; /* the head's prev is set to the address of p. */
		/* now the head and new node are linked */
		p->p_prev = (*tp); /* set the new node's prev to the tail */
		(*tp)->p_next = p; /* set the tail's next to the new node */
		(*tp) = p; /* the tail pointer is set to the new node */
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
	if ((*tp) == NULL) {
		return NULL;
	}

	/* If the queue has only one node */
	if (((*tp) -> p_next == (*tp)) | ((*tp) -> p_prev == (*tp))) {
		removed = (*tp);
		(*tp) = mkEmptyProcQ();
		return removed;
	}

	/* If there are multiple nodes in the queue */
	/* the tail's next is the head. This is what will be removed. */
	removed = (*tp) -> p_next;

	/* set the vice president node's prev to point to the tail */
	(*tp) -> p_next -> p_next -> p_prev = (*tp);
		/****** Walk through of this crazy line: ***********
		/* 1. get the tail's next which is the head address (let' make it up. say 0xpa...)
		/* 2. Now we are at the head address. 0xpa...
		/* 3. Go to the head's next which is the vice president node. Let's say it is 0xge...)
		/* 4. Now we are at the vice president node. 0xge...
		/* 5. Get the vice president's prev field. This would be 0xpa, the head.
		/* 6. Since we're deleting the head, reset the vice president's prev field to the tail */

	/* set the tail's next field to hold the address of the vice president */
	(*tp) -> p_next = ((*tp) -> p_next -> p_next);
	   /****** Walk through of this crazy line: ***********
		 /* 1. get the tail's next field. (An address like 0xpa...)
		 /* 2. On the other side of the equals sign, get the tail's next which is the head.
		 /* 3. Get the head's next which is the address of the vice president, (with like 0xge...)]
		 /* 4. Set the address of the vice president to be held by the next field of the tail */
	return removed;
}

/* Remove the pcb pointed to by p from the process queue whose
tail-pointer is pointed to by tp. Update the process queue’s tail
pointer if necessary. If the desired entry is not in the indicated queue
(an error condition), return NULL; otherwise, return p. Note that p
can point to any element of the process queue. */
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
	if ((*tp) == NULL){	/* Null handler */
		return NULL;
	}
	if ((*tp) == p) {		/* if the tail pointer is p, just run removeProcQ */
		return removeProcQ(tp);
	}
	pcb_t *tail = (*tp);		/* Keep track of the tail pointer */
	pcb_t *p_temp = p;		/* Create a temp pointer */
	while (tail -> p_next != (*tp)){	/* While loop if removing from middle */
		if (tail -> p_next == p) { /* if the next pcb from the tail is p: */
			p_temp -> p_prev -> p_next = p_temp -> p_next;
			p_temp -> p_next -> p_prev = p_temp -> p_prev;
			p_temp -> p_next = NULL;
			p_temp -> p_prev = NULL;
			return p_temp;
		}
		else {				/* Otherwise, adjust the tail var and return to the loop */
			tail = tail -> p_next;
		}
	}
	return NULL;
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


/* ---------------2.3 Process Tree Maintenance ---------------------------- */

/* Return TRUE if the pcb pointed to by p has no children. Return
FALSE otherwise. */
int emptyChild(pcb_t *p){
	return (p -> p_child == NULL);
	
	
	/*if (p -> p_child == NULL) {
		return TRUE;
	}
	else {
		return FALSE;
	} */
}

/* Make the pcb pointed to by p a child of the pcb pointed to by prnt.
*/
void insertChild(pcb_t *prnt, pcb_t *p) {

	/* If PCB has no children */
	if (emptyChild(prnt)) {		/* check if the pcb has no children, if so: */
		prnt -> p_child = p;	/* create child (p) from parent (prnt) */
		p -> p_prnt = prnt;	/* give child (p) a parent from p_prnt */
		p -> p_sibPrev = NULL;	/* We know parent had no children, thus p_sibPrev is NULL */
		p -> p_sib = NULL;	/* We know parent had no children, thus p_sib is NULL */
	}
	/* If PCB has children */
	else {
		p -> p_prnt = prnt;			/* p_prnt becomes parent */
		prnt -> p_child -> p_sibPrev = p;	/* Previous sibling of p_child under prnt becomes p */
		p -> p_sib = prnt -> p_child;		/* Sibling of p equals p_child of parent */
		p -> p_sibPrev = NULL;			/* Can now set the Previous sibling of p to NULL */
		prnt -> p_child = p;			/* p is now a child of the pcb pointed to by prnt */
	}
}


/* Make the first child of the pcb pointed to by p no longer a child of p.
Return NULL if initially there were no children of p. Otherwise, return a pointer
to this removed first child pcb. */
pcb_t *removeChild (pcb_t *p){
	pcb_t *removed = p -> p_child;
	
	/* If-Empty Handler */
	if (emptyChild(p)) {		/* Check if p has children, if not then return NULL */
		return NULL;
	}
	/* If there is only one child */
	else if (p -> p_child -> p_sibPrev == NULL) {	/* Check if there is only ONE child */
		p -> p_child = NULL;			/* child of p is split from p */
		return removed;
	}
	/* If there is more than one child */
	else {
		removed -> p_sibPrev -> p_sib = NULL;	/* update previous sibling of the temp's sibling to NULL */
		removed -> p_prnt -> p_child = removed -> p_sibPrev;
		removed -> p_sibPrev = NULL;		/* Previous sibling of p_temp is NULL */
		removed -> p_prnt = NULL;			/* parent of p is NULL */
		return removed;
	}
}
/* Make the pcb pointed to by p no longer the child of its parent. If the pcb pointed
to by p has no parent, return NULL; otherwise, return p. Note that the element pointed
to by p need not be the first child of its parent. */
pcb_t *outChild(pcb_t *p){

	/* If-Empty handler */
	if (p -> p_prnt == NULL) {	/* If parent of p is empty, return NULL */
		return NULL;
	}

	/* If p is the Last child */
	if (p -> p_sib == NULL) {	/* Check if p is the Last child */
		p -> p_sibPrev -> p_sib = NULL;	/* If true, set previous sib of p's Next sibling to NULL */
		p -> p_prnt = NULL;		/* Set parent of p to NULL */		
		p -> p_sibPrev = NULL;		/* Set sib of p to NULL */
		return p;
	}	
	
	/* If p is the Head child */
	if (p -> p_prnt -> p_child == p) {		/* Check if p is the Head child */
		p -> p_prnt -> p_child = p -> p_sib;	/* If true, the child of p's parent becomes p's Next sibling */
		p -> p_sib -> p_sibPrev = NULL;		/* Previous sibling of p's Next sibling becomes NULL */
		p -> p_prnt = NULL;			/* Parent of p becomes NULL */
		p -> p_sib = NULL;			/* Sibling of p becomes NULL */
		return p;
	}	

	/* If p is a middle child */
	if (p -> p_sib != NULL && p -> p_sibPrev != NULL) {	/* Check if there is a Next and Previous sibling to p */
		p -> p_sib -> p_sibPrev = p -> p_sibPrev;	/* The Previous sibling of p's Next sibling equals the Previous sibling of p */
		p -> p_sibPrev -> p_sib = p -> p_sib;		/* If true, the Next sibling of p's Previous sibling equals the Next sibling of p  */
		p -> p_prnt = NULL;				/* p's parent set to NULL */
		p -> p_sib = NULL;				/* p's Next sibling set to NULL */
		p -> p_sibPrev = NULL;				/* p's Previous sibling set NULL */
		return p;
	}
	return NULL;	/* if no conditions are met, useful for debugging as there should be a met condition */
}
