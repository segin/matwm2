/* readbin: A program for reading and displaying basic executable attributes.
 * Copyright (c) 2006-2011, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/* This program is rather simple; it reads program binaries and displays info
 * about them. If running under Minix/ELKS, ELF support is disabled, hell,
 * you need to #define __ELF__ to get ELF support anyways.
 *
 * And about the little MAGIC_BAD_EXEC joke: When you use system() to run
 * a program, and the program segfaults, the shell returns 139. Since segfaults
 * are signs of a bad program(mer), it seems fitting that 139 be used
 * for the define for bad executable formats (e.g. NOT a program file, or some
 * other format, e.g. PE, COFF, Go-COFF32, Mach-O, etc.)
 * 
 * I refuse any changes if gcc says ANYTHING with -Wall -pedantic
 *
 * for the record, I make a debuggable binary with this command line:
 * gcc -O0 -ggdb3 -D__ELF__ readbin.c -Wall -pedantic -o readbin
 *
 * a.out support (when it happens) is going to suck. a.out sucks
 * The major problem with a.out is the fact that there are many 
 * different versions of a.out. The Linux and ELKS a.out.h headers...
 * Let's not EVEN go there...
 */

#define _GNU_SOURCE
#ifdef __LINUX__
#include <features.h>
#endif
#include <sys/types.h>
#include <errno.h>
#ifdef __ELF__ /* ELKS doesn't do ELF, and Dev86 doesn't give a ELF header */
#include "elf.h"
#endif
#include <a.out.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#ifdef __PE__ /* This is actually MS-DOS, NE, LE/LX, and PE formats. */
#include "pe.h"
#endif /* __PE__ */

#ifndef ELFMAG
# define ELFMAG "\177ELF"
#endif


/* Portability defines */
#ifndef OMAGIC
# define OMAGIC 0407
#endif
#ifndef NMAGIC
/* Code indicating pure executable.  */
# define NMAGIC 0410
#endif
#ifndef ZMAGIC
/* Code indicating demand-paged executable.  */
# define ZMAGIC 0413
#endif
#ifndef QMAGIC
/* This indicates a demand-paged executable with the header in the text. 
   The first page is unmapped to help trap NULL pointer references.  */
# define QMAGIC 0314
#endif
#ifndef CMAGIC
/* Code indicating core file.  */
# define CMAGIC 0421
#endif

#ifdef __BCC__ /* More Linux a.out.h crap for ELKS */
# define N_MAGIC(exec)	((exec).a_info & 0xffff)
# define N_MACHTYPE(exec) ((enum machine_type)(((exec).a_info >> 16) & 0xff))
# define N_FLAGS(exec)	(((exec).a_info >> 24) & 0xff)
#endif

#define MAGIC_ELF32	  1
#define MAGIC_ELF64	  2
#define MAGIC_AOUT_OMAGIC 3
#define MAGIC_AOUT_QMAGIC 4
#define MAGIC_AOUT_ZMAGIC 5
#define MAGIC_AOUT_MINIX  6
#define MAGIC_MS_DOS	 10
#define MAGIC_MS_NE	 11
#define MAGIC_MS_LE	 12
#define MAGIC_MS_PE	 13
#define MAGIC_BAD_ELF	128
#define MAGIC_BAD_AOUT	129
#define MAGIC_BAD_EXEC	139 /* sh return value for segfaulted program (joke) */

static char *aout_binfmt_desc[] = {
	"Core file",
	"Impure executable",
	"Pure executable",
	"Demand-paged executable w/header in .text",
	"Demand-paged executable",
	0
};

static char *aout_magic[] = {
	"CMAGIC",
	"OMAGIC",
	"NMAGIC",
	"QMAGIC",
	"ZMAGIC",
	0
};

static char *aout_arch[] = {
	"Sun 2",
	"68010",
	"68020",
	"SPARC",
	"i386",
	"MIPS1",
	"MIPS2",
	0
};

