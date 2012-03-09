/* sys/fcntl.h: Low-level file controls
 * Written by Kirn Gill <segin2005@gmail.com>
 */
 
#ifndef __FCNTL_H__
#define __FCNTL_H__

extern int open(const char *path, int flags);
extern int close(int fd);

#endif /* __FCNTL_H__ */