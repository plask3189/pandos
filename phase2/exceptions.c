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
  state_PTR state;
	state = (state_PTR) BIOSDATAPAGE;

  int syscall;
  /* We need to access a0 to check its value for 1-8 if request was in kernel mode */
	syscall = state -> s_a0;

  /********************************* SYS 1 *********************************/

  /********************************* SYS 2 *********************************/

  /********************************* SYS 3 *********************************/

  /********************************* SYS 4 *********************************/

  /********************************* SYS 5 *********************************/

  /********************************* SYS 6 *********************************/

  /********************************* SYS 7 *********************************/

  /********************************* SYS 8 *********************************/
}
