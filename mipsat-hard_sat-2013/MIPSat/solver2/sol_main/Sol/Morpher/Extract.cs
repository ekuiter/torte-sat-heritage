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
		public void ExtractRemainder(int[] us, int uc, Morpher h)
		{
			FixInBase(0, us, uc, true);
			h.Initialize(degree);
			h.FixInBase(0, basis, basis_count, true);

			for (int i = uc; i < basis_count; ++i)
			{
				int u = basis[i];
				List l = lists[u];
				for (int j = 0; j < l.rc; ++j)
					if (owners[l.rs[j].g] == u)
						h.AddStrongGenerator(u, (int[])perms[l.rs[j].g].Clone());
			}
		}
		
		public void ExtractClosure(int[] us, int uc, Morpher h)
		{
			us = ComputeClosure(us, uc);
			FixInBase(0, us, us.Length, true);
			h.Initialize(degree);
			h.FixInBase(0, us, us.Length, true);

			for (int i = 0; i < us.Length; ++i)
				is_marked[us[i]] = true;

			for (int i = 0; i < us.Length; ++i)
			{
				int u = us[i];
				List l = lists[u];
				for (int j = 0; j < l.rc; ++j)
					if (owners[l.rs[j].g] == u)
					{
						int[] p = perms[l.rs[j].g];
						p0c = 0;
						for (int a = 0; a < p.Length; a += p[a])
							if (is_marked[p[a + 1]])
							{
								p0[p0c++] = p[a];
								for (int b = a + 1; b < a + p[a]; ++b)
									p0[p0c++] = p[b];
							}
						p = new int[p0c];
						Array.Copy(p0, p, p0c);
						h.AddStrongGenerator(u, p);
					}
			}

			for (int i = 0; i < us.Length; ++i)
				is_marked[us[i]] = false;
		}

		public void Trim(int n)
		{
			int[] us = new int[n];
			for (int i = 0; i < n; ++i)
				us[i] = i;
			FixInBase(0, us, n, false);

			us = new int[degree - n];
			for (int i = n; i < degree; ++i)
				us[i - n] = i;
			Reset(us);

			degree = basis_count = n;

			for (int u = 0; u < basis_count; ++u)
			{
				List l = lists[u];
				for (int i = 0; i < l.rc; ++i)
				{
					int g = l.rs[i].g;
					if (owners[g] == u)
					{
						int[] p = perms[g];
						p0c = 0;
						for (int a = 0; a < p.Length; a += p[a])
							if (p[a + 1] < n)
							{
								p0[p0c++] = p[a];
								for (int b = a + 1; b < a + p[a]; ++b)
									p0[p0c++] = p[b];
							}
						perms[g] = new int[p0c];
						Array.Copy(p0, perms[g], p0c);
					}
				}
			}
		}
	}
}
