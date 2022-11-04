/************ SCHEDULER.C ************/
/*
 * Written By: Kate Plas && Travis Wahl
 * For: PandOS project CSCI 320
 * This is a round-robin scheduling algorithm with a time slice value of 5ms.
 * The Scheduler is called! So dispatch next process from the readyQueue.
 */

#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/initial.h"
#include "../h/libumps.h"

/* Bring in the Start Time Of Day Clock variable */
extern cpu_t startTOD;

/* Start the scheduler! Round-Robin method is implemented and controls the schedule of 
 * each process that needs to be executed.
 */
void scheduler(){
    cpu_t howManyProcessorCyclesElapsed;
    if(currentProc != NULL){ 
        STCK(howManyProcessorCyclesElapsed); /* STCK() stores how many processor cycles have elapsed*/
        currentProc->p_time = currentProc->p_time + (howManyProcessorCyclesElapsed - startTOD);
        LDIT(IOCLOCK); /* load the interval timer with IOCLOCK value */
    }
    pcb_PTR next; /* next is a pointer to a pcb that will be removed from the readyQueue. */
    next = removeProcQ(&readyQueue); /* Remove a pcb from the head of the readyQueue */
    
    /* * * * If we CAN get a pcb * * * */
    if (next != NULL){ /* If there is a pcb that next can point to. */
        currentProc = next;
        STCK(startTOD);
        setTIMER(QUANTUM); /* start timer for 5 ms */
        loadState(&(currentProc->p_s)); /* BOOM! Context Switch */
   
        /* * * * If we CANNOT get a pcb * * * */
    } else { /* if next == NULL */
        if(processCount == 0){ /* If the readyQueue is empty so there are no more process to run! */
            HALT();
        }
       
        /* The deadlock case
         * There are processes, but they're not in the Blocked or Ready queues.
         */
        if ((softBlockCount == 0) && (processCount > 0)){
             PANIC(); /* Stop, Panic time */
        }
        
        /* If for some reason there are pcbs in the readyQueue and there are pcbs on the ASL but we can't make a pointer to it called next...*/
        if ((softBlockCount > 0) && (processCount > 0)){ 
            int maskForStatus = ALLOFF | IECON | IMON ;
            setSTATUS(maskForStatus); 
            WAIT(); /*WAIT() unblocks a pcb from the ASL and populates the readyQueue */
            } 
        }
 }


/* BOOM! Context Switch!
 * Pass in a new process that will now be in control of the machine */
void loadState(state_PTR ps){
    /* Load the passed-in process, becoming the new currentProc */ 
    LDST(ps);
}




