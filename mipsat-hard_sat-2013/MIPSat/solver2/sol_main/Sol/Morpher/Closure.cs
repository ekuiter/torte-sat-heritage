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
		public int[] ComputeClosure(int s)
		{
			gbc = 0;
			for (int i = s; i < basis_count; ++i)
			{
				int u = basis[i];
				List l = lists[u];
				for (int j = 0; j < l.rc; ++j)
					if (owners[l.rs[j].g] == u)
						gb[gbc++] = l.rs[j].g;
			}

			ubc = 0;
			for (int i = 0; i < gbc; ++i)
			{
				int[] p = perms[gb[i]];
				for (int a = 0; a < p.Length; a += p[a])
					for (int b = a + 1; b < a + p[a]; ++b)
						if (!is_touched[p[b]])
							is_touched[ub[ubc++] = p[b]] = true;
			}

			for (int i = 0; i < ubc; ++i)
				is_touched[ub[i]] = false;

			int[] us = new int[ubc];
			Array.Copy(ub, us, ubc);
			return us;
		}

		public int[] ComputeClosure(int[] us, int uc)
		{
			for (int i = 0; i < uc; ++i)
				if (lists[us[i]].rc > 0)
					is_touched[us[i]] = true;

			ubc = 0;
			for (int i = 0; i < uc; ++i)
				if (is_touched[us[i]])
				{
					t1c = ComputeTransversal(t1, us[i], 0);
					for (int j = 0; j < t1c; ++j)
					{
						is_touched[t1[j].u] = false;
						ub[ubc++] = t1[j].u;
					}
				}

			int[] vs = new int[ubc];
			Array.Copy(ub, vs, ubc);
			return vs;
		}
	}
}
