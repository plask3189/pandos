#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "../h/types.h"
#include "../h/const.h"

extern void SYSCALLExceptionHandler();
extern void stateCopy(state_PTR pointerToOldState, state_PTR pointertoNewState);

#endif
