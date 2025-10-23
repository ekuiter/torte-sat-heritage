#ifdef VISUALSTUDIO
#else
#include <strings.h>
#endif

#include <stdio.h>

#include "options.h"
#include "memutil.h"
#include "dimacs.h"



char  **booleanOptionTags, **stringOptionTags, **intOptionTags;

int getSATlimit(char * name)
{
	char * value;
	value = getenv(name);
	if (value == NULL)
		return(-1);
	return atoi(value);
}

void initOptions()
{
	
	booleanOptionTags = (char **) checkMalloc (MAX_BOOLEAN_OPTIONS*sizeof (char *),"");
	booleanOptions = (int *) checkMalloc (MAX_BOOLEAN_OPTIONS*sizeof(int),"");

	booleanOptionTags[FIX_ORDER] = "fix-order";
	booleanOptions[FIX_ORDER] = 0;
	booleanOptionTags[SOLVE] = "solve";
	booleanOptions[SOLVE] = 1;
	booleanOptionTags[EXPONENT] = "exponent";
	booleanOptions[EXPONENT] = 0;
	booleanOptionTags[PROOF_CLAUSES] = "proof-clauses";
	booleanOptions[PROOF_CLAUSES] = 1;
	booleanOptionTags[HELP] = "help";
	booleanOptions[HELP] = 0;
	booleanOptionTags[PRINT_PURE_LITERALS] = "print-pure-literals";
	booleanOptions[PRINT_PURE_LITERALS] = 0;
	booleanOptionTags[PRINT_LEVEL_COUNTS] = "print-level-counts";
	booleanOptions[PRINT_LEVEL_COUNTS] = 0;
	booleanOptionTags[PRINT_ARRANGEMENT] = "print-arrangement";
	booleanOptions[PRINT_ARRANGEMENT] = 0;
	booleanOptionTags[MERGE_CLAUSES] = "merge-clauses";
	booleanOptions[MERGE_CLAUSES] = 0;
	booleanOptionTags[PRE_FILL] = "pre-fill";
	booleanOptions[PRE_FILL] = 0;
	booleanOptionTags[TRY_BRANCH] = "try-branch";
	booleanOptions[TRY_BRANCH] = 0;
	booleanOptionTags[DYNAMIC_SINGLETONS] = "dynamic-singletons";
	booleanOptions[DYNAMIC_SINGLETONS] = 0;
	booleanOptionTags[PRINT_TRACE] = "print-trace";
	booleanOptions[PRINT_TRACE] = 0;
	booleanOptionTags[RPT_FILE] = "rpt-file";
	booleanOptions[RPT_FILE] = 0;
	booleanOptionTags[PRT_FILE] = "prt-file";
	booleanOptions[PRT_FILE] = 0;
	booleanOptionTags[TRACE_RESOLUTIONS] = "trace-res";
	booleanOptions[TRACE_RESOLUTIONS] = 0;
	booleanOptionTags[FILL_UP] = "fill-up";
	booleanOptions[FILL_UP] = 0;
	booleanOptionTags[UNIT_CLAUSE_PROPOGATION] = "ucp";
	booleanOptions[UNIT_CLAUSE_PROPOGATION] = 0;
	booleanOptionTags[UNIQUE_PARENT] = "unique-parent";
	booleanOptions[UNIQUE_PARENT] = 0;
	booleanOptionTags[RECURSIVE] = "recursive";
	booleanOptions[RECURSIVE] = 1;
	booleanOptionTags[WEIGHTS] = "weights";
	booleanOptions[WEIGHTS] = 1;
	booleanOptionTags[SKIP_SAVING] = "skip-saving";
	booleanOptions[SKIP_SAVING] = 1;


	intOptionTags = (char **) checkMalloc (MAX_INT_OPTIONS*sizeof (char *),"");
	intOptions = (int *) checkMalloc (MAX_INT_OPTIONS*sizeof(int),"");

	intOptionTags[PERMUTATION_SEED] = "perm-seed";
	intOptions[PERMUTATION_SEED] = 1;
	intOptionTags[SIMULATED_ANNEALING_DIVISOR] = "sa-divisor";
	intOptions[SIMULATED_ANNEALING_DIVISOR] = 5000;
	intOptionTags[PERMUTATION_METHOD] = "perm-method";
	intOptions[PERMUTATION_METHOD] = 100;
	intOptionTags[PROOF_FORMAT] = "proof-format";
	intOptions[PROOF_FORMAT] = 1;
	intOptionTags[REHEAT_COUNT] = "reheat-count";
	intOptions[REHEAT_COUNT] = 1;
	intOptionTags[ASSIGNMENT_LIMIT] = "assign-limit";
	intOptions[ASSIGNMENT_LIMIT] = -1;
	intOptionTags[CPU_LIMIT] = "cpu-limit";
	intOptions[CPU_LIMIT] = -1;
	intOptionTags[SET_SIZE_INCREMENT] = "set-size-increment";
	intOptions[SET_SIZE_INCREMENT] = 10;
	intOptionTags[ADD_COUNT] = "add-count";
	intOptions[ADD_COUNT] = 1;
	intOptionTags[INCREASE_REQUESTS] = "increase-requests";
	intOptions[INCREASE_REQUESTS] = 100000;
	intOptionTags[INITIAL_SET_SIZE] = "initial-set-size";
	intOptions[INITIAL_SET_SIZE] = 1000;
	intOptionTags[NUM_THREADS] = "num-threads";
	intOptions[NUM_THREADS] = 1;
	intOptionTags[FINAL_SET_SIZE] = "final-set-size";
	intOptions[FINAL_SET_SIZE] = 20;
	intOptionTags[SAT_RAM_SIZE] = "sat-ram-size";
	intOptions[SAT_RAM_SIZE] = -1;
	intOptionTags[SET_COUNT] = "set-count";
	intOptions[SET_COUNT] = 500;
	intOptionTags[SET_BASE] = "set-base";
	intOptions[SET_BASE] = 10000;
	

	stringOptionTags = (char **) checkMalloc (MAX_STRING_OPTIONS*sizeof (char *),"");
	stringOptions = (char **) checkMalloc (MAX_STRING_OPTIONS*sizeof(char *),"");

	stringOptionTags[PERMUTATION_IN] = "perm-in";
	stringOptions[PERMUTATION_IN] = NULL;
	stringOptionTags[PERMUTATION_OUT] = "perm-out";
	stringOptions[PERMUTATION_OUT] = NULL;
	stringOptionTags[PROOF_LENGTH_FILE] = "proof-length";
	stringOptions[PROOF_LENGTH_FILE] = NULL;
	stringOptionTags[PRINT_REDUCED] = "print-reduced";
	stringOptions[PRINT_REDUCED] = NULL;
	stringOptionTags[DOT_FILE] = "dot-file";
	stringOptions[DOT_FILE] = NULL;
	stringOptionTags[STATIC_DOT_FILE] = "static-dot-file";
	stringOptions[STATIC_DOT_FILE] = NULL;


}

