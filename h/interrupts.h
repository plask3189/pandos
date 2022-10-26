#ifndef INTERRUPTS
#define INTERRUPTS

#include "../h/types.h"
#include "../h/const.h"

extern void interruptHandler();
extern void stateStoring(state_t *stateObtained, state_t *stateStored);


#endif
