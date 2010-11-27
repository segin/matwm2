#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include "pe.h"

static int ierrno;
static int i_tbl;
static char *errorstrtbl[] = { 
	"Success",
	"Not a valid executable.",
	"LE executables unsupported.",
	NULL
};

enum {
	newExe_PE,
	newExe_NE,
	newExe_LE
};

struct dosExe { 
	int exeType;
	int peHasOptionalHeader;
	int xxx;
	IMAGE_DOS_HEADER dosHeader;
	union {
		IMAGE_OS2_HEADER neHeader;
		IMAGE_FILE_HEADER peHeader;
	};
};

void init_tbl(void)
{
	int i;
	for(i=0; ;i++) {
		if (!errorstrtbl[i]) {
			i_tbl = i;
			break;
		}
	}
	return;
}

const char *
istrerror(int ierror) 
{
	if (!i_tbl) init_tbl();
	if (ierror > i_tbl) return NULL;
	return errorstrtbl[ierror];
}

int 
main(int argc, char *argv[])
{
	/* we assume that everything on the commandline is an .exe to parse */
	FILE *pefile;
	int i;
	struct dosExe exe;
    
	for (i=1; i < argc; i++) {
		pefile = fopen(argv[i],"rb");
		if (!pefile) { 
			fprintf(stderr, "Cannot open %s: %s\n", 
				argv[i], strerror(errno));
			continue;
		}
		
		/* Check for exe header */
		fread(&exe.dosHeader, sizeof(IMAGE_DOS_HEADER),
			1, pefile);
		
		if(exe.dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
			fprintf(stderr, "Error parsing %s: %s\n",
				argv[i], istrerror(1));
			fclose(pefile);
			continue;
		}
		
		char e_magic[2];
		memcpy(e_magic, exe.dosHeader.e_magic, 2);
		
		printf("DOS header:\n");
		printf("Header magic:				%#x \"%2s\"\n", exe.dosHeader.e_magic, &exe.dosHeader.e_magic);
		printf("Bytes on last page: 			%d\n", exe.dosHeader.e_cblp);
		printf("Pages in file				%d\n", exe.dosHeader.e_cp);
		printf("Relocations count:			%d\n", exe.dosHeader.e_crlc);
		printf("Size of header in paragraphs:		%d\n", exe.dosHeader.e_cparhdr);
		printf("Minimum extra pgraph needed: 		%d\n", exe.dosHeader.e_minalloc);
		printf("Maximum extra pgraph needed:		%d\n", exe.dosHeader.e_maxalloc);
		printf("Initial SS value:			%#x\n", exe.dosHeader.e_ss);
		printf("Initial SP value:			%#x\n", exe.dosHeader.e_sp);
		printf("Checksum				%#x\n", exe.dosHeader.e_csum);
		printf("Initial IP value:			%#x\n", exe.dosHeader.e_ip);
		printf("Initial CS value:			%#x\n", exe.dosHeader.e_cs);
		printf("Offset of relocation table: 		%#x\n", exe.dosHeader.e_lfarlc);
		printf("Overlay number: 			%#x\n", exe.dosHeader.e_ovno);
		printf("OEM Identifier:				%#x\n", exe.dosHeader.e_oemid);
		printf("OEM Information:			%#x\n", exe.dosHeader.e_oeminfo);
		printf("Offset of next header:			%#x\n", exe.dosHeader.e_lfanew);
			
	/* Fuck all this for now. 
	static struct option long_options[] = { 
		{"version", no_argument, NULL, 'V'},
		{"help",    no_argument, NULL, 'h'}
	};
	while(opt = getopt_long(argc, argv, "hv:V", long_options, &tmp) != -1) {
		switch(opt) {
		}
	}
	*/
		if(exe.dosHeader.e_lfanew != 0) { 
			
		fclose(pefile);
	};
	
	system("PAUSE");	
	return 0;
}
