/* sto: segin's STOrage format, version 2.0
 * If there are any linker errors with strnlen,
 * please compile the provided strnlen.c with this.
 */

/* If compiling with Digital Mars C or any other comiler capable of 
 * generating Win32s binaries, please define DIGITAL_MARS_C with it.
 */

/* As of yet, it doesn't compile. It probably is broken on MS-DOS as well. */

/* TODO: Use mmap() on the archive. */

/* If using Turbo C 2.0 */
/* #define MSDOS */

/* This is required to compile cleanly on a few platforms */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#ifndef MSDOS
#ifndef DIGITAL_MARS_C
/* if NOT MS-DOS and NOT windows */
#include <unistd.h>
#endif
#endif
/* Crappy windows crap */
#ifdef DIGITAL_MARS_C
#include <windows.h>
#endif

/* BCC compiler, NOT gcc, intel cc, etc. */
#if __BCC__ 
#if __AS386_16__
typedef unsigned long int uint32_t;
typedef unsigned int uint16_t;
#else /* 32-bit code */
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
#endif
/* Not BCC (Turbo C, intel cc, gcc, lcc, msvc ( i guess) */
#else
#ifndef MSDOS
/* Doesn't exist in Turbo C. Don't need it for Turbo C anyays.
 * It keeps GCC quiet.
 */
#include <stdint.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef MSDOS
#if !defined(DIGITAL_MARS_C) || !defined(__TCC__)
#include <strings.h>
#endif
#ifndef __TCC__
#include <utime.h>
#endif /* __TCC__ */
#endif
#ifdef MSDOS
/* We use Turbo C 2.03 for DOS, so it's 16-bit code. */
typedef unsigned long int uint32_t;
#endif

/* Opening header */
typedef struct
{
	char magic[4];
	uint32_t os;
	uint32_t entries;
} sto_header;

/* File entry header */
typedef struct
{
	uint32_t fexist;
	uint32_t flen;
	char fname[256];
	uint32_t os;
	uint32_t fattrib;
	uint32_t fowner;
	uint32_t fgroup;
	time_t fatime;
	time_t fmtime;
	time_t fctime;
} sto_fentry;

char *flinkname;

/* STO_MAGIC is the string that identifies a file as a valid archive */
#define OLD_STO_MAGIC "STO!"
#define STO_MAGIC "STOx"
/* What OS the archive was made on */
#define OS_UNIX 1
#define OS_DOS  2 /* I don't see much hope of this ever being used */
#define OS_WIN  3
#define OS_VMS	4
#define OS_DEADBEEF	0xDEADBEEF /* Thanks, Microsoft. */

static struct stat ostat;
static char fpath[1024];
#ifndef MSDOS
static struct utimbuf tb;
#endif

void badarg(void *mem)
{
	puts("usage: sto [c|x] archive.sto [files ...]");
	free(mem);
	exit(1);
}

int getos()
{
#ifdef MSDOS
					return(OS_DOS);
/* If unix was defined, or is compiling under BCC for a 
 * ELKS or Linux target (It won't compile for a standalone target anyways.
 *
 * BCC can cross-compile to DOS .COM, but a native compile with Turbo C is
 * better.
 */
#elif unix || (__BCC__ && !__MSDOS__)
					return(OS_UNIX);
#elif __WIN32__
					return(OS_WIN);
#else /* DEADBEEF! */
					return(OS_DEADBEEF);
#endif /* MSDOS */
}

/* Windows is very gay, so we need to write a 'fake' sync() function cause
 * it's too gay to have one. */
#ifdef __WIN32__
sync(){}
#endif

#if defined(__WIN32__) || defined(MSDOS) || defined(__BCC__)
#include "strnlen.c"

/* Return the name-within-directory of a file name.
   Copyright (C) 1996,97,98,2002 Free Software Foundation, Inc.
   This function is part of the GNU C Library. */

