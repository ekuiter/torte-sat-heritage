
#include "memutil.h"
#include "clause.h"
#include "assignment.h"
#include "proposition.h"
#include "list.h"
#include "dimacs.h"
#include "ternarytree.h"
#include "permute.h"
#include "cputime.h"
#include "options.h"
#include "subsettree.h"

#ifdef PROOF
#include "proof.h"
#endif

#include "tts.h"

#ifdef VISUALSTUDIO
#else
#include <strings.h>
#endif

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#ifdef THREADS
#include <pthread.h>
#endif

#define MESSAGE_LENGTH 500


#define INACTIVE 0
#define ACTIVE 1
#define NUMBERED 2

char message[MESSAGE_LENGTH];

int *globalMapVariable, *globalUnMapVariable, globalMaxVariable, globalNumClauses, *singletonUsage, *isSingleton, *globalUsed, *globalPermute,
*globalUnpermute, *globalUnTransform, *globalTransform, *unitClauseUsage, *isUnitClause;

int readVarOrder, bestStored, bestCost, *bestPermutation, bestTries, *listLengths;
int globalId;
int numThreads;



void printReduced (char *filename, PropositionPtr p)
{
	FILE *f;
	int i, v;
	ClausePtr clause;

	if ((f = fopen(filename,"w")) == NULL)
	{
		fprintf (stderr,"Could not open <%s> for writing\n", filename);
		exit(0);
	}
	fprintf (f, "p cnf %d %d\n", p->maxVariable, p->numOfClauses);
	for (i=0;i<p->numOfClauses;i++)
	{
		clause = p->clauses[i];
		for (v=0;v<clause->numOfVariables;v++)
			fprintf (f, "%d ", clause->positive[v]?clause->variables[v]:-clause->variables[v]);
		fprintf (f,"0\n");
	}
	fclose (f);
}

static SetPtr startSet1;
static SetPtr startFalse1;

void printTreeInfo (TernaryTreePtr root, int level)
{
	
	if (root->isFalsifiable)
		printTreeInfo (root->both,level+1);

	printf ("l=%d\n",root->firstVar);
	clearSet (startSet1);
	addToSet (startSet1,(void *) root);
	clearSet (startFalse1);

}

#ifdef THREADS
pthread_t thread[100];
TernaryTreePtr threadTree;
PropositionPtr threadP;

void *threadStartRoutinName(void *tid)
{
	AssignmentPtr answer = newAssignment();
	setAssignmentNumOfVariables (answer, threadP->maxVariable);
	newTryTernary ((int)tid,answer, threadTree);
	
	return NULL;
}
#endif

AssignmentPtr ttsCore (PropositionPtr p)
{

	AssignmentPtr answer;
	TernaryTreePtr ternaryTree;
	SetPtr startFalse;
	SetPtr startSet;
	SetPtr startNewUnitVars;
	int limit, result;
	
#ifdef THREADS
	int tid;
#endif


	//writeDimacsProposition ("p.cnf",p);
	
	answer = newAssignment();
	setAssignmentNumOfVariables (answer, p->maxVariable);

	//printf ("given %s\n",toStringProposition(p));

	printf ("c about to sort\n"); fflush(stdout);
	sortClauses (p);
	printf ("c sorted\n"); fflush(stdout);

	initTernaryTrees (p);
	printf ("c clauses now %d\n", p->numOfClauses);
	writeCPUTime ("init finished");

	//ternaryTree = buildAllTernary (NULL);



	for (limit=1;limit<=p->maxVariable;limit++)
		answer->values[limit]  = 1;

	startSet = newSet();
	startFalse = newSet();
	startNewUnitVars = newSet();

	result = true;

	ternaryTree = ternaryTreeFromProposition2 (NULL, 1, 0);
	//ternaryTree = buildAllTernary(NULL);
	
	setSpine (ternaryTree);

#ifdef PROOF
	if (booleanOptions[RPT_FILE])
		resetToEndOfUnitClauses();
	if (booleanOptions[PRT_FILE])
		assignNodeIds (ternaryTree);
#endif

	writeCPUTime ("tree built");


	if (stringOptions[STATIC_DOT_FILE] != NULL)
		printStaticDotFile (stringOptions[STATIC_DOT_FILE], ternaryTree);


 


#ifdef THREADS

	threadTree = ternaryTree;
	threadP = p;
	
	clearAnswerFound();
	
	for (tid=0;tid<numThreads;tid++)
	{
		pthread_create(&thread[tid], NULL, threadStartRoutinName, (void *) tid);
	}

	for (tid=0;tid<numThreads;tid++)
	{
		pthread_join(thread[tid],NULL);
	}
	answer = getAnswer();
	
#else
	if (!newTryTernary (0,answer, ternaryTree))
		answer = NULL;
#endif
		






	finaliseTernaryTrees();
	freeSet (startSet);
	freeSet (startFalse);
	freeSet (startNewUnitVars);

	if (booleanOptions[PRINT_LEVEL_COUNTS])
		printTernaryInfo (p->maxVariable);
	return answer;
}


