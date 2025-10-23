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
	public partial class Solver
	{
		void Initialize(int n, int[][] clauses, Morpher morpher)
		{
			to_decide = new NHeap<int>(
				(x, y) => Math.Sign((activities[2 * y] + activities[2 * y + 1]) -
									(activities[2 * x] + activities[2 * x + 1])),
				x => to_decide_indices[x], (x, y) => to_decide_indices[x] = y);
			to_erase = new Vector<int>();
			series = new Luby();

			is_consistent = true;
			levels_count = 0;
			trail_index = 0;
			trail_count = 0;
			to_erase_total = 0;
			to_erase_power = 1.05F;
			activity_increment = 1.0F;
			activity_multiplier = 1.0F / 0.95F;
			activity_threshold = 1e100;
			activity_factor = 1e-100;
			polarity_threshold = 32.0F;
			no_restart_period = 0;
			no_restart_factor = 256;
			no_restart_threshold = 256;

			int m = Math.Max(n, variables_count);
			ReserveVariables(n);
			for (int u = 0; u < m; ++u)
				ResetVariable(u);

			for (int a = 0; a < clauses_limit; ++a)
				if (is_clause_created[a])
					DestroyClause(a);

			for (int u = 0; u < n; ++u)
				to_decide.Push(u);

			int[] closure = morpher.ComputeClosure(0);
			for (int i = 0; i < closure.Length; ++i)
				is_in_group[closure[i]] = true;

			for (int i = 0; i < clauses.Length; ++i)
			{
				int a = CreateClause(clauses[i]);
				int[] us = bodies[a];
				if (us.Length == 0)
				{
					is_consistent = false;
					return;
				}
				if (us.Length == 1)
				{
					if (is_set[us[0] ^ 1])
					{
						is_consistent = false;
						return;
					}
					if (!is_set[us[0]])
						Assert(us[0], a);
				}
				else
					Attach(a);
			}
			
			this.morpher = morpher;
		}

		void ResetVariable(int u)
		{
			watches[2 * u].rc = 0;
			watches[2 * u].rs = new Record[2];
			watches[2 * u + 1].rc = 0;
			watches[2 * u + 1].rs = new Record[2];
			is_set[2 * u] = false;
			is_set[2 * u + 1] = false;
			is_in_group[2 * u] = false;
			is_in_group[2 * u + 1] = false;
			activities[2 * u] = 0;
			activities[2 * u + 1] = 0;

			levels[u] = -1;
			reasons[u] = -1;
			last[u] = 2 * u;
			is_marked[u] = false;
			is_used[u] = false;
			to_decide_indices[u] = -1;
		}

		void ReserveVariables(int n)
		{
			variables_count = n;
			if (variables_limit == 0 || variables_count > variables_limit)
			{
				if (variables_limit == 0) variables_limit = 64;
				while (variables_count > (variables_limit *= 2)) ;

				Array.Resize(ref watches, 2 * variables_limit);
				Array.Resize(ref is_set, 2 * variables_limit);
				Array.Resize(ref is_in_group, 2 * variables_limit);
				Array.Resize(ref activities, 2 * variables_limit);

				Array.Resize(ref levels, variables_limit);
				Array.Resize(ref reasons, variables_limit);
				Array.Resize(ref last, variables_limit);
				Array.Resize(ref is_marked, variables_limit);
				Array.Resize(ref is_used, variables_limit);
				Array.Resize(ref to_decide_indices, variables_limit);

				Array.Resize(ref trail, variables_limit);
				Array.Resize(ref buffer, variables_limit);
			}
		}

		int CreateClause(int[] us)
		{
			int a = AllocateClause();
			is_clause_created[a] = true;
			
			bodies[a] = us;
			locks[a] = 0;
			
			return a;
		}

		void DestroyClause(int a)
		{
			if (!is_clause_created[a])
				throw new ApplicationException("Clause destruction failed.");
			
			bodies[a] = null;

			is_clause_created[a] = false;
			DeallocateClause(a);
		}

		int AllocateClause()
		{
			if (available_clauses_count == 0)
			{
				clauses_limit = (clauses_limit == 0) ? 128 : 2 * clauses_limit;
				
				Array.Resize(ref clauses, clauses_limit);
				Array.Resize(ref is_clause_created, clauses_limit);

				for (int i = allocated_clauses_count; i < clauses_limit; ++i)
					clauses[available_clauses_count++] = i;

				Array.Resize(ref bodies, clauses_limit);
				Array.Resize(ref locks, clauses_limit);
			}
			++allocated_clauses_count;
			return clauses[--available_clauses_count];
		}

		void DeallocateClause(int a)
		{
			clauses[available_clauses_count++] = a;
			--allocated_clauses_count;
		}

		struct Record
		{
			public int u;
			public int a;
		}

		struct List
		{
			public int rc;
			public Record[] rs;
		}

		bool is_consistent;

		int variables_count;
		int variables_limit;

		List[] watches;
		bool[] is_set;
		bool[] is_in_group;
		double[] activities;

		int levels_count;
		int[] levels;
		int[] reasons;
		int[] last;
		bool[] is_marked;
		bool[] is_used;
		int[] to_decide_indices;

		int trail_index;
		int trail_count;
		int[] trail;
		
		int[] buffer;

		int available_clauses_count;
		int allocated_clauses_count;
		int clauses_limit;
		int[] clauses;
		bool[] is_clause_created;
		
		int[][] bodies;
		int[] locks;

		double activity_increment;
		double activity_multiplier;
		double activity_threshold;
		double activity_factor;
		double polarity_threshold;
		NHeap<int> to_decide;

		long to_erase_total;
		double to_erase_power;
		Vector<int> to_erase;

		int no_restart_period;
		int no_restart_factor;
		int no_restart_threshold;
		Luby series;

		Morpher morpher;
	}
}
