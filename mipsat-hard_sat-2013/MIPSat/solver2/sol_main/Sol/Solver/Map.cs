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
		int[] Map(int a)
		{
			int[] bs;
			if (is_in_group[bodies[a][0]])
			{
				int s = 0;
				int[] us = bodies[a], vs = new int[us.Length];
				for (int i = 1; i < us.Length; ++i)
					if (is_in_group[us[i]])
						vs[s++] = us[i];

				morpher.FixInBase(0, vs, s, false);
				Morpher.Record[] t = morpher.ComputeTransversal(us[0], s);
				bs = new int[t.Length];

				for (int i = 1; i < t.Length; ++i)
				{
					//OnMap(new MapEventArgs(morpher.ComputeElement(t, i)));

					int[] ws = (int[])us.Clone();
					ws[0] = t[i].u;
					bs[i] = CreateClause(ws);
				}
			}
			else
				bs = new int[1];
			bs[0] = a;
			return bs;
		}
	}
}
