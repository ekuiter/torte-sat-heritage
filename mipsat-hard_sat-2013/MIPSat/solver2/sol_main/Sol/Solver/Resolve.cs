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
		int Resolve(int a)
		{
			activity_increment *= activity_multiplier;
			int i = 0, j = 0, k = trail_count - 1, l = 1;
			do
			{
				int[] us = bodies[a];
				for (; j < us.Length; ++j)
				{
					Register(us[j]);
					int u = us[j] >> 1;

					if (is_marked[u] || levels[u] == 0)
						continue;
					is_marked[u] = true;

					if (levels[u] == levels_count)
						++i;
					else
						buffer[l++] = us[j];
				}
				for (j = 1; !is_marked[trail[k] >> 1]; --k) ;
				a = reasons[trail[k] >> 1];
				is_marked[trail[k] >> 1] = false;
			}
			while (--i != 0);
			buffer[0] = trail[k] ^ 1;

			for (i = 1; i < l; ++i)
				is_marked[buffer[i] >> 1] = false;

			int[] vs = new int[l];
			Array.Copy(buffer, vs, l);

			//OnResolve(new ResolveEventArgs((int[])vs.Clone()));

			return CreateClause(vs);
		}

		int ResolveMinimize(int a)
		{
			activity_increment *= activity_multiplier;
			int i = 0, j = 0, k = trail_count - 1, l = 1;
			do
			{
				int[] us = bodies[a];
				for (; j < us.Length; ++j)
				{
					Register(us[j]);
					int u = us[j] >> 1;
					
					if (is_marked[u] || levels[u] == 0)
						continue;
					is_marked[u] = true;

					if (levels[u] == levels_count)
						++i;
					else
						buffer[l++] = us[j];
				}
				for (j = 1; !is_marked[trail[k] >> 1]; --k) ;
				a = reasons[trail[k] >> 1];
				is_marked[trail[k] >> 1] = false;
			}
			while (--i != 0);
			buffer[0] = trail[k] ^ 1;

			k = l;
			for (i = 1; i < k; ++i)
				is_used[levels[buffer[i] >> 1]] = true;

			for (i = 1, j = 1; i < k; ++i)
				if (reasons[buffer[i] >> 1] == -1 || !IsRedundant(buffer[i], ref l))
				{
					int u = buffer[j];
					buffer[j++] = buffer[i];
					buffer[i] = u;
				}

			for (i = 1; i < k; ++i)
				is_used[levels[buffer[i] >> 1]] = false;

			for (i = 1; i < l; ++i)
				is_marked[buffer[i] >> 1] = false;

			int[] vs = new int[j];
			Array.Copy(buffer, vs, j);

			//OnResolve(new ResolveEventArgs((int[])vs.Clone()));

			return CreateClause(vs);
		}

		bool IsRedundant(int u, ref int begin)
		{
			int end = begin;
			buffer[end++] = u;

			for (int i = begin; i < end; ++i)
			{
				int[] us = bodies[reasons[buffer[i] >> 1]];
				for (int j = 1; j < us.Length; ++j)
				{
					int v = us[j] >> 1;
					if (!is_marked[v] && levels[v] > 0)
					{
						if (reasons[v] != -1 && is_used[levels[v]])
						{
							is_marked[v] = true;
							buffer[end++] = us[j];
						}
						else
						{
							for (int k = begin + 1; k < end; ++k)
								is_marked[buffer[k] >> 1] = false;
							return false;
						}
					}
				}
			}

			buffer[begin] = buffer[--end];
			begin = end;
			return true;
		}

		void Register(int u)
		{
			if ((activities[u] += activity_increment) > activity_threshold)
			{
				activity_increment *= activity_factor;
				for (int v = 0; v < 2 * variables_count; ++v)
					activities[v] *= activity_factor;
			}

			if (to_decide.Contains(u >> 1))
				to_decide.Update(u >> 1);
		}
	}
}
