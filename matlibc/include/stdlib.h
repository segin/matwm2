#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NULL ((void *) 0)

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

extern void exit(int status);

#ifdef __cplusplus
}
#endif

#endif /* __STDLIB_H__ */
