#ifndef DIMACS_H

#define DIMACS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "memutil.h"
#include "proposition.h"

#define dimacsUNKNOWN 0
#define dimacsUNSAT 20
#define dimacsSAT 10

extern int dimacsVariables, dimacsClauses;

extern FILE *dimacsInputFile, *dimacsOutputFile;

extern void openDIMACSFiles (char *inputFileName);

extern void writeDIMACSSolution (PropositionPtr proposition, AssignmentPtr assignment, double cpusecs, int measure1, int measure2);

extern void closeDIMACSFiles ();

extern void writeCPUTime (char *stage);

extern PropositionPtr readDIMACSProposition ();

extern void abortDIMACS();

extern void exitDIMACS (AssignmentPtr assignment);

extern void dimacsComment (char *s);

#endif
