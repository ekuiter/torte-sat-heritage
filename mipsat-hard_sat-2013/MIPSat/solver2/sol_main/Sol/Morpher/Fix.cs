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
		public void FixInBase(int at, int[] us, int uc, bool is_stable)
		{
			int s = at;
			for (int i = at; i < basis_count; ++i)
			{
				int u = basis[i];
				if (counts[u] != 0)
				{
					basis[s] = u;
					basis_indices[u] = s++;
				}
				else
					basis_indices[u] = -1;
			}
			basis_count = s;
			
			for (int i = 0; i < uc; ++i)
			{
				int u = us[i];
				if (basis_indices[u] == -1)
				{
					basis_indices[u] = basis_count;
					basis[basis_count++] = u;
				}
				indices[u] = (is_stable) ? at + i : basis_indices[u];
			}

			s = -1;
			for (int i = 0; i < uc; ++i)
				if (basis_indices[us[i]] > s)
					s = basis_indices[us[i]];

			for (int i = s + 1; s > at; --s)
			{
				int u = basis[s - 1];
				if (indices[u] == -1)
				{
					for (int j = s; j < i; ++j)
						SwapBasePoints(u, basis[j]);
					--i;
				}
				else
					for (int j = s; j < i; ++j)
					{
						if (indices[u] < indices[basis[j]])
							break;
						SwapBasePoints(u, basis[j]);
					}
			}

			for (int i = 0; i < uc; ++i)
				indices[us[i]] = -1;
		}
	}
}
