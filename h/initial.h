/* initial.h */

#ifndef INITIAL
#define INITIAL

#include "../h/types.h"
#include "../h/const.h"

extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProcess;
extern pcb_PTR readyQueue;
extern int deviceSemaphores[NUMBEROFDEVICES];

extern int main();

#endif
