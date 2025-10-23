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
		int CCC;

		string T()
		{
			int[] us = new int[trail_count];
			Array.Copy(trail, us, trail_count);
			return S(us);
		}
		
		string TI()
		{
			int[] us = new int[trail_count];
			for (int i = 0; i < trail_count; ++i)
				us[i] = trail[i] ^ 1;
			return S(us);
		}

		string S(int[] us)
		{
			string s = "";
			for (int i = 0; true; ++i)
			{
				if (us[i] < 10)
					s += " ";
				s += us[i];
				if (i + 1 == us.Length)
					break;
				s += " ";
			}
			return s;
		}

		string A(int[] us)
		{
			string s = "";
			for (int i = 0; i < us.Length; ++i)
				s += is_set[us[i]] ? "1" : "0";
			return s;
		}

		string F(int[] us)
		{
			string s = "";
			for (int i = 0; i < us.Length; ++i)
				s += is_set[us[i] ^ 1] ? "1" : "0";
			return s;
		}
		
		void CheckTrail()
		{
			bool[] flags = new bool[2 * variables_count];
			for (int i = 0; i < trail_count; ++i)
			{
				int u = trail[i];

				if (u >= 2 * variables_count)
					throw new ApplicationException();
				
				if (flags[u])
					throw new ApplicationException();

				if (flags[u ^ 1])
					throw new ApplicationException();

				flags[u] = true;
				int a = reasons[u >> 1];
				
				if (a == -1)
					continue;

				int[] us = bodies[a];

				if (u != us[0])
					throw new ApplicationException();

				if (!is_set[u])
					throw new ApplicationException();

				if (is_set[u ^ 1])
					throw new ApplicationException();

				for (int j = 1; j < us.Length; ++j)
					if (is_set[us[j]])
						throw new ApplicationException();

				for (int j = 1; j < us.Length; ++j)
					if (!is_set[us[j] ^ 1])
						throw new ApplicationException();
				
				for (int j = 1; j < us.Length; ++j)
					if (flags[us[j]])
						throw new ApplicationException();

				for (int j = 1; j < us.Length; ++j)
					if (!flags[us[j] ^ 1])
						throw new ApplicationException();
			}
		}

		void CheckWatches()
		{
			for (int u = 0; u < 2 * variables_count; ++u)
			{
				List w = watches[u];
				for (int i = 0; i < w.rc; ++i)
				{
					int a = w.rs[i].a;
					int[] us = bodies[a];

					if (us[0] != u && us[1] != u)
						throw new ApplicationException();
				}
			}
		}

		void CheckWatches2()
		{
			for (int u = 0; u < 2 * variables_count; ++u)
			{
				List w = watches[u];
				for (int i = 0; i < w.rc; ++i)
				{
					int a = w.rs[i].a;
					int[] us = bodies[a];

					if (us[0] != u && us[1] != u)
						throw new ApplicationException();

					int n = 0;
					for (int j = 0; j < us.Length; ++j)
						if (is_set[us[j] ^ 1])
							++n;

					int m = 0;
					for (int j = 0; j < us.Length; ++j)
						if (is_set[us[j]])
							++m;

					if (n == us.Length)
						throw new ApplicationException();

					if (n + m > us.Length)
						throw new ApplicationException();

					if (n == us.Length - 1 && m != 1)
						throw new ApplicationException();
				}
			}
		}
	}
}
