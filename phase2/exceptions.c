#include <stdio.h>
#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "/usr/include/umps3/umps/libumps.h"

extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int processCount;
extern int softBlockCount;
extern int semDevices[DEVNUM];
extern cpu_t startTOD;
extern int* clockSem;
extern void loadState(state_PTR ps);

void tlbTrapHandler();
void programTrapHandler();
void createProc(state_PTR curr);
void terminateProc(pcb_PTR prnt);
void passeren(state_PTR curr);
void ver(state_PTR curr);
void waitForIO(state_PTR curr);
void getCPUTime(state_PTR curr);
void waitForClock(state_PTR curr);
void getSupport(state_PTR curr);

void passUpOrDie(state_PTR curr, int exception);
void otherExceptions();

void stateCopy(state_PTR oldState, state_PTR newState);


void SYSCALLHandler(){
    /* Save the processor state. */
    state_PTR ps = (state_PTR) BIOSDATAPAGE; 
    ps->s_pc = ps->s_pc+PCINC;
    ps->s_t9 = ps->s_pc+PCINC;
    
    /* Check if in usermode. */
    int userMode = (ps->s_status & UMOFF);
    if(userMode != ALLOFF){
	    passUpOrDie(ps, GENERALEXCEPT);
    }

    int syscallNumber = (ps->s_a0);
    switch (syscallNumber)
    {
    case CREATEPROCESS:{ /* if syscallNumber == 1 */
        createProc(ps);
        break;}
    
    case TERMINATEPROCESS:{ /* if syscallNumber == 2 */
        if(currentProc != NULL){
            terminateProc(currentProc);
        }
        scheduler();
        break;}
    
    case PASSEREN:{ /* if syscallNumber == 3 */
        passeren(ps);
        break;}
    
    case VERHOGEN:{ /* if syscallNumber == 4 */
        ver(ps);
        break;}
    
    case WAITIO:{ /* if syscallNumber == 5 */
        waitForIO(ps);
        break;}
    
    case GETCPUTIME:{ /* if syscallNumber == 6 */
        getCPUTime(ps);
        break;}
    
    case WAITCLOCK:{ /* if syscallNumber == 7 */
        waitForClock(ps);
        break;}

    case GETSUPPORTPTR:{ /* if syscallNumber == 8 */
        getSupport(ps);
        break;}
    
    default:{
        passUpOrDie(ps, GENERALEXCEPT); 
        break;}
    }
}

/* "The sys1 service is requested by the calling process by placing the value 1 in a0, a pointer to a processor state in a1, a pointer to a support struct in a2, and then executing the syscall instruction." p.25 pandos*/ 
void createProc(state_PTR oldState){
    pcb_PTR child = allocPcb();
    int returnStatus = -1;
    if(child != NULL){
        insertChild(currentProc, child);
        insertProcQ(&readyQueue, child);
        stateCopy((state_PTR) (oldState->s_a1), &(child->p_s));
        if(oldState->s_a2 != 0 || oldState->s_a2 != NULL){
            child->p_supportStruct = (support_t *) oldState->s_a2;
        } else {
            child->p_supportStruct = NULL;
        }
        processCount++;
        returnStatus = 0;
    }
    currentProc->p_s.s_v0 = returnStatus;
    loadState(oldState);   
}

void terminateProc(pcb_PTR parentProc){
    while(!emptyChild(parentProc)){
	    terminateProc(removeChild(parentProc));
    }

    if(currentProc == parentProc){
	    outChild(parentProc);
    } 

    if(parentProc->p_semAdd == NULL){
	    outProcQ(&readyQueue, parentProc);
    }
   else {
        
        pcb_PTR removed = outBlocked(parentProc);
        if(removed != NULL){
        	int* semdAdd = removed->p_semAdd;
            if( semdAdd >= &semDevices[ZERO] && semdAdd <= &semDevices[DEVNUM]){
                softBlockCount--;
            } else {
                (*semdAdd)++;
            }	
        }
        
    }
    freePcb(parentProc);
    processCount--;
    /* we call the scheduler in the switch case statements */
}
/* the wait() operation: When a process is waiting for IO and we want another process to execute while we're waiting.   */
void passeren(state_PTR oldState){
     int* semdAdd = (int*) oldState->s_a1; 
    (*semdAdd)--; /* decrement the number of processes waiting on this semaphore to indicate the increased magnitude of process waiting on the semaphore.*/
    if((*semdAdd)<0){ /* if semdAdd is negative then there are process waiting on the semaphore. */
	stateCopy(oldState, &(currentProc->p_s)); /* save the currentProc state, then we'll insert the currentProc on the asl */
       insertBlocked(semdAdd, currentProc); 
        scheduler();
    }
    loadState(oldState);   
}

/* the signal() operation */
void ver(state_PTR oldState){
    int* semdAdd = (int*)oldState->s_a1;
    (*semdAdd)++;
    if((*semdAdd)<=0){
        pcb_PTR temp = removeBlocked(semdAdd);
        if(temp != NULL) {
            insertProcQ(&readyQueue, temp);
        }
    }
    loadState(oldState);
}


void waitForIO(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    int lineNo = oldState->s_a1;
    int devNo = oldState->s_a2;
    int waitterm = oldState->s_a3;
    int devi = ((lineNo - 3 + waitterm) * DEVPERINT + devNo);
    semDevices[devi]--;
    softBlockCount++;
    insertBlocked(&(semDevices[devi]), currentProc);
    currentProc = NULL;
    scheduler();
}


void getCPUTime(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    cpu_t time;
    STCK(time);
    time -= startTOD;
    currentProc->p_time += time;
    currentProc->p_s.s_v0 = currentProc->p_time;
    loadState(&currentProc->p_s);   
}


void waitForClock(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    (*clockSem)--;
    if((*clockSem)<0){
        insertBlocked(clockSem, currentProc);
        softBlockCount++;
        currentProc = NULL;
        scheduler();
    }
}


void getSupport(state_PTR oldState){
    stateCopy(oldState, &(currentProc->p_s));
    currentProc->p_s.s_v0 =(int) currentProc->p_supportStruct;
    loadState(&currentProc->p_s);   
}

/* passes up process */
void passUpOrDie(state_PTR oldState, int exception){
	support_t *supportStruct = currentProc->p_supportStruct;
    if((supportStruct == NULL) || supportStruct == 0) {
        terminateProc(currentProc); 
        scheduler();
    }else{
       stateCopy(oldState, &(currentProc->p_supportStruct->sup_exceptState[exception])); 
        unsigned int stackPtrToLoad = currentProc->p_supportStruct->sup_exceptContext[exception].c_stackPtr;
       unsigned int statusToLoad = currentProc->p_supportStruct->sup_exceptContext[exception].c_status;
       unsigned int pcToLoad = currentProc->p_supportStruct->sup_exceptContext[exception].c_pc;
        /* load context */
        LDCXT(stackPtrToLoad, statusToLoad, pcToLoad);
    }
}

void stateCopy(state_PTR oldState, state_PTR newState){
    int i;
    for(i=0; i<STATEREGNUM; i++){
	    newState->s_reg[i] = oldState->s_reg[i];
    }
    newState->s_entryHI = oldState->s_entryHI;
    newState->s_cause = oldState->s_cause;
    newState->s_status = oldState->s_status;
    newState->s_pc = oldState->s_pc;
}


void tlbTrapHandler(){
	passUpOrDie((state_PTR) BIOSDATAPAGE,  PGFAULTEXCEPT);
}

void programTrapHandler(){
passUpOrDie((state_PTR) BIOSDATAPAGE,  GENERALEXCEPT);
}


