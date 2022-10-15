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

/*  "A System Call (SYSCALL) exception occurs when the SYSCALL assembly in- struction is executed." p.24 pandos */
void SYSCALLExceptionHandler(){
  /* go to page 9 pops and look at the status register diagram. */
  state_PTR process;
  /* the BIOSDATAPAGE holds processor states */
	process = (state_PTR) BIOSDATAPAGE;

  /* this variable "syscall" will hold the integer contained in a0 from the process that was interrupted. */
  int syscall;
  /* We need to access a0 so that we can check its value for 1-8 if request was in kernel mode */
	syscall = process -> s_a0;
  /* "The Nucleus will then perform some service on behalf of the process executing the SYSCALL instruction depending on the value found in a0." p. 25 pandos"*/
switch(syscall) {
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
A new pcb is allocated and fields are initialized:
- p_s from a1.
- p_supportStruct from a2. If no parameter is provided, this field is set to NULL.
- The process queue fields (e.g. p next) by the call to insertProcQ
- The process tree fields (e.g. p child) by the call to insertChild.
- p_time is set to zero; the new process has yet to accumulate any cpu time.
- p_semAdd is set to NULL; this pcb/process is in the “ready” state, not the “blocked” state. */
void createProcess(state_PTR oldState){
  /* "A new process, said to be a progeny of the caller, to be created,"(p. 25 pandos). */
  pcb_PTR child = allocPcb();
  /* If the new process cannot be created due to lack of resources (e.g. no more free pcb’s), an error code of -1 is placed/returned in the caller’s v0, otherwise, return the value 0 in the caller’s v0. */
  int returnStatus = -1;


}
/********************************* SYS 2 *********************************/

/********************************* SYS 3 *********************************/

/********************************* SYS 4 *********************************/

/********************************* SYS 5 *********************************/

/********************************* SYS 6 *********************************/

/********************************* SYS 7 *********************************/

/********************************* SYS 8 *********************************/
}
