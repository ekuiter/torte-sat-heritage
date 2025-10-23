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
		void ExecuteCompetition(string[] args)
		{
			if (args.Length != 1)
				throw new ApplicationException
				(
					"Sol - A satisfiability solver with symmetric reasoning.\n" +
					"Version 1.0.0 (SAT competition build: main track)\n" +
					//"Version 1.0.0 (SAT competition build: certified UNSAT track)\n" +
					"Copyright (c) 2009-2011 Michael S Ostapenko. All rights reserved.\n" +
					"\n" +
					"Syntax: Sol [cnf file]"
				);

			new Program().ExecuteMainTrack(args[0]);
			//new Program().ExecuteUnsatTrack(args[0]);
		}

		void ExecuteMainTrack(string path)
		{
			int[][] clauses;
			int vcount = CNF.Read(path, out clauses);
			clauses = CNF.Correct(clauses);
			CNF.Transform(clauses);

			int[] counts, adjacent, offsets;
			CNF.ToGraph(2 * vcount, clauses, out counts, out adjacent, out offsets);

			Morpher morpher = new Morpher();
			morpher.Initialize(counts.Length);

			Mapper mapper = new Mapper();
			mapper.Compute(adjacent, offsets, counts, morpher);

			mapper = null;
			counts = adjacent = offsets = null;

			morpher.Trim(2 * vcount);

			Solver solver = new Solver();
			solver.ModelEvent += new EventHandler<Solver.ModelEventArgs>(MainTrackModelHandler);
			bool is_satisfiable = solver.Solve(vcount, clauses, morpher);

			if (is_satisfiable)
			{
				Console.WriteLine("s SATISFIABLE");
				Console.WriteLine("v " + ToString(main_track_model) + " 0");
			}
			else
				Console.WriteLine("s UNSATISFIABLE");
		}

		void MainTrackModelHandler(object sender, Solver.ModelEventArgs e)
		{
			main_track_model = e.Model;
			CNF.B(main_track_model);
		}

		int[] main_track_model;

		void ExecuteUnsatTrack(string path)
		{
			int[][] clauses;
			int vcount = CNF.Read(path, out clauses);
			clauses = CNF.Correct(clauses);
			CNF.Transform(clauses);

			int[] counts, adjacent, offsets;
			CNF.ToGraph(2 * vcount, clauses, out counts, out adjacent, out offsets);

			Morpher morpher = new Morpher();
			morpher.Initialize(counts.Length);

			Mapper mapper = new Mapper();
			mapper.Compute(adjacent, offsets, counts, morpher);

			mapper = null;
			counts = adjacent = offsets = null;

			morpher.Trim(2 * vcount);

			Solver solver = new Solver();
			solver.ResolveEvent += new EventHandler<Solver.ResolveEventArgs>(UnsatTrackResolveHandler);
			solver.MapEvent += new EventHandler<Solver.MapEventArgs>(UnsatTrackMapHandler);
			bool is_satisfiable = solver.Solve(vcount, clauses, morpher);

			if (!is_satisfiable)
				Console.WriteLine("0");
		}

		void UnsatTrackResolveHandler(object sender, Solver.ResolveEventArgs e)
		{
			int[] resolvent = e.Resolvent;
			CNF.B(resolvent);
			Console.WriteLine(ToString(resolvent) + " 0");
		}

		void UnsatTrackMapHandler(object sender, Solver.MapEventArgs e)
		{
			int[] mapping = e.Mapping;
			CNF.BP(mapping);
			Console.WriteLine(Morpher.ToString(mapping));
		}
	}
}
