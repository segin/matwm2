#ifndef __FCNTL_H__
#define __FCNTL_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int open(const char *path, int flags);
extern int close(int fd);

#ifdef __cplusplus
}
#endif

#endif /* __FCNTL_H__ */
