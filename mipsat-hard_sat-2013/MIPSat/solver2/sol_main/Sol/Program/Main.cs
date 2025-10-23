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
using System.IO;

namespace Sol
{
	partial class Program
	{
		static void Main(string[] args)
		{
			try
			{
				//new Program().Execute(new Config(args));
				new Program().ExecuteCompetition(args);
			}
			catch (Exception e)
			{
				Console.WriteLine(e.Message);
			}
		}

		void Initialize(Config config)
		{
			input = config.Input;
			seed = config.Seed;
			times = config.Times;
			verbosity = config.Verbosity;
			
			rng = new Random(seed);
		}

		void Execute(Config config)
		{
			Initialize(config);

			PrintSummaryHead();

			if (File.Exists(input))
				ProcessFile(input);
			else
				ProcessDirectory(input);
		}

		void ProcessDirectory(string path)
		{
			string[] files, subdirectories;
			
			try
			{
				files = Directory.GetFiles(path);
			}
			catch
			{
				Console.WriteLine("Failed to read a directory: " + path);
				return;
			}
			
			foreach (string file in files)
				ProcessFile(file);

			try
			{
				subdirectories = Directory.GetDirectories(path);
			}
			catch
			{
				Console.WriteLine("Failed to read a directory: " + path);
				return;
			}

			foreach (string subdirectory in subdirectories)
				ProcessDirectory(subdirectory);
		}

		void ProcessFile(string path)
		{
			try
			{
				for (int i = 0; i < times; ++i)
					ProcessInstance(path, i);
			}
			catch
			{
				Console.WriteLine("Failed to read/parse a file: " + path);
			}
		}

		void ProcessInstance(string path, int time)
		{
			InitializeStatistics();

			int[][] clauses;
			int vcount = CNF.Read(path, out clauses);
			clauses = CNF.Correct(clauses);
			CNF.Transform(clauses);

			PrintInstanceStatistics(vcount, clauses.Length);

			print_seed = "";

			if (seed != -1 &&
				times == 1)
			{
				current_seed = seed;
				print_seed = "" + current_seed;
				CNF.Shuffle(clauses, 2 * vcount, current_seed);
			}
			else if (time != 0 &&
				times > 1)
			{
				current_seed = rng.Next();
				print_seed = "" + current_seed;
				CNF.Shuffle(clauses, 2 * vcount, current_seed);
			}

			int[] counts, adjacent, offsets;
			CNF.ToGraph(2 * vcount, clauses, out counts, out adjacent, out offsets);

			Morpher morpher = new Morpher();
			morpher.Initialize(counts.Length);

			Mapper mapper = new Mapper();
			Subscribe(mapper);
			PrintMapperHead();
			mapper.Compute(adjacent, offsets, counts, morpher);
			PrintMapperStatistics();

			mapper = null;
			counts = adjacent = offsets = null;

			morpher.Trim(2 * vcount);

			Solver solver = new Solver();
			Subscribe(solver);
			PrintSolverHead();
			bool is_satisfiable = solver.Solve(vcount, clauses, morpher);
			PrintSolverStatistics();

			PrintSummary(path, is_satisfiable ? "SATISFIABLE" : "UNSATISFIABLE");
		}

		void PrintSummaryHead()
		{
			if (verbosity == VerbosityOptions.Summary)
			{
				Console.WriteLine("{0, 15} {1, 12} {2, 18} {3}",
					"Result",
					"Seed",
					"Time",
					"Instance");
			}
		}

		void PrintSummary(string path, string result)
		{
			if (verbosity == VerbosityOptions.Statistics ||
				verbosity == VerbosityOptions.Result)
			{
				Console.WriteLine(result);
			}
			else if (verbosity == VerbosityOptions.Summary)
			{
				Console.WriteLine("{0, 15} {1, 12} {2, 18} {3}",
					result,
					print_seed,
					DateTime.Now.Subtract(start),
					path);
			}
		}

		string input;
		int seed;
		int times;
		VerbosityOptions verbosity;

		int current_seed;
		string print_seed;
		
		Random rng;
	}
}
