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
		void CheckArguments(int[] adjacent, int[] offsets, int[] counts, Morpher morpher)
		{
			if (adjacent == null || offsets == null || counts == null || morpher == null ||
				offsets.Length != counts.Length + 1 || offsets[0] < 0 ||
				offsets[offsets.Length - 1] != adjacent.Length ||
				morpher.Degree < counts.Length)
				throw new ArgumentException();

			for (int i = 0; i < counts.Length; ++i)
				if (counts[i] < 0 || counts[i] >= counts.Length)
					throw new ArgumentException();

			for (int i = 1; i < offsets.Length; ++i)
				if (offsets[i] < offsets[i - 1])
					throw new ArgumentException();

			for (int i = 0; i < adjacent.Length; ++i)
				if (adjacent[i] >= counts.Length)
					throw new ArgumentException();
		}

		void Initialize(int[] adjacent, int[] offsets, int[] counts, Morpher morpher)
		{
			this.adjacent = adjacent;
			this.offsets = offsets;
			this.counts = counts;
			this.morpher = morpher;

			Reserve(counts.Length);

			to_select = new NHeap<int>(
				(x, y) => 
				{
					int n = values[x] - values[y];
					if (n != 0)
						return n;
					
					n = (next[x] - x) - (next[y] - y);
					return (n != 0) ? n : x - y;
				}, 
				x => select_indices[x], 
				(x, y) => select_indices[x] = y);
			to_refine = new NHeap<int>(
				(x, y) => 
				{ 
					int n = (next[x] - x) - (next[y] - y); 
					return (n != 0) ? n : x - y;
				},
				x => refine_indices[x],
				(x, y) => refine_indices[x] = y);
			trail = new Vector<Vector<int[]>>();

			is_first = are_isomorphic = false;
			is_decomposition = true;
			trail_index = base_index = orbits_count = to_process_count =
				to_insert_count = prefix_count = 0;
			cells = cells1; vertices = vertices1; next = next1; indices = indices1; skip = skip1;

			for (int i = 0; i < degree; ++i)
			{
				representatives[i] = vertices1[i] = indices1[i] = i;
				select_indices[i] = refine_indices[i] = -1;
			}
			next1[0] = degree;
			skip1[0] = 0;
		}

		void Reserve(int n)
		{
			degree = n;
			if (limit == 0 || degree > limit)
			{
				if (limit == 0) limit = 1;
				while ((limit *= 2) < degree) ;

				cells1 = new int[limit];
				cells2 = new int[limit];
				indices1 = new int[limit];
				indices2 = new int[limit];
				representatives = new int[limit];
				regions = new Region[limit];

				vertices1 = new int[limit];
				vertices2 = new int[limit];
				next1 = new int[limit];
				next2 = new int[limit];
				skip1 = new int[limit];
				skip2 = new int[limit];
				select_indices = new int[limit];
				refine_indices = new int[limit];
				values = new int[limit];

				is_marked = new bool[limit];

				orbits = new int[limit];
				to_process = to_insert = prefix = perm = new int[2 * limit];
			}
		}

		struct Region
		{
			public int begin;
			public int end;
		}

		int limit;
		int degree;
		int[] offsets;
		int[] adjacent;

		int[] cells1, cells2;
		int[] indices1, indices2;
		int[] representatives;
		Region[] regions;

		int[] vertices1, vertices2;
		int[] next1, next2;
		int[] skip1, skip2;
		int[] select_indices, refine_indices;
		int[] values;

		bool[] is_marked;
		int[] counts;

		int[] cells, vertices, next, indices, skip;
		
		bool is_first;
		bool are_isomorphic;
		bool is_decomposition;

		NHeap<int> to_select;
		NHeap<int> to_refine;
		int trail_index;
		Vector<Vector<int[]>> trail;
		
		int base_index;
		
		int orbits_count;
		int[] orbits;
		int to_process_count, to_insert_count, prefix_count;
		int[] to_process, to_insert, prefix, perm;

		Morpher morpher;
	}
}
