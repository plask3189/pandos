/************ initProc.c ************/
/* Written by Kate Plas && Travis Wahl
 * CSCI 320 - Operating Systems
 *
 *
 * Instantiation of a single proicess whose PC is set to TEST. TEST is
 * now used as an instatiator process that will initialize the Level 4 / Phase 3 data
 * structures which includes trhe Swap Pool table and the Swap Pool semaphore as well as each
 * sharable peripheral I/O device getting its own semaphore defined for it while the Terminal
 * will get two semaphores (read and write). TEST will also initialize and launch between 1-8 U-procs
 * via SYS1 syscalls and either Terminate (SYS2) after all processes conclude (HALT) or perform
 * a P (SYS3) operation on a priovate semaphore intialized to 0, which wil result in PANIC
 * after all processes conclude.
 */
 
 
#include "../h/const.h"
#include "../h/types.h"
#include "../h/initial.h"
#include "../h/initProc.h"
#include "../h/vmSupport.h"
#include "../h/sysSupport.h"
#include "../h/libumps.h"
 
 int devSem[DEVCOUNT + DEVPEREINT]; /* The device semaphore list */
 int masterSem; /* The control sema4 */
 
 
 void test(){
 
 }
