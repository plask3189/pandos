/* ---------------- Nucleus Initialization ------------------------
 * Written by Kate Plas and Travis Wahl
 * For CSCI-320 Operating Systems
 *
 *
 *
 */

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"

/* Initialization of all Nucleus maintained variables */
int processCount;
int softBlockCount;
pcb_PTR readyQueue;
pcb_PTR currentProcess;
/* still need to initialize device semaphores. */

int main() {
  /* Initialization of the phase1 data structures */
  initPcbs();
  initSemd();

  processCount = 0;
  softBlockCount = 0;
  /* Remember that mkEmptyProcQ is used to initialize a variable to be tail pointer to a process queue. Return a pointer to the tail of an empty process queue; i.e. NULL. */
  readyQueue = mkEmptyProcQ();
  currentProcess = NULL;

  /* Populate the Processor 0 Pass Up Vector. The Pass Up Vector is part of the BIOS Data Page, and for Processor 0, is located at 0x0FFF.F900. The Pass Up Vector is where the BIOS finds the address of the Nucleus functions to pass control to for both TLB-Refill events and all other excep- tions. */
  (passupvector_t*) nucleusPointer = (passupvector_t*) PASSUPVECTOR;

  /* Set the Nucleus TLB-Refill event handler address. where memaddr,in types.h,has been aliased to unsigned int. Since address translation is not implemented until the Support Level, uTLB RefillHandler is a place holder function whose code is provided. [Section 3.3] This code will then be replaced when the Sup- port Level is implemented.*/
  nucleusPointer -> tlb_refll_handler = (memaddr) uTLB_RefillHandler;

  /* Set the stack pointer for the nucleus TBL-refill event handler to top of nucleus stack page. *** Side note: There's an assignment for the constant address of the nculeus nucleus stack page as 0x20001000 in const.h */
  nucleusPointer->tlb_refll_stackPtr = NUCLEUSSTACKPAGE;

  /* Set the Nucleus exception handler address to the address of your Level 3 Nucleus function that is to be the entry point for exception (and interrupt) handling [Section 3.4 */
  nucleusPointer->exception_handler = (memaddr) exceptionHandler;

  /* Set the Stack pointer for the Nucleus exception handler to the top of the Nucleus stack page: 0x2000.1000. */
  nucleusPointer->exception_stackPtr = NUCLEUSSTACKPAGE;

  /* Initialize all Nucleus maintained variables */
  processCount = 0;
  softBlcokCount = 0;
  readyQueue = mkEmptyProcQ();
  currentProc = NULL;

  /* Since the device semaphores will be used for synchronization, as opposed to mutual exclusion, they should all be initialized to zero. */
  
}
