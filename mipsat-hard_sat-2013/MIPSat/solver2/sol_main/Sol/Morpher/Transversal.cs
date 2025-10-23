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
		public Record[] ComputeTransversal(int u, int s)
		{
			t3c = ComputeTransversal(t3, u, s);
			Record[] t = new Record[t3c];
			Array.Copy(t3, t, t3c);
			return t;
		}

		int ComputeTransversal(Record[] t, int u, int s)
		{
			t[0].u = u;
			t[0].g = -1;
			is_marked[u] = true;
			int tc = UpdateTransversal(t, 1, 0, s);
			ResetMarks(t, tc);
			return tc;
		}

		int UpdateTransversal(Record[] t, int tc, int i, int s)
		{
			for (; i < tc; ++i)
			{
				List l = lists[t[i].u];
				for (int j = 0; j < l.rc; ++j)
				{
					Record r = l.rs[j];
					if (!is_marked[r.u] && owners[r.g] >= 0 && basis_indices[owners[r.g]] >= s)
					{
						is_marked[r.u] = true;
						t[tc++] = r;
					}
				}
			}
			return tc;
		}

		int UpdateTransversal(Record[] t, int tc, int g)
		{
			int[] p = perms[g];
			for (int a = 0; a < p.Length; a += p[a])
				for (int b = a + 1; b < a + p[a]; ++b)
				{
					int c = 1 + ((b + 1 == a + p[a]) ? a : b);
					if (is_marked[p[b]] && !is_marked[p[c]])
					{
						is_marked[p[c]] = true;
						t[tc].u = p[c];
						t[tc++].g = g;
					}
				}
			return tc;
		}
	}
}
