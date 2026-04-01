#ifndef STATE_MACHINE
#define STATE_MACHINE

#include "packet.h"

void stateInit(void);
void stateSet(obc_state_t newState);
obc_state_t getState(void);

#endif