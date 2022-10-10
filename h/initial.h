/* initial.h */

#define INITIAL

extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProcess;
extern pcb_PTR readyQueue;
extern int deviceSemaphores[NUMBEROFDEVICES];
int *clockSemaphore;
cpu_t startTimeOfDayClock;

#endif
