/* test.d - main() code to test scrobbleD. */

import std.md5;
import std.stdio;
import std.uri;
import std.string;
import std.file;

import comedown.scrobbler;

void main(string[] args) 
{
	try { 
		string config = cast(string) std.file.read("lastfm_config.txt");
		auto lines = std.string.splitlines(config);
		if(lines.length < 2) { 
			writefln("Config file contains too few lines!");
			return;
		}
		
		for(int i = 0; i < lines.length ; i++) { 
			lines[i] = lines[i].strip();
		}

		auto scrobbler = new Scrobbler("tst", "1.0", lines[0], lines[1]);

		
			
	} 

	catch(FileException e) {
		writefln("Cannot open config file: " ~ e.toString());
		return;	
	}

	writefln("Hello World!");
	//auto encoded = encodeComponent(orig);
	//writefln("Orginal: %s, Encoded: %s", orig, encoded);
}
