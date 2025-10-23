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
	public partial class CNF
	{
		public static int[][] Correct(int[][] bs)
		{
			if (bs.Length == 0)
				return bs;
			
			int j;
			for (int i = 0; i < bs.Length; ++i)
			{
				int[] us = bs[i];
				if (us.Length == 0)
					continue;
				Array.Sort(us);
				j = 0;
				for (int k = 1; k < us.Length; ++k)
					if (us[j] != us[k])
						us[++j] = us[k];
				if (j != us.Length - 1)
				{
					int[] vs = new int[j + 1];
					Array.Copy(us, vs, j + 1);
					bs[i] = vs;
				}
			}
			
			Array.Sort(bs, (x, y) => Compare(x, y));
			j = 0;
			for (int i = 1; i < bs.Length; ++i)
				if (Compare(bs[j], bs[i]) != 0)
					bs[++j] = bs[i];
			if (++j == bs.Length)
				return bs;
			int[][] cs = new int[j][];
			for (int i = 0; i < j; ++i)
				cs[i] = bs[i];
			return cs;
		}

		static int Compare(int[] us, int[] vs)
		{
			int d = us.Length - vs.Length;
			if (d != 0)
				return d;
			for (int i = 0; i < us.Length; ++i)
			{
				d = us[i] - vs[i];
				if (d != 0)
					return d;
			}
			return 0;
		}
	}
}