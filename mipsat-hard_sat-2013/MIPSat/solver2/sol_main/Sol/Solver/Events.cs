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
		public class DecideEventArgs : EventArgs
		{
			public DecideEventArgs(int decision)
			{
				this.decision = decision;
			}

			public int Decision
			{
				get
				{
					return decision;
				}
			}

			int decision;
		}

		public event EventHandler<DecideEventArgs> DecideEvent;

		protected void OnDecide(DecideEventArgs e)
		{
			EventHandler<DecideEventArgs> RaiseEvent = DecideEvent;
			if (RaiseEvent != null)
				RaiseEvent(this, e);
		}

		public class AssertEventArgs : EventArgs
		{
			public AssertEventArgs(int assertion)
			{
				this.assertion = assertion;
			}

			public int Assertion
			{
				get
				{
					return assertion;
				}
			}

			int assertion;
		}

		public event EventHandler<AssertEventArgs> AssertEvent;

		protected void OnAssert(AssertEventArgs e)
		{
			EventHandler<AssertEventArgs> RaiseEvent = AssertEvent;
			if (RaiseEvent != null)
				RaiseEvent(this, e);
		}

		public class ResolveEventArgs : EventArgs
		{
			public ResolveEventArgs(int[] resolvent)
			{
				this.resolvent = resolvent;
			}

			public int[] Resolvent
			{
				get
				{
					return resolvent;
				}
			}

			int[] resolvent;
		}

		public event EventHandler<ResolveEventArgs> ResolveEvent;

		protected void OnResolve(ResolveEventArgs e)
		{
			EventHandler<ResolveEventArgs> RaiseEvent = ResolveEvent;
			if (RaiseEvent != null)
				RaiseEvent(this, e);
		}

		public class MapEventArgs : EventArgs
		{
			public MapEventArgs(int[] mapping)
			{
				this.mapping = mapping;
			}

			public int[] Mapping
			{
				get
				{
					return mapping;
				}
			}

			int[] mapping;
		}

		public event EventHandler<MapEventArgs> MapEvent;

		protected void OnMap(MapEventArgs e)
		{
			EventHandler<MapEventArgs> RaiseEvent = MapEvent;
			if (RaiseEvent != null)
				RaiseEvent(this, e);
		}

		public class ModelEventArgs : EventArgs
		{
			public ModelEventArgs(int[] model)
			{
				this.model = model;
			}

			public int[] Model
			{
				get
				{
					return model;
				}
			}

			int[] model;
		}

		public event EventHandler<ModelEventArgs> ModelEvent;

		protected void OnModel(ModelEventArgs e)
		{
			EventHandler<ModelEventArgs> RaiseEvent = ModelEvent;
			if (RaiseEvent != null)
				RaiseEvent(this, e);
		}
	}
}