static char *format_names[] = {
	"INVALID",
	"32-bit ELF",
	"64-bit ELF",
	"Linux a.out OMAGIC",
	"Linux a.out QMAGIC",
	"Linux a.out ZMAGIC",
	"Minix or ELKS a.out",
	"Broken ELF",
	"Broken a.out",
	"unknown binary format",
	"Portable Executable (PE) Format",
	"MS-DOS .exe",
	"New Executable (NE) Format",
	0
};
static char *elf_data_encodings[] = {
 	"Invalid data encoding",
	"2's complement, little endian",
	"2's complement, big endian",
	0
};

static char *elf_version[] = {
	"Invalid version",
	"version 1 (current)",
	0
};

static char *elf_abi_ident[] = {
	"System V UNIX",
	"HP-UX",
	"NetBSD",
	"Linux",
	"GNU/Hurd", 
	"86Open Common UNIX ABI",
	"Sun Solaris",
	"IBM AIX",
	"SGI IRIX",
	"FreeBSD",
	"Compaq Tru64 UNIX",
	"Novell Modesto",
	"OpenBSD",
	"OpenVMS",
	"HP Non-Stop Kernel",
	"","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","ARM","","", /* ARM is 93 */
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","",
	"","","","","","","","","","", /* Embed is 255 */
	"","","","","","","","","","",
	"","","","","","Standalone (embedded) application",0
};

static char *elf_obj_type[] = {
	"NONE (No file type)",
	"REL (Relocatable object)",
	"EXEC (Executable file)",
	"DYN (Shared object file)",
	"CORE (Core dump file)",
	0
};

static char *elf_arch[] = {
	"No machine",
	"AT&T WE 32100",
	"SUN SPARC",
	"Intel 80386",
	"Motorola m68k",
	"Motorola m88k",
	"Intel 80486 (nonstandard)",
	"Intel 80860",
	"MIPS R3000 big-endian",
	"IBM System/370",
	"MIPS R3000 little-endian",
	"",
	"",
	"",
	"",
	"HPPA",
	"",
	"Fujitsu VPP500",
	"Sun's \"v8plus\"",
	"Intel 80960",
	"PowerPC",
	"PowerPC 64-bit",
	"IBM S390",
	"", /* ELF doesn't define what the next 10 archtypes are */
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"NEC V800 series",
	"Fujitsu FR20",
	"TRW RH-32",
	"Motorola RCE",
	"ARM",
	"Digital Alpha",
	"Hitachi SH",
	"SPARC v9 64-bit",
	"Siemens Tricore",
	"Argonaut RISC Core",
	"Hitachi H8/300",
	"Hitachi H8/300H",
	"Hitachi H8S",
	"Hitachi H8/500",
	"Intel Merced",
	"Stanford MIPS-X",
	"Motorola Coldfire",
	"Motorola M68HC12",
	"Fujitsu MMA Multimedia Accelerator",
	"Siemens PCP",
	"Sony nCPU embedded RISC",
	"Denso NDR1 microprocessor",
	"Motorola Star*Core processor",
	"Toyota ME16 processor",
	"STMicroelectronic ST100 processor",
	"Advanced Logic Corp. Tinyj Processor",
	"AMD x86-64 architecture",
	"Sony DSP Processor",
	"",
	"",
	"Siemens FX66 microcontroller",
	"STMicroelectronics ST9+ 8/16 mc",
	"STmicroelectronics ST7 8 bit mc",
	"Motorola MC68HC16 microcontroller",
	"Motorola MC68HC11 microcontroller",
	"Motorola MC68HC08 microcontroller",
	"Motorola MC68HC05 microcontroller",
	"Silicon Graphics SVx",
	"STMicroelectronics ST19 8 bit mc",
	"Digital VAX",
	"Axis Communications 32-bit embedded processor",
	"Infineon Technologies 32-bit embedded processor",
	"Element 14 64-bit DSP Processor",
	"LSI Logic 16-bit DSP Processor",
	"Donald Knuth's educational 64-bit processor",
	"Harvard University machine-independent object files",
	"SiTera Prism",
	"Atmel AVR 8-bit microcontroller",
	"Fujitsu FR30",
	"Mitsubishi D10V",
	"Mitsubishi D30V",
	"NEC v850",
	"Mitsubishi M32R",
	"Matsushita MN10300",
	"Matsushita MN10200",
	"picoJava",
	"OpenRISC 32-bit embedded processor",
	"ARC Cores Tangent-A5",
	"Tensilica Xtensa Architecture",
	0
};

