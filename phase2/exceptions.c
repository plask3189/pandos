/* EXCEPTIONS.C */
/* The Exception Handler for PandOS and its included helper functions
 * Written by Kate Plas && Travis Wahl
 * For CSCI-320 Operating Systems
 *
 * Exceptions.c handles the SYSCALLs of the operating system.  If a SYSCALL is being requested
 * by a program in user mode or if it is not a SYSCALL of 1 thru 8, PassUpOrDie gets invoked. Otherwise, 
 * a syscall exception can take place and handled properly below.  Helper functions are also defined here
 * such as handling a Program trap and a TLB trap. */

#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "/usr/include/umps3/umps/libumps.h"

extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int processCount;
extern int softBlockCount;
extern int semDevices[DEVNUM];
extern cpu_t startTOD;
extern int* clockSem;
extern void loadState(state_PTR ps);

void tlbTrapHandler();
void programTrapHandler();
void createProc(state_PTR curr);
void terminateProc(pcb_PTR prnt);
void passeren(state_PTR curr);
void ver(state_PTR curr);
void waitForIO(state_PTR curr);
void getCPUTime(state_PTR curr);
void waitForClock(state_PTR curr);
void getSupport(state_PTR curr);

void passUpOrDie(state_PTR curr, int exception);
void otherExceptions();

void stateCopy(state_PTR oldState, state_PTR newState);
HIDDEN void blocking(int* processToBlock);

void SYSCALLHandler(){
    /* Save the processor state. */
    state_PTR ps = (state_PTR) BIOSDATAPAGE;
    ps->s_pc = ps->s_pc+PCINC;
    ps->s_t9 = ps->s_pc+PCINC;

    /* Check if in usermode. */
    int userMode = (ps->s_status & UMOFF);
    if(userMode != ALLOFF){
	    passUpOrDie(ps, GENERALEXCEPT);
    }

    int syscallNumber = (ps->s_a0);
    switch (syscallNumber)
    {
    case CREATEPROCESS:{ /* if syscallNumber == 1 */
        createProc(ps);
        break;}

    case TERMINATEPROCESS:{ /* if syscallNumber == 2 */
        if(currentProc != NULL){
            terminateProc(currentProc);
        }
        scheduler();
        break;}

    case PASSEREN:{ /* if syscallNumber == 3 */
        passeren(ps);
        break;}

    case VERHOGEN:{ /* if syscallNumber == 4 */
        ver(ps);
        break;}

    case WAITIO:{ /* if syscallNumber == 5 */
        waitForIO(ps);
        break;}

    case GETCPUTIME:{ /* if syscallNumber == 6 */
        getCPUTime(ps);
        break;}

    case WAITCLOCK:{ /* if syscallNumber == 7 */
        waitForClock(ps);
        break;}

    case GETSUPPORTPTR:{ /* if syscallNumber == 8 */
        getSupport(ps);
        break;}

    default:{
        passUpOrDie(ps, GENERALEXCEPT);
        break;}
    }
}

/* "The sys1 service is requested by the calling process by placing the value 1 in a0, a pointer to a processor state 
in a1, a pointer to a support struct in a2, and then executing the syscall instruction." p.25 pandos*/
void createProc(state_PTR oldState){
    pcb_PTR child = allocPcb(); /* Allocate a pcb from the pcbFree list. */
    int returnStatus = -1;
    if(child != NULL){ /* If we can allocate a fresh pcb from the pcbFree list. */
        insertChild(currentProc, child); /* Make "child" a child of currentProc */
        insertProcQ(&readyQueue, child); /* Add the child to the readyQueue. */
        /* copy oldState's state registers into child's state registers. a1 holds the address (is a pointer to) 
	a process state pointer. This process state pointer holds the state registers in CP0. We copy the pointer 
	to a process state pointer into the address of child's process state pointer.  Since a1 points to p_s, 
	we've changed what child's a1 points to since child's p_s now points to a copy of oldState's p_s. */
	stateCopy((state_PTR) (oldState->s_a1), &(child->p_s)); 
        if(oldState->s_a2 != 0 || oldState->s_a2 != NULL){
            child->p_supportStruct = (support_t *) oldState->s_a2;
        } else {
            child->p_supportStruct = NULL;
        }
        processCount++; /* Since we added the child to the readyQueue, increment processCount. */
        returnStatus = 0;
    }
    currentProc->p_s.s_v0 = returnStatus;
    loadState(oldState); /* continue executing */
}

