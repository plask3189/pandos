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
#include "../h/libumps.h"
#include "../h/vmSupport.h"

void SysSupport(){
   support_t* supportStruct = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
   state_PTR exceptionState = &supportStruct->sup_exceptState[GENERALEXCEPT];
   int cause = exceptionState->s_cause;
   if(cause == SYSEXCEPTION){
    uSysHandler(supportStruct);
   }else{
     SYSCALL(TERMINATEPROCESS, ZERO, ZERO, ZERO);
   }
 }

void uSysHandler(support_t *supportStruct){
  /* The processor state at the time of the exception is in the Support Structure’s corresponding sup_exceptState field. */
  state_PTR exceptionState = &supportStruct->sup_exceptState[GENERALEXCEPT];
  /*  Increment the exceptionState's PC by 4 to return control to the instruction after the SYSCALL instruction. */
  exceptionState->s_pc = (exceptionState->s_pc + PCINC);
  /* The executing process places appropriate values in the general purpose registers a0–a3 immediately prior to executing the SYSCALL instruction. */
  int syscallNumber = exceptionState->s_a0;
  int processASID = supportStruct->sup_asid;
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
        terminateProcess(processASID); /* If none of the above match the syscallNumber, terminate the process. */
       }
       LDST(exceptionState);
  }

void interruptsSwitch(int onOrOff){
  if(!onOrOff){ /* if IEc == 0, keep status at 0. If IEc == 1, change status to 0. */
      setSTATUS(getSTATUS() & ~ONE);
  } else {
      setSTATUS(getSTATUS() | ONE); /* This bitwise operation always results in a one, making the IEc bit hold 1. When 1, interrupt acceptance is controlled by Status.IM (the interrupt mask). */
  }
}

/* The SYS9 service is essentially a user-mode “wrapper” for the kernel-mode restricted SYS2 service, so execute SYS2 aka TERMINATEPROCESS. */
void terminateProcess(int asid){
   SYSCALL(TERMINATEPROCESS,ZERO,ZERO,ZERO);
}

/* When this service is requested, it causes the number of microseconds since the system was last booted/reset to be placed/returned in the U-proc’s v0 register. */
cpu_t getTOD(){
  	support_t *supportStruct;
  	cpu_t time;
	STCK(time); /* Populate time with the value of how many microseconds since the system last booted. */
	supportStruct->sup_exceptState[GENERALEXCEPT].s_v0 = time; /* This causes the number of microseconds since the system was last booted/reset to be placed/returned in the U-proc’s v0 register. */
}

/* SYS11:  When requested, this service causes the requesting U-proc to be suspended until a line of output (string of characters) has been transmitted to the printer device associated with the U-proc.
Once the process resumes, the number of characters actually transmitted is returned in v0. */
int writeToPrinter(char *characterAddress, int stringLength, int processASID){
  /* Given an interrupt line (IntLineNo) and a device number (DevNo) one can compute the starting address of the device’s device register: devAddrBase = 0x1000.0054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10) (p.28 pops) */
  unsigned int* printer = (DEVICEREGISTERSBUSAREA + ((3) * 0x80) + (processASID * 0x10));
  unsigned int status;
  int deviceSemaphoreNumber = ((TERMINT - DISKINT) * DEVPERINT) + processASID;
  SYSCALL(PASSEREN, (int)&semDevices[deviceSemaphoreNumber], ZERO, ZERO);
  int i = 0;
  int ret = 0;
  for (i; i<stringLength; i++) {
    *(printer + 3) = PRINTCHR | (((unsigned int) *characterAddress) << BYTELENGTH);
    int status = SYSCALL(WAITIO, TERMINT, ZERO, ZERO);
    if ((status & TERMSTATMASK) != CODEFORCHARECTERCORRECTLYRECEIVEDORTRANSMITTED){
      ret = ret - status;
      break;
    }
    characterAddress++;
  }
  SYSCALL(VERHOGEN, (int)&semDevices[deviceSemaphoreNumber], 0, 0);
  return ret;
  return 0;
}

/* SYS 12: When requested, this service causes the requesting U-proc to be suspended until a line of output (string of characters) has been transmitted to the terminal device associated with the U-proc.
@param
char* characterAddress - The virtual address of the first character of the string is in a1. aka... char* characterAddress = (char*) supportStruct->sup_exceptState[GENERALEXCEPT].s_a1;
int length - The virtual address of the length of the string is in a2. aka... length = supportStruct->sup_exceptState[GENERALEXCEPT].s_a2; */

int writeToTerminal(char *characterAddress, int length, int processASID){
	support_t *supportStruct;
	unsigned int status;
	int error;
	devregarea_t* devReg = (devregarea_t*) RAMBASEADDR;
    	int deviceSemaphoreNumber = ((TERMINT - 3) * DEVPERINT) + processASID; /* essentially 32 + processASID */
    	if((int)characterAddress < KUSEG){ /* If there's a write to a terminal device from an address outside of the requesting U-proc’s logical address space, error. */
      	SYSCALL(TERMINATEPROCESS, ZERO, ZERO, ZERO);
    	}
    	if((length < 0) || (length > MAXSTRING)){ /* Error if request a SYS12 with a length less than 0, or a length greater than 128. */
      		SYSCALL(TERMINATEPROCESS, ZERO, ZERO, ZERO);
    	}
    	/* Start mutual exclusion by P-ing the semaphore. */
    	SYSCALL(PASSEREN, (int) &semDevices[deviceSemaphoreNumber], 0, 0);
    	int counter = 0;
    	error = FALSE;
		while((error == FALSE) && (counter < length)){
      	devReg->devreg[deviceSemaphoreNumber].t_transm_command = *characterAddress << BYTELENGTH | 2;
      	status = SYSCALL(WAITIO, TERMINT, processASID, 0);
			if((status & TERMSTATMASK) != CODEFORCHARECTERCORRECTLYRECEIVEDORTRANSMITTED){ /* If the character was incorrectly transmitted, the status code is not 5. */
			    error = TRUE;
			}else{
        counter++;
      	}
      	characterAddress++;
		}
		/* Release mutual exclusion by V-ing the semaphore. */
		SYSCALL(VERHOGEN, (int) &semDevices[deviceSemaphoreNumber], ZERO, ZERO);

		if(error){
		    /* If there's an error, the negative of the device’s status value is returned in v0 . */
        supportStruct->sup_exceptState[GENERALEXCEPT].s_v0 = (0 - (status & 0xFF));
		} else {
    	/* Once the process resumes, the number of characters actually transmitted is returned in v0 if the write was successful. */
      	supportStruct->sup_exceptState[GENERALEXCEPT].s_v0 = counter;
    	}
}

int readFromTerminal(char* virtualAddress){
  return 0;
}
