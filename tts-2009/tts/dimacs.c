#include "dimacs.h"
#include "assignment.h"
#include "proposition.h"
#include "cputime.h"

#ifdef PROOF
#include "proof.h"
#endif

char *dimacsBaseName;
FILE *dimacsInputFile, *dimacsOutputFile=NULL;
static int amAFilter;
int dimacsVariables, dimacsClauses;
static int nextCh;
static char format[100];

void openDIMACSFiles (char *inputFileName)
{
	char *outputFileName=NULL, *period;

	amAFilter = inputFileName == NULL;
	if (amAFilter)
	{
		dimacsInputFile = stdin;
		dimacsOutputFile = stdout;
		dimacsBaseName = "stdout";
	}
	else
	{
		dimacsBaseName = (char *) strdup (inputFileName);
		period = strchr (dimacsBaseName, (char)'.');
		if (period != NULL)
			*period = '\0';

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
	

	duration = getCPUTimeSinceStart();
				
	fprintf (dimacsOutputFile, "c %s %.3f seconds\n", stage, duration);
}

void writeDIMACSSolution (PropositionPtr proposition, AssignmentPtr assignment, double cpusecs, int elapsedSeconds, int measure1, int measure2)
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


	fprintf (dimacsOutputFile, "c CPU time = %.2f seconds\n", cpusecs);
	fprintf (dimacsOutputFile, "c Elapsed time = %d seconds\n", elapsedSeconds);


}


int getSATLimit(char * name) {
 char * value;

 value = getenv(name);
 if (value == NULL) return(-1);
 return atoi(value);
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
	double cpusecs = getCPUTimeSinceStart();
	if (cpusecs < 0)
		fprintf (dimacsOutputFile, "c Time could not be calculated (no CLK_TCK)\n");
	else
		fprintf (dimacsOutputFile, "c Time = %.2f seconds\n", cpusecs);
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


	for (i=0; i<dimacsClauses; i++)

	{


		clause = readDIMACSClause();
 

		addPropositionClause (result, clause);
#ifdef PROOF
		clause->resId = i+1;
#endif

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

void writeDimacsClause (FILE *f, ClausePtr clause)
{
    int i,v;
    for (i=0;i<clause->numOfVariables;i++)
    {
        v = clause->variables[i];
        if (clause->positive[i])
            fprintf (f, "%d ",v);
        else
            fprintf (f, "%d ", -v);
    }
    fprintf (f, "0\n");
}
void writeDimacsProposition (char *filename, PropositionPtr p)
{
    FILE *f;
    int c;
    ClausePtr clause;
    
    f = fopen (filename, "w");
    fprintf (f, "p cnf %d %d\n", p->maxVariable, p->numOfClauses);
    for (c=0;c<p->numOfClauses;c++)
    {
        clause = p->clauses[c];
        writeDimacsClause (f,clause);
    }
    
    fclose (f);
}

