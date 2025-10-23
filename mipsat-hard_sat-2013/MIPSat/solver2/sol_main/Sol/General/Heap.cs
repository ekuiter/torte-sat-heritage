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
	public class Heap<T>
	{
		public Heap(Comparison<T> comparison)
		{
			Compare = comparison;
		}

		public void Push(T item)
		{
			items.Push(item);
			PercolateUp(items.Size - 1);
		}

		public T Pop()
		{
			T item = items.Pop(0);
			if (items.Size != 0) PercolateDown(0);
			return item;
		}

		public T this[int index]
		{
			get
			{
				return items[index];
			}
		}

		public int Size
		{
			get
			{
				return items.Size;
			}
		}

		void PercolateUp(int i)
		{
			T item = items[i];
			while ((i != 0) && Compare(item, items[Parent(i)]) < 0)
			{
				items[i] = items[Parent(i)];
				i = Parent(i);
			}
			items[i] = item;
		}

		void PercolateDown(int i)
		{
			T item = items[i];
			while (Left(i) < items.Size)
			{
				int child = ((Right(i) < items.Size) && Compare(items[Right(i)], items[Left(i)]) < 0) ? Right(i) : Left(i);
				if (Compare(item, items[child]) <= 0) break;
				items[i] = items[child];
				i = child;
			}
			items[i] = item;
		}

		int Left(int i)
		{
			return 2 * i + 1;
		}

		int Right(int i)
		{
			return 2 * (i + 1);
		}

		int Parent(int i)
		{
			return (i - 1) >> 1;
		}

		readonly Vector<T> items = new Vector<T>();
		readonly Comparison<T> Compare;
	}

	class NHeap<T>
	{
		public NHeap(Comparison<T> comparison, Getter<T> getter, Setter<T> setter)
		{
			Compare = comparison;
			GetIndex = getter;
			SetIndex = setter;
		}

		public void Push(T item)
		{
			SetIndex(item, items.Size);
			items.Push(item);
			PercolateUp(GetIndex(item));
		}

		public T Pop()
		{
			T item = items.Pop(0);
			SetIndex(item, -1);
			if (items.Size != 0)
			{
				SetIndex(items[0], 0);
				PercolateDown(0);
			}
			return item;
		}

		public void Remove(T item)
		{
			int i = GetIndex(item);
			items.Pop(i);
			SetIndex(item, -1);
			if (items.Size != i)
			{
				SetIndex(items[i], i);
				Update(items[i]);
			}
		}

		public void Update(T item)
		{
			PercolateUp(GetIndex(item));
			PercolateDown(GetIndex(item));
		}

		public bool Contains(T item)
		{
			return GetIndex(item) != -1;
		}

		public T this[int index]
		{
			get
			{
				return items[index];
			}
		}

		public int Size
		{
			get
			{
				return items.Size;
			}
		}

		void PercolateUp(int i)
		{
			T item = items[i];
			while ((i != 0) && Compare(item, items[Parent(i)]) < 0)
			{
				items[i] = items[Parent(i)];
				SetIndex(items[i], i);
				i = Parent(i);
			}
			items[i] = item;
			SetIndex(item, i);
		}

		void PercolateDown(int i)
		{
			T item = items[i];
			while (Left(i) < items.Size)
			{
				int child = ((Right(i) < items.Size) && Compare(items[Right(i)], items[Left(i)]) < 0) ? Right(i) : Left(i);
				if (Compare(item, items[child]) <= 0) break;
				items[i] = items[child];
				SetIndex(items[i], i);
				i = child;
			}
			items[i] = item;
			SetIndex(item, i);
		}

		int Left(int i)
		{
			return 2 * i + 1;
		}

		int Right(int i)
		{
			return 2 * (i + 1);
		}

		int Parent(int i)
		{
			return (i - 1) >> 1;
		}

		readonly Vector<T> items = new Vector<T>();
		readonly Comparison<T> Compare;
		readonly Getter<T> GetIndex;
		readonly Setter<T> SetIndex;
	}

	delegate int Getter<T>(T item);
	delegate void Setter<T>(T item, int index);
}