void printSingletons (int maxVariable, int *used, int *isSingleton, int *isUnitClause)
{
	int i,v;
	char varBuffer[50],lineBuffer [500];
	i = 0;

	strcpy (lineBuffer, "Unused ");

	for (v=1;v<=maxVariable;v++)
		if (!used[v])
		{
			sprintf (varBuffer, "%d ",v);
			strcat (lineBuffer, varBuffer);
			i++;
			if (i == 10)
			{
				dimacsComment (lineBuffer);
				strcpy (lineBuffer, "Unused ");
				i = 0;
			}
		}

	if (i > 0)
		dimacsComment (lineBuffer);

	i = 0;
	strcpy (lineBuffer, "Pure literals ");

	for (v=1;v<=maxVariable;v++)
		if (used[v] && isSingleton[v])
		{
			sprintf (varBuffer, "%d ",v);
			strcat (lineBuffer, varBuffer);
			i++;
			if (i == 10)
			{
				dimacsComment (lineBuffer);
				strcpy (lineBuffer, "Pure literals ");
				i = 0;
			}
		}

	if (i > 0)
		dimacsComment (lineBuffer);

	i=0;
	strcpy (lineBuffer, "Unit Clause Variables ");

	for (v=1;v<=maxVariable;v++)
		if (used[v] && isUnitClause[v])
		{
			sprintf (varBuffer, "%d ",v);
			strcat (lineBuffer, varBuffer);
			i++;
			if (i == 10)
			{
				dimacsComment (lineBuffer);
				strcpy (lineBuffer, "Unit Clause Variables ");
				i = 0;
			}
		}

	if (i > 0)
		dimacsComment (lineBuffer);
}


SubProblemPtr newSubProblem(int maxVariable)
{
	SubProblemPtr result;
	int v;

	result = (SubProblemPtr) checkMalloc (sizeof (SubProblem), "subProblem");
	result->mapVariable = (int *) checkMalloc ((maxVariable+1)*sizeof(int), "mapVariable");
	result->unMapVariable = (int *) checkMalloc ((maxVariable+1)*sizeof(int), "unMapVariable");
	//result->map = (int *) checkMalloc ((maxVariable+1)*sizeof(int), "unPermute");
	//result->unMap = (int *) checkMalloc ((maxVariable+1)*sizeof(int), "unPermute");
	for (v=1;v<=maxVariable;v++)
	{
		result->mapVariable[v] = 0;
		result->unMapVariable[v] = 0;
	}

	return result;
}

void combineMaps (int numOfVariables, int *result, int *m1, int *m2)
{
	int i;
	for (i=1;i<=numOfVariables;i++)
		result[i] = m2[m1[i]];
}


int subProblemCompare (const void *p1, const void *p2)
{
	SubProblemPtr prob1, prob2;
	int result;

	prob1 = * ( (SubProblemPtr *) p1);
	prob2 = * ( (SubProblemPtr *) p2);


	if (prob1->numOfVariables < prob2->numOfVariables)
		result = -1;
	else if (prob1->numOfVariables < prob2->numOfVariables)
		result = 0;
	else
		result = 1;

	return result;
}




