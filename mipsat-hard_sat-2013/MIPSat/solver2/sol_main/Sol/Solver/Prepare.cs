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
	public partial class Solver
	{
		int Prepare(int[] bs)
		{
			int i = levels_count;
			for (int k = 0; k < bs.Length; ++k)
			{
				int j = Prepare(bs[k]);
				if (j < i)
					i = j;
			}
			return i;
		}

		int Prepare(int a)
		{
			int[] us = bodies[a];

			if (us.Length == 1)
				return 0;

			int i, j, k, u;

			for (i = 0; i < us.Length; ++i)
				if (!is_set[us[i] ^ 1])
					break;

			j = i;

			if (i == us.Length)
				for (i = 0, k = 1; k < us.Length; ++k)
					if (levels[us[i] >> 1] < levels[us[k] >> 1])
						i = k;

			u = us[0];
			us[0] = us[i];
			us[i] = u;

			if (j != us.Length)
				while (++j < us.Length)
					if (!is_set[us[j] ^ 1])
						break;

			if (j == us.Length)
				for (j = 1, k = 2; k < us.Length; ++k)
					if (levels[us[j] >> 1] < levels[us[k] >> 1])
						j = k;

			u = us[1];
			us[1] = us[j];
			us[j] = u;

			if (is_set[us[1] ^ 1] &&
				(!is_set[us[0]] ||
				levels[us[1] >> 1] < levels[us[0] >> 1]))
				return levels[us[1] >> 1];
			return levels_count;
		}
	}
}
