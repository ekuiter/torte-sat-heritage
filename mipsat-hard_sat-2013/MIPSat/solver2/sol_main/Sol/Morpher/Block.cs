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
		public void ComputeMinimalBlockSystem(int[] o, int i, int s, int[] blocks)
		{
			ubc = 0;
			blocks[o[0]] = o[0];
			for (int j = 1; j < i; ++j)
				blocks[ub[ubc++] = o[j]] = o[0];
			for (int j = i; j < o.Length; ++j)
				blocks[o[j]] = o[j];
			
			for (int j = 0; j < ubc; ++j)
			{
				int u = ub[j], v = Find(u, blocks), w = v;
				if (lists[v].rc < lists[u].rc)
				{
					v = u;
					u = w;
				}
				List l1 = lists[u], l2 = lists[v];

				for (int k = 0; k < l1.rc; ++k)
				{
					int g = l1.rs[k].g;
					if (basis_indices[owners[g]] >= s)
						mapped[g] = l1.rs[k].u;
				}
				for (int k = 0; k < l2.rc; ++k)
				{
					int g = l2.rs[k].g;
					if (basis_indices[owners[g]] >= s)
					{
						Unite((mapped[g] == -1) ? u : mapped[g], l2.rs[k].u, w, blocks);
						mapped[g] = -1;
					}
				}
				for (int k = 0; k < l1.rc; ++k)
				{
					int g = l1.rs[k].g;
					if (mapped[g] != -1)
					{
						Unite(l1.rs[k].u, v, w, blocks);
						mapped[g] = -1;
					}
				}
			}
			
			for (int j = 0; j < o.Length; ++j)
				Find(o[j], blocks);
		}

		int Find(int u, int[] blocks)
		{
			int v = u;
			
			while (v != blocks[v])
				v = blocks[v];
			
			while (v != blocks[u])
			{
				int w = u;
				u = blocks[u];
				blocks[w] = v;
			}
			
			return v;
		}

		void Unite(int u, int v, int w, int[] blocks)
		{
			u = Find(u, blocks);
			v = Find(v, blocks);

			if (u == v)
				return;

			if (v == w)
			{
				v = u;
				u = w;
			}

			blocks[v] = u;
			ub[ubc++] = v;
		}
	}
}
