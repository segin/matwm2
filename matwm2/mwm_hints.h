#ifndef __MWM_HINTS_H__
#define __MWM_HINTS_H__
typedef struct { /* yes these values are long and not uint32_t, experience tells me this doesn't work on 64bit systems with uint32_t */
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long          input_mode;
	unsigned long status;
} MWMHints;

#define MWM_HINTS_FUNCTIONS     (1L << 0) /* wheter the functions member of the struct schould be listened to */
#define MWM_HINTS_DECORATIONS   (1L << 1) /* same for decorations member */

#define MWM_FUNC_ALL            (1L << 0) /* means as much as we schould reversely interpret all other bits */
#define MWM_FUNC_RESIZE         (1L << 1) /* self explanatory */
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)

#define MWM_DECOR_ALL           (1L << 0) /* equivalent of MWM_HINTS_FUNCTIONS */
#define MWM_DECOR_BORDER        (1L << 1) /* show border */
#define MWM_DECOR_RESIZEH       (1L << 2) /* show resize handle */
#define MWM_DECOR_TITLE         (1L << 3) /* show title bar */
#define MWM_DECOR_MENU          (1L << 4) /* wich buttons to show */
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)
#endif /* __MWM_HINTS_H__ */
