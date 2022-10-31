#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"

extern int semDevices[DEVNUM];
extern pcb_PTR readyQueue;
extern pcb_PTR currentProc;
extern int softBlockCount;
extern int * clockSem;
extern cpu_t startTOD;
extern void stateCopy(state_PTR oldState, state_PTR newState);

cpu_t stopTOD;
void prepToSwitch();

void IOHandler(){
    state_PTR  oldState = (state_PTR) BIOSDATAPAGE;

    int ip_bits = ((oldState->s_cause & IPMASK) >> 8);
    int intlNo = 0;
    if(ip_bits & LINE0INTON){

       PANIC();
    } else if (ip_bits & LINE1INTON) {

        prepToSwitch();
    } else if (ip_bits & LINE2INTON) {

        LDIT(IOCLOCK);
 
        STCK(stopTOD);

        pcb_PTR proc = removeBlocked(clockSem);
        while (proc!=NULL)
        {
            proc->p_time += (stopTOD- startTOD);
            insertProcQ(&readyQueue, proc);
            proc = removeBlocked(clockSem);

            softBlockCount--;
        }
        *clockSem = 0;
        prepToSwitch();
    } 
    if (ip_bits & LINE3INTON) { 
        intlNo = 3;
    } else if (ip_bits & LINE4INTON) {
        intlNo = 4;
    } else if (ip_bits & LINE5INTON) {
        intlNo = 5;
    } else if (ip_bits & LINE6INTON) {
        intlNo = 6;
    } else if (ip_bits & LINE7INTON) {
        intlNo = 7;
    }
    devregarea_t * ram = (devregarea_t *) RAMBASEADDR;
    int dev_bits = ram->interrupt_dev[intlNo-3];
    int devNo;
    if(dev_bits & LINE0INTON){
        devNo = 0;
    } else if (dev_bits & LINE1INTON) {
        devNo = 1;
    } else if (dev_bits & LINE2INTON) {
        devNo = 2;
    } else if (dev_bits & LINE3INTON) {
        devNo = 3;
    } else if (dev_bits & LINE4INTON) {
        devNo = 4;
    } else if (dev_bits & LINE5INTON) {
        devNo = 5;
    } else if (dev_bits & LINE6INTON) {
        devNo = 6;
    } else if (dev_bits & LINE7INTON) {
        devNo = 7;
    }

    if( intlNo >= 3){
        int devi = (intlNo - 3) * DEVPERINT + devNo;
        int devAddrbase = 0x10000054 + ((intlNo -3) * 0x80) + (devNo * 0x10);
        int statusCp;
        device_t * dev = (device_t *) devAddrbase;
        if (intlNo == 7){
            if(dev->t_transm_command & TRANSBITS){
                statusCp = dev->t_transm_status;
                dev->t_transm_command = ACK;
            } else {
                statusCp = dev->t_recv_status;
                dev->t_recv_command = ACK;
                devi+=DEVPERINT;
            }
        } else {
            statusCp = dev->d_status;
            dev->d_command = ACK;
        }
        int *semad = &semDevices[devi];
        (*semad)++;
        if(*semad>=ZERO){
            pcb_PTR proc = removeBlocked(semad);
            if(proc!=NULL){
                STCK(stopTOD);
                proc->p_time += (stopTOD- startTOD);
 
                proc->p_s.s_v0 = statusCp;

                softBlockCount--;
                insertProcQ(&readyQueue, proc);
            }
        }

        prepToSwitch();
    }

}


void prepToSwitch(){
    state_PTR oldState = (state_PTR) BIOSDATAPAGE;

    if(currentProc!=NULL){
        stateCopy(oldState, &(currentProc->p_s));
        insertProcQ(&readyQueue, currentProc);
    }

    scheduler();
}