/* Remove the parent process and its children. */
void terminateProc(pcb_PTR parentProc){
    while(emptyChild(parentProc)==TRUE){ /* While the pcb pointed to by parentProc has children. */
      pcb_PTR kidToTerminate = removeChild(parentProc);/* get the pointer to kid to removed which is 
      							its parent pcb pointed to by parentProc.*/
	    terminateProc(kidToTerminate);
    }
    if(currentProc == parentProc){
	    outChild(parentProc); /* make the pcb pointed to by parentProc no longer the child of its parent */
    }
    /* if the process to remove is in the readyQueue (aka the pointer to the semaphore on which the process is 
    blocked is NULL b/c it's not there. )*/
    if(parentProc->p_semAdd == NULL){
	outProcQ(&readyQueue, parentProc);
    }
   else {
        pcb_PTR removed = outBlocked(parentProc);
        if(removed != NULL){
        	int* semdAdd = removed->p_semAdd;
            if(semdAdd >= &semDevices[ZERO] && semdAdd <= &semDevices[DEVNUM]){
                softBlockCount--;
            } else {
                (*semdAdd)++;
            }
        }
    }
    freePcb(parentProc); /* push the parentProc onto the pcbFree list. */
    processCount--;
    /* we call the scheduler in the switch case statements */
}
/* the wait() operation: When a process is waiting for IO and we want another process to execute while we're waiting.   */
void passeren(state_PTR oldState){
     int* semdAdd = (int*) oldState->s_a1;
    (*semdAdd)--; /* decrement the number of processes waiting on this semaphore to indicate the increased magnitude 
    		  of process waiting on the semaphore.*/
    if((*semdAdd)<0){ /* if semdAdd is negative then there are process waiting on the semaphore. */
	blocker(semdAdd);
    }
    loadState(oldState);
}

/* the signal() operation */
void ver(state_PTR oldState){
    int* semdAdd = (int*)oldState->s_a1;
    (*semdAdd)++;
    if((*semdAdd)<=0){
        pcb_PTR temp = removeBlocked(semdAdd);
        if(temp != NULL) {
            insertProcQ(&readyQueue, temp);
        }
    }
    loadState(oldState);
}

/* the initiating process is blocked until the IO completes. */
void waitForIO(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    int lineNumber = oldState->s_a1;
    int deviceNumber = oldState->s_a2;
    int waitterm = oldState->s_a3;
    int deviceSemaphore = ((lineNumber - 3 + waitterm) * DEVPERINT + deviceNumber);
    semDevices[deviceSemaphore]--;
    softBlockCount++;
    blocker(&(semDevices[DEVNUM]));
}

/* requests the accumulated processor time used by the requesting process. places it in the caller's v0. */
void getCPUTime(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    cpu_t time;
    STCK(time);
    time = time - startTOD;
    currentProc->p_time = currentProc->p_time + time;
    currentProc->p_s.s_v0 = currentProc->p_time;
    loadState(&currentProc->p_s);
}


void waitForClock(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    (*clockSem)--;
    if((*clockSem)<0){
        softBlockCount++;
        blocker(&(semDevices[DEVNUM-1]));
    }
}

/* requests a pointer to the current process' support structure. */
void getSupport(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    currentProc->p_s.s_v0 =(int) currentProc->p_supportStruct;
    loadState(&currentProc->p_s);
}

/* passes up process */
void passUpOrDie(state_PTR oldState, int exception){
	support_t *supportStruct = currentProc->p_supportStruct;
    if((supportStruct == NULL) || supportStruct == 0) {
        terminateProc(currentProc);
        scheduler();
    }else{
       stateCopy(oldState, &(currentProc->p_supportStruct->sup_exceptState[exception]));
        unsigned int stackPtrToLoad = currentProc->p_supportStruct->sup_exceptContext[exception].c_stackPtr;
       unsigned int statusToLoad = currentProc->p_supportStruct->sup_exceptContext[exception].c_status;
       unsigned int pcToLoad = currentProc->p_supportStruct->sup_exceptContext[exception].c_pc;
        /* load context */
        LDCXT(stackPtrToLoad, statusToLoad, pcToLoad);
    }
}

/* Copy the oldState into the newState. A process' state includes its general purpose registers and some of the CPO registers.  */
void stateCopy(state_PTR oldState, state_PTR newState){
    /* * * * The General Purpose Registers * * * */
    int i;
    for(i=0; i<STATEREGNUM; i++){
	    newState->s_reg[i] = oldState->s_reg[i];
    }
    /* * * * The CP0 Registers * * * */
    newState->s_entryHI = oldState->s_entryHI;
    newState->s_status = oldState->s_status;
    newState->s_cause = oldState->s_cause;
    newState->s_pc = oldState->s_pc;
}


void tlbTrapHandler(){
	passUpOrDie((state_PTR) BIOSDATAPAGE,  PGFAULTEXCEPT);
}

void programTrapHandler(){
passUpOrDie((state_PTR) BIOSDATAPAGE,  GENERALEXCEPT);
}

/* store time that the proc took to run, insert it into the asl, get the next proc by calling the scheduler. */
HIDDEN void blocker(int* processToBlock){
	cpu_t stopTOD;
	STCK(stopTOD);
	currentProc -> p_time = currentProc -> p_time + (stopTOD-startTOD);
	insertBlocked(processToBlock, currentProc);
	currentProc = NULL;
	scheduler();
}
