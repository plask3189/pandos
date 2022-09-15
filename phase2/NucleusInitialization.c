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
  /* Initialization of the Level 2 data structures */
  initPcbs();
  initSemd();

  processCount = 0;
  softBlockCount = 0;
  /* Remember that mkEmptyProcQ is used to initialize a variable to be tail pointer to a process queue. Return a pointer to the tail of an empty process queue; i.e. NULL. */
  readyQueue = mkEmptyProcQ();
  currentProcess = NULL;

  /* Populate the Processor 0 Pass Up Vector. The Pass Up Vector is part of the BIOS Data Page, and for Processor 0, is located at 0x0FFF.F900. The Pass Up Vector is where the BIOS finds the address of the Nucleus functions to pass control to for both TLB-Refill events and all other excep- tions. */
  (passupvector_t*) nucleusPointer = (passupvector_t*) PASSUPVECTOR;

  /* Set the Nucleus TLB-Refill event handler address  */
  nuke->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
}
