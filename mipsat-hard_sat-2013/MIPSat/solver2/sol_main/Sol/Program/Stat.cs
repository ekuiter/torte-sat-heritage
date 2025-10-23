/*************************************************************************
 Copyright 2009-2011, Michael S Ostapenko.  All rights reserved. 
 By using this software the USER indicates that he or she has read, 
 understood and will comply with the following:

 --- Michael S Ostapenko (the "author") hereby grants USER nonexclusive
 permission to use, copy and/or modify this software for internal,
 noncommercial, research purposes only. Any distribution, including
 commercial sale or license, of this software, copies of the software,
 its associated documentation and/or modifications of either is strictly
 prohibited without the prior consent of the author. Title to copyright
 to this software and its associated documentation shall at all times 
 remain with the author. Appropriate copyright notice shall be placed
 on all software copies, and a complete copy of this notice shall be
 included in all copies of the associated documentation. No right is
 granted to use in advertising, publicity or otherwise any trademark,
 service mark, or the name of the author. 

 --- This software and any associated documentation is provided "as is" 

 THE AUTHOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS 
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A 
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR 
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, 
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.  

 The author shall not be liable under any circumstances for any direct,
 indirect, special, incidental, or consequential damages with respect to
 any claim by USER or any third party on account of or arising from the
 use, or inability to use, this software or its associated documentation,
 even if the author has been advised of the possibility of those damages.
*************************************************************************/

using System;

namespace Sol
{
	partial class Program
	{
		void InitializeStatistics()
		{
			start = DateTime.Now;

			decisions_threshold = 128;
			decisions = 0;
			assertions = 0;
			resolvents = 0;
			mappings = 0;
			models = 0;

			refinements_threshold = 128;
			refinements = 0;
			splits = 0;
			automorphisms = 0;
		}
		
		void PrintInstanceStatistics(int variables, int clauses)
		{
			if (verbosity != VerbosityOptions.Statistics)
				return;
			
			Console.WriteLine("Variables:\t{0}", variables);
			Console.WriteLine("Clauses:\t{0}", clauses);
		}

		void RegisterDecision()
		{
			if (++decisions >= decisions_threshold)
			{
				decisions_threshold *= 2;
				PrintSolverStatistics();
			}
		}

		void RegisterAssertion()
		{
			++assertions;
		}

		void RegisterResolvent()
		{
			++resolvents;
		}

		void RegisterMapping()
		{
			++mappings;
		}

		void RegisterModel()
		{
			++models;
		}
	
		void PrintSolverHead()
		{
			if (verbosity != VerbosityOptions.Statistics)
				return;
			
			Console.WriteLine();
			Console.WriteLine("{0, 12} {1, 18} {2, 12} {3, 12} {4, 12} {5, 18}", "Decisions", "Assertions", "Resolvents", "Mappings", "Models", "Time");
		}

		void PrintSolverStatistics()
		{
			if (verbosity != VerbosityOptions.Statistics)
				return;
			
			Console.WriteLine("{0, 12} {1, 18} {2, 12} {3, 12} {4, 12} {5, 18}", decisions, assertions, resolvents, mappings, models, DateTime.Now.Subtract(start));
		}

		void RegisterRefinement()
		{
			if (++refinements >= refinements_threshold)
			{
				refinements_threshold *= 2;
				PrintMapperStatistics();
			}
		}

		void RegisterSplit()
		{
			++splits;
		}

		void RegisterAutomorphism()
		{
			++automorphisms;
		}

		void PrintMapperHead()
		{
			if (verbosity != VerbosityOptions.Statistics)
				return;

			Console.WriteLine();
			Console.WriteLine("{0, 12} {1, 18} {2, 18} {3, 18}", "Refinements", "Splits", "Automorphisms", "Time");
		}

		void PrintMapperStatistics()
		{
			if (verbosity != VerbosityOptions.Statistics)
				return;

			Console.WriteLine("{0, 12} {1, 18} {2, 18} {3, 18}", refinements, splits, automorphisms, DateTime.Now.Subtract(start));
		}

		DateTime start;

		int decisions_threshold;
		int decisions;
		long assertions;
		int resolvents;
		int mappings;
		int models;

		int refinements_threshold;
		int refinements;
		long splits;
		int automorphisms;
	}
}
