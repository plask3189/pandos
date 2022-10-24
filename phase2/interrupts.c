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

void InterruptHandler() {
  /* Variable and STCK creation */
  cpu_t stopTimer;
  cpu_t timeRemaining;
  
  STCK (stopTimer);
  timeRemaining = getTIMER();
  unsigned int interruptCause = ((state_t*) BIOSDATAPAGE) -> s_cause;
  
  
  /* If DISK Interrupt? */
  if ((((state_PTR)BIOSDATAPAGE) -> s_cause & DISKINT) != 0) {
    deviceInterruptHandler(DISK);
  }
  
  /* If FLASH Interrupt? */
  if ((((state_PTR)BIOSDATAPAGE) -> s_cause & FLASHINT) != 0) {
    deviceInterruptHandler(FLASH);
  }
  
  /* If PRINTER Interrupt? */
  if ((((state_PTR)BIOSDATAPAGE) -> s_cause & PRINTERINT) != 0) {
    deviceInterruptHandler(PRINTER);
  }
  
  /* If Terminal Interrupt? */
  if ((((state_PTR)BIOSDATAPAGE) -> s_cause & TERMINT) != 0) {
    deviceInterruptHandler(TERMINAL);
  }
  

}


/* Interrupt Handler specifically for Devices */
void deviceInterruptHandler(int lineNum) {
  unsigned int bitM;
  volatile devregarea_t *deviceReg;
  int deviceNum;
  
  deviceReg = (devregarea_t *) RAMBASEADDR;
  bitM = deviceReg -> interrupt_dev[lineNum - DISK];
  
  /* Which device is causing the interrupt? */
  if ((bitM & DEVREG0) !=0) {
      deviceNum = 0;
  } else if ((bitM & DEVREG1) !=0) {
      deviceNum = 1;
  } else if ((bitM & DEVREG2) !=0) {
      deviceNum = 2;
  } else if ((bitM & DEVREG3) !=0) {
      deviceNum = 3;
  } else if ((bitM & DEVREG4) !=0) {
      deviceNum = 4;
  } else if ((bitM & DEVREG5) !=0) {
      deviceNum = 5;
  } else if ((bitM & DEVREG6) !=0) {
      deviceNum = 6;
  } else if ((bitM & DEVREG7) !=0) {
      deviceNum = 7;
  } else {
    PANIC();
  }
  
}
