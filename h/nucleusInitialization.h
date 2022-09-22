/* nucleusInitialization.h */

#define nucleusInitialization

extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProcess;
extern pcb_PTR readyQueue;
extern int deviceSemaphores[NUMBEROFDEVICES];
int *clockSemaphore;
cpu_t startTOD;

#endif
