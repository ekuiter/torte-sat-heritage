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
		public void FindPoint(int[] p, int u, out int a, out int b)
		{
			FindPoint(p, p.Length, u, out a, out b);
		}

		void FindPoint(int[] p, int pc, int u, out int a, out int b)
		{
			for (a = 0; a < pc; a += p[a])
				for (b = a + 1; b < a + p[a]; ++b)
					if (p[b] == u)
						return;
			a = b = -1;
		}

		public int[] RaiseToPower(int[] p, int n)
		{
			p0c = RaiseToPower(p, p.Length, n, p0);
			int[] r = new int[p0c];
			Array.Copy(p0, r, p0c);
			return r;
		}

		int RaiseToPower(int[] p, int pc, int n, int[] r)
		{
			int a, b, c, d, e, m, rc;
			rc = 0;
			for (a = 0; a < pc; a += p[a])
				if ((m = n % (p[a] - 1)) != 0)
					for (b = a + 1; b < a + p[a]; ++b)
					{
						if (images[p[b]] == p[b])
						{
							e = rc++;
							c = b;
							do
							{
								d = c + m;
								if (d >= a + p[a])
									d -= (p[a] - 1);
								r[rc++] = images[p[c]] = p[d];
								c = d;
							}
							while (d != b);
							r[e] = rc - e;
						}
						images[p[b]] = p[b];
					}
			return rc;
		}

		public int[] Multiply(int[] p, int[] q)
		{
			p0c = Multiply(p, p.Length, q, q.Length, p0);
			int[] r = new int[p0c];
			Array.Copy(p0, r, p0c);
			return r;
		}

		int Multiply(int[] p, int pc, int[] q, int qc, int[] r)
		{
			int a, b, d, u, v, rc;
			for (a = 0; a < qc; a += q[a])
			{
				for (b = a + 2; b < a + q[a]; ++b)
					images[q[b - 1]] = q[b];
				images[q[b - 1]] = q[a + 1];
			}

			rc = 0;
			for (a = 0; a < pc; a += p[a])
			{
				for (b = a + 2; b < a + p[a]; ++b)
				{
					r[rc++] = p[b - 1];
					r[rc++] = images[p[b]];
				}
				r[rc++] = p[b - 1];
				r[rc++] = images[p[a + 1]];
			}

			for (a = 0; a < rc; )
				images[r[a++]] = r[a++];

			rc = 0;
			for (a = 0; a < pc; a += p[a])
				for (b = a + 1; b < a + p[a]; ++b)
					if (images[u = p[b]] != u)
					{
						d = rc++;
						do
						{
							v = images[u];
							r[rc++] = images[u] = u;
							u = v;
						}
						while (images[u] != u);
						r[d] = rc - d;
					}

			for (a = 0; a < qc; a += q[a])
				if (images[q[a + 1]] != q[a + 1])
				{
					r[rc++] = q[a];
					for (b = a + 1; b < a + q[a]; ++b)
						r[rc++] = images[q[b]] = q[b];
				}

			return rc;
		}
	}
}
