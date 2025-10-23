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
		enum VerbosityOptions
		{
			Result,
			Statistics,
			Proof,
			Summary
		}

		class Config
		{
			public Config(string[] args)
			{
				if (args.Length == 0)
					throw new ApplicationException
					(
						"Sol - A satisfiability solver with symmetric reasoning.\n" +
						"Version 1.0.0\n" +
						"Copyright (c) 2009-2011 Michael S Ostapenko. All rights reserved.\n" +
						"\n" +
						"Try -help for more information."
					);
				
				if (args.Length > 4)
					throw new ApplicationException
					(
						"Number of switches must be less or equal to 4."
					);

				for (int i = 0; i < args.Length; ++i)
					Process(args[i]);

				Verify();
				Initialize();
			}

			void Process(string arg)
			{
				if (arg[0] != '-')
				{
					ProcessInput(arg);
					return;
				}
				
				string[] parts = arg.Split
				(
					new char[] { '-', '=' },
					StringSplitOptions.RemoveEmptyEntries
				);
				
				switch (parts[0])
				{
					case "help":
						ProcessHelp();
						break;
					case "input":
						ProcessInput(parts[1]);
						break;
					case "seed":
						ProcessSeed(parts[1]);
						break;
					case "times":
						ProcessTimes(parts[1]);
						break;
					case "verbosity":
						ProcessVerbosity(parts[1]);
						break;
					default:
						ProcessDefault();
						break;
				}
			}

			void ProcessHelp()
			{
				throw new ApplicationException
				(
					"Syntax:               Sol (options) [cnf file|directory]\n" +
					"\n" +
					"Description:          Tests a cnf instance(s) in specified file/directory\n" +
					"                      for satisfiability\n" +
					"\n" +
					"Switches:\n" +
					"\n" +
					"-help                 Displays this usage message.\n" +
					"\n" +
					"-input=<file|directory>\n" +
					"<file|directory>\n" +
					"                      Specifies a file with a cnf instance to solve.\n" +
					"                      If a directory is specified, the solver searches it\n" +
					"                      recursively and solves all cnf instances it can find.\n" +
					"\n" +
					"-seed=<number>        Specifies a seed for a random number generator, which\n" +
					"                      is used to shuffle the input instance. If a seed is not\n" +
					"                      provided, the original instance is solved.\n" +
					"                      If the instance is to be solved multiple times, the solver\n" +
					"                      uses shuffled instances for all runs except for the first;\n" +
					"                      it also uses default seed if it is not provided.\n" +
					"\n" +
					"-times=<number>       Specifies the number of times a cnf instance(s) should be\n" +
					"                      solved.\n" +
					"\n" +
					"-verbosity=<result|statistics|proof|summary>\n" +
					"                      Specifies the contents of solver's output."
				);
			}

			void ProcessInput(string value)
			{
				CheckDuplicates(input);
				input = value;
				CheckInput();
			}

			void CheckInput()
			{
				if (!System.IO.File.Exists(input) &&
					!System.IO.Directory.Exists(input))
					throw new ApplicationException
					(
						"Input values must be valid file or directory names."
					);
			}

			void ProcessSeed(string value)
			{
				CheckDuplicates(seed);
				seed = ParseInteger("Seed", value);
				CheckInteger("Seed", (int)seed, 0, int.MaxValue);
			}
			
			void ProcessTimes(string value)
			{
				CheckDuplicates(times);
				times = ParseInteger("Times", value);
				CheckInteger("Times", (int)times, 1, int.MaxValue);
			}

			void ProcessVerbosity(string value)
			{
				CheckDuplicates(verbosity);
				verbosity = ParseVerbosity(value);
			}

			VerbosityOptions ParseVerbosity(string value)
			{
				switch (value)
				{
					case "result":
						return VerbosityOptions.Result;
					case "statistics":
						return VerbosityOptions.Statistics;
					case "proof":
						return VerbosityOptions.Proof;
					case "summary":
						return VerbosityOptions.Summary;
					default:
						throw new ApplicationException
						(
							"Verbosity values must be from a set { result, statistics, proof, summary }."
						);
				}
			}

			void ProcessDefault()
			{
				throw new ApplicationException
				(
					"Switches must be from a set { help, seed, times, verbosity }."
				);
			}

			int ParseInteger(string name, string value)
			{
				try
				{
					return int.Parse(value);
				}
				catch
				{
					throw new ApplicationException
					(
						name + " values must be integers."
					);
				}
			}

			void CheckInteger(string name, int value, int min, int max)
			{
				if (value < min)
					throw new ApplicationException
					(
						name + " values must be greater or equal to " + min + "."
					);

				if (value > max)
					throw new ApplicationException
					(
						name + " values must be less or equal to " + max + "."
					);
			}

			void CheckDuplicates(object value)
			{
				if (value != null)
					throw new ApplicationException
					(
						"Switches must be defined only once."
					);
			}

			void Verify()
			{
				VerifyInput();
				VerifyVerbosity();
			}

			void VerifyInput()
			{
				if (input == null)
					throw new ApplicationException
					(
						"An input value must be provided."
					);
			}

			void VerifyVerbosity()
			{
				if (verbosity != null &&
					(times > 1 ||
					System.IO.Directory.Exists(input)) &&
					verbosity != VerbosityOptions.Result &&
					verbosity != VerbosityOptions.Summary)
					throw new ApplicationException
					(
						"Verbosity values must be from a set { result, summary },\n" +
						"when (potentially) multiple instances are to be solved."
					);
			}

			void Initialize()
			{
				InitializeSeed();
				InitializeTimes();
				InitializeVerbosity();
			}

			void InitializeSeed()
			{
				if (seed == null)
					seed = -1;
			}

			void InitializeTimes()
			{
				if (times == null)
					times = 1;
			}

			void InitializeVerbosity()
			{
				if (verbosity == null)
				{
					verbosity =
					(
						times > 1 ||
						System.IO.Directory.Exists(input)
					) ?
					VerbosityOptions.Summary :
					VerbosityOptions.Statistics;
				}
			}

			public string Input
			{
				get
				{
					return input;
				}
			}

			public int Seed
			{
				get
				{
					return (int)seed;
				}
			}

			public int Times
			{
				get
				{
					return (int)times;
				}
			}

			public VerbosityOptions Verbosity
			{
				get
				{
					return (VerbosityOptions)verbosity;
				}
			}

			string input;
			int? seed;
			int? times;
			VerbosityOptions? verbosity;
		}
	}
}
