#ifndef INTERRUPTS
#define INTERRUPTS

#include "../h/types.h"
#include "../h/const.h"

extern void interruptHandler();
extern void pltInterruptHandler(int stopTimer);
extern void intervalInterruptHandler();
extern void deviceInterruptHandler(int lineNum);
extern void terminalInterruptHandler(int sema4_d);
extern void stateStoring(state_t *stateObtained, state_t *stateStored);


#endif
