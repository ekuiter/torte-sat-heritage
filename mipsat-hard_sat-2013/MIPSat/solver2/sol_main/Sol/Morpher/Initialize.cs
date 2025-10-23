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
		public void Initialize(int n)
		{
			if (limit == 0 || n > limit)
			{
				if (limit == 0) limit = 1;
				while ((limit *= 2) < n) ;
				Reserve(limit);

				for (int u = 0; u < limit; ++u)
				{
					basis_indices[u] = indices[u] = -1;
					counts[u] = 0;
					images[u] = u;
					lists[u].rc = 0;
					lists[u].rs = new Record[2];
				}

				gens_count = 2 * limit;
				for (int g = 0; g < gens_count; ++g)
				{
					gens[g] = g;
					is_created[g] = false;
					perms[g] = null;
					mapped[g] = -1;
				}
			}
			else
				Reset();
			
			degree = n;
			basis_count = 0;
		}

		void Reset()
		{
			Reset(ComputeClosure(0));
			for (int i = 0; i < basis_count; ++i)
				basis_indices[basis[i]] = -1;
		}

		void Reset(int[] us)
		{
			for (int i = 0; i < us.Length; ++i)
			{
				int u = us[i];
				List l = lists[u];
				for (int j = 0; j < l.rc; ++j)
					if (owners[l.rs[j].g] == u)
						DestroyGenerator(l.rs[j].g);
				basis_indices[u] = -1;
				counts[u] = 0;
				lists[u].rc = 0;
				lists[u].rs = new Record[2];
			}
		}
		
		void Reserve(int n)
		{
			basis = new int[n];
			basis_indices = new int[n];
			trans_indices = new int[n];
			indices = new int[n];
			counts = new int[n];
			lists = new List[n];
			is_marked = new bool[n];
			is_touched = new bool[n];
			images = new int[n];

			gens = new int[2 * n];
			is_created = new bool[2 * n];

			perms = new int[2 * n][];
			owners = new int[2 * n];
			mapped = new int[2 * n];

			t1 = new Record[n];
			t2 = new Record[n];
			t3 = new Record[n];

			p0 = new int[2 * n];
			p1 = new int[2 * n];
			p2 = new int[2 * n];
			p3 = new int[2 * n];
			p4 = new int[2 * n];
			p5 = new int[2 * n];

			ub = new int[n];
			gb = new int[n];
		}

		int CreateGenerator(int u, int[] p)
		{
			if (gens_count == 0)
				throw new ApplicationException("Generator creation failed.");

			int g = gens[--gens_count];
			is_created[g] = true;

			perms[g] = p;
			owners[g] = u;

			return g;
		}

		void DestroyGenerator(int g)
		{
			if (!is_created[g])
				throw new ApplicationException("Generator destruction failed.");

			perms[g] = null;

			is_created[g] = false;
			gens[gens_count++] = g;
		}

		public int Size
		{
			get
			{
				return basis_count;
			}
		}

		public int Degree
		{
			get
			{
				return degree;
			}
		}

		public struct Record
		{
			public int u;
			public int g;
		}

		struct List
		{
			public int rc;
			public Record[] rs;
		}

		int limit;
		int degree;
		int basis_count;
		int[] basis;
		int[] basis_indices;
		int[] trans_indices;
		int[] indices;
		int[] counts;
		List[] lists;
		bool[] is_marked;
		bool[] is_touched;
		int[] images;

		int gens_count;
		int[] gens;
		bool[] is_created;

		int[][] perms;
		int[] owners;
		int[] mapped;

		int t1c, t2c, t3c;
		Record[] t1, t2, t3;

		int p0c, p1c, p2c, p3c, p4c, p5c;
		int[] p0, p1, p2, p3, p4, p5;

		int ubc, gbc;
		int[] ub, gb;
	}
}
