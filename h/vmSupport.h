#ifndef VMSUPPORT
#define VMSUPPORT

#include "../h/const.h"
#include "../h/types.h"

extern int devSem[DEVCOUNT + DEVPERINT];
extern int masterSema4;

extern void initTLB();
extern void uTLBRefillHandler();
extern void pager();


#endif
