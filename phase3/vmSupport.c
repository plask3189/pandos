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

HIDDEN void flashIO(int writeOrRead, int blockNumber, memaddr data, int flashDeviceNumber);
HIDDEN void pickFrameFromSwapPool()

swap_t swapPool[POOLSIZE];
int swapperSema4;
int swap = 0;


/* Initializing TLB data structure with a swapping pool */
void initTLB() {
	int i;
	/* The swap pool semaphore is initialized to 1 for mutual exclusion since it controls access to the swap pool data structure. */
	swapperSema4 = 1;
	for (i = 0, i < POOLSIZE, i++) {
		/* Since all valid ASID values are positive numbers, one can indicate that a frame is unoccupied with an entry of -1 in that frame’s ASID entry in the Swap Pool table. */
		swapPool[i].sw_asid = -1;
	}
}

/* The TLB Refill Handler: gets called by a TLB Exception when there is no TLB entry that can be found.  This function will locate the correct Page Table entry in some Support Level data structure (i.e. a U-proc’s Page Table), write it into the TLB, and return control (LDST) to the Current Process to restart the address translation process.
*/
void uTLBRefillHandler() {

	state_PTR oldState;
	int pageNumber;

	oldState = (state_PTR)BIOSDATAPAGE;
  /* Locate the correct page table entry in the current process's page table. */
	pageNumber = (((oldState -> s_entryHI) & TURNOFFVPNBITS) >> VIRTSHIFT);
	pageNumber = (pageNumber % PAGEMAX);

	setENTRYHI((currentProc -> p_supportStruct -> sup_PgTable[pageNumber]).entryHI);
	setENTRYLO((currentProc -> p_supportStruct -> sup_PgTable[pageNumber]).entryLO);
  /* Write EntryHi and EntryLo into the TLB using the TLBWR instruction.*/
	TLBWR();
  /* Return control to the current process to restart the address translation process.*/
	loadState(oldState);
}

/* Page faults are passed up by the Nucleus to the Support Level’s TLB exception handler – the Pager. μMPS3 defines three different TLB exceptions:
    * Page fault on a load operation: TLB-Invalid exception – TLBL
    * Page fault on a store operation: TLB-Invalid exception – TLBS
    * An attempted write to a read-only page: TLB-Modification exception- Mod */
void pager(){
  /* Obtain the pointer to the Current Process’s Support Structure: SYS8. */
  support_t* support = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
  state_PTR exceptionState = &support->sup_exceptState[PGFAULTEXCEPT];
  /* Determine the cause of the TLB exception. The saved exception state responsible for this TLB exception should be found in the Current Process’ Support Structure for TLB exceptions.*/
  int cause = exceptionState->s_cause;
  if(cause != 1){
    /* Gain mutual exclusion over the Swap Pool table. (SYS3 – P operation on the Swap Pool semaphore) */
    SYSCALL(PASSEREN, &swapSem, ZERO, ZERO);
    /*Determine the missing page number found in the saved exception state’s EntryHi.*/
    int missingPageNumber = (exceptionState->s_entryHI & TURNOFFVPNBITS);
    /* Pick a frame, i, from the Swap Pool. Which frame is selected is determined by the Pandos page replacement algorithm. */
    int frame = pickFrameFromSwapPool();
    /* Use the selected frame */
    memaddr page = (memaddr) (SWPSTARTADDR + ((frame)* PAGESIZE));
    /* If frame i is currently occupied, assume it is occupied by logical page number k belonging to process x (ASID) and that it is “dirty” (i.e. been modified): */
    if(swapPool[frame].sw_asid!=-ONE){
        interruptsSwitch(0);
        swapPool[frame].sw_pte->entryLO &= ~(VALIDON);
        TLBCLR();
        interruptsSwitch(1);
        /* Write out the used page to flash */
        flashIO(1, frame, page, swapPool[frame].sw_asid-ONE);
    }
    /* Read the contents of the Current Process’ backing store/flash device logical page p into frame i. [Section 4.5.1] */
    flashIO(0, missingPageNumber, page, support->sup_asid-ONE);
    /* Update the Swap Pool table’s entry i to reflect frame i’s new contents: page p belonging to the Current Process’s ASID, and a pointer to the Current Process’s Page Table entry for page p. */
    swapPool[frame].sw_asid = support->sup_asid;
    swapPool[frame].sw_pageNo = missingPageNumber;
    /* Update the Current Process’s Page Table entry for page p to indicate it is now present (V bit) and occupying frame i (PFN field). */
    swapPool[frame].sw_pte = &(supports[support->sup_asid-ONE].sup_privatPgTb[missingPageNumber]);
    interruptsSwitch(0);
    swapPool[frame].sw_pte->entryLO = page | DIRTYON | VALIDON;
    TLBCLR();
    interruptsSwitch(1);
    /* Release mutual exclusion over the Swap Pool table. (SYS4 – V operation on the Swap Pool semaphore) */
    SYSCALL(VERHOGEN, &swapSem, ZERO, ZERO);
    /* Return control to the Current Process to retry the instruction that caused the page fault: LDST on the saved exception state. */
    LDST(BIOSDATAPAGE);
  } else {   /* If the Cause is a TLB-Modification exception, treat this exception as a program trap [Section 4.8], otherwise continue. */
		SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
	}
}

/* Pick a frame to satisfy a page fault. */
pickFrameFromSwapPool(){
	static int i = 0;
	i = (i + 1) % POOLSIZE;
	return i;
}

/*
writeOrRead is 0 or 1. O means read; 1 means write.

*/
void flashIO(int writeOrRead, int blockNumber, memaddr data, int flashDeviceNumber){
    interruptsSwitch(0);
    devregarea_t * ram = RAMBASEADDR;
    device_t* flash = (&ram->devreg[8+flashDeviceNumber]);
		/* Write the flash device’s DATA0 field with the appropriate starting physical address of the 4k block to be read (or written); the particular frame’s starting address.*/
    flash->d_data0 = data;
		/* Write the flash device’s COMMAND field with the device block number and the command to read (or write). An operation on a flash device is started by loading the appropriate value into the COMMAND field.
		The flash device command code is 2 for READBLK and 3 for WRITEBLK. That's why we need to add 2 to the writeOrRead value which is 0 or 1.
		The flash device's COMMAND field also has the BLOCKNUMBER which is bits 8-31. For this reason, we shift right by 8. */
    flash->d_command = (blockNumber << 8) | (writeOrRead + 2);
    interruptsSwitch(1);
		/* As with all I/O operations, this should be immediately followed by a SYS5 aka WAITIO */
    int res = SYSCALL(WAITIO, FLASHINT, flashDeviceNumber, 0);
    if (res != READY){
        SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
    }
}
