/************ initProc.c ************/
/* Written by Kate Plas && Travis Wahl
 * CSCI 320 - Operating Systems
 *
 *
 * Instantiation of a single proicess whose PC is set to TEST. TEST is
 * now used as an instatiator process that will initialize the Level 4 / Phase 3 data
 * structures which includes trhe Swap Pool table and the Swap Pool semaphore as well as each
 * sharable peripheral I/O device getting its own semaphore defined for it while the Terminal
 * will get two semaphores (read and write). TEST will also initialize and launch between 1-8 U-procs
 * via SYS1 syscalls and either Terminate (SYS2) after all processes conclude (HALT) or perform
 * a P (SYS3) operation on a priovate semaphore intialized to 0, which wil result in PANIC
 * after all processes conclude.
 */
 
 
#include "../h/const.h"
#include "../h/types.h"
#include "../h/initial.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "../h/libumps.h"
 
 int devSem[DEVCOUNT + DEVPERINT]; /* The device semaphore list */
 int masterSema4; /* The control sema4 */
 
 
 void test(){
 	for (int i = 0; i < (DEVCOUNT + DEVPERINT); i++) {
 		devSem[i] = 1;
 	}
 	
 	/* Initialize the TLB from vmSupport.c */
 	initTLB();
 	
 	/* Initialize the User Processes, defined below */
 	initUserProcesses();
 	
 	/* Initialize the Control Sema4 */
 	masterSema4 = 0;
 	
 	/* Blocking the control process */
 	for (int i = 0, i < USERPROCMAX; i++) {
 		SYSCALL(PASSEREN, (int) &masterSema4, 0, 0);
 	}
 	
 	/* Closing Time */
 	SYSCALL(TERMINATEPROCESS, 0, 0, 0);
 }
 
 /* Initializing the User Processes */
 void initUserProcesses() {
 	int id;
 	int create;
 	support_t supp[USERPROCMAX + 1];
 	state_t procState;
 	
 	/* Initialize the user processes! Maximum user processes is set to 8 in const.h */
 	for(id = 1; id <= USERPROCMAX; id++) {
 		procState.s_entryHI = id << IDSHIFT;
 		procState.s_sp = (int) USTACK;
 		procState.s_pc = procState.s_t9 = (memaddr) USTART;
 		procState.s_status = ALLOFF | IEON | IMON | KUON | TEBITON;
 		
 		supp[id].sup_asid = id;
 		
 		supp[id].sup_exceptContext[GENERALEXCEPT].c_status = ALLOFF | IEON | IMON | TEBITON;
 		supp[id].sup_exceptContext[PGFAULTEXCEPT].c_status = ALLOFF | IEON | IMON | TEBITON'
 		
 		supp[id].sup_exceptContext[GENERALEXCEPT].c_stackPTR = (int) &(supp[id].sup_stackGEN[499];
 		supp[id].sup_exceptContext[PGFAULTEXCEPT].c_stackPTR = (int) &(supp[id].sup_stackPG[499];
 		
 		supp[id].sup_exceptContext[GENERALEXCEPT].c_pc = (memaddr) SysSupport;
 		supp[id].sup_exceptContext[PGFAULTEXCEPT].c+pc = (memaddr) pager;
 		
 		/* Time to make a page table for the process! */
 		for(int i = 0; i < PAGEMAX; i++) {
 			supp[id].sup_PgTable[i].entryHI = ((0x800000 + i) << VIRTSHIFT) | (id << IDSHIFT);
 			supp[id].sup_PgTable[i].entryLO = ALLOFF | DON;
 		}
 		
 		supp[id].sup_PgTable[PAGEMAX - 1].entryHI = (0xBFFFF << VIRTSHIFT) | (id << IDSHIFT);
 		/*SYSCALL 1 */
 		create = SYSCALL(CREATEPROCESS, (int) &procState, (int) &(supp[id]), 0);
 		
 		/* If the SYS 1 did not work, call SYS 2 */
 		if(create != READY) {
 			SYSCALL(TERMINATEPROCESS, 0, 0, 0);
 		}
 	}
 }