char *
basename (filename)
     const char *filename;
{
  char *p = strrchr (filename,
#ifdef MSDOS
  '\\'
#else /* UNIX, Linux, ELKS, etc. */
  '/'
#endif /* MSDOS */
  );

  return p ? p + 1 : (char *) filename;
}
#endif

#ifdef MSDOS

/* MS-DOS does not have a sync() function, and
 * has no buffering system except SMARTdrive.
 * so call up smartdrv.exe with /c to flush
 * the buffers.
 *
 * We make sure that no nasty "Bad command or file name"
 * appears as well :)
 */

void sync(void)
{
	char *path;
	path = searchpath("SMARTDRV.EXE");
	if (path == NULL) return;
	system("smartdrv /c");
}

/* The C library provided with Turbo C 2.0 doesn't have bzero.
 * Just map the call to memset.
 */
#endif /* MSDOS */
#if __WIN32__ || MSDOS
void bzero(void *buf, size_t len)
{
	memset(buf,0,len);
	return;
}
#endif

void print_attrib(sto_fentry fentry)
{
	/* This function is inefficent */
	int islnk = 0;
	/* First letter of mode */
	if(S_ISREG(fentry.fattrib)) {
		printf("-");
		islnk = 0;
	} else if (S_ISLNK(fentry.fattrib)) {
		printf("l");
		islnk = 1;
	} else if (S_ISDIR(fentry.fattrib)) {
		printf("d");
		islnk = 0;
	}
	if (fentry.fattrib & S_IRUSR) { printf("r"); } else { printf("-"); }
	if (fentry.fattrib & S_IWUSR) { printf("w"); } else { printf("-"); }
	if (fentry.fattrib & S_IXUSR) { printf("x"); } else { printf("-"); }
	if (fentry.fattrib & S_IRGRP) { printf("r"); } else { printf("-"); }
	if (fentry.fattrib & S_IWGRP) { printf("w"); } else { printf("-"); }
	if (fentry.fattrib & S_IXGRP) { printf("x"); } else { printf("-"); }
	if (fentry.fattrib & S_IROTH) { printf("r"); } else { printf("-"); }
	if (fentry.fattrib & S_IWOTH) { printf("w"); } else { printf("-"); }
	if (fentry.fattrib & S_IXOTH) { printf("x"); } else { printf("-"); }
	printf("  %d/%d	%d	%s%s",fentry.fowner,fentry.fgroup,fentry.flen,fpath,fentry.fname);
	if (islnk == 1) { 
		printf(" -> %s\n",flinkname);
	} else {
		printf("\n");
	}
}

void addfile(FILE *archive, char *file, sto_fentry fentry, char *linkname);

void enddir(FILE *archive)
{
	sto_fentry fentry;
	fentry.fexist = 3; /* End of directory */
	fentry.flen = 0;
	fentry.fattrib = 0;  /* Make sure this "file" as no attributes */
	fentry.fowner = 0;
	fentry.fgroup = 0;
	fentry.fatime = 0;
	fentry.fmtime = 0;
	fentry.fctime = 0;
	fentry.fname[0] = '\0';
	fentry.os = getos();
	strcat(fentry.fname,"..");
	fwrite(&fentry,sizeof(sto_fentry),1,archive);	
}

void adddir(FILE* archive, char *dirname, sto_fentry fentry, char *linkname) 
{ 
  DIR *dir = opendir(dirname); 
  struct dirent *de; 
  while((de = readdir(dir)) != NULL)
    { 
     addfile(archive, de->d_name, fentry, linkname);
    } 
  closedir(dir); 
  enddir(archive);
} 

