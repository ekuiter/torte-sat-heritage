#include <sys/times.h>
#include <time.h>

#include "cputime.h"
#include "dimacs.h"

static double startCPUTime;

static struct tms tmsBuffer;

double currentCPUTime()
{
	double result;

	if (CLK_TCK == -1)
	{
		result = 0.0;
	}
	else
	{
		times (&tmsBuffer);
		result = (tmsBuffer.tms_utime + tmsBuffer.tms_stime)/((double) CLK_TCK);
	}

	return result;
}

int getSATlimit(char * name) {
 char * value;

 value = getenv(name);
 if (value == NULL) return(-1);
 return atoi(value);
}

static double CPUTimeLimit;
static int limitSet = 0, nextDisplay;

void initCPUTime()
{
	int satLimit;
	nextDisplay = 1;
	startCPUTime = currentCPUTime();
	satLimit = getSATlimit ("SATTIMEOUT");
	if (satLimit >= 0)
		setCPUTimeLimit ( (double) satLimit);

}

double getCPUTimeSinceStart()
{
	double result;
	result = currentCPUTime() - startCPUTime;

	return result;
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
