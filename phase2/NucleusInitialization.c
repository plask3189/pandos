/* ---------------- Nucleus Initialization ------------------------
 * Written by Kate Plas and Travis Wahl
 * For CSCI-320 Operating Systems
 *
 *
 *
 */

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"

/* Initialization of all Nucleus maintained variables */
int processCount;
int softBlockCount;
pcb_PTR readyQueue;
pcb_PTR currentProcess;
/* still need to initialize device semaphores. */

int main() {
  processCount = 0;
  softBlockCount = 0;
  /* Remember that mkEmptyProcQ is used to initialize a variable to be tail pointer to a process queue. Return a pointer to the tail of an empty process queue; i.e. NULL. */
  readyQueue = mkEmptyProcQ();
  currentProcess = NULL;
  
  /* Initialization of the Level 2 data structures */
  initPcbs();
  initSemd();

}
