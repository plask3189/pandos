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
   state_PTR exceptionState = &support->sup_exceptState[GENERALEXCEPT];
   exceptionState->s_pc+=PCINC;
   int cause = exceptionState->s_cause;

}
