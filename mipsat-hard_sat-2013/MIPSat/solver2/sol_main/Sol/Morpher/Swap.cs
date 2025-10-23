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
	public partial class Morpher
	{
		void SwapBasePoints(int u1, int u2)
		{
			basis[basis_indices[u1]] = u2;
			basis[basis_indices[u2]] = u1;

			if ((counts[u1] == 0 && counts[u2] == 0) ||
				lists[u1].rc == 0 || lists[u2].rc == 0)
			{
				++basis_indices[u1];
				--basis_indices[u2];
				return;
			}

			t1c = ComputeTransversal(t1, u1, basis_indices[u1]);
			if (!ContainsOwnedBy(t1, t1c, 1, u2) &&
				!ContainsOwnedBy(lists[u2].rs, lists[u2].rc, 0, u1))
			{
				++basis_indices[u1];
				--basis_indices[u2];
				return;
			}

			t2c = ComputeTransversal(t2, u2, basis_indices[u2]);
			ChangeOwner(u1, u2);
			++basis_indices[u1];
			--basis_indices[u2];
			t3c = ComputeTransversal(t3, u2, basis_indices[u2]);
			int n2 = t3c, n1 = (t1c * t2c) / n2;
			CompleteStabilizer(n1);
			RemoveRedundantPermutations(u1, n1);
			RemoveRedundantPermutations(u2, n2);
		}

		bool ContainsOwnedBy(Record[] rs, int rc, int i, int u)
		{
			for (; i < rc; ++i)
				if (owners[rs[i].g] == u)
					return true;
			return false;
		}

		void ChangeOwner(int u1, int u2)
		{
			List l = lists[u2];
			for (int i = 0; i < l.rc; ++i)
				if (owners[l.rs[i].g] == u1)
					owners[l.rs[i].g] = u2;
		}

		void CompleteStabilizer(int n)
		{
			int u1 = t1[0].u, u2 = t2[0].u;
			t3[0].u = u1;
			t3[0].g = -1;
			is_marked[u1] = true;
			t3c = UpdateTransversal(t3, 1, 0, basis_indices[u1]);
			for (int i = 1; n != t3c; ++i)
				if (t1[i].u != u2 && !is_marked[t1[i].u])
				{
					int[] p;
					int a, b;
					p1c = ComputeElement(t1, t1c, i, p1);
					FindPoint(p1, p1c, u2, out a, out b);
					
					if (a != -1)
					{
						int j = FindPoint(t2, t2c, p1[-1 + ((a + 1 == b) ? (a + p1[a]) : b)]);
						if (j == -1)
							continue;
						p2c = ComputeElement(t2, t2c, j, p2);
						p1c = Multiply(p2, p2c, p1, p1c, p3);
						p = p1; p1 = p3; p3 = p;
					}
					
					p = new int[p1c];
					Array.Copy(p1, p, p1c);
					int g = AddStrongGenerator(u1, p);
					int k = t3c;
					t3c = UpdateTransversal(t3, t3c, g);
					t3c = UpdateTransversal(t3, t3c, k, basis_indices[u1]);
				}
			ResetMarks(t3, t3c);
		}

		void RemoveRedundantPermutations(int u, int n)
		{
			gbc = 0;
			List l = lists[u];
			for (int i = 0; i < l.rc; ++i)
				if (owners[l.rs[i].g] == u)
				{
					owners[l.rs[i].g] = ~u;
					gb[gbc++] = l.rs[i].g;
				}
			Algorithm.Sort(gb, 0, gbc, (x, y) => perms[x].Length - perms[y].Length);

			t3[0].u = u;
			t3[0].g = -1;
			t3c = 1;
			is_marked[u] = true;
			for (int i = 0; n != t3c; ++i)
			{
				int j = t3c;
				t3c = UpdateTransversal(t3, t3c, gb[i]);
				if (j != t3c)
				{
					owners[gb[i]] = ~owners[gb[i]];
					t3c = UpdateTransversal(t3, t3c, j, basis_indices[u]);
				}
			}
			ResetMarks(t3, t3c);

			ubc = 0;
			counts[u] = gbc;
			for (int i = 0; i < gbc; ++i)
				if (owners[gb[i]] < 0)
				{
					--counts[u];
					int[] p = perms[gb[i]];
					for (int a = 0; a < p.Length; a += p[a])
						for (int b = a + 1; b < a + p[a]; ++b)
							if (!is_marked[p[b]])
								is_marked[ub[ubc++] = p[b]] = true;
					DestroyGenerator(gb[i]);
				}
			
			for (int i = 0; i < ubc; ++i)
			{
				is_marked[ub[i]] = false;
				l = lists[ub[i]];
				l.rc = RemoveDestroyed(l.rs, l.rc, 0);
				Algorithm.Shrink(ref l.rs, l.rc);
				lists[ub[i]] = l;
			}
		}

		int RemoveDestroyed(Record[] rs, int rc, int i)
		{
			for (int k = rc; --k >= i; )
				if (!is_created[rs[k].g])
					rs[k] = rs[--rc];
			return rc;
		}
	}
}
