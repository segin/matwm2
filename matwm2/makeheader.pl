#!/usr/bin/env perl
####################################################################
# generates all.h wich defines most global variables and functions #
####################################################################
open H, ">", "all.h";
opendir DIR, ".";
foreach(readdir DIR) {
	next if not(m/.c$/);
	open F, "<", $_;
	@file = <F>;
	close F;
	my @gv;
	foreach(@file) {
		if(s/^([\n\w]+\s+[\s\*\n\w\s\_\-\(\)\[\],=]+;).*$/$1/gi) {
			s/\s*=.+?([,;])/$1/g;
			push(@gv, "extern $_");
		}
	}
	if(@gv) {
		print H "/* global variables from $_ */\n";
		print H @gv;
		print H "\n";
	}
	my @f;
	foreach(@file) {
		if(s/^([\n\w]+\s+[\s\*\n\w\_\-\(\)\[\]]+\(.*\))\s*{.*$/$1/gi) {
			chomp;
			push(@f, "$_;\n");
		}
	}
	if(@f) {
		print H "/* functions from $_ */\n";
		print H @f;
		print H "\n";
	}
}
closedir DIR;
close H;
