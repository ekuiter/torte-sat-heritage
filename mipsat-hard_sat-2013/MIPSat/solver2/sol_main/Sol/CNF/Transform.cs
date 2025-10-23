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
	public partial class CNF
	{
		public static void Transform(int[][] bs)
		{
			for (int i = 0; i < bs.Length; ++i)
				F(bs[i]);
		}

		public static void F(int[] us)
		{
			for (int i = 0; i < us.Length; ++i)
				us[i] = F(us[i]);
		}

		public static int F(int u)
		{
			if (u < 0)
				return 2 * (-u - 1) + 1;
			return 2 * (u - 1);
		}

		public static void BP(int[] p)
		{
			for (int a = 0; a < p.Length; a += p[a])
				for (int b = a + 1; b < a + p[a]; ++b)
					p[b] = CNF.B(p[b]);
		}

		public static void B(int[] us)
		{
			for (int i = 0; i < us.Length; ++i)
				us[i] = B(us[i]);
		}

		public static int B(int u)
		{
			if ((u & 1) == 1)
				return -((u >> 1) + 1);
			return ((u >> 1) + 1);
		}

		public static void ToGraph(int n, int[][] cs, out int[] counts, out int[] adjacent, out int[] offsets)
		{
			int m = n + cs.Length;
			counts = new int[m];
			if (m > 1)
				for (int i = n; i < m; ++i)
					++counts[i];
			offsets = new int[m + 1];
			for (int i = 0; i < cs.Length; ++i)
			{
				offsets[n + i + 1] = cs[i].Length;
				for (int j = 0; j < cs[i].Length; ++j)
					++offsets[cs[i][j] + 1];
			}
			for (int i = 0; i < n; ++i)
				++offsets[i + 1];
			for (int i = 0; i < m; ++i)
				offsets[i + 1] += offsets[i];
			int[] indices = new int[n];
			Array.Copy(offsets, indices, n);
			adjacent = new int[offsets[m]];
			for (int i = 0; i < cs.Length; ++i)
			{
				Array.Copy(cs[i], 0, adjacent, offsets[n + i], cs[i].Length);
				for (int j = 0; j < cs[i].Length; ++j)
					adjacent[indices[cs[i][j]]++] = n + i;
			}
			for (int i = 0; i < n; i += 2)
			{
				adjacent[indices[i]] = i + 1;
				adjacent[indices[i + 1]] = i;
			}
		}
	}
}
