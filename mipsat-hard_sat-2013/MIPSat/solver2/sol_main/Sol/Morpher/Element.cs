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
		public int[] ComputeElement(Record[] t, int i)
		{
			if (i == 0)
				return new int[0];
			ComputeElement(t, t.Length, i);
			int[] p = new int[p5c];
			Array.Copy(p5, p, p5c);
			return p;
		}

		int ComputeElement(Record[] t, int tc, int i, int[] p)
		{
			ComputeElement(t, tc, i);
			Array.Copy(p5, p, p5c);
			return p5c;
		}

		void ComputeElement(Record[] t, int tc, int i)
		{
			for (int j = 0; j < tc; ++j)
				trans_indices[t[j].u] = j;
			p5c = PrepareFactor(t[i].u, perms[t[i].g], out i, p5);
			while (i != 0)
			{
				int[] p = p5; p5 = p4; p4 = p; p4c = p5c;
				p3c = PrepareFactor(t[i].u, perms[t[i].g], out i, p3);
				p5c = Multiply(p3, p3c, p4, p4c, p5);
			}
		}

		int PrepareFactor(int u, int[] p, out int i, int[] r)
		{
			int b, c, d, e;
			FindPoint(p, u, out b, out c);
			d = e = c;
			for (c = b + 1; c < b + p[b]; ++c)
				if (trans_indices[p[c]] < trans_indices[p[d]])
					d = c;
			int n = e - d;
			if (n < 0)
				n += p[b] - 1;
			i = trans_indices[p[d]];
			return RaiseToPower(p, p.Length, n, r);
		}
	}
}
