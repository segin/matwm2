#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "md5.h"

void usage(void);
void version(void);
int do_md5sum(const char *file, md5_byte_t *sum);

static int flags[8];
static char *argv0;

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
		md = "rb";
	} else {
		md = "rt";
	}
	if((fd = fopen(file, md)) == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", file, strerror(errno));
		return -1;
	}

	/* Compute MD5 here */

	fclose(fd);
	return 0;	
}

void usage(void)
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

void set_file_mode(int mode)
{
	if(flags[FLAG_FILEMODE] != 0) { 
		fprintf(stderr, "%s: --binary and --text are mutually "
			        "exclusive.\n", argv0);
		exit(1);
	}
	flags[FLAG_FILEMODE] = mode;
}	


int main(int argc, char *argv[])
{
	md5_state_t state;
	int ch;
	md5_byte_t sum[16];

	static char sumstr[36]; /* 33 + alignment */
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

	argv0 = argv[0];

	*((uint32_t *) (&sumstr[32])) = 0;
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
					exit(1);
				} 
				break;
			case 'b':
				if(flags[FLAG_FILEMODE] == 0) {
					flags[FLAG_FILEMODE] = MODE_BINARY;
				} else {
					fprintf(stderr, "%s: --binary and "
						"--text are mutually "
						"exclusive.\n", argv[0]);
					exit(1);
				} 
				break;
			case 'c':
				flags[FLAG_CHECK] = 1;
				printf("%s: -%c flag unimplemented.\n", 
				      argv[0], ch);
				break;
			case 'q':
				flags[FLAG_QUIET] = 1;
				printf("%s: --quiet unimplemented.\n",
				       argv[0]);
				break;
			case 's':
				flags[FLAG_STATUS] = 1;
				printf("%s: --silent unimplemented.\n",
				       argv[0]);
				break;
			case 'w':
				flags[FLAG_WARN] = 1;
				printf("%s: -%c flag unimplemented.\n", 
				      argv[0], ch);
				break;
			case 'h':
				version();
				usage();	
				break;
			case 'v':
				version();
				exit(0);
				break;
			case '?':
				flags[FLAG_BAD] = 1;
				usage();
				break; 
		};
	};
	if((flags[FLAG_FILEMODE] > 0) && (flags[FLAG_CHECK] > 0)) {
		fprintf(stderr, "%s: --binary and --text are "
				"meaningless with -c or --check.\n", argv[0]);
		exit(1);
	}
	if(((flags[FLAG_QUIET] | flags[FLAG_STATUS] | flags[FLAG_WARN]) != 0)
 	   && (flags[FLAG_CHECK] == 0)) { 
		fprintf(stderr, "%s: --quiet, --status, -w, and --warn are "
			"useless without -c or --check.\n", argv[0]);
		exit(1);
	}
	for(ch = 0; ch < 8; ch++) printf("%d ", flags[ch]);
	putchar('\n');
	while(optind < argc) {
		if(flags[FLAG_CHECK] == 1) {
		/* check_list(argv[optind++]); */
		} else {
			do_md5sum(argv[optind++], sum);
			for(ch = 0; ch < 16; ch++) { 
				printf("%02x", sum[ch]);
			}
			putchar(32);
			putchar(flags[FLAG_FILEMODE] == MODE_BINARY? '*':' ');
			printf("%s\n", argv[optind - 1]);
		}
	};
	return 0;	
}
