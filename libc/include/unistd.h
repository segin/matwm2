/* unistd.h: standard symbolic constants and types 
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <sys/types.h>

extern	void	_exit(int);	
extern	pid_t	fork(void);
extern	pid_t	vfork(void);

extern	ssize_t	read(int, void *, size_t);
extern	ssize_t	write(int, void *, size_t);

#endif /* __UNISTD_H__ */
