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
		void Refine1()
		{
			cells = cells1; vertices = vertices1; next = next1; indices = indices1; skip = skip1;
			trail.Push(new Vector<int[]>());
			Refine();
		}

		bool Refine2()
		{
			cells = cells2; vertices = vertices2; next = next2; indices = indices2; skip = skip2;
			trail_index = 0;
			Refine();
			return (are_isomorphic && (trail[trail.Size - 1].Size == trail_index));
		}

		void Refine()
		{
			OnRefine(EventArgs.Empty);
			do
			{
				Process();
			}
			while (Prepare());
		}

		void Process()
		{
			while (to_process_count != 0)
			{
				int a = to_process[--to_process_count];
				if (HasSplit(a))
				{
					int[] bs = Split(a);
					if (!AreIsomorphic(bs))
					{
						Merge(bs);
						Reset();
						return;
					}
					Update(bs);
				}
			}
		}

		bool HasSplit(int a)
		{
			if (a != skip[a])
				return true;

			int n = counts[vertices[a]], b = next[a];
			for (int c = a + 1; c < b; ++c)
				if (counts[vertices[c]] != n)
					return true;

			skip[a] = b;
			for (; a < b; ++a)
				counts[vertices[a]] = 0;
			return false;
		}

		int[] Split(int a)
		{
			OnSplit(EventArgs.Empty);
			Sort(vertices, counts, skip[a], next[a]);
			
			int b = next[a], c = skip[a], d = a, m = -1, n = 1;
			for (; c < b; ++c)
			{
				int u = vertices[c];
				if (counts[u] != m)
				{
					d = next[d] = skip[d] = c;
					m = counts[u];
					++n;
				}
				cells[u] = d;
				counts[u] = 0;
				indices[u] = c;
			}
			next[d] = skip[d] = c;
			
			int[] bs = new int[n + 1];
			for (int i = 0; a < b; a = next[a])
				bs[i++] = a;
			bs[n] = b;
			return bs;
		}

		bool AreIsomorphic(int[] bs)
		{
			Vector<int[]> ps = trail[trail.Size - 1];
			return (are_isomorphic = (cells == cells1 || 
				(trail_index < ps.Size && AreEqual(bs, ps[trail_index]))));
		}

		bool AreEqual(int[] bs, int[] cs)
		{
			if (bs.Length != cs.Length)
				return false;
			for (int i = 0; i < bs.Length; ++i)
				if (bs[i] != cs[i])
					return false;
			return true;
		}

		void Merge(Vector<int[]> ps, int last)
		{
			to_insert_count = 0;
			for (int i = 0; i < last; ++i)
			{
				int a = ps[i][0], b = ps[i][ps[i].Length - 1],
					c = cells[vertices[a]], d = next[c];
				if (a == c && d < b)
					to_insert[to_insert_count++] = Merge(ps[i]);
			}
		}

		int Merge(int[] bs)
		{
			int a = bs[0], b = bs[bs.Length - 1];
			for (int c = next[a]; c < b; ++c)
				cells[vertices[c]] = a;
			next[a] = skip[a] = b;
			return a;
		}

		void Reset()
		{
			while (to_process_count != 0)
			{
				int a = to_process[--to_process_count];
				int b = next[a], c = skip[a];
				for (; c < b; ++c)
					counts[vertices[c]] = 0;
				skip[a] = b;
			}
			while (to_refine.Size != 0)
				to_refine.Pop();
		}

		void Update(int[] bs)
		{
			if (to_refine.Contains(bs[0]))
				to_refine.Update(bs[0]);
			else
				to_refine.Push(bs[0]);
			for (int i = 1; i < bs.Length - 1; ++i)
				to_refine.Push(bs[i]);
			
			if (cells == cells2)
				++trail_index;
			else
				trail[trail.Size - 1].Push(bs);
		}

		bool Prepare()
		{
			if (to_refine.Size == 0)
				return false;
			
			int a = to_refine.Pop(), b = degree + (next[a] - a);
			Array.Copy(vertices, a, to_process, degree, next[a] - a);
			a = degree;

			for (; a < b; ++a)
			{
				int u = to_process[a];
				for (int i = offsets[u], j = offsets[u + 1]; i < j; ++i)
				{
					int v = adjacent[i], c = cells[v];
					if (next[c] - c > 1)
					{
						if (counts[v]++ == 0)
						{
							int d = indices[v], e = --skip[c], w = vertices[e];
							vertices[d] = w;
							vertices[e] = v;
							indices[v] = e;
							indices[w] = d;
						}
						
						if (!is_marked[c])
						{
							is_marked[c] = true;
							to_process[to_process_count++] = c;
						}
					}
				}
			}
			
			for (int i = 0; i < to_process_count; ++i)
				is_marked[to_process[i]] = false;
			Array.Sort(to_process, 0, to_process_count);
			Array.Reverse(to_process, 0, to_process_count);
			return true;
		}
	}
}