void read_aout_header(FILE *fd, struct exec *header)
{
	rewind(fd);
	fread(header,sizeof(struct exec),1,fd);
}

#ifdef __ELF__
void read_elf32_header(FILE *fd, Elf32_Ehdr *header)
{
	rewind(fd);
	fread(header,sizeof(Elf32_Ehdr),1,fd);
}

void read_elf64_header(FILE *fd, Elf64_Ehdr *header)
{
	rewind(fd);
	fread(header,sizeof(Elf64_Ehdr),1,fd);
}
#endif

int get_file_type(FILE *fd)
{
	char *file_magic; /* note to compiler: shut the fuck up,
			   * it should be void *
			   */
	char *tmp;
	char object_class;
	long pos;
	uint16_t exehdr;
	
	file_magic = (char *)malloc(1024); /* should be enough for almost any header */
	pos = ftell(fd);
	if (pos > 0) rewind(fd);
	fread(file_magic,4,1,fd);
#ifdef __PE__
	exehdr = *(uint16_t *)file_magic;
	if (exehdr == IMAGE_DOS_SIGNATURE) {
		/* It's at least a MS-DOS EXE */
		printf("PE executable support coming soon!\n");
		return MAGIC_BAD_EXEC;	
	}
#endif
	if (strncmp(file_magic,ELFMAG, 4) == 0) {
		fread(&object_class,1,1,fd);
		if (object_class == 0) {
			free((void *)file_magic);
			return MAGIC_BAD_ELF;
		} 
		if (object_class == 1) {
			free((void *)file_magic);
			return MAGIC_ELF32;
		}
		if (object_class == 2) {
			free((void *)file_magic);
			return MAGIC_ELF64;
		} else {
			free((void *)file_magic);
			return MAGIC_BAD_ELF;
		}
	}
	/* Test for a.out formats */
	tmp = malloc(3);
	memcpy(tmp,file_magic,2);
	/* Linux OMAGIC */
	if (tmp[0]==7 && tmp[1] == 1) {
		free((void *)tmp);
		return MAGIC_AOUT_OMAGIC;
	}
	/* Linux QMAGIC */
	if (tmp[0]=='\xCC' && tmp[1] == 0) {
		free((void *)tmp);
		return MAGIC_AOUT_QMAGIC;
	}
	/* Linux ZMAGIC */
	if (tmp[0]==11 && tmp[1] == 1) {
		free((void *)tmp);
		return MAGIC_AOUT_ZMAGIC;
	}
	free((void *)tmp);
	free((void *)file_magic);
	/* Fuck a.out for now. */
	return MAGIC_BAD_EXEC;
}

int aout_get_id(int magic)
{
	switch(magic) {
		case CMAGIC:
			return 0;
		case OMAGIC:
			return 1;
		case NMAGIC:
			return 2;
		case QMAGIC:
			return 3;
		case ZMAGIC:
			return 4;
	};
	return -1;
}

int aout_arch_id(int arch)
{
	if (arch == 0) return 0;
   	if (arch == 1) return 1;
	if (arch == 2) return 2;
	if (arch == 3) return 3;
	if (arch == 100) return 4;
	if (arch == 151) return 5;
	if (arch == 152) return 6;
	return -1;
}

char *get_aout_string(int id, int class)
{
	switch(class) {
		case 1: /* magic strings */
			if (id > -1 && id < 6)
				return aout_magic[id];
			return "unknown magic type";
		case 2: /* format descriptions */
			if (id > -1 && id < 6)
				return aout_binfmt_desc[id];
			return "unknown description";
		case 3: /* arch */
			if (id > -1 && id < 7)
				return aout_arch[id];
			return "unknown arch";
	};	
	return "unknown";
}

