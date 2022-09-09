/* ---------------- 2.4 The Active Semaphore List (ASL) ------------------------
/* Written by Kate Plas and Travis Wahl
 * For CSCI-320 Operating Systems
 * ----- Includes two main parts: -------------------
 *   1. The ASL is a null-terminated single, linearly lined list whose nodes each have a s_next field.
 *      The ASL has a head pointer called semd_h. A semaphore is active if there is at least one pcb on the process queue associated with it.
 *   2. The semdFree list which is also a single linearly linked list. It keeps the semaphores that are free.
*/

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "pcb.c"

HIDDEN semd_t *semd_h, *semdFreeList_h;
/* semd_h is the head pointer of the active semaphore list .
/* semdFree_h is the head pointer to the semdFree list that holds the unused semaphore descriptors.
*/



/**************************** semdFreeList Supporting Methods ***************************/
/* semndFreeList is a singly linked NULL terminated stack that holds the free semnds.*/

/* Pushes a node pointed to by s onto the stack that is the semdFreeList
   Note that the next pointer of each node points downwards in the stack */
void freeSemd(semd_t *s){
	if (semdFreeList_h == NULL){ /* if the freeList is empty: */
				s -> s_next = NULL; /* set the new node's next to NULL since there is no other node in the stack */
  if (semdFreeList_h != NULL){ /* if the freeList is not empty: */
        s -> s_next = semdFreeList_h; /* set the new node's next to hold the head address because the head will be below the new node on the stack. */
    }
	semdFreeList_h = s;  /* the head points to the new node. */
	}
}

/* Pop a semd from the FreeList */
semd_t *popSemdFromFreeList(){
	semd_t *temp = semdFreeList_h;
	if(semdFreeList_h != NULL){ /* if the free list has nodes already */
		semdFreeList_h = semdFreeList_h -> s_next;
		temp -> s_next = NULL;
		temp -> s_semAdd = NULL;
		temp -> s_procQ = mkEmptyProcQ(); /* mkEMptyProcQ() Returns a pointer to the tail of an empty process queue; i.e. NULL. */
		return temp;
	}
	else { /* if the free list is already empty, can't pop anything else, return NULL */
		return NULL;
	}
}


/**************************** Active Semaphore List Supporting Methods ***************************/
/* Look through the active semaphore list for the semAdd (pointer to the semaphore) */
/* "A semaphore is active if there is at least one pcb on the process queue associated with it." p. 22 Pandos*/
semd_t *search(int *semAdd){
	semd_t *temp = semd_h;
	if(semAdd == NULL || semd_h->s_next == NULL){
		return NULL;
	} else {
		while(temp->s_next != NULL){ /* Look through the list until we reach the ending dumb node whose next is null */
			if(temp->s_next->s_semAdd == semAdd){
				return(temp->s_next); /* oooo here it is */
			}
			else{
				temp = temp->s_next; /* to increment through list */
			}
		}
		return NULL;
	}
}


/* Insert the pcb pointed to by p at the tail of the process queue associated with the
semaphore whose physical address is semAdd and set the semaphore address of p to semAdd
If the semaphore is currently not active (i.e. there is no descriptor for it in the ASL),
allocate a new descriptor from the semdFree list, insert it in the ASL (at the appropriate position),
initialize all of the fields (i.e. set s_semAdd to semAdd, and s_procQ to mkEmptyProcQ()), and proceed as above.
If a new semaphore descriptor needs to be allocated and the semdFree list is empty, return TRUE.
In all other cases return FALSE. */
int insertBlocked (int *semAdd, pcb_t *p) {
	semd_t* temp = search(semAdd); /* find what semaphore's queue to insert the pcb on */
  /* If a new semaphore descriptor needs to be allocated and the semdFree list is empty, return TRUE.*/
	if(temp -> s_next -> s_semAdd != semAdd) { /* if a location to insert a new pcb does not exist */
		semd_t *semaphoreForPcb = popSemdFromFreeList();
		if(semaphoreForPcb == NULL) {
				return TRUE;
	}
	semaphoreForPcb -> s_next = temp -> s_next;
	temp -> s_next = semaphoreForPcb;
	semaphoreForPcb -> s_procQ = mkEmptyProcQ();
	insertProcQ(&(semaphoreForPcb -> s_procQ),p);
	semaphoreForPcb -> s_semAdd = semAdd;
	p -> p_semAdd = semAdd;
	return FALSE;
	}
	else {
	/* If the spot to insert a pcb is available*/
		p -> p_semAdd = semAdd;
		insertProcQ(&(temp -> s_next -> s_procQ), p);
		return FALSE;
	}
}


