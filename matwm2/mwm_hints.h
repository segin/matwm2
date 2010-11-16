#ifndef __MWM_HINTS_H__
#define __MWM_HINTS_H__
typedef struct {
	unsigned long	flags;
	unsigned long	functions;
	unsigned long	decorations;
	long					input_mode;
	unsigned long	status;
} MWMHints;

#define MWM_HINTS_FUNCTIONS			(1L << 0)
#define MWM_HINTS_DECORATIONS		(1L << 1)

#define MWM_FUNC_ALL						(1L << 0)
#define MWM_FUNC_RESIZE					(1L << 1)
#define MWM_FUNC_MOVE						(1L << 2)
#define MWM_FUNC_MINIMIZE				(1L << 3)
#define MWM_FUNC_MAXIMIZE				(1L << 4)
#define MWM_FUNC_CLOSE					(1L << 5)

#define MWM_DECOR_ALL						(1L << 0)
#define MWM_DECOR_BORDER				(1L << 1)
#define MWM_DECOR_RESIZEH				(1L << 2)
#define MWM_DECOR_TITLE					(1L << 3)
#define MWM_DECOR_MENU					(1L << 4)
#define MWM_DECOR_MINIMIZE			(1L << 5)
#define MWM_DECOR_MAXIMIZE			(1L << 6)
#endif /* __MWM_HINTS_H__ */
