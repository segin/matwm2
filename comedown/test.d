/* test.d - main() code to test scrobbleD. */

import std.md5;
import std.stdio;
import std.uri;
import std.string;
import std.file;

// import comedown.scrobbler;

void main(string[] args) 
{
	try { 
		string config = cast(string) std.file.read("lastfm_config.txt");
	} 

	catch(FileException e) {
		writefln("Cannot open config file: " ~ e.toString());
		return;	
	}
	
	writefln("Hello World!");
	//auto encoded = encodeComponent(orig);
	//writefln("Orginal: %s, Encoded: %s", orig, encoded);
}