#ifndef __NO_AOUT__
void display_aout_header(FILE *fd, struct exec header)
{
	int id,magic,flags;
	magic = N_MAGIC(header);
	id = aout_get_id(magic);
	printf("Magic:				   %s\n",get_aout_string(id,1));
	id = aout_arch_id(N_MACHTYPE(header));
	printf("Machine:			   %s\n",get_aout_string(id,3));
	flags = N_FLAGS(header);
	printf("Flags:				   %#x\n",flags);
	printf("Symbol table size:	 	   %d (bytes)\n",header.a_syms);
	printf(".text relocation size?:		   %d (bytes)\n",N_TRSIZE(header));
	printf(".data relocation size?:		   %d (bytes)\n",N_DRSIZE(header));
	printf(".text segment offset:		   %d\n",N_TXTOFF(header));
	printf(".data segment offset:		   %d\n",N_DATOFF(header));
	printf(".text relocation offset:	   %d\n",N_TRELOFF(header));
	printf(".data relocation offset:	   %d\n",N_DRELOFF(header));
	printf("Symbol table offset:		   %d\n",N_SYMOFF(header));
	printf("String table offset?:		   %d\n",N_STROFF(header));
	printf(".text address when loaded:	   %p\n",(void *)N_TXTADDR(header));
	printf(".data address when loaded:	   %p\n",(void *)N_DATADDR(header));
	printf(".bss address when loaded:	   %p\n",(void *)N_BSSADDR(header));
}
#endif
#ifdef __ELF__
void display_elf32_header(FILE *fd, Elf32_Ehdr header)
{
	
	int x;
	printf("Magic:  ");
	for(x=0;x<16;x++) {
		printf(" %02x",header.e_ident[x]);
	};
	printf("\n");
	
	switch (header.e_ident[EI_DATA]) {
		case 0: /* Got my cast net ready, must cast a char to int   */
		case 1: /* because that shuts up gcc with "-Wall -pedantic" */
		case 2:   
		printf("Data:			 	   %s\n",elf_data_encodings[
			(int) header.e_ident[EI_DATA]]);
	};
	printf("Version: 			   %s\n",
		elf_version[(int) header.e_ident[EI_VERSION]]);
	printf("ABI:	 			   %s\n",
		elf_abi_ident[(int) header.e_ident[EI_OSABI]]);
	printf("ABI Version: 			   %d\n",(int) header.e_ident[EI_ABIVERSION]);
	switch(header.e_type) {
		case 0:
		case 1: 
		case 2:	/* Damn ISO assholes, I have to cast EVERYTHING! */
		case 3:
		case 4:
		printf("Type: 				   %s\n",
			elf_obj_type[(int) header.e_type]);
	}
	if(header.e_type > 4) 
		printf("Type:				   Unknown type %#x\n",
			(int) header.e_type);
	printf("Machine:			   %s\n",elf_arch[(int) header.e_machine]);
	printf("Version:			   %#x\n",(int) header.e_version);
	printf("Entry Point:			   %p\n", (void *) header.e_entry);
	printf("Start of program headers:	   %d (bytes into file)\n",header.e_phoff);
	printf("Start of section headers:	   %d (bytes into file)\n",header.e_shoff);
	printf("Flags:				   %#x\n",header.e_flags);
	printf("Size of this header:		   %d (bytes)\n",header.e_ehsize);
	printf("Size of program headers:	   %d (bytes)\n",header.e_phentsize);
	printf("Number of program headers:	   %d\n",header.e_phnum);
	printf("Size of section headers:	   %d (bytes)\n",header.e_shentsize);
	printf("Number of section headers:	   %d\n",header.e_shnum);
	printf("Section header string table index: %d\n",header.e_shstrndx);
}

