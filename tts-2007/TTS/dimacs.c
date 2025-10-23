#include "dimacs.h"
#include "assignment.h"
#include "proposition.h"
#include "cputime.h"

FILE *dimacsInputFile, *dimacsOutputFile=NULL;
static int amAFilter;
int dimacsVariables, dimacsClauses;
static int nextCh;
static char format[100];

void openDIMACSFiles (char *inputFileName)
{
	char *baseFileName, *outputFileName=NULL, *period;

	amAFilter = inputFileName == NULL;
	if (amAFilter)
	{
		dimacsInputFile = stdin;
		dimacsOutputFile = stdout;
	}
	else
	{
		baseFileName = (char *) strdup (inputFileName);
		period = strchr (baseFileName, (char)'.');

/*
		if (period != NULL)
			*period = '\0';
		outputFileName = (cha strdup (baseFileName);
		outputFileName = realloc (outputFileName, (strlen(outputFileName)+5)*sizeof(char));
		strcat (outputFileName, ".out");
*/
		dimacsInputFile = fopen (inputFileName,"r");
		if (dimacsInputFile == NULL)
		{
			fprintf (stderr, "Could not open <%s> for reading\n", inputFileName);
			exit (0);
		}
		else
		{
/*
			dimacsOutputFile = fopen (outputFileName,"w");
*/
			dimacsOutputFile = stdout;
			if (dimacsOutputFile == NULL)
			{
				fclose (dimacsInputFile);
				fprintf (stderr, "Could not open <%s> for writing\n", outputFileName);
				exit (0);
			}
		}
	}
}

void writeCPUTime (char *stage)
{
	double duration;
	
	if (CLK_TCK == -1)
		duration = -1;
	else
		duration = getCPUTimeSinceStart();
				
	if (duration != -1)
		fprintf (dimacsOutputFile, "c %s %.3f seconds\n", stage, duration);
}

void writeDIMACSSolution (PropositionPtr proposition, AssignmentPtr assignment, double cpusecs, int measure1, int measure2)
{
	int i, count;
	char *solution;
	solution = (assignment == NULL) ? "UNSATISFIABLE" : "SATISFIABLE";

	fprintf (dimacsOutputFile, "s %s\n", solution);


	if (assignment != NULL)
	{
		count = 0;
		fprintf (dimacsOutputFile, "v ");
		for (i=1; i<=assignment->numOfVariables; i++)
		{

			if (count == 10)
			{
				count = 0;
				fprintf (dimacsOutputFile,"\nv ");
			}
			
			switch (assignment->values[i])
			{
				case 1:
					fprintf (dimacsOutputFile, "%d ", i);
					count++;
					break;
				
				case 0:
					fprintf (dimacsOutputFile, "%d ", -i);
					count++;
					break;
				
				case -1:
					break;  
			}                
		}
		fprintf (dimacsOutputFile, "0\n");
	}


	if (cpusecs < 0)
		fprintf (dimacsOutputFile, "c Time could not be calculated (no CLK_TCK)\n");
	else
		fprintf (dimacsOutputFile, "c Time = %.2f seconds\n", cpusecs);


}



void closeDIMACSFiles ()
{
	if (!amAFilter)
	{
		fclose (dimacsInputFile);
	}
}

void abortDIMACS()
{
	fprintf (dimacsOutputFile, "s %s\n", "UNKNOWN");
	exit (dimacsUNKNOWN);
}

void exitDIMACS(AssignmentPtr assignment)
{
	if (assignment == NULL)
		exit (dimacsUNSAT);
	else
		exit (dimacsSAT);
}

void readDIMACSPreamble()
{
	nextCh = fgetc (dimacsInputFile);
	while (nextCh == 'c' || nextCh == '\n')
	{
		while (nextCh != EOF && nextCh != '\n')
			nextCh = fgetc (dimacsInputFile);
		nextCh = fgetc (dimacsInputFile);
	}
	if (nextCh == 'p')
	{
		fscanf (dimacsInputFile, "%s%d%d", format, &dimacsVariables, &dimacsClauses);
		assert (strcmp (format,"cnf") == 0);
	}
}

int readDIMACSVariable()
{
	int result, status;
	status = fscanf (dimacsInputFile, "%d", &result);
	if (status != 1)
		result = 0;
	return result;
}

ClausePtr readDIMACSClause ()

{

	ClausePtr result;

	int variable;

	result = newClause();


	while ( (variable = readDIMACSVariable())  != 0)

		addClauseVariable (result, variable);
		
	return result;

}

PropositionPtr readDIMACSProposition ()

{

	PropositionPtr result;

	ClausePtr clause;

	int i;



	readDIMACSPreamble();



	result = newProposition(1);

#ifdef PROOF
	noteNumOfRealClauses (dimacsClauses);
#endif

	for (i=0; i<dimacsClauses; i++)

	{


		clause = readDIMACSClause();
		
#ifdef PROOF
		noteRealClause (clause, i+1);
		clause->realClauseNum = i+1;
#endif

		addPropositionClause (result, clause);

	}

	if (dimacsClauses == 0)
		result = trueProposition;

	return result;

}

void dimacsComment (char *s)
{
	if (dimacsOutputFile == NULL)
		dimacsOutputFile = stdout;
	fprintf (dimacsOutputFile, "c %s\n", s);
}
