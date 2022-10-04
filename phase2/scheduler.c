/* ---------------- Nucleus Initialization ------------------------
 * Written by Kate Plas and Travis Wahl
 * For CSCI-320 Operating Systems
 *
 * The scheduler ensures that every ready process has an opportunity to execute. It is a preemptive round robin algorithm with a time slice value of 5 miliseconds
 *
 */

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/nucleusInitialization.h"
#include "../h/scheduler.h"


void scheduler() {
  cpu_t elapsedTime;
  /* currentProcess is a pointer to a pcb and is definined in nucleusPointer.h */
  if(currentProcess == NULL){ /* if the currentProcess points to NULL, there is nothing executing so get a pcb from the readyQueue */
    pcb_PTR nextProcess = removeProcQ(&readyQueue);
  } else { /* If currentProcess is not null, there is a process running, so check its time */
    STCK(elapsedTime); /* get how long the process has been executing */
    /* Adjust the CPU time used by the process by adding how much time has elapsed since the clock started added to the CPU time used by the processor so far (p_time) */
    currentProcess -> p_time = currentProcess -> p_time + (elapsedTime - startTOD);
    LDIT(INTERVALTIMER);
    pcb_PTR nextProcess = removeProcQ(&readyQueue);
  }

  if(nextProcess != NULL){
    currentProcess = nextl
    STCK(startTimeOfDayClock);
    /* more stuff */
  } else {
    /* processCount is initialized in nucleusInitialization.c */
    if(processCount > 0) {
      /* do stuff */
    }
  }
}
