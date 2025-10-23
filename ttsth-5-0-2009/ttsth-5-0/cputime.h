#ifndef CPUTIME_H
#define CPUTIME_H

#include <limits.h>

#ifndef CLK_TCK
/* cant calculate time */
#define CLK_TCK -1
#endif


void initCPUTime();

double getCPUTimeSinceStart();

double currentCPUTime();

void setCPUTimeLimit(double limit);

int CPUTimeExpired();

#endif
