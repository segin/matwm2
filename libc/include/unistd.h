/* unistd.h: standard symbolic constants and types 
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <sys/types.h>

void	_exit(int);	
pid_t	fork(void);
pid_t	vfork(void);

ssize_t	read(int, void *, size_t);
ssize_t	write(int, void *, size_t);
int	close(int);

#endif /* __UNISTD_H__ */
