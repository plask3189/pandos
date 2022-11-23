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
   /*  Increment the exceptionState's PC by 4 to return control to the instruction after the SYSCALL instruction. */
   exceptionState->s_pc = (exceptionState->s_pc + PCINC);
   int cause = exceptionState->s_cause;

   if(cause == ONE){
       SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
   }else{
     /* The executing process places appropriate values in the general purpose registers a0–a3 immediately prior to executing the SYSCALL instruction. */
       int syscallNumber = exceptionState->s_a0;
       int processASID = support->sup_asid;
       int arg1 = exceptionState->s_a1;
       int arg2 = exceptionState->s_a2;
       int arg3 = exceptionState->s_a3;
       int ret = 0;
       switch(syscallNumber){ /* Determine which syscall handler to pass control to by comparing the syscallNumber to the syscall codes. */
           case TERMINATE: /* SYS 9: Terminate process */
               terminateProcess(processASID);
               break;
           case GETTOD: /* SYS 10: Get TOD */
               exceptionState->s_v0 = getTOD(); /* The number of microseconds since the system was last booted is placed in the U-proc's v0 register. */
               break;
           case WRITETOPRINTER: /* SYS 11: Write to the printer */
               exceptionState->s_v0 = writeToPrinter(arg1, arg2, processASID-1);
               break;
           case WRITETOTERMINAL: /* SYS 12: Write to the terminal */
               exceptionState->s_v0 = writeToTerminal(arg1, arg2, processASID-1);
               break;
           case READFROMTERMINAL: /* SYS 13: Read to the terminal */
               exceptionState->s_v0 =  readFromTerminal(arg1);
               break;
           default:
               terminateProcess(processASID); /* If none of the above match the syscallNumber, terminate the process aka SYS 2. */
       }
       LDST(exceptionState);
   }
}
void interruptsSwitch(int onOrOff){
  if(!onOrOff){ /* if IEc == 0, keep status at 0. If IEc == 1, change status to 0. */
      setSTATUS(getSTATUS() & ~ONE);
  } else {
      setSTATUS(getSTATUS() | ONE); /* This bitwise operation always results in a one, making the IEc bit hold 1. When 1, interrupt acceptance is controlled by Status.IM (the interrupt mask). */
  }
}

void terminateProcess(int asid){
   /* The SYS9 service is essentially a user-mode “wrapper” for the kernel-mode restricted SYS2 service, so execute SYS2 aka TERMINATEPROCESS. */
   SYSCALL(TERMINATEPROCESS,ZERO,ZERO,ZERO);
}

int writeToTerminal(char *msg, int stringLength, int processASID){
}

int readFromTerminal(char* virtualAddress){
  return 0;
}

int writeToPrinter(char *msg, int stringLength, int processASID){

}

cpu_t getTOD(){
  cpu_t time;
	STCK(time); /* Populate time with the value of how many microseconds since the system last booted. */
	supportStruct->sup_exceptState[GENERALEXCEPT].s_v0 = time; /* This causes the number of microseconds since the system was last booted/reset to be placed/returned in the U-proc’s v0 register. */
}