void addfile(FILE *archive, char *file, sto_fentry fentry, char *linkname)
{
	int fstat,x;
	
	fstat = lstat(file,&ostat);
	if(fstat == -1) {
		perror(file);
		goto end;
	}
	if(S_ISLNK(ostat.st_mode)) {
		readlink(file, linkname, 256);	
		fentry.fexist = (char) 1;
		fentry.flen = ostat.st_size;
		fentry.fattrib = ostat.st_mode;
		fentry.fowner = ostat.st_uid;
		fentry.fgroup = ostat.st_gid;
		fentry.fatime = ostat.st_atime;
		fentry.fmtime = ostat.st_mtime;
		fentry.fctime = ostat.st_ctime;
		fentry.fname[0] = '\0';
		fentry.os = getos();
		strcat(fentry.fname,basename(file));
		fwrite(&fentry,sizeof(sto_fentry),1,archive);
		fwrite(linkname,256,1,archive);
		goto end;
	}
	if(S_ISDIR(ostat.st_mode)) {
		/* here we go! */
		fentry.fexist = (char) 1;
		fentry.flen = ostat.st_size;
		fentry.fattrib = ostat.st_mode;
		fentry.fowner = ostat.st_uid;
		fentry.fgroup = ostat.st_gid;
		fentry.fatime = ostat.st_atime;
		fentry.fmtime = ostat.st_mtime;
		fentry.fctime = ostat.st_ctime;
		fentry.fname[0] = '\0';
		fentry.os = getos();
		strcat(fentry.fname,basename(file));
		/* Possible source of infinite recursion */
		adddir(archive, fentry.fname, fentry, linkname);
	}
	end:
	x=x;
}