int checkBooleanOption (char *option)
{
	int o;

	for (o=0;o<MAX_BOOLEAN_OPTIONS;o++)
	{
		if (strcmp (booleanOptionTags[o], &option[1]) == 0)
		{
			booleanOptions[o] = 1;
			return 1;
		}
	}

	if (strlen (option) > 3 && option[1] == 'n' && option[2] == 'o')
		for (o=0;o<MAX_BOOLEAN_OPTIONS;o++)
		{
			if (strcmp (booleanOptionTags[o], &option[3]) == 0)
			{
				booleanOptions[o] = 0;
				return 1;
			}
		}
	return 0;
}

int checkStringOption (char *option, char *val)
{
	int o;

	for (o=0;o<MAX_STRING_OPTIONS;o++)
	{
		if (strcmp (stringOptionTags[o], &option[1]) == 0)
		{
			stringOptions[o] = val;
			return 1;
		}
	}

	return 0;
}

int checkIntOption (char *option, char *val)
{
	int o;

	for (o=0;o<MAX_INT_OPTIONS;o++)
	{
		if (strcmp (intOptionTags[o], &option[1]) == 0)
		{
			intOptions[o] = atoi (val);
			return 1;
		}
	}

	return 0;
}

void printHelp()
{
	int i;

	for (i=0;i<MAX_BOOLEAN_OPTIONS;i++)
		fprintf (stdout, "-[no]%s\n", booleanOptionTags[i]);
	fprintf (stdout, "\n");

	for (i=0;i<MAX_INT_OPTIONS;i++)
		fprintf (stdout, "-%s int\n", intOptionTags[i]);
	fprintf (stdout, "\n");

	for (i=0;i<MAX_STRING_OPTIONS;i++)
		fprintf (stdout, "-%s string\n", stringOptionTags[i]);
	fprintf (stdout, "\n");

}

void getOptions (int *argc, char * argv[])
{
	char **newArgv;
	int oldp, newp, p, pos;
	char commandLine[1000];

	strcpy (commandLine, "Command: ");
	pos=strlen(commandLine);
	for (p=0; p<*argc && pos+1+strlen(argv[p])<1000;p++)
	{
		strcat (commandLine, argv[p]);
		strcat (commandLine, " ");
		pos += 1 + strlen (argv[p]);
	}
	dimacsComment (commandLine);


	initOptions();

	newArgv = (char **) checkMalloc ( (*argc)*sizeof(char *),"");

	oldp = 0;
	newp = 0;

	newArgv[newp++] = argv[oldp++];

	while (oldp < *argc)
	{
		if (strlen (argv[oldp]) == 0 || argv[oldp][0] != '-')
			newArgv[newp++] = argv[oldp++];
		else
		{
			if (checkBooleanOption (argv[oldp]))
				oldp++;
			else
				if (oldp+1 < *argc && checkStringOption (argv[oldp],argv[oldp+1]))
					oldp += 2;
			else
				if (oldp+1 < *argc && checkIntOption (argv[oldp],argv[oldp+1]))
					oldp += 2;
			else
			{
				fprintf (stderr, "Unrecognised option %s\n", argv[oldp]);
				oldp++;
			}
		}

	}

	for (p=0;p<newp;p++)
		argv[p] = newArgv[p];
	*argc = newp;

	free (newArgv);

	if (booleanOptions[HELP])
	{
		printHelp();
		exit(1);
	}
	
	if (intOptions[CPU_LIMIT] == -1)
	{
		intOptions[CPU_LIMIT] = getSATLimit("SATTIMEOUT");
		if (intOptions[CPU_LIMIT] != -1)
		{
			sprintf (commandLine,"Using SATTIMEOUT of %d",intOptions[CPU_LIMIT]);
			dimacsComment (commandLine);
		}
	}

	if (intOptions[SAT_RAM_SIZE] == -1)
	{
		intOptions[SAT_RAM_SIZE] = getSATLimit("SATRAM");
		if (intOptions[SAT_RAM_SIZE] != -1)
		{
			sprintf (commandLine,"Using SATRAM of %d",intOptions[SAT_RAM_SIZE]);
			dimacsComment (commandLine);
		}
	}
	
#ifdef PROOF
	booleanOptions[RPT_FILE] = true;
	booleanOptions[UNIQUE_PARENT] = true;
#endif
}
