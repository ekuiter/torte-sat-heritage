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

extern char *dimacsBaseName;

extern int dimacsVariables, dimacsClauses;

extern FILE *dimacsInputFile, *dimacsOutputFile;

extern void openDIMACSFiles (char *inputFileName);

extern void writeDIMACSSolution (PropositionPtr proposition, AssignmentPtr assignment, double cpusecs, int elapsedSeconds, int measure1, int measure2);

extern void writeDimacsProposition (char *filename, PropositionPtr p);

extern void closeDIMACSFiles ();

extern void writeCPUTime (char *stage);

extern PropositionPtr readDIMACSProposition ();

extern void abortDIMACS();

extern void exitDIMACS (AssignmentPtr assignment);

extern void dimacsComment (char *s);

extern int getSATLimit(char * name);

#endif
