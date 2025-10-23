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
	static class Algorithm
	{
		public static void Extend<T>(ref T[] array, int size)
		{
			if (size >= array.Length)
			{
				int n = array.Length;
				while (size >= (n *= 2)) ;
				Array.Resize(ref array, n);
			}
		}

		public static void Shrink<T>(ref T[] array, int size)
		{
			if (size < array.Length / 4)
			{
				int n = array.Length;
				while (size < (n /= 2) / 4) ;
				Array.Resize(ref array, n);
			}
		}
		
		public static int Find<T>(T[] array, int first, int last, Predicate<T> match)
		{
			for (; first < last; ++first)
				if (match(array[first]))
					return first;
			return -1;
		}

		public static bool Exists<T>(T[] array, int first, int last, Predicate<T> match)
		{
			return (Find(array, first, last, match) != -1);
		}

		public static int Count<T>(T[] array, int first, int last, Predicate<T> match)
		{
			int count = 0;
			for (; first < last; ++first)
				if (match(array[first]))
					++count;
			return count;
		}

		public static int Remove<T>(T[] array, int first, int last, Predicate<T> match)
		{
			for (int index = last; --index >= first; )
				if (match(array[index]))
					array[index] = array[--last];
			return last;
		}

		public static int RemoveStable<T>(T[] array, int first, int last, Predicate<T> match)
		{
			int index = first;
			for (; first < last; ++first)
				if (!match(array[first]))
					array[index++] = array[first];
			return index;
		}

		public static int Min<T>(T[] array, int first, int last, Comparison<T> comparison)
		{
			int index = first++;
			for (; first < last; ++first)
				if (comparison(array[index], array[first]) > 0)
					index = first;
			return index;
		}

		public static void Sort<T>(T[] array, int first, int last, Comparison<T> comparison)
		{
			Array.Sort(array, first, last - first, new Comparer<T>(comparison));
		}

		class Comparer<T> : System.Collections.Generic.IComparer<T>
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

	}
}
