#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include "md5.h"

void usage(const char *argv0);
void version(void);
int do_md5sum(const char *file, md5_byte_t *sum);

static int flags[8];

enum {
	FLAG_FILEMODE = 0,
	FLAG_CHECK,
	FLAG_QUIET,
	FLAG_STATUS,
	FLAG_WARN,
	FLAG_HELP,
	FLAG_VERSION,
	FLAG_BAD
};

enum {
	MODE_TEXT = 1,
	MODE_BINARY
};

void version(void)
{
	puts("md5sum version 0.1");
}

int do_md5sum(const char *file, md5_byte_t *sum)
{
	FILE *fd;
	char *md;
	if(flags[FLAG_FILEMODE] == MODE_BINARY) {
		md = "bt";
	} else {
		md = "rt";
	}
	/* if((fd = fopen( */
}

void usage(const char *argv0)
{
	printf("usage: %s [OPTION]... [FILE]...\n", argv0);
	printf("\nWhen FILE is - or not specified, read standard input.\n");
	printf("\nOptions:\n\n\t-b, --binary\tread in binary mode\n"
	       "\t-c, --check\tcheck files from MD5 lists in FILES\n"
	       "\t-t, --text\tread in text mode (default)\n"
	       "\nThe next three are only valid when checking"
	       "files against lists:\n"
	       "\t    --quiet\tdon't print OK for each file passing as valid\n"
	       "\t    --status\tdon't output anything, status codes show success\n"
	       "\t-w, --warn\twarn about invalid formatting in listfiles\n\n"
	       "\t    --help\tPrints usage and quits.\n"
	       "\t    --version\t Prints version and quits.\n\n"
	       "The -c, --check flag is unimplemented.\n"
	       "Send bug reports to <segin2005@gmail.com>.\n");
	exit(flags[FLAG_BAD]);
}

int main(int argc, char *argv[])
{
	md5_state_t state;
	int ch;
	md5_byte_t sum[16];

	static struct option longopts[] = {
		{ "text",	no_argument,	0,	't' },
		{ "binary",	no_argument,	0,	'b' },
		{ "check",	no_argument,	0,	'c' },
		{ "quiet",	no_argument,	0, 	'q' },
		{ "status",	no_argument,	0,	's' },
		{ "warn",	no_argument,	0,	'w' },
		{ "help",	no_argument,	0,	'h' },
		{ "version",	no_argument,	0,	'v' }
	};
	
	memset(flags, 0, sizeof(flags));
	memset(sum, 0, sizeof(sum));

	while((ch = getopt_long(argc, argv, "bctw", longopts, NULL)) != -1) {
		switch(ch) { 
			case 't':
				if(flags[FLAG_FILEMODE] == 0) {
					flags[FLAG_FILEMODE] = MODE_TEXT;
				} else {
					fprintf(stderr, "%s: --binary and "
						"--text are mutually "
						"exclusive.\n", argv[0]); 
				break;
			case 'b':
				flags[FLAG_FILEMODE] = MODE_BINARY;
				break;
			case 'c':
				flags[FLAG_CHECK] = 1;
				break;
			case 'q':
				flags[FLAG_QUIET] = 1;
				break;
			case 's':
				flags[FLAG_STATUS] = 1;
				break;
			case 'w':
				flags[FLAG_WARN] = 1;
				break;
			case 'h':
				version();
				usage(argv[0]);	
				break;
			case 'v':
				version();
				exit(0);
				break;
			case '?':
				flags[FLAG_BAD] = 1;
				usage(argv[0]);
				break; 
		};
	};
	if((flags[FLAG_FILEMODE] > 0) && (flags[FLAG_CHECK] > 0)) {
		fprintf(stderr, "%s: --binary and --text are "
				"meaningless with --check.\n", argv[0]);
		exit(2);
	}
	for(ch = 0; ch < 8; ch++) printf("%d ", flags[ch]);
	putchar('\n');
	while(optind < argc) {
		if(flags[FLAG_CHECK] == 1) {
			/* check_list(argv[optind++]); */
		} else {
			do_md5sum(argv[optind++], sum);
		}
	};
	return 0;	
}
