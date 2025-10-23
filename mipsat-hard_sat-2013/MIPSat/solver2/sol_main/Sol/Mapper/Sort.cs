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
		void Sort(int[] items, int[] keys, int first, int last)
		{
			int n = 0;
			regions[0].begin = regions[0].end = first;
			for (int i = first; i < last; ++i)
			{
				if (n < keys[items[i]])
					n = keys[items[i]];
				++regions[keys[items[i]]].end;
			}
			for (int i = 1; i <= n; ++i)
				regions[i].end += (regions[i].begin = regions[i - 1].end);

			Region r;
			r.begin = r.end = 0;
			for (int i = 0; i <= n; ++i)
			{
				for (; regions[i].begin < regions[i].end; ++regions[i].begin)
				{
					int a = items[regions[i].begin];
					if (keys[a] != i)
						for (int j = keys[a]; ; ++regions[j].begin)
						{
							int b = items[regions[j].begin];
							if (keys[b] != j)
							{
								items[regions[i].begin--] = b;
								items[regions[j].begin++] = a;
								break;
							}
						}
				}
				regions[i] = r;
			}
		}
	}
}
