/*  SysSupport contains the portion of the OS that handles user system calls. This is Phase 3's exception handler. It gets data from the support struct to determine the proper action to take.
* If the cause is a syscall exception, it calls the function uSysHandler, which is the user syscall handler, otherwise
* it terminates the process.
*
*/

#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/initProc.h"


void uSyscallHandler(){
   support_t * support = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
   /* The processor state at the time of the exception is in the Support Structure’s corresponding sup_exceptState field. */
   state_PTR exceptionState = &support->sup_exceptState[GENERALEXCEPT];
   exceptionState->s_pc = (exceptionState->s_pc + PCINC);
   int cause = exceptionState->s_cause;

   if(cause == ONE){
       SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
   }else{ /*If cause != ONE, then the cause is a syscall exception. */
       int syscallNumber = exceptionState->s_a0;
       int processASID = support->sup_asid;
       int arg1 = exceptionState->s_a1;
       int arg2 = exceptionState->s_a2;
       int arg3 = exceptionState->s_a3;
       int ret = 0;
       switch(syscallNumber){
           case TERMINATE:
               terminateProcess(processASID);
               break;
           case GETTOD:
               exceptionState->s_v0 = getTOD();
               break;
           case WRITETOPRINTER:
               exceptionState->s_v0 = writeToPrinter(arg1, arg2, processASID-1);
               break;
           case WRITETOTERMINAL:
               exceptionState->s_v0 = writeToTerminal(arg1, arg2, processASID-1);
               break;
           case READFROMTERMINAL:
               exceptionState->s_v0 =  readFromTerminal(arg1);
               break;
           default:
               terminate(processASID);
       }
       LDST(exceptionState);
   }
}
void interruptsSwitch(int on){

}

void terminateProcess(int asid){
    SYSCALL(TERMINATETHREAD,0,0,0);
}

int writeToTerminal(char *msg, int stringLength, int processASID){
  unsigned int * base = (unsigned int *) (TERM0ADDR + (processASID * 0x10));
  unsigned int status;
  SYSCALL(PASSEREN, (int)&devicesSem[34], 0, 0);
  int i = 0;
  int ret = 0;
  for (i;i<strlen;i++) {
    *(base + 3) = PRINTCHR | (((unsigned int) *msg) << BYTELEN);
    int status = SYSCALL(WAITIO, TERMINT, 0, 0);
      if ((status & TERMSTATMASK) != RECVD){
    ret = -status;
          break;
      }
  msg++;
  }
  SYSCALL(VERHOGEN, (int)&devicesSem[34], 0, 0);
  return ret;
}

int readFromTerminal(char* virtualAddress){
  return 0;
}

int writeToPrinter(char *msg, int stringLength, int processASID){

}

cpu_t getTOD(){
  cpu_t time;
	STCK(time);
	supportStruct->sup_exceptState[GENERALEXCEPT].s_v0 = time;
}
