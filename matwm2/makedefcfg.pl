#!/usr/bin/env perl
open CFG, "<", "default_matwmrc";
my @lines = <CFG>;
close CFG;
my $len = @lines;
open HEADER, ">", "defcfg.h";
print HEADER << "HEADER_END";
#define DEF_CFG_LINES $len

extern char *def_cfg[DEF_CFG_LINES];
HEADER_END
close HEADER;
open DEF, ">", "defcfg.c";
print DEF << "DEF_END";
#include "defcfg.h"

char *def_cfg[DEF_CFG_LINES] = { /* we split up the default configuration into a lot of separate strings (one for each line) because ISO C90 forbids strings longer then 509 chars */
DEF_END
foreach(@lines) {
  chomp;
	print DEF "\t\"$_\",\n";
}
print DEF "};\n";
close DEF;
