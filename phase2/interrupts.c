/* * * * * * * * * * * interrupts.c * * * * * * * * *

Interrupts alert the CPU to events that require attention.

An interrupt happens when a previously initiated IO request completes or when the PLT or Interval Timer makes a 0x0000.0000 -> 0xFFFFFFFF transition.

Each element in the interrupt vector points to the head of a list of interrupt handlers. When an interrupt is raised, the handlers on the corresponding list are called one by one, until one is found that can service the request.

The highest priority is the interval timer, then descends to lowest priority as device number increases.

Interrupted processes are not given their full time back.

*/

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
    int whatClassOfDevice = 0; /* Can be a number 0-7 */
    state_PTR oldState = (state_PTR) BIOSDATAPAGE;
    int ipBits = ((oldState->s_cause & IPMASK) >> 8); /* Do an AND bit operation between IPMASK (0x00FF00 which is 1111111100000000 in binary) and the cause CP0 register. Then we need to shift by 8 so that we have the IP bits (8-15) which indicate on which interrupt lines the interrupts are currently pending. Shifting is pushing the shift's # as the # of 0s pushed onto the left side. If an interrupt is pending on interrupt line i, then Cause.IP[i] is set to 1. That way when we AND the Cause.IP bits and the IPMASK, the result has a 1 only where the interrupt is pending.
    Example: if the cause register is 0...0001000000000000 and we AND it to 1111111100000000, we get 0...0001000000000000. Then we shift by 8, so we get 0...000100 which is 4!!!  */

    /* * * * Inter-processor interrupts * * * */
    if(ipBits == LINE0INTON){
       PANIC();
    }
    /* * * * Local Timer Interrupts * * * */
    else if (ipBits == LINE1INTON) {
      prepToSwitchProcessAfterIoHandled();
    }
    /* * * * Interval Timer Interrupts * * * */
    else if (ipBits == LINE2INTON) {
        LDIT(IOCLOCK); /* Acknowledge the PLT interrupt by loading the timer with a new value. */
        /* * * * * Unblock a pcb, add to readyQueue * * * */
        STCK(stopTOD); /* Assigns stopTOD */
        pcb_PTR proc = removeBlocked(clockSem);  /* unblock pcb blocked on the pseudo-clock semphore.*/
        if (proc != NULL) {
        	softBlockCount--; /* decrement softBlockCount. */
        }
        while (proc!=NULL){
            proc->p_time = (proc->p_time) + (stopTOD- startTOD);
            insertProcQ(&readyQueue, proc);
            proc = removeBlocked(clockSem);
        }
        *clockSem = 0;
        prepToSwitchProcessAfterIoHandled();
    }
    /* * * * * Peripheral devices * * * * *
    Find which class of device has an interrupt pending on one of its instances. */
    if (ipBits == LINE3INTON) {
        whatClassOfDevice = 3; /* Interrupt line 3 corresponds to disk devices. */
    } else if (ipBits == LINE4INTON) {
        whatClassOfDevice = 4; /* Interrupt line 4 corresponds to flash devices. */
    } else if (ipBits == LINE5INTON) {
        whatClassOfDevice = 5; /* Interrupt line 5 corresponds to network devices. */
    } else if (ipBits == LINE6INTON) {
        whatClassOfDevice = 6; /* Interrupt line 6 corresponds to printer devices. */
    } else if (ipBits == LINE7INTON) {
        whatClassOfDevice = 7; /* Interrupt line 7 corresponds to terminal devices. */
    }
    /* * * * * Find the correct instance of the interrupting device * * * * */
    int deviceNumber;
    devregarea_t * ramBasePhysicalAddressPtr = (devregarea_t *) RAMBASEADDR;
    int whichInstanceOfTheDevice = ramBasePhysicalAddressPtr->interrupt_dev[whatClassOfDevice-3]; /* Get the interrupting device. In the bus register area, there's an array with size five because we're only looking at devices classes 3-7. There are up to eight instances for each of the five device classes. */
    if(whichInstanceOfTheDevice == LINE0INTON){
        deviceNumber = 0;
    } else if (whichInstanceOfTheDevice == LINE1INTON) {
        deviceNumber = 1;
    } else if (whichInstanceOfTheDevice == LINE2INTON) {
        deviceNumber = 2;
    } else if (whichInstanceOfTheDevice == LINE3INTON) {
        deviceNumber = 3;
    } else if (whichInstanceOfTheDevice == LINE4INTON) {
        deviceNumber = 4;
    } else if (whichInstanceOfTheDevice == LINE5INTON) {
        deviceNumber = 5;
    } else if (whichInstanceOfTheDevice == LINE6INTON) {
        deviceNumber = 6;
    } else if (whichInstanceOfTheDevice == LINE7INTON) {
        deviceNumber = 7;
    }
