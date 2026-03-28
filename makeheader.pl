#!/usr/bin/env perl
####################################################################
# generates all.h wich defines most global variables and functions #
####################################################################
use strict;
use warnings;

sub chkifdef {
	my ($line, $stack) = @_;
	if ($line =~ m/^\s*#ifn?def\s|^\s*#if\s/ || $line =~ m/^\s*#else/) {
		push(@{$stack}, $line);
	} elsif ($line =~ m/^\s*#endif/) {
		pop(@{$stack}) if(${$stack}[$#{$stack}] && ${$stack}[$#{$stack}] =~ m/^\s*#else/);
		if(${$stack}[$#{$stack}] && ${$stack}[$#{$stack}] =~ m/^\s*#ifn?def\s|^\s*#if\s/) {
			pop(@{$stack});
		} else {
			push(@{$stack}, $line);
		}
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
	foreach (@file) {
		&chkifdef($_, \@gv);
		if (s/^([\n\w]+\s+[\s\*\n\w\W\_\-\(\)\[\],=]+;).*$/$1/gi) {
			s/\s*=.+?([,;])/$1/g;
			push(@gv, "extern $_");
		}
	}
	if (@gv) {
		print H "/* global variables from $_ */\n";
		print H @gv;
		print H "\n";
	}
	my @f;
	foreach (@file) {
		&chkifdef($_, \@f);
		if (s/^([\n\w]+\s+[\s\*\n\w\_\-\(\)\[\]]+\(.*\))\s*{.*$/$1/gi) {
			chomp;
			push(@f, "$_;\n");
		}
	}
	if (@f) {
		print H "/* functions from $_ */\n";
		print H @f;
		print H "\n";
	}
}
closedir DIR;
print H "#endif /* __ALL_H__ */\n";
close H;
