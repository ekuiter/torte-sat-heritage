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
		public void Compute(int[] adjacent, int[] offsets, int[] counts, Morpher morpher)
		{
			CheckArguments(adjacent, offsets, counts, morpher);
			Initialize(adjacent, offsets, counts, morpher);
			if (degree > 1)
			{
				Decompose();
				Search();
			}
		}

		void Decompose()
		{
			to_select.Push(0);
			to_refine.Push(0);
			to_process[to_process_count++] = 0;
			do
			{
				Refine1();
				RemoveSplitted();
				InsertSplitting();
			}
			while (Distinguish3());
			
			Array.Copy(cells1, cells2, degree);
			Array.Copy(vertices1, vertices2, degree);
			Array.Copy(next1, next2, degree);
			Array.Copy(indices1, indices2, degree);
			Array.Copy(skip1, skip2, degree);
			trail_index = trail[trail.Size - 1].Size;
			
			prefix_count = 0;
			if (base_index > 0)
			{
				for (int i = 1; i < trail.Size; ++i)
					prefix[prefix_count++] = vertices1[trail[i][0][1]];
				morpher.FixInBase(0, prefix, prefix_count, true);
			}

			Backtrack2();
			is_decomposition = false;
		}

		void Search()
		{
			while (trail.Size > 1)
			{
				while (Distinguish2())
					if (Refine2())
					{
						InsertSplitting();
						if (Distinguish1())
						{
							Refine1();
							RemoveSplitted();
						}
						else
							Backtrack1();
					}
					else
						Backtrack2();
				Backtrack3();
			}
		}

		void Backtrack1()
		{
			cells = cells2; vertices = vertices2; next = next2; indices = indices2; skip = skip2;
			Merge(trail[trail.Size - 1], trail[trail.Size - 1].Size);
			int[] p = null;
			if (IsAutomorphism())
			{
				OnAutomorphism(EventArgs.Empty);
				p = ComputePermutation();
				Backtrack(base_index);
			}
			Stabilize(p);
		}

		void Backtrack2()
		{
			cells = cells2; vertices = vertices2; next = next2; indices = indices2; skip = skip2;
			Merge(trail[trail.Size - 1], trail_index);
			Stabilize(null);
		}

		void Backtrack3()
		{
			Backtrack(trail.Size - 2);
			Stabilize(null);
		}

		void Backtrack(int level)
		{
			while (level < trail.Size - 1)
			{
				Vector<int[]> ps = trail.Pop();
				cells = cells1; vertices = vertices1; next = next1; indices = indices1; skip = skip1;
				Merge(ps, ps.Size);
				InsertSplitted();
				cells = cells2; vertices = vertices2; next = next2; indices = indices2; skip = skip2;
				RemoveSplitting();
				Merge(trail[trail.Size - 1], trail[trail.Size - 1].Size);
			}
		}
	}
}
