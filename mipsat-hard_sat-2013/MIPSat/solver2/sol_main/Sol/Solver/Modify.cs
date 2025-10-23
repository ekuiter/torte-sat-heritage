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
		void Attach(int a)
		{
			int u = bodies[a][0], v = bodies[a][1];
			Attach(u, u, a);
			Attach(v, u, a);
		}

		void Attach(int u, int v, int a)
		{
			List w = watches[u];
			Algorithm.Extend(ref w.rs, w.rc);
			w.rs[w.rc].u = v;
			w.rs[w.rc++].a = a;
			watches[u] = w;
		}

		void Detach(int a)
		{
			Detach(bodies[a][0], a);
			Detach(bodies[a][1], a);
		}

		void Detach(int u, int a)
		{
			List w = watches[u];
			for (int i = 0; i < w.rc; ++i)
				if (w.rs[i].a == a)
				{
					w.rs[i] = w.rs[--w.rc];
					Algorithm.Shrink(ref w.rs, w.rc);
				}
			watches[u] = w;
		}
	}
}
