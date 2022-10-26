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
 #include "../h/interrupts.h"
 #include "../h/libumps.h"
 

extern pcb_PTR currentProcess;
extern pcb_PTR readyQueue;
extern int processCount;
extern int softBlockCount;
extern int deviceSemaphores[NUMBEROFDEVICES];
extern cpu_t startTimeOfDayClock;
extern int* clockSemaphore;
extern void loadState(state_PTR process);

void createProcess(state_PTR currentProcess1);
void terminateProcess(pcb_PTR parentProcess1);
void passeren(state_PTR currentProcess1);
void verhogen(state_PTR currentProcess1);
void waitForIO(state_PTR currentProcess1);
void getCPUTime(state_PTR currentProcess1);
void waitForClock(state_PTR currentProcess1);
void getSupport(state_PTR currentProcess1);

void passUpOrDie(state_PTR currentProcess1, int exception);
void stateStoring(state_PTR pointerToOldState, state_PTR pointertoNewState);

/*  "A SYSCALL exception occurs when the SYSCALL assembly instruction is executed.  The SYSCALL instruction is used by processes to request operating system services. */
void SYSCALLExceptionHandler(){
  int toCheckIfInUserMode;
  /*----------- Initializing processSyscallState -------------*/
  state_PTR processSyscallState;
  /* Casting! Make a state_PTR hold 0x0FFFF000 (BIOSDATAPAGE)
  Now we have processSyscallState which points to the BIOSDATAPAGE  */
	processSyscallState = (state_PTR) BIOSDATAPAGE;
  state_PTR t9processSyscallState = processSyscallState -> s_t9;
  state_PTR pcOfProcessSyscallState = processSyscallState -> s_pc;
  t9processSyscallState = (processSyscallState -> s_pc + FOURTOINCREMENTTHEPC);
  pcOfProcessSyscallState = (processSyscallState -> s_pc + FOURTOINCREMENTTHEPC);
  /*----------- Initializing syscallCodeNumber1234567or8 ---------*/
  /* this variable will hold the integer contained in a0 from the process that was interrupted. */
  int syscallCodeNumber1234567or8;
  /* We need to access a0 so that we can check its value for 1-8 if request was in kernel mode */
	syscallCodeNumber1234567or8 = processSyscallState -> s_a0;
  /* "The Nucleus will then perform some service on behalf of the process executing the SYSCALL instruction depending on the value found in a0." p. 25 pandos"*/
  /*------------- Check for user mode -----------*/
/* Mode will be either 0 (kernel mode) or 1. If in user mode, passUpOrDie.   */
  /* int mode = (processSyscallState -> s_status... idk */
  toCheckIfInUserMode = (processSyscallState -> s_status & USERMODEOFF);
  if(toCheckIfInUserMode != ALLOFF){
    passUpOrDie(toCheckIfInUserMode, GENERALEXCEPT);
  }
switch(syscallCodeNumber1234567or8) {
  case CREATEPROCESS:{
    createProcess(processSyscallState);
    break;
  }
  case TERMINATEPROCESS:{
    if(currentProcess != NULL){
            terminateProcess(currentProcess);
        }
    break;
  }
  case PASSEREN:{
    passeren(processSyscallState);
    break;
  }
  case VERHOGEN:{
    verhogen(processSyscallState);
    break;
  }
  case WAITFORIO:{
    waitForIO(processSyscallState);
    break;
  }
  case GETCPUTIME:{
    getCPUTime(processSyscallState);
    break;
  }
  case WAITFORCLOCK:{
    waitForClock(processSyscallState);
    break;
  }
  case GETSUPPORTPOINTER:{
    getSupport(processSyscallState);
    break;
  }
  default:{
     passUpOrDie(processSyscallState, GENERALEXCEPT);
     break;
   }
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
        child -> p_supportStruct = (support_t*) pointerToOldState -> s_a2;
      } else {
        child -> p_supportStruct = NULL;
      }
    }
    currentProcess -> p_s.s_v0 = returnStatusCode;
    /* loadState is perfomed on a state saved in the BIOS. */
    loadState(pointerToOldState);
  }
}
/* * * * * * * * * * * * * * * * SYS2 * * * * * * * * * * * * * * * * * * * * *
This causes the executing process to cease to exist. All progeny of this process are terminated as well. "Processes (i.e. pcb’s) can’t hide. A pcb is either the Current Process (“run- ning”), sitting on the Ready Queue (“ready”), blocked on a device semaphore (“blocked”), or blocked on a non-device semaphore (“blocked”)." p.39 pandos */
void terminateProcess(pcb_PTR parentProcess){
  processCount--;
  pcb_PTR processToTerminate = parentProcess;
  /* -------------------- Process Tree Adjustments ----------------------------*/
  /* emptyChild() asks if the pcb pointed to by p has children. If there are progeny, (aka emptyChild() is false (0), remove them. */
  while(emptyChild(processToTerminate) == 0){
    terminateProcess(removeChild(processToTerminate));
  }
  /* if the current process is the one we want to remove. */
  if(processToTerminate == currentProcess){
      /* outChild() makes the pcb pointed to by parentProcess an orphan (no longer the child of its parent on the process tree). */
    outChild(processToTerminate);
    currentProcess = NULL;
  }
  else{
      /*-----------------------ReadyQueue Adjustments---------------------------*/
     if(processToTerminate -> p_semAdd == NULL){
       /* Remove the pcb pointed to by processToTerminate from the process queue whose tail pointer is pointed to by the tail pointer held as a value in ReadyQueue. (ReadyQueue is a pointer to a tail pointer of a queue of pcbs. We need it to identify the readyQueue)*/
       outProcQ(&readyQueue, processToTerminate);
     }
     else {
       /*---------------------- Process Blocked on Semaphore ----------------------*/
      /* outBlocked removes the pcb pointed to by processToTerminate from the process queue associated with processToTerminate's semaphore. If pcb pointed to by processToTerminate is in the process queue associated with processToTerminate’s semaphore this is NOT NULL;*/
      if((outBlocked(processToTerminate)) != NULL){
          if(((processToTerminate -> p_semAdd) >= &deviceSemaphores[0]) && ((processToTerminate -> p_semAdd) <= &deviceSemaphores[NUMBEROFDEVICES])){
              softBlockCount--;
              /* "If a terminated process is blocked on a device semaphore, the semaphore should NOT be adjusted. When the interrupt eventually occurs the semaphore will get V’ed (and hence incremented) by the interrupt handler." p.39 pandos */
          } else {
            /* if pcb pointed to by processToTerminate is not in the process queue associated with processToTerminate’s semaphore (OutBlocked(processToTerminate)) == NULL)). Increment semaphore*/
              *(processToTerminate -> p_semAdd)++;
          }
        }
      }
  freePcb(processToTerminate);
  scheduler();
 }
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
/* When an I/O operation is initiated, the initiating process is blocked until the IO completes. SYS5 transitions the current process from running to blocked. We get the semaphore for the I/O device indicated by a1, a2, and a3. Then do a P operation. */
void waitForIO(state_PTR pointerToOldState){
  copyState(pointerToOldState, &(currentProcess -> p_s));
  /* a1 holds the interrupt line number ([3. . .7]) */
  int lineNumberForDevice = pointerToOldState -> s_a1;
  /* Need to subract 3 because "when bit i in word j is set to one then device i attached to interrupt line j + 3 has a pending interrupt." p. 29 pops.  */
  int wordNumberForDevice = (lineNumberForDevice - 3);
  /* a2 holds the device number ([0. . .7]) */
  int deviceNumber = pointerToOldState -> s_a2;
  /* a3 holds true (1) or false (0) to tell if waiting for a terminal read operation */
  int areWeWaitingForTerminalReadOperation = pointerToOldState -> s_a3;
  /* calculate the semaphore index that the device has.
      DEVPERINT is in const.h and represents the # of devices per interrupt line, which is 8. */
  int deviceSemaphoreIndex = ((wordNumberForDevice + areWeWaitingForTerminalReadOperation) * DEVPERINT + deviceNumber);
  deviceSemaphores[deviceSemaphoreIndex]--;
  softBlockCount++;
  insertBlocked(&(deviceSemaphores[deviceSemaphoreIndex]), currentProcess);
  currentProcess = NULL;
  scheduler();
}
/* * * * * * * * * * * * * * * * * * SYS 6 * * * * * * * * * * * * * * * * * */
/* This service requests that the accumulated processor time (in microseconds) used by the requesting process be placed/returned in the caller’s v0.  */
void getCPUTime(state_PTR pointerToOldState){
  copyState(pointerToOldState, &(currentProcess -> p_s));
  cpu_t whatTimeIsIt;
  /*  STCK(whatTimeIsIt) does this: whatTimeIsIt = TODLOADD / TIMESCALEADDR */
  STCK(whatTimeIsIt);
  /* Remember that the TOD clock increments every processor cycle */
  whatTimeIsIt = whatTimeIsIt - startTimeOfDayClock;
  /* add time to current process time */
  (currentProcess -> p_time) = (currentProcess -> p_time) + whatTimeIsIt;
  currentProcess -> p_s.s_v0 = currentProcess -> p_time;
  loadState(&currentProcess -> p_s);
}
/* * * * * * * * * * * * * * * * * * SYS 7 * * * * * * * * * * * * * * * * * */
void waitForClock(state_PTR pointerToOldState){
  copyState(pointerToOldState, &(currentProcess -> p_s));
  (*clockSemaphore)--;
  if(!((*clockSemaphore)>= 0)){
    insertBlocked(clockSemaphore, currentProcess);
    currentProcess = NULL;
    softBlockCount++;
    scheduler();
  }
}
/* * * * * * * * * * * * * *  * * * * SYS 8 * * * * * * * * * * * * * * * * * */
void getSupport(state_PTR pointerToOldState){
  copyState(pointerToOldState, &(currentProcess -> p_s));
  currentProcess -> p_s.s_v0 = (int) currentProcess -> p_supportStruct;
  loadState(&(currentProcess -> p_s));
}

/* * * * * * * * * * * * * *  * * * * Pass up or Die * * * * * * * * * * * * * * * * * */
void passUpOrDie(state_PTR pointerToOldState, int exception){
  /* if the currentProcess has a support structure */
  if(currentProcess -> p_supportStruct != NULL){
    stateStoring(pointerToOldState, &(currentProcess -> p_supportStruct -> sup_exceptState[exception]));
    LDCXT(currentProcess->p_supportStruct->sup_exceptContext[exception].c_stackPtr, currentProcess->p_supportStruct->sup_exceptContext[exception].c_status, currentProcess->p_supportStruct->sup_exceptContext[exception].c_pc);
  }
  else {
    terminateProcess(currentProcess);
    currentProcess = NULL;
    scheduler();
  }
}


/* * * * * * * * * * * * * * * * Support methods * * * * * * * * * * * * * * * */
void otherException(int cause){
  if(cause >=4 ){
    passUpOrDie((state_PTR) BIOSDATAPAGE, GENERALEXCEPT);
  } else {
    passUpOrDie((state_PTR) BIOSDATAPAGE, PGFAULTEXCEPT);
  }
}
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
	    pointertoNewState -> s_reg[i] = pointerToOldState -> s_reg[i];
      i++;
    }
}
