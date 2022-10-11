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

void SYSCALLHandler(){
  /* go to page 9 pops and look at the status register diagram. */
  state_PTR pointerToHoldTheStateOfARudelyInterruptedProcess;
  /* the BIOSDATAPAGE holds processor states */
	pointerToHoldTheStateOfARudelyInterruptedProcess = (state_PTR) BIOSDATAPAGE;

  /* this variable "syscall" will hold the integer contained in a0 from the process that was interrupted. */
  int syscall;
  /* We need to access a0 so that we can check its value for 1-8 if request was in kernel mode */
	syscall = pointerToHoldTheStateOfARudelyInterruptedProcess -> s_a0;

}


/********************************* SYS 1 *********************************/

/********************************* SYS 2 *********************************/

/********************************* SYS 3 *********************************/

/********************************* SYS 4 *********************************/

/********************************* SYS 5 *********************************/

/********************************* SYS 6 *********************************/

/********************************* SYS 7 *********************************/

/********************************* SYS 8 *********************************/
