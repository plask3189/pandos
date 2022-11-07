/************ SCHEDULER.C ************/
/*
 * Written By: Kate Plas && Travis Wahl
 * For: PandOS project CSCI

 * Our goal is to have some process running at all times to maximize CPU utilization. Whenever the CPU becomes idle (like after an IO when waiting for response), the OS selects one of the pcbs in the ready queue to be executed.

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

/* Start the scheduler! Round-Robin method is implemented and controls the schedule of each process that needs to be executed.
* A possible situation where the scheduler() was called: a process just sent out an IO request so now it's waiting. We want to put this process on pause so we can run another process while we're waiting. */

void scheduler(){
    cpu_t howManyProcessorCyclesElapsed; /* Initialize variable to track proc cycles */
    if(currentProc != NULL){ /* There is a process currently running. We want to check how long it has been executing on the CPU/ */
        STCK(howManyProcessorCyclesElapsed); /* This is an assignment statement!! We populate howManyProcessorCyclesElapsed with the TOD clock/time scale (We need this to check how long this currently running process has been executing). */
        currentProc->p_time = currentProc->p_time + (howManyProcessorCyclesElapsed - startTOD); /* update how long this executing process has been hogging the CPU. */
        LDIT(IOCLOCK); /* Set the interval timer to generate an interrupt if necessary. */
    }
    pcb_PTR next; /* next is a pointer to a pcb that will be removed from the readyQueue. */
    next = removeProcQ(&readyQueue); /* Remove a pcb from the head of the readyQueue that we will dispatch to the CPU to execute. */

    /* * * * If we CAN get a pcb that should start executing on the CPU * * * */
    if (next != NULL){ /* If there is a pcb in the readyQueue that next can point to. */
        currentProc = next; /* Set the pcb that we just removed from the readyQueue (aka next) to be the currentProc since we're putting this pcb on the CPU to execute. */
        STCK(startTOD); /* Populate startTOD with which clock tick we start executing the process on.*/
        setTIMER(QUANTUM); /* Set timer for 5 ms. The PLT will begin counting down and will generate an interrupt when the 5ms are up and it's time to relinquish the CPU to another process.  */
        loadState(&(currentProc->p_s)); /* load this process onto the CPU to begin executing on the CPU. */

    /* * * * If we CANNOT get a pcb that should start executing on the CPU * * * */
    } else { /* if next == NULL */
        if(processCount == 0){ /* If the readyQueue is empty so there are no more processes to run! */
            HALT();
        }

        /* The deadlock case */
        if ((softBlockCount == 0) && (processCount > 0)){ /* In this case, the processes on the readyQueue would be ready forever since no interrupt would ever happen to the processes waiting on the asl (since there are none). */
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


/* Returning to a previously interrupted execution stream is ac- complished via the LDST command which performs the complementary pop operation on the KU/IE stacks. Thus returning the processor to whatever interrupt state and mode was in effect when the exception occurred. */


 /* Pass in a new process that will now be in control of the machine */
void loadState(state_PTR ps){
    /* Load the passed-in process, becoming the new currentProc */
    LDST(ps);
}
