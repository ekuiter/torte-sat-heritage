#ifdef VISUALSTUDIO
#include <time.h>
#else
#include <unistd.h>
#include <sys/times.h>
#endif

#include "cputime.h"
#include "dimacs.h"

#define VISUALSTUDIO

#ifdef VISUALSTUDIO

time_t startCPUTime;

#else

static double startCPUTime;
static double ticksPerSecond;
static struct tms tmsBuffer;
#endif
double currentCPUTime()
{
	double result;

#ifdef VISUALSTUDIO
	result = 1.0;
#else
	{
		times (&tmsBuffer);
		result = (tmsBuffer.tms_utime + tmsBuffer.tms_stime)/((double) ticksPerSecond);
	}
#endif
	return result;
}


static double CPUTimeLimit;
static int limitSet = 0, nextDisplay;

void initCPUTime()
{
#ifdef VISUALSTUDIO
	startCPUTime = clock();
#else
	ticksPerSecond = (double) sysconf(_SC_CLK_TCK);
	nextDisplay = 1;
	startCPUTime = currentCPUTime();
#endif
}

double getCPUTimeSinceStart()
{
#ifdef VISUALSTUDIO
	time_t nowCPUTime;
	
	nowCPUTime = clock();
	
	return difftime (nowCPUTime, startCPUTime)/CLOCKS_PER_SEC;
	
#else
	double result;
	result = currentCPUTime() - startCPUTime;
	return result;
#endif
}


void setCPUTimeLimit(double limit)
{
	char message[100];
	CPUTimeLimit = limit;
	limitSet = 1;
	sprintf (message, "CPU limit set to %f", limit);
	dimacsComment (message);
}

int CPUTimeExpired()
{
	if (getCPUTimeSinceStart() > nextDisplay)
	{
		printf ("\r%d(%d)", nextDisplay, (int)CPUTimeLimit);
		fflush (stdout);
		nextDisplay = getCPUTimeSinceStart()+1;
	}
	return limitSet && (getCPUTimeSinceStart() > CPUTimeLimit);
}
