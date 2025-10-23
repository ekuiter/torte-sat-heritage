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
	public partial class Mapper
	{
		bool IsAutomorphism()
		{
			bool is_automorphism = true;
			for (int a = 0; a < degree; a = next1[a])
				if (next1[a] - a == 1 && vertices1[a] != vertices2[a])
				{
					for (int i = offsets[vertices1[a]], j = offsets[vertices1[a] + 1]; i < j; ++i)
					{
						int b = cells1[adjacent[i]];
						counts[(next1[b] - b == 1) ? vertices2[b] : adjacent[i]] = 1;
					}
					
					for (int i = offsets[vertices2[a]], j = offsets[vertices2[a] + 1]; i < j; ++i)
						is_automorphism &= (counts[adjacent[i]] == 1);
					
					for (int i = offsets[vertices1[a]], j = offsets[vertices1[a] + 1]; i < j; ++i)
					{
						int b = cells1[adjacent[i]];
						counts[(next1[b] - b == 1) ? vertices2[b] : adjacent[i]] = 0;
					}
					
					if (!is_automorphism)
						break;
				}
			return is_automorphism;
		}

		int[] ComputePermutation()
		{
			int c = 0;
			for (int a = 0; a < degree; a = next1[a])
				if (next1[a] - a == 1 && vertices1[a] != vertices2[a] && !is_marked[a])
				{
					int b = c++;
					for (int u = vertices1[a]; !is_marked[a]; u = vertices2[a], a = cells1[u])
					{
						is_marked[a] = true;
						perm[c++] = u;
						representatives[u] = -1;
					}
					perm[b] = c - b;
				}

			for (int a = 0; a < degree; a = next1[a])
				if (is_marked[a])
					is_marked[a] = false;
			
			int[] p = new int[c];
			Array.Copy(perm, p, c);
			return p;
		}

		void Stabilize(int[] p)
		{
			for (int i = 0; i < orbits_count; ++i)
				representatives[orbits[i]] = -1;
			orbits_count = 0;

			if (p != null)
				morpher.AddStrongGenerator(vertices1[trail[base_index][0][1]], p);
			else if (base_index == trail.Size)
				--base_index;

			prefix_count = 0;
			if (base_index > 0)
			{
				for (int i = base_index + 1; i < trail.Size; ++i)
				{
					int u = vertices1[trail[i][0][1]];
					if (representatives[u] == -1)
						prefix[prefix_count++] = u;
				}
				morpher.FixInBase(base_index, prefix, prefix_count, false);
			}
		}

		int OR(int u)
		{
			if (representatives[u] == -1)
			{
				Morpher.Record[] t = morpher.ComputeTransversal(u, base_index - 1 + prefix_count);
				int v = t[0].u;
				for (int i = 0; i < t.Length; ++i)
					representatives[orbits[orbits_count++] = t[i].u] = v;
			}
			return representatives[u];
		}
	}
}
