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
		bool Distinguish1()
		{
			cells = cells1; vertices = vertices1; next = next1; indices = indices1; skip = skip1;
			if (to_select.Size == 0)
				return false;
			int a = to_select.Pop(), u = SelectVertex1(a);
			Distinguish(a, u);
			is_first = true;
			return true;
		}

		int SelectVertex1(int a)
		{
			int b = next1[a], c = a;
			for (int d = a; d < b; ++d)
				counts[vertices2[d]] = 1;
			
			for (; c < b; ++c)
				if (counts[vertices1[c]] == 0)
					break;
			for (int d = c + 1; d < b; ++d)
				if (counts[vertices1[d]] == 0 && vertices1[d] < vertices1[c])
					c = d;
			
			for (int d = a; d < b; ++d)
				counts[vertices2[d]] = 0;
			return vertices1[c];
		}

		bool Distinguish2()
		{
			cells = cells2; vertices = vertices2; next = next2; indices = indices2; skip = skip2;
			int a = trail[trail.Size - 1][0][0], u = SelectVertex2(a);
			if (u == -1)
				return false;
			Distinguish(a, u);
			return true;
		}

		int SelectVertex2(int a)
		{
			int b = next2[a], u = vertices2[b - 1], o = -1, s = degree, t = s;
			if (is_first)
			{
				is_first = false;
				u = -1;
			}
			
			if (base_index == trail.Size - 1)
			{
				o = vertices1[b - 1];
				if (u == o)
					u = -1;
			}
			
			for (int c = a; c < b; ++c)
				counts[vertices1[c]] = 1;
			
			int p = (u == -1) ? 0 : counts[u], v = (p == 0) ? -1 : u;
			
			for (int c = a; c < b; ++c)
			{
				int w = vertices2[c], q = counts[w] & 1;
				if (q < p || (w <= u && q == p))
					counts[is_first ? w : OR(w)] |= 2;
			}
			
			for (int c = a; c < b; ++c)
			{
				int w = vertices2[c], q = counts[w] & 1, r = counts[is_first ? w : OR(w)] & 2;
				if (q == 0 && w > u && w < s && r == 0 && w != o)
					s = w;
				if (q == 1 && w > v && w < t && r == 0 && w != o)
					t = w;
			}
			
			for (int c = a; c < b; ++c)
				counts[vertices1[c]] = counts[is_first ? vertices2[c] : OR(vertices2[c])] = 0;

			return (p == 0 && s != degree) ? s : (t != degree) ? t : -1;
		}

		bool Distinguish3()
		{
			cells = cells1; vertices = vertices1; next = next1; indices = indices1; skip = skip1;
			if (to_select.Size == 0)
				return false;
			int a = to_select.Pop(), u = SelectVertex3(a);
			Distinguish(a, u);
			++base_index;
			return true;
		}

		int SelectVertex3(int a)
		{
			for (int b = next1[a], c = a + 1; c < b; ++c)
				if (vertices1[c] < vertices1[a])
					a = c;
			return vertices1[a];
		}

		void Distinguish(int a, int u)
		{
			int b = indices[u], c = next[a] - 1, v = vertices[c];
			vertices[c] = u;
			vertices[b] = v;
			indices[u] = c;
			indices[v] = b;

			++counts[u];
			--skip[a];
			to_process[to_process_count++] = a;
		}

		void InsertSplitted()
		{
			for (int i = 0; i < to_insert_count; ++i)
				if (IsSelectable(to_insert[i]))
				{
					ComputeCellValue(to_insert[i]);
					to_select.Push(to_insert[i]);
				}
		}

		bool IsSelectable(int a)
		{
			int b = next1[a], c = a;
			if (b - a == 1)
				return false;
			if (is_decomposition)
				return true;
			
			for (int d = a; d < b; ++d)
				counts[vertices2[d]] = 1;
			
			for (; c < b; ++c)
				if (counts[vertices1[c]] == 0)
					break;
			
			for (int d = a; d < b; ++d)
				counts[vertices2[d]] = 0;
			
			return (b != c);
		}

		void ComputeCellValue(int a)
		{
			int u = vertices1[a], n = 0;
			for (int i = offsets[u], j = offsets[u + 1]; i < j; ++i)
				++counts[cells1[adjacent[i]]];
			
			for (int i = offsets[u], j = offsets[u + 1]; i < j; ++i)
			{
				int b = cells1[adjacent[i]];
				if (counts[b] != 0)
				{
					if (next1[b] - b != counts[b])
						++n;
					counts[b] = 0;
				}
			}
			values[a] = n;
		}

		void InsertSplitting()
		{
			Vector<int[]> ps = trail[trail.Size - 1];
			for (int i = 0; i < ps.Size; ++i)
				for (int j = 0; j < ps[i].Length - 1; ++j)
				{
					int a = ps[i][j];
					if (to_select.Contains(a))
					{
						ComputeCellValue(a);
						to_select.Update(a);
					}
					else if (IsSelectable(a))
					{
						ComputeCellValue(a);
						to_select.Push(a);
					}
				}
		}

		void RemoveSplitted()
		{
			Vector<int[]> ps = trail[trail.Size - 1];
			for (int i = 0; i < ps.Size; ++i)
				if (to_select.Contains(ps[i][0]))
					to_select.Remove(ps[i][0]);
		}

		void RemoveSplitting()
		{
			Vector<int[]> ps = trail[trail.Size - 1];
			for (int i = 0; i < ps.Size; ++i)
				for (int j = 0; j < ps[i].Length - 1; ++j)
					if (to_select.Contains(ps[i][j]))
						to_select.Remove(ps[i][j]);
		}
	}
}