/* An interrupt happens when a previously initiated IO request completes or when the PLT or Interval Timer makes a 0x0000.0000 -> 0xFFFFFFFF transition. */

#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "/usr/include/umps3/umps/libumps.h"

extern int semDevices[DEVNUM];
extern pcb_PTR readyQueue;
extern pcb_PTR currentProc;
extern int softBlockCount;
extern int * clockSem;
extern cpu_t startTOD;
extern void stateCopy(state_PTR oldState, state_PTR newState);

cpu_t stopTOD;
void prepToSwitchProcessAfterIoHandled();

void IOHandler(){
    state_PTR  oldState = (state_PTR) BIOSDATAPAGE;

    int ipBits = ((oldState->s_cause & IPMASK) >> 8);
    int interruptLineNumber = 0;
    if(ipBits & LINE0INTON){ /* "Interrupt line 0 is reserved for inter-processor interrupts corrdination" p.27 pops. */
       PANIC();
    } else if (ipBits & LINE2INTON) { /* "Interrupt line 2 is reserved for the system wide interval timer." p.27 pops */
        LDIT(IOCLOCK); /* acknowledge the PLT interrupt by loading the timer with a new value. */
        /* * * * * Unblock a pcb, add to readyQueue * * * */
        STCK(stopTOD);  
        pcb_PTR proc = removeBlocked(clockSem);  /* unblock pcb blocked on the pseudo-clock semphore.*/
        if (proc != NULL) {
        	softBlockCount--; /* since we unblocked pcb from the asl, decrement softBlockCount. */
        }
        while (proc!=NULL)
        {
            proc->p_time = (proc->p_time) + (stopTOD- startTOD);
            insertProcQ(&readyQueue, proc);
            proc = removeBlocked(clockSem);
            
        }
        *clockSem = 0;
        prepToSwitchProcessAfterIoHandled(); 
    } 
    else if (ipBits & LINE1INTON) { /* Processor Local Timer Interrupt. The PLT supports the Scheduler*/
        prepToSwitchProcessAfterIoHandled(); 
    } 
    /* For interrupt lines 3-7, the Interrupting Devices Bit Map will indicate which devices on each of these interrupt lines have a pending interrupt. */
    if (ipBits & LINE3INTON) { 
        interruptLineNumber = 3; /* Interrupt line 3 corresponds to disk devices. */
    } else if (ipBits & LINE4INTON) { 
        interruptLineNumber = 4; /* Interrupt line 4 corresponds to flash devices. */
    } else if (ipBits & LINE5INTON) {
        interruptLineNumber = 5; /* Interrupt line 5 corresponds to network devices. */
    } else if (ipBits & LINE6INTON) {
        interruptLineNumber = 6; /* Interrupt line 6 corresponds to printer devices. */
    } else if (ipBits & LINE7INTON) {
        interruptLineNumber = 7; /* Interrupt line 7 corresponds to terminal devices. */
    }
    /* * * * * Find the interrupting device * * * */
    int deviceNumber;
    devregarea_t * ramBasePhysicalAddressPtr = (devregarea_t *) RAMBASEADDR;
    int bitMap = ramBasePhysicalAddressPtr->interrupt_dev[interruptLineNumber-3]; /* get the interrupting device */
    /* Remember truth tables */
    if(bitMap & LINE0INTON){
        deviceNumber = 0;
    } else if (bitMap & LINE1INTON) {
        deviceNumber = 1;
    } else if (bitMap & LINE2INTON) {
        deviceNumber = 2;
    } else if (bitMap & LINE3INTON) {
        deviceNumber = 3;
    } else if (bitMap & LINE4INTON) {
        deviceNumber = 4;
    } else if (bitMap & LINE5INTON) {
        deviceNumber = 5;
    } else if (bitMap & LINE6INTON) {
        deviceNumber = 6;
    } else if (bitMap & LINE7INTON) {
        deviceNumber = 7;
    }
/* * * * Non-timer interrupts * * * */
    if( interruptLineNumber >= 3){
    	int actualDeviceNumber = (interruptLineNumber - 3);
        int deviceSemaphore = actualDeviceNumber * DEVPERINT + deviceNumber; 
        int startingAddressOfDeviceRegister = DEVICEREGISTERSSTARTINGADDRESS + ((interruptLineNumber -3) * 0x80) + (deviceNumber * 0x10); /* according to p.28 pandos, this is how to compute the starting address of the device's device register */
        int statusOfCurrentProcess;
        device_t* dev = (device_t *) startingAddressOfDeviceRegister;
        if (interruptLineNumber != 7){ /* If the interrupt line number does NOT correspond to terminal devices. */
            statusOfCurrentProcess = dev -> d_status;
            dev->d_command = ACKNOWLEDGE;
        }
        if (interruptLineNumber == 7){ /* If the interrupt line number corresponds to terminal devices. */
            if(dev -> t_transm_command & TRANSBITS){
                statusOfCurrentProcess = dev->t_transm_status;
                dev -> t_transm_command = ACKNOWLEDGE;
            } else {
                statusOfCurrentProcess = dev -> t_recv_status;
                dev -> t_recv_command = ACKNOWLEDGE;
                deviceSemaphore = deviceSemaphore + DEVPERINT;
            }
        }
        /* * * * Unblock from the asl. * * * */
        int *semad = &semDevices[deviceSemaphore]; 
        (*semad)++; /* increment the device's semaphore. */
        if(*semad >= ZERO){
            pcb_PTR proc = removeBlocked(semad); /* unblock the process from the semaphore. We're not blocked anymore, now we're ready! */
            if(proc!=NULL){
                STCK(stopTOD);
                softBlockCount--;
                proc->p_time = (proc->p_time) +(stopTOD- startTOD);
                proc->p_s.s_v0 = statusOfCurrentProcess;
                insertProcQ(&readyQueue, proc);
            }
        }
        prepToSwitchProcessAfterIoHandled();
    }
	if (currentProc == NULL){
		scheduler();
	}
}

/* switch processes after IO is handled. copies state and puts the io device in the readyQueue. Then call scheduler. */
void prepToSwitchProcessAfterIoHandled(){
    state_PTR oldState = (state_PTR) BIOSDATAPAGE;
    if(currentProc != NULL){ /* change the current process state from running to ready */
        stateCopy(oldState, &(currentProc->p_s)); 
        insertProcQ(&readyQueue, currentProc); /* place the current process on the ready queue; transitioning the current process from running to ready state. */
    }
    scheduler();
}
