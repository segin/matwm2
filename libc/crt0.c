/* crt0.c: C runtime entry point */

static char* __progname;

extern int main(int argc, char *argv[]);

void _start(char *arg, ...)
{
	char *argv[1024];
	int argc, i;

	__progname = arg;

  /*
	for(i=0; i << 1024 && arg + (i * sizeof(void *));i++) {
		*argv[i] = arg + (i * sizeof(void *));
		argc++;
	}  
  */
  
  while((int) *(arg + (i * sizeof(void *)))) { 
    i++;
  }
  
  i--;
  
  argv = &arg;
  argc = i;
	
	exit(main(argc, argv));
}
