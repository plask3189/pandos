/* Interrupt Handlers
* Kate Plas / Travis Wahl
* CSCI-320 Operating Systems */

/* Inclusions from Phase 1 */
#include "../h/const.h"
#include "../h/types.h"
#include "../h/asl.h"
#include "../h/pcb.h"

/* Inclusions from Phase 2 */
#include "../h/initial.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "../h/interrupts.h"

/* Inclusions from Initial.c */
extern int processCount;
extern int softBlockCount;
extern pcb_PTR readyQueue;
extern pcb_PTR currentProcess;
extern int deviceSemaphores[NUMBEROFDEVICES];
extern cpu_t startTimeOfDayClock;

void InterruptHandler() {
  /* Variable and STCK creation */
  cpu_t stopTimer;
  cpu_t timeRemaining;
  
  STCK (stopTimer);
  timeRemaining = getTIMER();
  unsigned int CAUSE = ((state_PTR)BIOSDATAPAGE) -> s_cause;
  
  /* If PLT Interrupt? */
  if ((CAUSE & PLTINT) != 0) {
    pltInterruptHandler(stopTimer)
  }
  
  /* If DISK Interrupt? */
  if ((CAUSE & DISKINT) != 0) {
    deviceInterruptHandler(DISK);
  }
  
  /* If FLASH Interrupt? */
  if ((CAUSE & FLASHINT) != 0) {
    deviceInterruptHandler(FLASH);
  }
  
  /* If PRINTER Interrupt? */
  if ((CAUSE & PRINTERINT) != 0) {
    deviceInterruptHandler(PRINTER);
  }
  
  /* If Terminal Interrupt? */
  if ((CAUSE & TERMINT) != 0) {
    deviceInterruptHandler(TERMINAL);
  }
}



/* Interrupt Handler specifically for PLT */
void pltInterruptHandler(int stopTimer){
  if(currentProcess != NULL) {
    currentProcess -> p_time = currentProcess -> p_time + (stopTimer - startTimeOfDayClock)
    stateStoring((state_PTR)BIOSDATAPAGE, &(currentProcess -> p_s));
    insertProcQ(&readyQueue, currentProcess);
    scheduler();    
    }
    else {
      PANIC();
    }
}

/* Interrupt Handler specifically for Devices */
void deviceInterruptHandler(int lineNum) {
  unsigned int bitM;
  unsigned int status;
  volatile devregarea_t *dReg;
  int devNum;
  int sema4_d;
  
  dReg = (devregarea_t *) RAMBASEADDR;
  bitM = dReg -> interrupt_dev[lineNum - DISK];
  
  /* Which device is causing the interrupt? */
  if ((bitM & DEVREG0) !=0) {
      devNum = 0;
  } else if ((bitM & DEVREG1) !=0) {
      devNum = 1;
  } else if ((bitM & DEVREG2) !=0) {
      devNum = 2;
  } else if ((bitM & DEVREG3) !=0) {
      devNum = 3;
  } else if ((bitM & DEVREG4) !=0) {
      devNum = 4;
  } else if ((bitM & DEVREG5) !=0) {
      devNum = 5;
  } else if ((bitM & DEVREG6) !=0) {
      devNum = 6;
  } else if ((bitM & DEVREG7) !=0) {
      devNum = 7;
  } else {
    PANIC();
  }  
  
  /* Set device semaphore */
  sema4_d = ((lineNum - DISK) * DEVPERINT) + devNum;
  
  /* Terminal Interrupt Handler */
  if (lineNum == TERMINAL){
    status = terminalInterruptHandler(&sema4_d);
  }
  
  /*Finally, call the scheduler if nothing is running */
  if(currentProc == NULL) {
    scheduler();
  }
  
  
}


/* Creation of a helper function to handle TERMINAL type interrupts, to be used in the deviceInterruptHandler */
int terminalInterruptHandler(int sema4_d){
  unsigned int status;
  volatile devregarea_t *dReg;
  dReg = (devregarea_t *) RAMBASEADDR;
  
  /* Priority is being given to Writing over Reading in the Terminal */
  if((dReg -> devreg[(*sema4_d)].t_transm_status & SHIFT) != READY) {
    status = dReg -> devreg[(*sema4_d)].t_transm_status;
    dReg -> devreg[(*sema4_d)].t_transm_command = ACK;
  
  } else {
    status = dReg -> devreg[(*sema4_d)].t_recv_status;
    dReg -> devreg[(*sema4_d)].t_recv_command = ACK;
    (*sema4_d) = (*sema4_d) + DEVPERINT;
  }
  return (status);
}
  
/* Function to help with storing the processor state */
void stateStoring(state_t *stateObtained, state_t *stateStored) {
  int i:
  for (i = 0; i < STATEREGNUM; i++) {
    stateStored -> s_reg[i] = stateObtained -> s_reg[i];
  }
  
  /* Begin shifting the old states to the new states */
  stateStored -> s_entryHI = stateObtained -> s_entryHI;
  stateStored -> s_cause = stateObtained -> s_cause;
  stateStored -> s_status = stateObtained -> s_status;
  stateStored -> s_pc = stateObtained -> s_pc;
}
  

