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
		int Propagate()
		{
			int a = -1;
			for (; trail_index < trail_count; ++trail_index)
			{
				int u = trail[trail_index] ^ 1;
				List w = watches[u];
				
				for (int i = 0; i < w.rc; )
				{
					if (!is_set[w.rs[i].u])
					{
						int[] us = bodies[w.rs[i].a];

						if (us[0] == u)
						{
							w.rs[i].u = us[0] = us[1];
							us[1] = u;
						}

						if (!is_set[us[0]])
						{
							for (int j = 2; j < us.Length; ++j)
								if (!is_set[us[j] ^ 1])
								{
									us[1] = us[j];
									us[j] = u;
									Attach(us[1], us[0], w.rs[i].a);
									w.rs[i] = w.rs[--w.rc];
									goto FoundWatch;
								}

							if (is_set[us[0] ^ 1])
							{
								a = w.rs[i].a;
								break;
							}

							Assert(us[0], w.rs[i].a);
						}
					}

					++i;
				FoundWatch: ;
				}

				Algorithm.Shrink(ref w.rs, w.rc);
				watches[u] = w;

				if (a != -1)
					break;
			}
			
			return a;
		}
	}
}
