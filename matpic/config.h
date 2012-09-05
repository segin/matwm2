#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CALC_MAX       512       /* bigger number = more memore usage, but bigger expressions allowed */
#define ARG_MAX        256       /* maximum number of arguments for instructions and directives */
#define BLOCK          2048      /* amount of memory to allocate in one go */

#define INFILE_DEFAULT "<stdin>" /* input file default name */
#define ARCH_DEFAULT   pic14b    /* default architecture */
#define RADIX_DEFAULT  10        /* default radix */

#endif /* __CONFIG_H__ */