/* Search the ASL for a descriptor of this semaphore. If none is found, return NULL;
otherwise, remove the first (i.e. head) pcb from the process queue of the found semaphore descriptor and
return a pointer to it. If the process queue for this semaphore becomes empty (emptyProcQ(s procq) is TRUE),
remove the semaphore de- scriptor from the ASL and return it to the semdFree list. */
pcb_t *removeBlocked(int *semAdd) {
	semd_t *temp = search(semAdd);	/* Set a temp var using the search method on semADD */
	if (temp -> s_next -> s_semAdd == semAdd) {		/* If pointer to sempahor (s_semAdd) of the next element on the ASL from temp == semADD */
		pcb_t *removed = removeProcQ(&temp -> s_next -> s_procQ);	/* Creation of removed var to track removed pcb */
		if (emptyProcQ(temp -> s_next -> s_procQ)){	/* run emptyProcQ to test if empty */
			semd_t *emptySemd = temp -> s_next;	/* Create emptySemd to track what we will use freeSemd on */
			temp -> s_next = emptySemd -> s_next;	/* next element from temp is equal to the next element of emptySemd */
			freeSemd(emptySemd);			/* run freeSemd on emptySemd */
			removed -> p_semAdd = NULL;		/* reset p_semADD to NULL */
			return removed;
		}
		else {
			removed -> p_semAdd = NULL;		/* Otherwise set the blocked pointer to NULL */
			return removed;
		}
	}
	else {							/* Otherwise return NULL */
		return NULL;
	}

}


/* Remove the pcb pointed to by p from the process queue associated with p’s semaphore
(p→ p semAdd) on the ASL. If pcb pointed to by p does not appear in the process queue associated
with p’s semaphore, which is an error condition, return NULL; otherwise, re- turn p. */
pcb_t *outBlocked(pcb_t *p) {
	semd_t *temp = search(p -> p_semAdd);	/* Set a temp var using the search method on p _> p_semADD */
	if(temp -> s_next -> s_semAdd == p -> p_semAdd) {	/* If the pointer to the semaphore from temp's s_next equals p's blocked pointer */
		pcb_t *outted = outProcQ(&temp -> s_next -> s_procQ, p);	/* Create outted to track the outProcQ pcb */
		if(emptyProcQ(temp -> s_next -> s_procQ)) {	/* if emptyProcQ returns True */
			semd_t *emptySemd = temp -> s_next;	/* Create emptySemd to track what we will later use freeSemd on */
			temp -> s_next = emptySemd -> s_next;	/* Set s_next of temp equal to s-next of emptySemd */
			freeSemd(emptySemd);			/* run FreeSemd on emptySemd */
			return outted;
		}
		else {
			outted -> p_semAdd = NULL;		/* Otherwise set the blocked pointer to NULL */
			return outted;
		}
	}
	else {							/* Otherwise return NULL */
		return NULL;
	}
}


/* Return a pointer to the pcb that is at the head of the process queue associated with the semaphore
semAdd. Return NULL if semAdd is not found on the ASL or if the process queue associated with semAdd is empty. */
pcb_t *headBlocked(int *semAdd){
	semd_t *temp = search(semAdd);		/* Create a temp var using the search method on semADD */
	if ((temp == NULL) || (emptyProcQ(temp -> s_next -> s_procQ))) { /* If the temp var is NULL OR if emptyProcQ returns false */
		return NULL;
	}
	else {
		return headProcQ(temp -> s_next -> s_procQ);		/* otherwise run headProcq (pcb.c) */
	}
}


/* Initialize the semdFree list to contain all the elements of the array
static semd_t semdTable[MAXPROC]
This method will be only called once during data structure initialization. */
void initASL(){
	static semd_t semdTableArray[MAXPROC + 2]; /* need two more bc of dumb nodes on either end */
	semd_h = NULL; /* the head of the active semaphore list is set to NULL */
	semdFreeList_h = NULL; /* the head of the free list is set to NULL */
	int i = 0;
	/* increment through nodes of semdTableArray and insert MAXPROC nodes onto the semdFreeList */
	while(i < MAXPROC){
		freeSemd(&(semdTableArray[i]));
		i++;
	}
	/* initialize dumb head */
	/* initialize dumb tail */

}