void extractSubProblem (PropositionPtr proposition,
						SubProblemPtr subProblem, int startPos, int endPos, int maxVariable,
						int *gmv, int *globalUnMapVariable,
						int *used, int *isSingleton, int *distributedVariables, int *distributed, int *variables)
{
	int i, v, newV;
	int *newSubProblemUnMapVariable;

	newV = 0;
	for (i=startPos;i<=endPos;i++)
	{
		v = variables[i];
		(*distributedVariables)++;
		newV++;
		subProblem->mapVariable[globalUnMapVariable[v]] = newV;
		subProblem->unMapVariable[newV] = globalUnMapVariable[v];
	}

	subProblem->proposition = mapProposition (proposition, subProblem->mapVariable);
	free (subProblem->mapVariable);
	newSubProblemUnMapVariable = (int *) checkMalloc ( (1+newV)*sizeof(int),"");
	for (v=1;v<=newV;v++)
		newSubProblemUnMapVariable[v] = subProblem->unMapVariable[v];
	free (subProblem->unMapVariable);
	subProblem->unMapVariable = newSubProblemUnMapVariable;
	subProblem->numOfVariables = newV;
	subProblem->permute = (int *) checkMalloc ((newV+1)*sizeof(int), "permute");
	subProblem->unPermute = (int *) checkMalloc ((newV+1)*sizeof(int), "unPermute");

}

void partitionProblem (PropositionPtr proposition, int maxVariable, int *globalMapVariable,
						int *globalUnMapVariable, int *used, int *isSingleton, int *isUnitClause, ListPtr subProblems)
{
	int actualVariables, v, v1, v2, v3, distributedVariables,i,c;
	SubProblemPtr subProblem;
	int *distributed, *variables, p,q, startP;
	IntSetPtr *neighbours;
	ClausePtr clause;



	actualVariables = 0;
	for (v=1;v<=maxVariable;v++)
		if (globalMapVariable[v] != 0)
			actualVariables++;

	neighbours = (IntSetPtr *) checkMalloc ( (1+actualVariables)*sizeof (IntSetPtr), "neighbours");
	for (v=1;v<=actualVariables;v++)
		neighbours[v] = newIntSet();

	printf ("c a2\n"); fflush(stdout);

	for (c=0;c<proposition->numOfClauses;c++)
	{
		clause = proposition->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
		{
			v1 = clause->variables[v];
			if (used[v1] && !isSingleton[v1] && !isUnitClause[v1])
			{
				for (v2=v;v2<clause->numOfVariables;v2++)
				{
					v3 = clause->variables[v2];
					if (used[v3] && !isSingleton[v3] && !isUnitClause[v3])
					{
						addToIntSetNoCheck (neighbours[globalMapVariable[v1]],globalMapVariable[v3]);
						addToIntSetNoCheck (neighbours[globalMapVariable[v3]],globalMapVariable[v1]);
					}
				}
			}
		}

	}

	for (v=1;v<=actualVariables;v++)
		removeDuplicatesIntSet (neighbours[v]);


   printf ("c a1\n"); fflush(stdout);

	distributedVariables = 0;
	distributed = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"distributed");
	for (v=1;v<=maxVariable;v++)
		distributed[v] = false;

	variables = (int *) checkMalloc (sizeof(int)*(1+actualVariables), "variables");

	startP = 0;

	while (startP < actualVariables)
	{
		p = startP;
		q = p + 1;

		for (v=1;distributed[v];v++);
		distributed[v] = true;
		variables[startP] = v;

		while (p < q)
		{
			v = variables[p++];
			for (v1=0; v1 < neighbours[v]->numOfElements; v1++)
			{
				i = (int) neighbours[v]->elements[v1];
				if (!distributed[i])
				{
					distributed[i] = true;
					variables[q++] = i;
				}
			}
		}

		if (stringOptions[PERMUTATION_IN] != NULL || stringOptions[PERMUTATION_OUT] != NULL || intOptions[PERMUTATION_METHOD] == 0)
			for (q=0;q<actualVariables;q++)
				variables[q] = q+1;

		subProblem = newSubProblem (maxVariable);
		extractSubProblem (proposition, subProblem, startP, p-1, maxVariable, globalMapVariable,
							globalUnMapVariable, used, isSingleton, &distributedVariables, distributed, variables);
		addListLast (subProblems, subProblem);

		startP = p;
	}
	free (variables);
	free(distributed);
	for (v=1;v<=actualVariables;v++)
		freeIntSet (neighbours[v]);

	free (neighbours);
	freeProposition (proposition);
	sortList (subProblems, subProblemCompare);

}



