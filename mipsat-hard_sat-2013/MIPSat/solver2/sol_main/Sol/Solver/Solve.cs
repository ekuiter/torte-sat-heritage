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
		public bool Solve(int n, int[][] clauses, Morpher morpher)
		{
			Initialize(n, clauses, morpher);
			while (is_consistent)
			{
				int a = Propagate();
				if (a != -1)
				{
					int b = Analyse(a);
					if (b != -1)
					{
						is_consistent = false;
						Process(b);
						break;
					}
					TryRestart();
				}
				else
				{
					int u = Decide();
					if (u == -1)
					{
						Process();
						break;
					}
					Assert(u, -1);
				}
			}
			return is_consistent;
		}

		int Analyse(int a)
		{
			while (levels_count != 0 &&
					a != -1)
			{
				int b = ResolveMinimize(a);
				int[] bs = Map(b);
				int i = Prepare(bs);
				Backtrack(i);
				Learn(bs);
				a = Assert(bs);
			}
			return a;
		}

		void TryRestart()
		{
			if (++no_restart_period > no_restart_threshold)
			{
				no_restart_period = 0;
				no_restart_threshold = no_restart_factor * series.Next();
				Backtrack(0);
			}
		}

		int Decide()
		{
			while (to_decide.Size != 0)
			{
				int u = to_decide.Pop();

				if (is_set[2 * u] || is_set[2 * u + 1])
					continue;

				u = last[u];
				if (activities[u ^ 1] - activities[u] > polarity_threshold)
					u ^= 1;
				++levels_count;

				//OnDecide(new DecideEventArgs(u));
				
				return u;
			}
			return -1;
		}

		void Process()
		{
			int[] model = new int[variables_count];
			Array.Copy(trail, model, variables_count);
			OnModel(new ModelEventArgs(model));
		}

		void Process(int a)
		{
		}
	}
}
