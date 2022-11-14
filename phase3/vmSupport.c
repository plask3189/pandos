/************ vmSupport.c ************/
/* Written by Kate Plas && Travis Wahl
 * CSCI 320 - Operating Systems
 *
 *
 * The Virtual Memory Support file! Contains support functions to assist in
 * virtual memeory management for PandOS such as initializing the TLB, 
 * the TLB Refill Handler, and the Page Fault Handler.
 *
 */
 
#include "../h/const.h"
#include "../h/types.h"
#include "../h/initial.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "../h/libumps.h"


swap_t swapPool[POOL];
int swapperSema4;
int swap = 0;


/* Initializing TLB data structure with a swapping pool */
void initTLB() {
	int i;
	swapperSema4 = 1;
	for (i = 0, i < POOL, i++) {
		swapPool[i].sw_asid = -1;
	}
}

/* The TLB Refill Handler. Gets called by a TLB Exception when there is no TLB entry able to be found */
void uTLBRefillHandler() {

	state_PTR oldState;
	int pageNum;
	
	oldState = (state_PTR)BIOSDATAPAGE;
	
	pageNum = (((oldState -> s_entryHI) & FINDPAGENUM) >> VIRTSHIFT);
	pageNum = (pageNum % PAGEMAX);
	
	setENTRYHI((currentProc -> p_supportStruct -> sup_PgTable[pageNum]).entryHI);
	setENTRYLO((currentProc -> p_supportStruct -> sup_PgTable[pageNum]).entryLO);
	
	TLBWR();
	loadState(oldState);
	

}
