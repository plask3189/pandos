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
 #include "../h/initial.h"


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
  /* Casting! Make a state_PTR hold 0x0FFFF000 (BIOSDATAPAGE)
  Now we have processSyscallState which points to the BIOSDATAPAGE  */
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

/* * * * * * * * * * * * * * * * SYS1 * * * * * * * * * * * * * * * *
A new pcb is allocated as the child of the currently running process and fields are initialized:
- p_s from a1.
- p_supportStruct from a2. If no parameter is provided, this field is set to NULL.
- The process queue fields (e.g. p next) by the call to insertProcQ
- The process tree fields (e.g. p child) by the call to insertChild.
- p_time is set to zero; the new process has yet to accumulate any cpu time.
- p_semAdd is set to NULL; this pcb/process is in the “ready” state, not the “blocked” state. */
void createProcess(state_PTR pointerToOldState){
  /* Allocate a new pcb as the child of the currently running process */
  pcb_PTR child = allocPcb();
  /* The returnStatusCode is set as a default -1 which is placed in v0 to show an error. */
  int returnStatusCode = -1;
  /* If the child pointer points to NULL, a new process cannot be allocated. */
  if(child == NULL) {
    returnStatusCode = -1;
    /* Puts -1 into the process' v0. */
    currentProcess -> p_s.s_v0 = returnStatusCode;
    loadState(pointerToOldState);
  }
  /* If a new process can be allocated */
  else {
    if(child != NULL){
      processCount++;
      /* initialize the process queue fields */
      insertProcQ(&(readyQueue), child);
      /* initialize the process tree fields */
      insertChild(currentProcess, child);
      /* copy the value that  the child's state pointer holds. This value is actually a pointer to a state. Copy this state into the oldState's a1, pointed to by pointerToOldState */
      copyState((state_PTR) (pointerToOldState -> s_a1), &(child -> p_s));
      /* a2 holds pointer to a support structure */
      if((pointerToOldState -> s_a2 == 0) || (pointerToOldState -> s_a2 == NULL)) {
        child -> p_supportStruct = (support_t *) pointerToOldState -> s_a2;
      } else {
        child -> p_supportStruct = NULL;
      }
    }
    currentProcess -> p_s.s_v0 = returnStatus;
    /* loadState is perfomed on a state saved in the BIOS. */
    loadState(pointerToOldState);
  }
}
/* * * * * * * * * * * * * * * * SYS2 * * * * * * * * * * * * * * * * * * * * *
This causes the executing process to cease to exist. All progeny of this process are terminated as well */
void terminateProcess(pcb_PTR parentProcess){
  /* -------------------- Process Tree Adjustments ----------------------------*/
  /* emptyChild() asks if the if the pcb pointed to by p has children. If there are progeny, (aka emptyChild() is false (0), remove them. */
  while(emptyChild(parentProcess) == 0){
    terminateProcess(removeChild(parentProcess));
  }
  if(parentProcess == currentProcess){
      /* outChild() makes the pcb pointed to by parentProcess an orphan (no longer the child of its parent on the process tree). */
    outChild(parentProcess);
    currentProcess = NULL;
  }
  else{
      /*-----------------------ReadyQueue Adjustments---------------------------*/
     if(parentProcess -> p_semAdd == NULL){
       /* Remove the pcb pointed to by parentProcess from the process queue whose tail pointer is pointed to by the tail pointer held as a value in ReadyQueue. (ReadyQueue is a pointer to a tail pointer of a queue of pcbs)*/
       outProcQ(&readyQueue, parentProcess);
     }
     else {
       /*-----------------------Semaphore Adjustments---------------------------*/
      /* outBlocked removes the pcb pointed to by parentProcess from the process queue associated with parentProcess’s semaphore */
      if((outBlocked(parentProcess)) != NULL){
          if(((parentProcess -> p_semAdd) >= &deviceSemaphores[0]) && ((parentProcess -> p_semAdd) <= &deviceSemaphores[DEVNUM]){
              softBlockCount--;
          } else {
              (*(parentProcess -> p_semAdd))++;
          }
        }
      }
  freePcb(parentProcess);
  processCount--;
  scheduler();
}

/* * * * * * * * * * * * * * * * SYS3 * * * * * * * * * * * * * * * */
void passeren(state_PTR pointerToOldState){
  /* The semaphore's physical address to be P'ed on is in a1 */
  int* semAdd;
  semAdd = pointerToOldState -> s_a1;
  (*semAdd)--;
   /*block the process on the ASL if semaphore less than zero.*/
   if((*semAdd) < 0){
     currentProcess -> p_s = *pointerToOldState;
     /* The process transitions from running to blocked */
     insertBlocked(semAdd, currentProcess);
     scheduler();
   }
   loadState(pointerToOldState);
}

/* * * * * * * * * * * * * * * * SYS4 * * * * * * * * * * * * * * * */
void verhogen(state_PTR pointerToOldState){
  pcb_PTR temp;
  /*a1 holds a pointer to a process state. semAdd is a pointer to a semaphore */
  int* semAdd;
  semAdd = pointerToOldState -> s_a1;
  (*semAdd)++;
  if((*semAdd) <= 0){
    temp = removeBlocked(semAdd);
    if(temp != NULL) {
      insertProcQ((&readyQueue), temp);
    }
  }
  loadState(pointerToOldState);
}
/* * * * * * * * * * * * * * * * SYS5 * * * * * * * * * * * * * * * */
void waitForIO(state_PTR pointerToOldState){
  copyState(pointerToOldState, &(currentProcess -> p_s));
}
/********************************* SYS 6 *********************************/

void getCPUTime(state_PTR pointerToOldState){
}
/********************************* SYS 7 *********************************/
void waitForClock(state_PTR pointerToOldState){
}
/* * * * * * * * * * * * * * * * SYS 8 * * * * * * * * * * * * * * * */
void getSupport(state_PTR pointerToOldState){
  /* copy the value (which is a value of the process state (aka pointer)) held in the currentProcess state and copy it into the pointer of the oldState. This is to save the currentProcess' state. */
  copyState(pointerToOldState, &(currentProcess -> p_s));
  /* assign v0 to hold support data */
  currentProcess -> p_s.s_v0 = (int) currentProcess -> p_supportStruct;
  loadState(&(currentProcess -> p_s));
}


void passUpOrDie(state_PTR pointerToOldState, int exception){
}


/* * * * * * * * * * * * * * * * Support methods * * * * * * * * * * * * * * * */


/* copyState takes two state pointers and copies the oldState's state into newState's state. This is done by:
* Copying the registers
* Copying entryHI, cause, status, and pc (which are part of the state)
 */
void copyState(state_PTR pointerToOldState, state_PTR pointertoNewState){
    int i = 0;
    pointertoNewState -> s_entryHI = pointerToOldState -> s_entryHI;
    pointertoNewState -> s_cause = pointerToOldState -> s_cause;
    pointertoNewState -> s_status = pointerToOldState -> s_status;
    pointertoNewState -> s_pc = pointerToOldState -> s_pc;
    /* There are 31 total state registers (STATEREGNUM). For each register, copy the register*/
    while(i < STATEREGNUM){
	    newStatePointer -> s_reg[i] = pointerToOldState -> s_reg[i];
      i++;
    }
}
