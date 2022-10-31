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
#include "../h/initial.h"
#include "../h/scheduler.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/libumps.h"

extern cpu_t startTimeOfDayClock;
extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProcess;
extern pcb_PTR readyQueue;

void scheduler() {
  cpu_t elapsedTime;
  pcb_PTR nextProcess;

  /* currentProcess is a pointer to a pcb */
  if(currentProcess == NULL){ /* if the currentProcess points to NULL, there is nothing executing so get a pcb from the readyQueue */
    nextProcess = removeProcQ(&readyQueue);
  } else { /* If currentProcess is not null, there is a process running, so check its time */
    STCK(elapsedTime); /* get how long the process has been executing */
    /* Adjust the CPU time used by the process by adding how much time has elapsed since the clock started added to the CPU time used by the processor so far (p_time) */
    currentProcess -> p_time = currentProcess -> p_time + (elapsedTime - startTimeOfDayClock);
    LDIT(INTERVALTIMER);
    pcb_PTR nextProcess = removeProcQ(&readyQueue);
  }
  if(nextProcess != NULL){
    currentProcess = nextProcess;
    timerPrep(currentProcess, QUANTUM);

    /* more stuff */
  } else { /* if the Ready Queue is empty */
    /* processCount and softBlockCount are initialized in nucleusInitialization.c */
    /* "If the Process Count > 0 and the Soft-block Count > 0 enter a Wait State." -p. 23 pandos */
    if((processCount > 0) && (softBlockCount > 0)){
      /* " The Scheduler must first set the Status register to enable interrupts and disable the processor Local Timer (also through the Status register)*/
      /* "Interrupts enabled via the STATUS register(setSTATUS)[Section 7.1-pops]" */
      currentProcess = NULL;
      setTIMER(MAXINT);
      
      unsigned int cool;
      cool = (ALLOFF | IECON | TEBITON | IMON);
      setSTATUS(cool);
      WAIT();
    }
    if((processCount > 0) && (softBlockCount == 0)){ /* this is a deadlock */
      PANIC();
    }
    /* "If the Process Count is zero invoke the HALT BIOS service/instruction." - p. 23 pandos  */
    if(processCount == 0){
      HALT();
    }
  }
}

/* LDST aka load state would be too significant since it overwrites so much. So we make a handler to easily identify issues. "Observe that the correct processor state to load (LDST) is the saved exception state (located at the start of the BIOS Data Page [Section 3.4]) and not the obsolete processor state stored in the Current Process's pcb." p.30 pandos. So LDST is performed on the state located in the BIOS!!!
*/
void loadState(state_PTR process){
  LDST(process);
}

/* Context Switch! */
void BOOM(pcb_PTR nowProcess) {
  currentProcess = nowProcess;
  loadState(&(nowProcess -> p_s));
 }
 
 /* will add comments when I know this works */
 void timerPrep(pcb_PTR currentProcess, cpu_t time) {
   STCK(startTimeOfDayClock);
   setTIMER(time);
   BOOM(currentProcess);
 }