int solveSubProblem (SubProblemPtr subProblem)
{


	int i;


	if (subProblem->proposition == trueProposition)
	{
		subProblem->assignment = newAssignment();
		printf ("True prop\n");
		setAssignmentNumOfVariables (subProblem->assignment, subProblem->numOfVariables);
	}
	else
	{


		//printf ("Start = %s\n",toStringProposition(subProblem->proposition));
/*
		allPermutations (subProblem);
*/

		//printf ("Original = %s\n", toStringProposition(subProblem->proposition));

		if (stringOptions[PRINT_REDUCED] != NULL)
			printReduced (stringOptions[PRINT_REDUCED], subProblem->proposition);

		sprintf (message, "Sub-problem with %d variables and %d clauses", subProblem->proposition->maxVariable,
		subProblem->proposition->numOfClauses);
		dimacsComment (message);

		findPermutation(subProblem->proposition, subProblem->d, subProblem->initialD, subProblem->r,
							subProblem->numOfVariables,
							subProblem->permute, subProblem->unPermute);

		writeCPUTime ("Ordering finished");
		subProblem->permProposition = mapProposition (subProblem->proposition, subProblem->permute);
		freeProposition (subProblem->proposition);
		writeCPUTime ("mapped");
		globalUnpermute = subProblem->unPermute;

		for (i=1;i<=subProblem->numOfVariables;i++)
		{
			globalUnTransform [i] = subProblem->unMapVariable[subProblem->unPermute[i]];
			globalTransform [subProblem->unMapVariable[subProblem->unPermute[i]]] = i;
		}

		writeCPUTime ("permuted");
		subProblem->permAssignment = ttsCore(subProblem->permProposition);
		//printf ("Permuted = %s\n", toStringProposition(subProblem->permProposition));
		subProblem->assignment = mapAssignment (subProblem->permAssignment, subProblem->numOfVariables, subProblem->unPermute);
		//printf ("Solution = %s\n", toStringAssignment(subProblem->permAssignment));
	}

	return subProblem->assignment != NULL;
}


int solveSubProblems (ListPtr subProblems)
{
	LinkPtr link;
	int satFound;

	satFound = true;

	for (link=subProblems->first;satFound && (link != NULL); link=link->next)
	{
		satFound = solveSubProblem (link->element);
		if (subProblems->length > 1)
		{
			if (satFound)
				dimacsComment ("Satisfiable sub-problem");
			else
				dimacsComment ("UnSatisfiable sub-problem");
		}
	}

	return satFound;
}

void combineSolutions (ListPtr subProblems, AssignmentPtr assignment)
{
	LinkPtr link;
	SubProblemPtr subProblem;

	for (link=subProblems->first;link!=NULL;link=link->next)
	{
		subProblem = (SubProblemPtr) link->element;
		loadMappedAssignment (assignment, subProblem->assignment, subProblem->unMapVariable);
	}
}

void replaceSingletons (AssignmentPtr assignment, int maxVariable, int *globalMapVariable, int *globalUnMapVariable,
						int *used, int *singletonUsage, int *isSingleton, int *unitClauseUsage, int *isUnitClause)
{
	int v;
	for (v=1;v<=maxVariable;v++)
	{
		if (used[v] && isSingleton[v])
		{
			if (singletonUsage[v] == -1)
				setAssignmentValue (assignment, v, true);
			else
				setAssignmentValue (assignment, v, singletonUsage[v]);
		}
		if (used[v] && isUnitClause[v])
		{
			setAssignmentValue (assignment, v, unitClauseUsage[v]);
		}
   }
}

int main (int argc, char *argv[])

