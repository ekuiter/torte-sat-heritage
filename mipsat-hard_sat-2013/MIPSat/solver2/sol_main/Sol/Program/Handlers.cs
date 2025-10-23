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
	partial class Program
	{
		void Subscribe(Solver solver)
		{
			solver.DecideEvent += new EventHandler<Solver.DecideEventArgs>(DecideHandler);
			solver.AssertEvent += new EventHandler<Solver.AssertEventArgs>(AssertHandler);
			solver.ResolveEvent += new EventHandler<Solver.ResolveEventArgs>(ResolveHandler);
			solver.MapEvent += new EventHandler<Solver.MapEventArgs>(MapHandler);
			solver.ModelEvent += new EventHandler<Solver.ModelEventArgs>(ModelHandler);
		}

		void DecideHandler(object sender, Solver.DecideEventArgs e)
		{
			RegisterDecision();
			TraceDecision(e.Decision);
			OutputDecision();
		}

		void AssertHandler(object sender, Solver.AssertEventArgs e)
		{
			RegisterAssertion();
			TraceAssertion(e.Assertion);
			OutputAssertion();
		}

		void ResolveHandler(object sender, Solver.ResolveEventArgs e)
		{
			RegisterResolvent();
			TraceResolvent(e.Resolvent);
			OutputResolvent();
		}

		void MapHandler(object sender, Solver.MapEventArgs e)
		{
			RegisterMapping();
			TraceMapping(e.Mapping);
			OutputMapping();
		}

		void ModelHandler(object sender, Solver.ModelEventArgs e)
		{
			RegisterModel();
			TraceModel(e.Model);
			OutputModel();
		}

		void Subscribe(Mapper mapper)
		{
			mapper.RefineEvent += new EventHandler<EventArgs>(RefineHandler);
			mapper.SplitEvent += new EventHandler<EventArgs>(SplitHandler);
			mapper.AutomorphismEvent += new EventHandler<EventArgs>(AutomorphismHandler);
		}

		void RefineHandler(object sender, EventArgs e)
		{
			RegisterRefinement();
		}

		void SplitHandler(object sender, EventArgs e)
		{
			RegisterSplit();
		}

		void AutomorphismHandler(object sender, EventArgs e)
		{
			RegisterAutomorphism();
		}
	}
}
