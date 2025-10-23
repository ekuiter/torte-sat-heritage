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
using System.Diagnostics;

namespace Sol
{
	public class Vector<T>
	{
		public Vector()
		{
		}

		public Vector(int size)
		{
			Size = size;
		}

		public Vector(Vector<T> that)
		{
			Insert(0, that);
		}

		public Vector(Vector<T> that, int first)
		{
			Insert(0, that, first);
		}

		public Vector(Vector<T> that, int first, int last)
		{
			Insert(0, that, first, last);
		}

		public Vector(Vector<T> that, Predicate<T> match)
		{
			Insert(0, that, match);
		}

		public Vector(Vector<T> that, int first, Predicate<T> match)
		{
			Insert(0, that, first, match);
		}

		public Vector(Vector<T> that, int first, int last, Predicate<T> match)
		{
			Insert(0, that, first, last, match);
		}

		public void Push(T item)
		{
			if (size == items.Length)
				Array.Resize<T>(ref items, items.Length << 1);

			items[size++] = item;
		}

		public void Push(int index, T item)
		{
			Debug.Assert(index > -1 && index <= size);

			if (size == items.Length)
				Array.Resize<T>(ref items, items.Length << 1);

			items[size++] = items[index];
			items[index] = item;
		}

		public T Pop()
		{
			Debug.Assert(size > 0);

			if (size < items.Length >> 2)
				Array.Resize<T>(ref items, items.Length >> 1);

			return items[--size];
		}

		public T Pop(int index)
		{
			Debug.Assert(index > -1 && index < size);
			T item = items[index];

			if (size < items.Length >> 2)
				Array.Resize<T>(ref items, items.Length >> 1);

			items[index] = items[--size];
			return item;
		}

		public void Insert(Vector<T> that)
		{
			Insert(size, that);
		}

		public void Insert(int index, Vector<T> that)
		{
			Insert(index, that, 0);
		}

		public void Insert(int index, Vector<T> that, int first)
		{
			Insert(index, that, first, that.size);
		}

		public void Insert(int index, Vector<T> that, int first, int last)
		{
			Insert(index, that, first, last, x => true);
		}

		public void Insert(Vector<T> that, Predicate<T> match)
		{
			Insert(size, that, match);
		}

		public void Insert(int index, Vector<T> that, Predicate<T> match)
		{
			Insert(index, that, 0, match);
		}

		public void Insert(int index, Vector<T> that, int first, Predicate<T> match)
		{
			Insert(index, that, first, that.size, match);
		}

		public void Insert(int index, Vector<T> that, int first, int last, Predicate<T> match)
		{
			Debug.Assert(index > -1 && index <= size && first > -1 && last <= that.size);

			for (; first < last; ++first)
				if (match(that[first]))
					Push(index++, that[first]);
		}

		public void Remove()
		{
			Remove(0);
		}

		public void Remove(int first)
		{
			Remove(first, size);
		}

		public void Remove(int first, int last)
		{
			Remove(first, last, x => true);
		}

		public void Remove(Predicate<T> match)
		{
			Remove(0, match);
		}

		public void Remove(int first, Predicate<T> match)
		{
			Remove(first, size, match);
		}

		public void Remove(int first, int last, Predicate<T> match)
		{
			Debug.Assert(first > -1 && last <= size);

			while (--last >= first)
				if (match(items[last]))
					Pop(last);
		}

		public void ForEach(Action<T> action)
		{
			ForEach(0, action);
		}

		public void ForEach(int first, Action<T> action)
		{
			ForEach(first, size, action);
		}

		public void ForEach(int first, int last, Action<T> action)
		{
			Debug.Assert(first > -1 && last <= size);

			for (; first < last; ++first)
				action(items[first]);
		}

		public int Find(Predicate<T> match)
		{
			return Find(0, match);
		}

		public int Find(int first, Predicate<T> match)
		{
			return Find(first, size, match);
		}

		public int Find(int first, int last, Predicate<T> match)
		{
			Debug.Assert(first > -1 && last <= size);

			for (; first < last; ++first)
				if (match(items[first]))
					return first;

			return last;
		}

		public int Count(Predicate<T> match)
		{
			return Count(0, match);
		}

		public int Count(int first, Predicate<T> match)
		{
			return Count(first, size, match);
		}

		public int Count(int first, int last, Predicate<T> match)
		{
			Debug.Assert(first > -1 && last <= size);
			int count = 0;

			for (; first < last; ++first)
				if (match(items[first]))
					++count;

			return count;
		}

		public int Min(Comparison<T> comparison)
		{
			return Min(0, comparison);
		}

		public int Min(int first, Comparison<T> comparison)
		{
			return Min(first, size, comparison);
		}

		public int Min(int first, int last, Comparison<T> comparison)
		{
			Debug.Assert(first > -1 && last <= size);
			int index = first++;

			for (; first < last; ++first)
				if (comparison(items[index], items[first]) > 0)
					index = first;

			return index;
		}

		public void Sort()
		{
			Sort(0);
		}

		public void Sort(int first)
		{
			Sort(first, size);
		}

		public void Sort(int first, int last)
		{
			Debug.Assert(first > -1 && first <= last && last <= size);
			Array.Sort<T>(items, first, last - first);
		}

		public void Sort(Comparison<T> comparison)
		{
			Sort(0, comparison);
		}

		public void Sort(int first, Comparison<T> comparison)
		{
			Sort(first, size, comparison);
		}

		public void Sort(int first, int last, Comparison<T> comparison)
		{
			Debug.Assert(first > -1 && first <= last && last <= size);
			Array.Sort<T>(items, first, last - first, new Comparer(comparison));
		}

		class Comparer : System.Collections.Generic.IComparer<T>
		{
			public Comparer(Comparison<T> comparison)
			{
				this.comparison = comparison;
			}

			public int Compare(T x, T y)
			{
				return comparison(x, y);
			}

			Comparison<T> comparison;
		}

		public void Copy(Vector<T> that)
		{
			Copy(0, that);
		}

		public void Copy(int index, Vector<T> that)
		{
			Copy(index, that, 0);
		}

		public void Copy(int index, Vector<T> that, int first)
		{
			Copy(index, that, first, that.size);
		}

		public void Copy(int index, Vector<T> that, int first, int last)
		{
			Debug.Assert(index >= 0 && index <= size && first >= 0 && first <= last && last <= that.size && size - index >= last - first);
			Array.Copy(that.items, first, items, index, last - first);
		}

		public void Reverse()
		{
			Reverse(0);
		}

		public void Reverse(int first)
		{
			Reverse(first, size);
		}

		public void Reverse(int first, int last)
		{
			Debug.Assert(first > -1 && first <= last && last <= size);
			Array.Reverse(items, first, last - first);
		}

		public void Swap(int first, int second)
		{
			Debug.Assert(first >= 0 && first < size && second >= 0 && second < size);
			T item = items[first];
			items[first] = items[second];
			items[second] = item;
		}

		public T this[int index]
		{
			get
			{
				Debug.Assert(index >= 0 && index < size);
				return items[index];
			}

			set
			{
				Debug.Assert(index >= 0 && index < size);
				items[index] = value;
			}
		}

		public int Size
		{
			get
			{
				return size;
			}

			set
			{
				Debug.Assert(value >= 0);
				size = value;

				if (value >= items.Length || value < items.Length >> 2)
				{
					int i = 0;
					for (; value > 0; value >>= 1, ++i) ;
					value = Math.Max(1 << i, 4);
					Array.Resize<T>(ref items, value);
				}
			}
		}


		int size;
		T[] items = new T[4];
	}
}
