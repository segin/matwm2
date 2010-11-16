#!/usr/bin/env perl
#############################################################################
# converts default_matwmrc into C code that can be compiled into the binary #
#############################################################################
use strict;
use warnings;

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

char *def_cfg[DEF_CFG_LINES] = {
DEF_END
foreach(@lines) {
  chomp;
	print DEF "\t\"$_\",\n";
}
print DEF "};\n";
close DEF;
