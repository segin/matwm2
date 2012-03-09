/* unistd.h: standard symbolic constants and types 
 * Written by Kirn Gill <segin2005@gmail.com>,
 * Mattis Michel <sic_zer0@hotmail.com>
 */

#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <sys/types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern	void	_exit(int);	
extern	pid_t	fork(void);
extern	pid_t	vfork(void);

extern	ssize_t	read(int, void *, size_t);
extern	ssize_t	write(int, void *, size_t);

extern off_t lseek(int fd, off_t off, int whence);
extern int brk(void *addr);


#endif /* __UNISTD_H__ */
