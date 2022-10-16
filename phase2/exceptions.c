/* ------------------------------------- Exceptions.c -----------------------------------------
 * Written by Kate Plas and Travis Wahl
 * For CSCI-320 Operating Systems
 *
 *
 */

 #include "../h/types.h"
 #include "../h/const.h"
 #include "../h/pcb.h"
 #include "../h/asl.h"
 #include "../h/scheduler.h"
 #include "../h/exceptions.h"
 #include "../h/NucleusInitialization.h"


/* Get global variables */
extern pcb_PTR currentProcess;
extern pcb_PTR readyQueue;
extern int processCount;
extern int softBlockCount;
extern int deviceSemaphores[NUMBEROFDEVICES];
extern cpu_t startTimeOfDayClock;
extern int* clockSemaphore;

/*  "A SYSCALL exception occurs when the SYSCALL assembly instruction is executed.  The SYSCALL instruction is used by processes to request operating system services. */
void SYSCALLExceptionHandler(){
  /*----------- Initializing processSyscallState -------------*/
  state_PTR processSyscallState;
  /* the BIOSDATAPAGE holds processor states */
	processSyscallState = (state_PTR) BIOSDATAPAGE;
  /*----------- Initializing syscallCodeNumber1234567or8 ---------*/
  /* this variable will hold the integer contained in a0 from the process that was interrupted. */
  int syscallCodeNumber1234567or8;
  /* We need to access a0 so that we can check its value for 1-8 if request was in kernel mode */
	syscallCodeNumber1234567or8 = processSyscallState -> s_a0;
  /* "The Nucleus will then perform some service on behalf of the process executing the SYSCALL instruction depending on the value found in a0." p. 25 pandos"*/
  /*------------- Check for user mode -----------*/
/* Mode will be either 0 (kernel mode) or 1. If in user mode, passUpOrDie.   */
  /* int mode = (processSyscallState -> s_status... idk */
  }
switch(syscallCodeNumber1234567or8) {
  case createProcessCase:{
    /* create process */
    break;
  }
  case terminateProcessCase:{
    /* */
    break;
  }
  case passerenCase:{
    /*  */
    break;
  }
  case verhogenCase:{
    /* */
    break;
  }
  case waitForIODeviceCase:{
    /* */
    break;
  }
  case getCPUTimeCase:{
    /*  */
    break;
  }
  case waitForClockCase:{
    /*  */
    break;
  }
  case getSupportDataCase:{
    /* */
    break;
  }
}

/********************************* SYS 1 ********************************
A new pcb is allocated as the child of the currently running process and fields are initialized:
- p_s from a1.
- p_supportStruct from a2. If no parameter is provided, this field is set to NULL.
- The process queue fields (e.g. p next) by the call to insertProcQ
- The process tree fields (e.g. p child) by the call to insertChild.
- p_time is set to zero; the new process has yet to accumulate any cpu time.
- p_semAdd is set to NULL; this pcb/process is in the “ready” state, not the “blocked” state. */
void createProcess(state_PTR oldState){
  /* Allocate a new pcb as the child of the currently running process */
  pcb_PTR child = allocPcb();
  /* If the new process cannot be created due to lack of resources (e.g. no more free pcb’s), an error code of -1 is placed/returned in the caller’s v0. The returnStatusCode is set as a default -1 to show failure. */
  int returnStatusCode = -1;
  /* If a new process can be allocated */
  if(child != NULL) {
    processCount++;
    /* child holds the address to p_s. p_s is replaced with the parent's state */
    child -> p_s = (oldState -> s_a1);
    /* MORE TO COME */
    insertChild(currentProcess, child);
    insertProcQ(&(readyQueue), child);

    oldState->s_v0 = 0;
  } else {
      oldState->s_v0 = -1;
  }
  loadState(oldState);
}
/********************************* SYS 2 *********************************/
void terminateProcess(pcb_PTR parentProcess){
  outChild(currentProcess);
  removeProgeny(currentProcess);
  currentProcess = NULL;
  scheduler();
}

/********************************* SYS 3 *********************************/
void passeren(state_PTR oldState){
  /* The semaphore's physical address to be P'ed on is in a1 */
  int* semdAdd = oldState -> s_a1;
   /* decrement semaphore */
   (*semdAdd)--;
   /*block the process on the ASL if semaphore less than zero.*/
   if((*semdAdd) < 0){
     currentProcess -> p_s = *oldState;
     /* The process transitions from running to blocked */
     insertBlocked(semdAdd, currentProcess);
     scheduler();
   }
   loadState(oldState);
}
/********************************* SYS 4 *********************************/
void verhogen(state_PTR oldState){
  /* make a pointer to an int called semAdd. */
  int* semAdd;
  /* semAdd holds the address of an address to a process' state
  semAdd = oldState -> s_a1
}
/********************************* SYS 5 *********************************/
void waitForIO(state_PTR oldState){
}
/********************************* SYS 6 *********************************/

void getCPUTime(state_PTR oldState){
}
/********************************* SYS 7 *********************************/
void waitForClock(state_PTR oldState){
}
/********************************* SYS 8 *********************************/
void getSupport(state_PTR oldState){
}


void passUpOrDie(state_PTR oldState, int exception){
}


/************* Termination support methods **********************/
HIDDEN void nukeProgeny(pcb_t* toRemove){
    if (toRemove == NULL){
      return;
    }
    while (!(emptyChild(toRemove)))
        nukeProgeny(removeChild(toRemove));
        removeSingleProcess(toRemove);
}
HIDDEN void removeOneProcess(pcb_t* toRemove){
    processCount--;
    outProcQ(&readyQueue, toRemove);
    if (toRemove -> p_semAdd != NULL) {
        if (&(deviceSemaphores[0]) <= toRemove -> p_semAdd && toRemove->p_semAdd <= &(deviceSemaphores[48])){
            softBlockedCount--;
        }
        else{
            *(toRemove->p_semAdd)++;
        }
        /* outBlocked() removes the pcb pointed to by toRemove from the process queue associated with p’s semaphore */
        outBlocked(toRemove);
    }
    freePcb(toRemove);
}