/* * * * Non-timer interrupts * * * *
When a non-timer interrupt occurs, determine which semaphore the device is blocked on and get the device's address. Then, find if it is a terminal device or another class of device. If it is a terminal device, save the status code, acknowledge the interrupt, unblock it from the asl, and  */
    if(whatClassOfDevice >= 3){
    	int actualDeviceNumber = (whatClassOfDevice - 3);
      int deviceSemaphore = actualDeviceNumber * DEVPERINT + deviceNumber; /* according to p.28 pandos, this is how to compute the starting address of the device's device register */
      int startingAddressOfDeviceRegister = DEVICEREGISTERSSTARTINGADDRESS + ((whatClassOfDevice -3) * 0x80) + (deviceNumber * 0x10);
      int statusOfCurrentProcess; /* We will save off the status code from the device's device register. */
      device_t* dev = (device_t *) startingAddressOfDeviceRegister; /* Retrieve the device from the device address */
        if (whatClassOfDevice != 7){ /* If the interrupt line number does NOT correspond to terminal devices. */
            statusOfCurrentProcess = dev -> d_status; /* Save off the status code from the device's device register. */
            dev->d_command = ACKNOWLEDGE; /* Acknowledge the outstanding interrupt. This is accomplished by writing the acknowledge command code in the interrupting device’s device register. */
        }
        if (whatClassOfDevice == 7){ /* If the interrupt line number corresponds to terminal devices. */
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
        int *semad = &semDevices[deviceSemaphore]; /* Declare a pointer that holds the address of the semaphore on which the device is blocked. */
        (*semad)++; /* *semad gets the value of the semaphore that semad points to. Increment the device's semaphore to reflect that we'll be taking the device off of that semaphore. */
        if(*semad >= ZERO){ /* If the value of a semaphore is negative, it is an invariant that the absolute value of the semaphore equal the number of pcb’s blocked on that semaphore. So we want to ensure that the value of the semaphore is positive. */
            pcb_PTR proc = removeBlocked(semad); /* unblock the process from the semaphore. */
            if(proc!=NULL){
                STCK(stopTOD);
                softBlockCount--;
                proc->p_time = (proc->p_time) +(stopTOD- startTOD);
                proc->p_s.s_v0 = statusOfCurrentProcess; /* Place the stored off status code in the newly unblocked pcb's v0 register. */
                insertProcQ(&readyQueue, proc); /* The process is now ready. */
            }
        }
        prepToSwitchProcessAfterIoHandled();
    }
	if (currentProc == NULL){
		scheduler();
	}
}

/* This method switch processes after IO is handled by changing the current process state from running to ready. This is done by copying the state and putting the io device in the readyQueue. Then call scheduler. */
void prepToSwitchProcessAfterIoHandled(){
    state_PTR oldState = (state_PTR) BIOSDATAPAGE;
    if(currentProc != NULL){
        stateCopy(oldState, &(currentProc->p_s));
        insertProcQ(&readyQueue, currentProc); /* place the current process on the ready queue; transitioning the current process from running to ready state. */
    }
    scheduler();
}
