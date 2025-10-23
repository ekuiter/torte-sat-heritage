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
using System.IO;
using System.Text;

namespace Sol
{
	public partial class CNF
	{
		public static int Read(string location, out int[][] clauses)
		{
			int vcount, ccount;
			StreamReader input = new StreamReader(location);
			SkipComments(input);
			ReadHeader(input, out vcount, out ccount);
			ReadClauses(input, vcount, ccount, out clauses);
			input.Close();
			return vcount;
		}

		static void SkipComments(StreamReader input)
		{
			SkipWhitespace(input);
			while ((char)input.Peek() == 'c')
			{
				input.ReadLine();
				SkipWhitespace(input);
			}
		}

		static void ReadHeader(StreamReader input, out int vcount, out int ccount)
		{
			if ((char)input.Read() != 'p')
				throw new ArgumentException("Unexpected character!");
			SkipWhitespace(input);
			if ((char)input.Read() != 'c' || (char)input.Read() != 'n' || (char)input.Read() != 'f')
				throw new ArgumentException("Unexpected character!");
			SkipWhitespace(input);
			vcount = int.Parse(ReadFragment(input));
			SkipWhitespace(input);
			ccount = int.Parse(ReadFragment(input));
		}
		
		static void ReadClauses(StreamReader input, int vcount, int ccount, out int[][] clauses)
		{
			clauses = new int[ccount][];
			int[] literals = new int[vcount];
			for (int i = 0; i < ccount; ++i)
			{
				int lcount = 0;
				while (true)
				{
					SkipWhitespace(input);
					int x = int.Parse(ReadFragment(input));
					if (x == 0)
						break;
					if (Math.Abs(x) > vcount)
						throw new ArgumentException("Variable is out of range!");
					literals[lcount++] = x;
				}
				int[] clause = new int[lcount];
				Array.Copy(literals, clause, lcount);
				clauses[i] = clause;
			}
		}
		
		static void SkipWhitespace(StreamReader input)
		{
			while (input.Peek() != -1 && char.IsWhiteSpace((char)input.Peek()))
				input.Read();
		}

		static string ReadFragment(StreamReader input)
		{
			StringBuilder sb = new StringBuilder();
			while (input.Peek() != -1 && !char.IsWhiteSpace((char)input.Peek()))
				sb.Append((char)input.Read());
			return sb.ToString();
		}
	}
}
