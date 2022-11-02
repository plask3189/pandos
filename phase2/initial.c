#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"
#include "/usr/include/umps3/umps/libumps.h"

int processCount; 
int softBlockCount; /* count of processes waiting for IO */
pcb_PTR readyQueue; /* A tail pointer to a queue of pcbs that are in the “ready” state. */ 
pcb_PTR currentProc; /* Pointer to the pcb that is in the “running” state, i.e. the current executing process. */
int semDevices[DEVNUM]; 
cpu_t startTOD;
int *clockSem = &semDevices[DEVNUM-ONE];

HIDDEN void exceptionHandler();
extern void test();
extern void uTLB_RefillHandler();



int main(){
    devregarea_t* deviceBus = (devregarea_t*) RAMBASEADDR;
    int topOfRAM = (deviceBus->rambase + deviceBus->ramsize); 
    /* * * * Initialize the pass up vector. * * * */
    passupvector_t* nucleusFunctionAddressThatWillReceiveControl = (passupvector_t *) PASSUPVECTOR; /* The pass up vector is where the BIOS finds the address of the Nucleus functions to pass control to. */
    nucleusFunctionAddressThatWillReceiveControl->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
    nucleusFunctionAddressThatWillReceiveControl->tlb_refll_stackPtr = NUCLEUSSTACKPAGE; 
    nucleusFunctionAddressThatWillReceiveControl->exception_handler = (memaddr) exceptionHandler;
    nucleusFunctionAddressThatWillReceiveControl->exception_stackPtr = NUCLEUSSTACKPAGE;
    
    initPcbs();
    initASL();
    currentProc = NULL;
    readyQueue = mkEmptyProcQ(); /* make a new empty process queue with tail pointer called readyQueue. The initial pcb will be placed in this queue. */
    
     processCount = 0;
    softBlockCount = 0;
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
        firstProc->p_s.s_status = (ALLOFF | IEON | IMON | TEBITON);
        firstProc->p_s.s_sp = topOfRAM;
        firstProc->p_supportStruct = NULL;
        insertProcQ(&readyQueue, firstProc); /* onto the readyQueue, insert firstProc */
        processCount++;
        LDIT(IOCLOCK); /* load the system-wide interval timer w/ 100 ms */
         firstProc = NULL; 
        STCK(startTOD); 
        scheduler();
    }
    return 0;
}


void exceptionHandler(){
    state_PTR oldstate;
    oldstate = (state_PTR) BIOSDATAPAGE;
    int reason = ((oldstate->s_cause & EXCODEMASK) >> SHIFT);
    if(reason == IOINTERRUPT){
     IOHandler();
     }
    if(reason <= TLBEXCEPTION){
    	tlbTrapHandler();
    }
    if(reason == SYSEXCEPTION){
     SYSCALLHandler();
     }
     else{
     	programTrapHandler();
     }
}


