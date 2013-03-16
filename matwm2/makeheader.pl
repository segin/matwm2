#!/usr/bin/env perl
####################################################################
# generates all.h wich defines most global variables and functions #
####################################################################
use strict;
use warnings;

my $level = 0;
my $w = 0;

sub chkifdef {
	my ($line, $stack) = @_;
	if($level && ($line =~ m/^#else/ || $line =~ m/^#endif/)) {
		if(${$stack}[$#{$stack}] && ${$stack}[$#{$stack}] =~ m/^#ifn?def/) {
			pop(@{$stack});
			$w++ if($line =~ m/^#else/);
		} elsif(!($line =~ m/^#endif/ && $w)) {
			push(@{$stack}, $line);
		} else {
			$w--;
		}
		$level-- if($line =~ m/^#endif/);
	}
	if($line =~ m/^#ifn?def/) {
		push(@{$stack}, $line);
		$level++;
	}
}

open H, ">", "all.h";
print H "/* automatically generated header from makeheader.pl */\n";
print H "#ifndef __ALL_H__\n";
print H "#define __ALL_H__\n\n";
opendir DIR, ".";
foreach(readdir DIR) {
	next if not(m/.c$/);
	open F, "<", $_;
	my @file = <F>;
	close F;
	my @gv;
	foreach(@file) {
		&chkifdef($_, \@gv);
		if(s/^([\n\w]+\s+[\s\*\n\w\W\_\-\(\)\[\],=]+;).*$/$1/gi) {
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
		&chkifdef($_, \@f);
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
print H "#endif /* __ALL_H__ */\n";
close H;
