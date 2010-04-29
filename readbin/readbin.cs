/* I fuxoringly hate .NET */

using System;

class MainClass
{
	public static void Main(string[] argv)
	{
		int argc;
		argc = argv.GetLength(0);
		
		if (argc == 0) {
			Console.WriteLine("usage: readbin /path/to/exec");
			Environment.Exit(1);
		}
	}
}
