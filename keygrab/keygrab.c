/* keygrab.c: Key grabber for StarCraft and Brood War 
 * 
 * Copyright (c) 2010, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Based upon Logos' CD-Key Grabber for StarCraft, see http://www.gamethreat.net/
 * in the forum section for more information.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <windows.h>

static char __sig[] = "dadef37f6c65947a628de3b301ffb729";

enum {
     offset_1_16_1 = 0x12FD10,
     offset_1_16_0 = 0x12FD10,
     offset_1_15_3 = 0x12FD10,
     offset_1_15_2 = 0x13FD10,
     offset_1_15_2_Alt = 0x13FDF0,
     offset_1_15_1 = 0x12FD10
};

struct versions { 
       char *version;
       int ptr;
} offset_table[] = { 
               { "1.15.1", offset_1_15_1 },
               { "1.15.2", offset_1_15_2 },
               { "1.15.2-alt", offset_1_15_2_Alt },
               { "1.15.3", offset_1_15_3 },
               { "1.16.0", offset_1_16_0 },
               { "1.16.1", offset_1_16_1 }
};

void version(void)
{
     puts("keygrab 0.0 - Free Software key grabber for StarCraft / Brood War\n"
          "Based on Logos' CD-Key Grabber for StarCraft");
}

void version2(void)
{
     puts("\nSend bug reports and patches to <segin2005@gmail.com>\n"
          "If you have offsets for older or newer versions, email those too.");
}

void usage(void)
{
     version();
     puts("usage: keygrab [--help] [--version] [--license] [-v <version>]\n"
          "Attempt to connect to Battle.net in StarCraft.\n\n"
          "keygrab can retrieve CD-Keys from StarCraft and Brood War,\n"
          "\n\nOptions:\n\n"
          "\t--help, -h\tDisplays this help.\n"
          "\t--version\tDisplays the program version.\n"
          "\t-v scversion\tAttempt to grab the CD-Key from StarCraft\n"
          "\t\t\twhere scversion is one of these\n\t\t\tsupported versions:\n\n"
          "\t1.15.1\n"
          "\t1.15.2\n"
          "\t1.15.2-alt (in case the regular 1.15.2 doesn't work for you)\n"
          "\t1.15.3\n"
          "\t1.16.0\n"
          "\t1.16.1 (this is default if no version is specified)");
     version2();
}

void *find_offset_for_version(char *ver)
{
     int vercount = sizeof(offset_table) / sizeof(struct versions);
     void *ret = NULL;
     for(int x=0; x<vercount; x++) {
         if(strcmp(ver,offset_table[x].version) == 0) 
             ret = (void *) offset_table[x].ptr;
     };
     return ret;
}

void fatalerror(char *str)
{
    char *buf;
    int errcode = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, errcode, 0, (LPTSTR) &buf, 0, NULL);
    printf("%s: %s", str, buf);
    LocalFree(buf);
    exit(1);
}

void enable_debug(int doit)
{
    DWORD tpSize;
    HANDLE thisproc;
    HANDLE token;
    LUID luid;
    TOKEN_PRIVILEGES tp, oldtp;
    
    int r; 
    
    thisproc = GetCurrentProcess();
    
    r = OpenProcessToken(thisproc, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    r = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);
        
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = 0;
    
    /* Get attributes */
    AdjustTokenPrivileges(token, 0, &tp, sizeof(tp), &oldtp, &tpSize);

    oldtp.PrivilegeCount = 1;
    oldtp.Privileges[0].Luid = luid;
    if(doit) {
	    oldtp.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
    } else  {
	    oldtp.Privileges[0].Attributes &= (0xffffffff ^ SE_PRIVILEGE_ENABLED);
    }
    AdjustTokenPrivileges(token, 0, &oldtp, tpSize, NULL, NULL);
    CloseHandle(token);
}

int main(int argc, char *argv[])
{
    int opt;
    static int tmp;
    int os_major;
    int os_minor;
    int os_build;
    //usage();
    void *offset;
    int scpid;
    HWND SChWnd;
    char cdkey[31], user[31];
    
    char verstr[16] = "1.16.1";
    
    static struct option long_options[] = { 
           {"version", no_argument, NULL, 'V'},
           {"help",    no_argument, NULL, 'h'}
    };
    while(opt = getopt_long(argc, argv, "hv:V", long_options, &tmp) != -1) {
        switch(opt) {
            case 'v':
                 strncpy(verstr, optarg, 16);
                 break;
            case 'V':
                 version();
                 version2();
                 exit(0);
                 break;
            case 'h':
                 usage();
                 exit(0);
                 break;
            case '?':
                 printf("keygrab: Unknown option -%c\n", optopt);
                 usage();
                 exit(1);
                 break;
            default:
                 printf("** FATAL: getopt_long() returned %#x (%c)\n", opt, opt);
                 abort();
            break;
        }
    }

    tmp = GetVersion();    
    os_major = (0x00ff & tmp);
    os_minor = (0xff00 & tmp) >> 8;
    offset = find_offset_for_version(verstr);
    if (os_major == 6) {
        int tmp = (int) offset - 0x10020;
        offset = (void *) tmp;
    }

    /* Get StarCraft window */
    
    SChWnd = FindWindow("SWarClass", "Brood War");
    if(SChWnd == NULL) SChWnd = FindWindow("SWarClass", "StarCraft");
    if(SChWnd == NULL) fatalerror("Unable to locate StarCraft");
    
    /* Enable debug privlieges */
    enable_debug(1);    
    GetWindowThreadProcessId(SChWnd, (DWORD *) &scpid);
    HANDLE scproc = OpenProcess(PROCESS_ALL_ACCESS, 0, scpid);
    if(!scproc) fatalerror("Unable to attach to StarCraft");
    enable_debug(0);

    ReadProcessMemory(scproc, offset, &tmp, 4, 0);
    ReadProcessMemory(scproc, (LPCVOID) tmp, cdkey, 30, 0);
    ReadProcessMemory(scproc, (void *) ((int) offset + 4), &tmp, 4, 0);
    ReadProcessMemory(scproc, (LPCVOID) tmp, user, 30, 0);
    cdkey[30] = 0; user[30] = 0; /* Make sure strings are null-terminated */
    int keylen = strlen(cdkey);

    
    printf("Registered user: %s\n", user);
    char prettykey[31];
    switch(keylen) {
        case 13:
            strncat(prettykey, cdkey, 4);
            strncat(prettykey, "-", 1);
            strncat(prettykey, &cdkey[4], 5);
            strncat(prettykey, "-", 1);
            strncat(prettykey, &cdkey[9], 4);
            printf("CD-Key: %s\n", prettykey);
            break;
        case 26:
            strncat(prettykey, cdkey, 6);
            strncat(prettykey, "-", 1);
            strncat(prettykey, &cdkey[6], 4);
            strncat(prettykey, "-", 1);
            strncat(prettykey, &cdkey[10], 6);
            strncat(prettykey, "-", 1);
            strncat(prettykey, &cdkey[16], 4);
            strncat(prettykey, "-", 1);
            strncat(prettykey, &cdkey[20], 6);
            printf("CD-Key: %s\n", prettykey);
            break;
        default:
            printf("We have retrieved a key of weird size, wtf?\n"
                   "Connect to Battle.net but do not login before running this!\n");
            printf("CD-Key?: %s\n", cdkey);
            break;
    };
    return 0;
}
