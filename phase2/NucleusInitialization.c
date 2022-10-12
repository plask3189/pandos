/* ---------------- Nucleus Initialization ------------------------
 * Written by Kate Plas and Travis Wahl
 * For CSCI-320 Operating Systems
 *
 * The Nucleus initialization dispatches a process
 *
 */

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"

/* Initialization of all Nucleus maintained variables */
int processCount;
int softBlockCount;
pcb_PTR readyQueue; /* readyQueue is a pointer to a tail pointer of a queue of pcbs */
pcb_PTR currentProcess;
/* initialization of an array of ints which are semaphores with a size of how many devices there are. */
int deviceSemaphores[NUMBEROFDEVICES];
/* the TOD clock counts up and is CPU time. */
cpu_t startTimeOfDayClock;
/* allocate the first process from the pcbFree list: Return NULL if the pcbFree list is empty. Otherwise, remove an element from the pcbFree list, provide initial values for ALL of the pcb's ﬁelds (i.e. NULL and/or 0) and then return a pointer to the removed element.  */
pcb_PTR initialProcess = allocPcb();

int main() {
  /* Initialization of the phase1 data structures */
  initPcbs();
  initSemd();
  processCount = 0;
  softBlockCount = 0;
  /* RAMTOP is calculated by adding the RAM base physical address (fixed at 0x2000.0000) to the installed RAM size. */
  /* RAMBASEADDR is the RAM Base Physical Address Bus Register which is located at 0x1000.0000. It is set to 0x2000.0000. Create a pointer called ramPointer that points to the address held by RAMBASEADDR which is of TYPE devregarea_t*/
  devregarea_t* ramPointer = (devregarea_t*) RAMBASEADDR;
  unsigned int RAMTOP;
  /* add the RAM Base Physical Address Bus Register to the Installed RAM Size Bus Register. rambase and ramsize are in types.h of a bus register area structure. */
  RAMTOP = (ramPointer -> rambase) + (ramPointer -> ramsize);
  /* Remember that mkEmptyProcQ is used to initialize a variable to be tail pointer to a process queue. Return a pointer to the tail of an empty process queue; i.e. NULL. */
  readyQueue = mkEmptyProcQ();
  currentProcess = NULL;
  /* Populate the Processor 0 Pass Up Vector. The Pass Up Vector is part of the BIOS Data Page, and for Processor 0, is located at 0x0FFF.F900. The Pass Up Vector is where the BIOS finds the address of the Nucleus functions to pass control to for both TLB-Refill events and all other exceptions. */
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
  currentProcess = NULL;

  /* Since the device semaphores will be used for synchronization, as opposed to mutual exclusion, they should all be initialized to zero. */
  int i;
  for(i = 0; i < NUMBEROFDEVICES; i++){
    deviceSemaphores[i] = 0;
  }
  /***************************** Initiate a single process ***************************************/
  if(initialProcess != NULL){  /* If there a process that was removed from the pcbFree list*/
    pcb_PTR initialProcess = allocPcb();
    /* Test is a supplied function/process that will help you debug your Nucleus.PC gets the address of a function. "For rather technical reasons, whenever one assigns a value to the PC one must also assign the same value to the general purpose register t9. (a.k.a. s t9 as defined in types.h." p.21 pandos" "PC set to the address of test" */
    initialProcess -> p_s.s_pc = initialProcess->p_s.s_t9 = (memaddr) test;
    /* In const.h, STCK(T) takes an unsigned integer as its input parameter and populates it with the value of the low-order word of the TOD clock divided by the Time Scale" p.21 principles of operations */
    STCK(startTimeOfDayClock);
    /* LDIT(T) which loads the Interval Timer with the value T (unsigned int) multiplied by the Time Scale value. */
    LDIT(INTERVALTIMER);
    /* Set status of process */
    initialProcess -> p_s.s_status = ALLOFF | IEON |IMON | TEON;
    /* The SP is set to RAMTOP (i.e. use the last RAM frame for its stack) */
    initialProcess -> p_s.s_sp = RAMTOP;
    initialProcess -> p_time = 0;
    initialProcess -> p_semAdd = NULL;
    initialProcess -> p_supportStruct = NULL;
    /* insert the current process into the ready queue */
    insertProcQ((&readyQueue), initialProcess);
    processCount++;
    initialProcess = NULL;
    /* main() calls the Scheduler, and its task is done! */
    scheduler();
  }
  else { /* If the initialProcess == NULL then there is no pcb to remove from the pcbFreeList, so PANIC */
    PANIC();
  }
 return 0;
}

/* The only reason "for re-entering the Nucleus is through an exception which includes device interrupts." p.22 pandos */
void exceptionHander(){
  state_PTR oldState;
  oldState = (state_PTR) BIOSDATAPAGE;
  /* The cause of the exception is in the cause register. Although, to make the register address an integer, we need to do some adjustments: The >> is a Binary Right Shift Operator. The left operands value is moved right by the number of bits specified by the right operand. */
  int cause = ((oldState -> s_cause) >> 2);
  /* More to come */
  if(cause == 0){
    /* For exception code 0 (Interrupts), processing should be passed along to your Nucleus’s device interrupt handler. */
    IOHANDLER(); /* WILL DEFINE LATER !!!!!!!!!! */
  }
  if(cause > 8) {
    /* do stuff */
  }
  if(cause == 8) {
    /* For exception code 8 (SYSCALL), processing should be passed along to your Nucleus’s SYSCALL exception handler.  */
    SYSCALLHandler(); /* WILL DEFINE LATER WHEN WE WRITE THIS IN EXCEPTIONS.C !!!!!!!!!! */
  }
}
