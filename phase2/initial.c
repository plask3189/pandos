/************ INITIAL.C ************/
/*
 * Written By: Kate Plas && Travis Wahl
 * For: PandOS project CSCI 320
 *
 * Initial.c serves as the housing for the main() function that starts the PandOS Operating System.
 If possible, we remove a pcb from the pcbFree list and put it on the readyQueue.
 * Global variables are initialized here and will continue to be used throughout the Phase 2 implementation
 * of this Operating System. Methods from other files including Phase 1 ASL and PCB are also included here.
 */

#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

/* Define Global Variables */
int processCount; /* count the number of processes within the readyQueue */
int softBlockCount; /* count of processes waiting for IO */
pcb_PTR readyQueue; /* A tail pointer to a queue of pcbs that are in the “ready” state. */
pcb_PTR currentProc; /* Pointer to the pcb that is in the “running” state, i.e. the current executing process. */
int semDevices[DEVNUM]; /* There are 49 device semaphores, defined in const.h */
cpu_t startTOD; /* Hold the start of the Time Of Day Clock */
int *clockSem = &semDevices[DEVNUM-ONE]; /* Clock semaphores within the device semaphores list (49 - 1) = 48 */

/* Declaration of internal exceptionHandler and external test() and TLB Refill functions */
HIDDEN void exceptionHandler();
extern void test();
extern void uTLB_RefillHandler();


/* main() serves as the beginning of PandOS where the global variables get intitialized to be used throughout
 * the Operatring System and memory addresses are created.  The scheduler then takes over once main has finished
 */
int main(){
    devregarea_t* deviceBus = (devregarea_t*) RAMBASEADDR;
    int topOfRAM = (deviceBus->rambase + deviceBus->ramsize);
    /* * * * Initialize the pass up vector. * * * */
    /* The pass up vector is where the BIOS finds the address of the Nucleus functions to pass control to. */
    passupvector_t* nucleusFunctionAddressThatWillReceiveControl = (passupvector_t *) PASSUPVECTOR;
    /* Set the Nucleus TLB-Refill event handler address */
    nucleusFunctionAddressThatWillReceiveControl->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
    /* Set the Stack Pointer for the Nucleus TLB-Refill event handler to the top of the stack page */
    nucleusFunctionAddressThatWillReceiveControl->tlb_refll_stackPtr = NUCLEUSSTACKPAGE;
    /* Set the Nucleus exception handler address to the address of the ntry point for Exception Handling */
    nucleusFunctionAddressThatWillReceiveControl->exception_handler = (memaddr) exceptionHandler;
    /* Set the Stack Pointer for the Nucleus exception handler to the top od the Nucleus Stack page */
    nucleusFunctionAddressThatWillReceiveControl->exception_stackPtr = NUCLEUSSTACKPAGE;

    /* Initialize the PCBs and ASL */
    initPcbs();
    initASL();

    /* Ensure the current process is NULL as no process has been called yet */
    currentProc = NULL;
    /* make a new empty process queue with tail pointer called readyQueue. The initial pcb will be 
     * placed in this queue. */
    readyQueue = mkEmptyProcQ();

    /* Initialize a Process Count and Soft Block Count of 0 */
    processCount = 0;
    softBlockCount = 0;

    /* Initialize the semaphores within the device semaphores list */
    int i;
    for(i=0; i<DEVNUM; i++){
      semDevices[i] = ZERO;
     }

     /* * * * Dispatch a pcb * * * */
    pcb_PTR firstProc = allocPcb();
    if(firstProc == NULL){ /* if the pcbFree list is empty (aka allocPcb() returns null) */
        PANIC();
    }else{ /* if a pcb was successfully allocated from the pcbFree list */
        firstProc->p_s.s_pc = (memaddr) test; /* the pc is set to test for help w/ testing */
        firstProc->p_s.s_t9 = (memaddr) test;
        firstProc->p_s.s_status = (ALLOFF | IEON | IMON | TEBITON); /* Set the status register's bits. */
        firstProc->p_s.s_sp = topOfRAM;
        firstProc->p_supportStruct = NULL;
        insertProcQ(&readyQueue, firstProc); /* onto the readyQueue, insert firstProc */
        processCount++; /* Increment process count to accurately reflect how many pcbs are in the readyQueue. */
        LDIT(IOCLOCK); /* load the system-wide interval timer w/ 100 ms * time scale value. When it starts to 
                        count down, it indicates how long a pcb has been in the readyQueue. We don't want the 
                        pcb to stay in the readyQueue forever, so we need to generate an interrupt when it's 
                        time for the process to execute. */
        firstProc = NULL;
        STCK(startTOD);
        scheduler(); /* Pass off the reins to the Scheduler so that the process can execute on the CPU. */
    }
    return 0;
}


/* The General Exception Handler
 * Method used to invoke the necessary exception handler wtether tha be the
 * IO Handler, TLB Trap Handler, SYSCALL Handler, or Program Trap Handler
 */

void exceptionHandler(){
    state_PTR oldstate;
    oldstate = (state_PTR) BIOSDATAPAGE;

    /* initiailze the variable holding the Exception cause from the BIOSDATAPAGE */
    int reason = ((oldstate->s_cause & EXCODEMASK) >> SHIFT);
    if(reason == IOINTERRUPT){ /* IOInterrupt = 0 , const.h */
        IOHandler();
    }
    if(reason <= TLBEXCEPTION){ /* TLBException = 3, const.h */
    	tlbTrapHandler();
    }
    if(reason == SYSEXCEPTION){ /* SYSException = 8, const.h */
        SYSCALLHandler();
    }
    else{ /* If none of the above, call the program trap handler */
     	programTrapHandler();
     }
}