int main(int argc, char **argv)
{
	FILE *fd;
	FILE *archive;
	int x,y;
	char *mem;
	int done;
	sto_header header;
	sto_fentry fentry;
	char linkname[256];

	/* Doing some basic size checks */
	if (sizeof(uint32_t) != 4) {
		puts("uint32_t not properly defined!");
		free(mem);
		exit(1);
	}
	
	flinkname = malloc(256);
	mem = malloc(2);
	/* Make sure the memory that holds the file name is blank,
	 * otherwise we get a lot of garbage in the archive */
	bzero(fentry.fname,256);
	bzero(linkname,256);
	bzero(flinkname,256);
	/* Die if not enough arguments */
	if (argc < 2) {
		badarg(mem);
	}

	/* Die if first argument is longer than one letter */
	if (strnlen(argv[1],2) != 1) {
		badarg(mem);
	}

	/* Check if first arg is c or x, if not, die */
	switch(argv[1][0]) {
		case 'c':
			/* Someone didn't specify a new archive name
			 * and a file to add
			 */
			if (argc < 4) {
				badarg(mem);
			}
			/* Here we add support for gzipping .sto archives 
			 * with piping the .sto archive to gzip via 
			 * stdout.
			 */
			if (strcmp(argv[2],"-") == 0) {
				archive = stdin;
			} else {
				archive = fopen(argv[2],"wb");
			}
			/* Archive file can't be used, exiting */
			if (archive == NULL) {
				perror(argv[0]);
				free(mem);
				exit(1);
			}
			strcpy(header.magic,STO_MAGIC);
			/* This is the totall possible number of files
			 * in the archive. A header is created for each
			 * file specified, and a null header is generated
			 * if the file is unreadable. Wastes space, I know,
			 * but it works.
			 */
			header.entries = argc - 3;
			/* 
			 * Identifies the OS the archive was created on. 
			 */
			header.os = getos();
			/* write opening header */
			fwrite(&header,(sizeof header),1,archive);
			/* Main archival loop */
			/* for(x=0;x<header.e;x++) { */
			x=0;
			while(done != 1) {			
				addfile(archive,argv[x+3],fentry,linkname);
				x++;
			}
#if 0
				/* old loop */
				fd = fopen(argv[x+3],"rb");
				if (fd == NULL) { /* File is unreadable */
					perror(argv[x+3]); /* bitch about it */
					fentry.fexist = 0;
					fentry.flen = 0;
					fentry.fname[0] = '\0';
					fstatus = fwrite(&fentry,sizeof(sto_fentry),1,archive);
					/* We were unable to write the entire 
					 * header, which means our archive file
					 * is invalid or unwritable. Bitch 
					 * about it and die. 
					 */
				if (fstatus != sizeof(sto_fentry)) {
						perror(argv[2]);
						free(mem);
						exit(1);
					}
				} else {
					stat(argv[x+3],&ostat);
					fentry.fexist = (char) 1;
					fentry.flen = ostat.st_size;
					fentry.fattrib = ostat.st_mode;
					fentry.fowner = ostat.st_uid;
					fentry.fgroup = ostat.st_gid;
					fentry.fatime = ostat.st_atime;
					fentry.fmtime = ostat.st_mtime;
					fentry.fctime = ostat.st_ctime;
					fentry.fname[0] = '\0';
					strcat(fentry.fname,basename(argv[x+3]));
					fstatus = fwrite(&fentry,sizeof(sto_fentry),1,archive);
					/* We were unable to write the entire 
					 * header, which means our archive file
					 * is invalid or unwritable. Bitch 
					 * about it and die. 
					 */
					if (fstatus != sizeof(sto_fentry)) {
						perror(argv[2]);
						free(mem);
						exit(1);
					}
					/* File copy loop -- ignores errors */
					while (fread(mem,1,1,fd))
						fwrite(mem,1,1,archive);

				}
				/* Clean out the garbage, close the archive,
				 * and flush the OS's write buffers
				 */
				bzero(fentry.fname,256);
				fclose(fd);
				sync();
				end:
				x++;
			};
#endif
			break;
		case 'x':
			/* Need archive name to extract */
			if (argc < 3) {
				badarg(mem);
			}
			
			/* Here we add support for gzip'd .sto archives 
			 * with piping the zcat'ed the .sto.gz to stdin
			 */
			if (strcmp(argv[2],"-") == 0) {
				archive = stdin;
			} else {
				archive = fopen(argv[2],"rb");
			}
			
			/* Archive doesn't exist or is unreadable */
			if (archive == NULL) {
				perror(argv[2]);
				free(mem);
				exit(1);
			}
			fread(&header,sizeof(sto_header),1,archive);
			/* Check the header. If it contains junk
			 * data (i.e. not "STO!"), bitch and die.
			 */
			if (strncmp(header.magic,STO_MAGIC,4) != 0 &&
				strncmp(header.magic,OLD_STO_MAGIC,4) != 0) {
				puts("Bad archive!");
				free(mem);
				exit(1);
			}
			if (strncmp(header.magic,STO_MAGIC,4 == 0)) {
			/* New Extraction Loop */
			/* while */

			} else {
			/* old Extraction loop */
			for(x=0;x<header.entries;x++) {
				fread(&fentry,sizeof(sto_fentry),1,archive);
				if (fentry.fexist == 0)
					goto end2;
				if (S_ISLNK(fentry.fattrib)) {
					fread(flinkname,256,1,archive);
					symlink(flinkname, fentry.fname);
					goto attribs;
				}	
				fd = fopen(fentry.fname,"wb");
				if (fentry.fexist == 2) {
					fclose(archive);
					free(mem);
#ifdef DIGITAL_MARS_C
					MessageBox(0,"STO completed sucessfully.","STO",0);
#endif
					exit(0);
				}
				if (fd == NULL) perror(fentry.fname);
				if (fentry.flen != 0)
					for(y=0;y<fentry.flen;y++) {
						fread(mem,1,1,archive);
						fwrite(mem,1,1,fd);
					};
				fclose(fd);
				/* MS-DOS has no equilvant functions 
				 * that give compatable UNIX time
				 */
				attribs:
#ifndef MSDOS
				tb.actime = fentry.fatime;
				tb.modtime = fentry.fmtime;
				utime(fentry.fname,&tb);
#endif
				chmod(fentry.fname,fentry.fattrib);
				print_attrib(fentry);
				end2:
				x=x; /* Come up with a better no-op and i'll use it. */
			};
			} /* end old loop */
			break;
	};
	free(mem);
	free(flinkname);
#ifdef DIGITAL_MARS_C
	MessageBox(0,"STO completed sucessfully.","STO",0);
#endif
	exit(0);
}