void display_elf64_header(FILE *fd, Elf64_Ehdr header)
{
	
	int x;
	printf("Magic:  ");
	for(x=0;x<16;x++) {
		printf(" %02x",header.e_ident[x]);
	};
	printf("\n");
	
	switch (header.e_ident[EI_DATA]) {
		case 0: /* Got my cast net ready, must cast a char to int   */
		case 1: /* because that shuts up gcc with "-Wall -pedantic" */
		case 2:   
		printf("Data:			 	   %s\n",elf_data_encodings[
			(int) header.e_ident[EI_DATA]]);
	};
	printf("Version: 			   %s\n",
		elf_version[(int) header.e_ident[EI_VERSION]]);
	printf("ABI:	 			   %s\n",
		elf_abi_ident[(int) header.e_ident[EI_OSABI]]);
	printf("ABI Version: 			   %d\n",(int) header.e_ident[EI_ABIVERSION]);
	switch(header.e_type) {
		case 0:
		case 1: 
		case 2:	/* Damn ISO assholes, I have to cast EVERYTHING! */
		case 3:
		case 4:
		printf("Type: 				   %s\n",
			elf_obj_type[(int) header.e_type]);
	}
	if(header.e_type > 4) 
		printf("Type:				   Unknown type %#x\n",
			(int) header.e_type);
	printf("Machine:			   %s\n",elf_arch[(int) header.e_machine]);
	printf("Version:			   0x%x\n",(int) header.e_version);
	printf("Entry Point:			   0x%llx\n", header.e_entry);
	printf("Start of program headers:	   %lld (bytes into file)\n",header.e_phoff);
	printf("Start of section headers:	   %lld (bytes into file)\n",header.e_shoff);
	printf("Flags:				   0x%x\n",header.e_flags);
	printf("Size of this header:		   %d (bytes)\n",header.e_ehsize);
	printf("Size of program headers:	   %hd (bytes)\n",header.e_phentsize);
	printf("Number of program headers:	   %hd\n",header.e_phnum);
	printf("Size of section headers:	   %hd (bytes)\n",header.e_shentsize);
	printf("Number of section headers:	   %hd\n",header.e_shnum);
	printf("Section header string table index: %hd\n",header.e_shstrndx);
}

#endif

int main(int argc, char **argv)
{
	FILE *fd;
	int filetype,tmp;
#ifndef __NO_AOUT__
	struct exec aout_header;
#endif
#ifdef __ELF__
	Elf32_Ehdr header;
	Elf64_Ehdr header64;
#endif

	if (argc == 1) {
		printf("usage: readbin /path/to/executable\n");
		exit(1);
	}
	fd = fopen(argv[1],"rb"); 
	if (fd == NULL) { 
		fprintf(stderr,"Error opening %s: %s.\n",argv[1],strerror(errno));
		exit(1);
	}
	filetype = get_file_type(fd);	
	if (filetype == 0) {
		fprintf(stderr,"This program is broken!\n");
		exit(1);
	} else { 
		tmp = filetype;
		if (tmp == MAGIC_BAD_ELF) tmp = 6;
		if (tmp == MAGIC_BAD_AOUT) tmp = 7;
		if (tmp == MAGIC_BAD_EXEC) tmp = 8;
		if (tmp == 0 || tmp > 8) tmp = 0;
		printf("Filetype:			   %s\n",format_names[tmp]);
		if (filetype == MAGIC_ELF32) {
		#ifdef __ELF__
			read_elf32_header(fd, &header);
			display_elf32_header(fd, header);
		#else
			fprintf(stderr,"Fucking hell, ELF not supported!\n");
			fprintf(stderr,"Which means that nothing is supported...\n");
		#endif
		}
		if (filetype == MAGIC_ELF64) {
		#ifdef __ELF__
			read_elf64_header(fd, &header64);
			display_elf64_header(fd, header64);
		#else
			fprintf(stderr,"Fucking hell, ELF not supported!\n");
			fprintf(stderr,"Which means that nothing is supported...\n");
		#endif
		}
#ifndef __NO_AOUT__
		if (filetype == MAGIC_AOUT_OMAGIC || filetype == MAGIC_AOUT_QMAGIC ||
			filetype == MAGIC_AOUT_ZMAGIC) {
			read_aout_header(fd, &aout_header);
			display_aout_header(fd, aout_header);
			printf("a.out support is in the works.\n");
		}
#endif
	}
	return(0);
}