{

	int v, maxVariable, variableCount, propositionSize;
	time_t startSeconds, endSeconds;
	double duration;
	AssignmentPtr assignment;
	PropositionPtr proposition;
	ListPtr subProblems;

	time(&startSeconds);
	getOptions (&argc, argv);
	
#ifdef THREADS
	numThreads = intOptions[NUM_THREADS];
#else
	numThreads = 1;
#endif

	srand (intOptions[PERMUTATION_SEED]);
	initCPUTime();
	initMemUtil();
	initClauses();
	initPropositions();
	initSubsetTree();
	//initIntSubsetTree();

	if (argc == 1)
		openDIMACSFiles (NULL);
	else
		openDIMACSFiles (argv[1]);


	proposition = readDIMACSProposition();

 #ifdef PROOF
	 openProofFile();
#endif

   //proposition = make3SAT (proposition);
	//printf (toStringProposition(proposition));

	maxVariable = proposition->maxVariable;


	globalMapVariable = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	globalUnMapVariable = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	globalUnTransform = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	globalTransform = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	isSingleton = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	singletonUsage = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	unitClauseUsage = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	globalUsed = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");
	isUnitClause = (int *) checkMalloc ((maxVariable+1)*sizeof(int),"globalMapVariable");

	for (v=1;v<=maxVariable;v++)
	{
		globalMapVariable[v] = v;
		globalUnMapVariable[v] = v;
	}

	subProblems = newList();
	assignment = newAssignment();
	setAssignmentNumOfVariables (assignment, proposition->maxVariable);


	variableCount = countUsed (proposition, maxVariable, globalUsed);
	propositionSize = getPropositionSize (proposition);
	sprintf (message, "Variables = %d", proposition->maxVariable);
	dimacsComment (message);
	sprintf (message, "Clauses = %d", proposition->numOfClauses);
	dimacsComment (message);
	sprintf (message, "Literals = %d", propositionSize);
	dimacsComment (message);


	tries = 0;
	improvements = 0;
	subsetptrs = 0;


	proposition = removePropositionDuplicates (proposition);
	proposition = removePropositionTautologies (proposition);
	proposition = removePropositionSingletons (proposition, maxVariable, globalMapVariable, globalUnMapVariable,
												globalUsed, singletonUsage, isSingleton, unitClauseUsage, isUnitClause);


#ifdef PROOF
	if (booleanOptions[RPT_FILE])
		noteEndOfUnitClauses();
#endif

	dimacsComment ("Reduced size:");
	sprintf (message, "Variables = %d", variableCount);
	dimacsComment (message);
	sprintf (message, "Clauses = %d", proposition->numOfClauses);
	dimacsComment (message);
	sprintf (message, "Literals = %d", getPropositionSize (proposition));
	dimacsComment (message);
	if (booleanOptions[PRINT_PURE_LITERALS])
	{
		printSingletons (maxVariable, globalUsed, isSingleton, isUnitClause);
	}


	setUsedAssignmentValues (assignment, globalUsed);

	if (proposition == falseProposition)
	{
		assignment = NULL;
		sprintf (message, "Independent sub-problems = 1 (trivially false)");
		dimacsComment (message);
	}
	else if (proposition == trueProposition)
	{
			sprintf (message, "Independent sub-problems = 1 (trivially true)");
			dimacsComment (message);
	}
	else
	{
		partitionProblem (proposition, maxVariable, globalMapVariable, globalUnMapVariable, globalUsed, isSingleton, isUnitClause, subProblems);
		{
			sprintf (message, "Independent sub-problems = %ld", subProblems->length);
			dimacsComment (message);
		}

		if (solveSubProblems (subProblems))
			combineSolutions (subProblems, assignment);
		else
			assignment = NULL;
	}

	if (assignment != NULL)
		replaceSingletons (assignment, maxVariable, globalMapVariable, globalUnMapVariable, globalUsed, singletonUsage, isSingleton,
		unitClauseUsage, isUnitClause);


	duration = getCPUTimeSinceStart();

	sprintf (message, "Partial assignments = %d, improvements = %d  PA/sec = %.2f, subset nodes = %d", tries, improvements,
		duration <= 0?0:tries/duration, subsetptrs)
		;
	dimacsComment (message);


	if (assignment != NULL)
		if (false && !evaluateProposition (proposition,assignment))
		{
			dimacsComment ("Error - incorrect assignment found");
			exit (dimacsUNKNOWN);
		}


	duration = getCPUTimeSinceStart();

	time(&endSeconds);
	
	writeDIMACSSolution (proposition, assignment, duration, difftime (endSeconds,startSeconds), propositionSize,0);
	closeDIMACSFiles();

#ifdef TRACE
	printTraceFile();
#endif

#ifdef PROOF
	closeProofFile (assignment != NULL,globalId);
#endif


	exitDIMACS (assignment);

	return 1; // never reached
}
