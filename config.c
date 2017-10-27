#include "matwm.h"

XColor bg, ibg, fg, ifg, bfg, ibfg;
GC gc, igc, bgc, ibgc;
int border_spacing, border_width, button_spacing, wlist_margin, wlist_maxwidth, wlist_item_height, text_height, title_height, button_size, title_spacing, snapat, dc, first = 1, *buttons_right = NULL, nbuttons_right = 0, *buttons_left = NULL, nbuttons_left = 0, doubleclick_time, fullscreen_stacking, ewmh_screen;
bool center_title, center_wlist_items, click_focus, click_raise, focus_new, taskbar_ontop, map_center, drag_warp, allow_focus_stealing, correct_center, correct_center_unmanaged, correct_center_separate, click_root;
action *button1 = NULL, *button2 = NULL, *button3 = NULL, *button4 = NULL, *button5 = NULL, *double1 = NULL, *double2 = NULL, *double3 = NULL, *double4 = NULL, *double5 = NULL;
action *root_button1 = NULL, *root_button2 = NULL, *root_button3 = NULL, *root_button4 = NULL, *root_button5 = NULL, *root_double1 = NULL, *root_double2 = NULL, *root_double3 = NULL, *root_double4 = NULL, *root_double5 = NULL;
#ifdef USE_XFT
XftFont *xftfont = NULL;
XftColor xftfg, xftbg, xftifg, xftibg;
#endif
XFontStruct *font = NULL;
char *no_title = NO_TITLE;

void cfg_read(int initial) {
	char *home = getenv("HOME");
	char *cfg, *cfgfn;
	XGCValues gv;
	int i, j;
	/* read compiled-in default config */
	cfg_parse_defaults(initial);
	/* read global configuration file */
	if(read_file(GCFGFN, &cfg) > 0) {
		cfg_parse(cfg, initial);
		free((void *) cfg);
	}
	/* read per-user configuration file */
	if(home) {
		i = strlen(home);
		j = strlen(CFGFN);
		cfgfn = (char *) _malloc(i + j + 2);
		strncpy(cfgfn, home, i + 1);
		strncat(cfgfn, "/" CFGFN, j + 1);
		if((i = read_file(cfgfn, &cfg)) > 0) {
			cfg_parse(cfg, initial);
			free((void *) cfg);
		}
		free((void *) cfgfn);
	}
	/* check if a valid font was set */
	#ifdef USE_XFT
	if(!font && !xftfont) {
	#else
	if(!font) {
	#endif
		fprintf(stderr, NAME ": error: font not found\n");
		exit(EXIT_FAILURE);
	}
	/* set variables that depend on font dimensions */
	#ifdef USE_XFT
	if(xftfont)
		text_height = xftfont->ascent + xftfont->descent;
	else
	#endif
	text_height = font->max_bounds.ascent + font->max_bounds.descent;
	title_height = text_height + title_spacing;
	button_size = text_height - ((text_height % 2) ? 0 : 1);
	wlist_item_height = text_height + (wlist_margin * 2);
	/* create graphics contexts */
	gv.line_width = 1;
	gv.foreground = fg.pixel;
	if(font)
		gv.font = font->fid;
	gc = XCreateGC(dpy, root, GCLineWidth | GCForeground | (font ? GCFont : 0), &gv);
	gv.foreground = ifg.pixel;
	igc = XCreateGC(dpy, root, GCLineWidth | GCForeground | (font ? GCFont : 0), &gv);
	gv.foreground = bg.pixel;
	bgc = XCreateGC(dpy, root, GCLineWidth | GCForeground | (font ? GCFont : 0), &gv);
	gv.foreground = ibg.pixel;
	ibgc = XCreateGC(dpy, root, GCLineWidth | GCForeground | (font ? GCFont : 0), &gv);
	#ifdef USE_XFT
	/* set Xft colors */
	set_xft_color(&xftfg, fg);
	set_xft_color(&xftbg, bg);
	set_xft_color(&xftifg, ifg);
	set_xft_color(&xftibg, ibg);
	#endif
	/* grab keys etc */
	keys_update();
}

void cfg_parse_defaults(int initial) {
	char *opt, *key, *value;
	int i, j;
	for(i = 0; i < DEF_CFG_LINES; i++) {
		j = strlen(def_cfg[i]);
		opt = _malloc(j + 1);
		strncpy(opt, def_cfg[i], j + 1);
		value = eat(&opt, "#");
  	unescape(value);
		key = eat(&value, " \t");
		if(value) {
			while(*value == ' ' || *value == '\t')
				value++;
			for(j = strlen(value) - 1; value[j] == ' ' || value[j] == '\t'; j--);
			value[j + 1] = 0;
		}
		cfg_set_opt(key, value, initial);
		free(opt);
	}
	first = 0;
}

void cfg_parse(char *cfg, int initial) {
	char *opt, *key;
	int i;
	while(cfg) {
		opt = eat(&cfg, "\n");
		opt = eat(&opt, "#");
		unescape(opt);
		key = eat(&opt, " \t");
		if(opt) {
			while(*opt == ' ' || *opt == '\t')
				opt++;
			for(i = strlen(opt) - 1; opt[i] == ' ' || opt[i] == '\t'; i--);
			opt[i + 1] = 0;
		}
		cfg_set_opt(key, opt, initial);
	}
}

void cfg_set_opt(char *key, char *value, int initial) {
	#ifdef USE_XFT
	XftFont *newxftfont;
	#endif
	XFontStruct *newfont;
	long i;
	if(strcmp(key, "resetkeys") == 0)
		keys_free();
	if(!value)
		return;
	if(strcmp(key, "key") == 0)
		key_bind(value);
	if(strcmp(key, "exec") == 0 && initial)
		spawn(value);
	if(strcmp(key, "exec_onload") == 0)
		spawn(value);
	if(strcmp(key, "background") == 0)
		str_color(value, &bg);
	if(strcmp(key, "inactive_background") == 0)
		str_color(value, &ibg);
	if(strcmp(key, "foreground") == 0)
		str_color(value, &fg);
	if(strcmp(key, "inactive_foreground") == 0)
		str_color(value, &ifg);
	if(strcmp(key, "border_color") == 0)
		str_color(value, &bfg);
	if(strcmp(key, "inactive_border_color") == 0)
		str_color(value, &ibfg);
	if(strcmp(key, "font") == 0) {
		newfont = XLoadQueryFont(dpy, value);
		if(newfont) {
			if(font)
				XFreeFont(dpy, font);
			#ifdef USE_XFT
			if(xftfont) {
				XftFontClose(dpy, xftfont);
				xftfont = NULL;
			}
			#endif
			font = newfont;
		}
		#ifdef USE_XFT
		else {
			newxftfont = XftFontOpenName(dpy, screen, value);
			if(newxftfont) {
				if(xftfont)
					XftFontClose(dpy, xftfont);
				if(font) {
					XFreeFont(dpy, font);
					font = NULL;
				}
				xftfont = newxftfont;
			}
		}
		#endif
	}
	if(strcmp(key, "border_width") == 0) {
		i = strtol(value, NULL, 0);
		if(i >= 0)
		  border_width = i;
	}
	if(strcmp(key, "border_spacing") == 0) {
		i = strtol(value, NULL, 0);
		if(i >= 0)
			border_spacing = i;
	}
	if(strcmp(key, "title_spacing") == 0) {
		i = strtol(value, NULL, 0);
		if(i >= 0)
			title_spacing = i;
	}
	if(strcmp(key, "button_spacing") == 0) {
		i = strtol(value, NULL, 0);
		if(i >= 0)
			button_spacing = i;
	}
	if(strcmp(key, "wlist_margin") == 0) {
		i = strtol(value, NULL, 0);
		if(i >= 0)
			wlist_margin = i;
	}
	if(strcmp(key, "wlist_maxwidth") == 0) {
		i = strtol(value, NULL, 0);
		if(i >= 0)
			wlist_maxwidth = i;
	}
	if(strcmp(key, "doubleclick_time") == 0) {
		i = strtol(value, NULL, 0);
		if(i > 0)
			doubleclick_time = i;
	}
	if(strcmp(key, "snap") == 0)
		snapat = strtol(value, NULL, 0);
	if(strcmp(key, "desktops") == 0) {
		i = strtol(value, NULL, 0);
		if(i > 0)
			dc = i;
	}
	if(strcmp(key, "ewmh_screen") == 0) {
		i = strtol(value, NULL, 0);
		if(i > 0)
			ewmh_screen = i;
	}
	if(strcmp(key, "button1") == 0)
	 	str_action(value, &button1);
	if(strcmp(key, "button2") == 0)
		str_action(value, &button2);
	if(strcmp(key, "button3") == 0)
		str_action(value, &button3);
	if(strcmp(key, "button4") == 0)
		str_action(value, &button4);
	if(strcmp(key, "button5") == 0)
		str_action(value, &button5);
	if(strcmp(key, "double1") == 0)
		str_action(value, &double1);
	if(strcmp(key, "double2") == 0)
		str_action(value, &double2);
	if(strcmp(key, "double3") == 0)
		str_action(value, &double3);
	if(strcmp(key, "double4") == 0)
		str_action(value, &double4);
	if(strcmp(key, "double5") == 0)
		str_action(value, &double5);
	if(strcmp(key, "root_button1") == 0)
	 	str_action(value, &root_button1);
	if(strcmp(key, "root_button2") == 0)
		str_action(value, &root_button2);
	if(strcmp(key, "root_button3") == 0)
		str_action(value, &root_button3);
	if(strcmp(key, "root_button4") == 0)
		str_action(value, &root_button4);
	if(strcmp(key, "root_button5") == 0)
		str_action(value, &root_button5);
	if(strcmp(key, "root_double1") == 0)
		str_action(value, &root_double1);
	if(strcmp(key, "root_double2") == 0)
		str_action(value, &root_double2);
	if(strcmp(key, "root_double3") == 0)
		str_action(value, &root_double3);
	if(strcmp(key, "root_double4") == 0)
		str_action(value, &root_double4);
	if(strcmp(key, "root_double5") == 0)
		str_action(value, &double5);
	if(strcmp(key, "click_focus") == 0)
		str_bool(value, &click_focus);
	if(strcmp(key, "click_raise") == 0)
		str_bool(value, &click_raise);
	if(strcmp(key, "focus_new") == 0)
		str_bool(value, &focus_new);
	if(strcmp(key, "fullscreen_stacking") == 0)
		str_fsstacking(value, &fullscreen_stacking);
	if(strcmp(key, "taskbar_ontop") == 0)
		str_bool(value, &taskbar_ontop);
	if(strcmp(key, "center_title") == 0)
		str_bool(value, &center_title);
	if(strcmp(key, "center_wlist_items") == 0)
		str_bool(value, &center_wlist_items);
	if(strcmp(key, "map_center") == 0)
		str_bool(value, &map_center);
	if(strcmp(key, "drag_warp") == 0)
		str_bool(value, &drag_warp);
	if(strcmp(key, "allow_focus_stealing") == 0)
		str_bool(value, &allow_focus_stealing);
	if(strcmp(key, "correct_center") == 0)
		str_bool(value, &correct_center);
	if(strcmp(key, "correct_center_unmanaged") == 0)
		str_bool(value, &correct_center_unmanaged);
	if(strcmp(key, "correct_center_separate") == 0)
		str_bool(value, &correct_center_separate);
	if(strcmp(key, "click_root") == 0)
		str_bool(value, &click_root);
	if(strcmp(key, "mouse_modifier") == 0)
		str_key(&value, &mousemodmask);
	if(strcmp(key, "no_snap_modifier") == 0)
		str_key(&value, &nosnapmodmask);
	if(strcmp(key, "ignore_modifier") == 0) {
		if(mod_ignore) {
			nmod_ignore = 0;
			free((void *) mod_ignore);
			mod_ignore = NULL;
		}
		while(value) {
			mod_ignore = (unsigned int *) _realloc((void *) mod_ignore, (nmod_ignore + nmod_ignore + 1) * sizeof(unsigned int));
			skiprealloc:
			mod_ignore[nmod_ignore] = str_modifier(eat(&value, " \t"));
			if(mod_ignore[nmod_ignore] == None)
				goto skiprealloc;
			for(i = 0; i < nmod_ignore; i++)
				mod_ignore[nmod_ignore + 1 + i] = mod_ignore[i] | mod_ignore[nmod_ignore];
			nmod_ignore += nmod_ignore + 1;
		}
	}
	if(strcmp(key, "buttons_left") == 0)
		str_buttons(value, &buttons_left, &nbuttons_left);
	if(strcmp(key, "buttons_right") == 0)
		str_buttons(value, &buttons_right, &nbuttons_right);
}

void cfg_reinitialize(void) {
	int i;
	#ifdef USE_XFT
	int xft = xftfont ? 1 : 0;
	#endif
	/* free things from old configuration */
	XFreeGC(dpy, gc);
	XFreeGC(dpy, igc);
	XFreeGC(dpy, bgc);
	XFreeGC(dpy, ibgc);
	keys_free();
	/* read config again */
	cfg_read(0);
	/* update some things */
	p_attr.background_pixel = fg.pixel;
	p_attr.border_pixel = ibfg.pixel;
	select_root_events();
	/* update clients */
	for(i = 0; i < cn; i++) {
		#ifdef USE_XFT
		if(xftfont && !xft)
			clients[i]->wlist_draw = XftDrawCreate(dpy, clients[i]->wlist_item, visual, colormap);
		if(!xftfont && xft) {
			XftDrawDestroy(clients[i]->title_draw);
			XftDrawDestroy(clients[i]->wlist_draw);
		}
		#endif
		XDestroyWindow(dpy, clients[i]->button_parent_left);
		XDestroyWindow(dpy, clients[i]->button_parent_right);
		free((void *) clients[i]->buttons);
		clients[i]->flags ^= clients[i]->flags & HAS_BUTTONS;
		buttons_create(clients[i]); /* buttons are now on top of the client window */
		XRaiseWindow(dpy, clients[i]->window); /* hence this line */
		client_update(clients[i]);
		client_update_name(clients[i]);
		(clients[i] == current) ? client_set_bg(clients[i], bg, fg) : client_set_bg(clients[i], ibg, ifg);
		if(clients[i]->flags & IS_TASKBAR)
			client_set_layer(clients[i], taskbar_ontop ? TOP : NORMAL);
		XUngrabButton(dpy, AnyButton, AnyModifier, clients[i]->parent);
		client_grab_buttons(clients[i]);
		if(clients[i]->flags & FULLSCREEN && clients[i]->layer <= NORMAL && fullscreen_stacking != FS_NORMAL)
			client_update_layer(clients[i], (fullscreen_stacking == FS_ALWAYS_ONTOP) ? NORMAL : TOP);
		if(clients[i]->flags & HAS_BORDER)
			XSetWindowBorderWidth(dpy, clients[i]->parent, border_width);
		ewmh_update_extents(clients[i]);
	}
	ewmh_update_number_of_desktops();
	ewmh_update_geometry();
	ewmh_update_strut();
}

void str_color(char *str, XColor *c) {
	XColor newcolor, dummy;
	if(XAllocNamedColor(dpy, colormap, str, &newcolor, &dummy)) {
		if(!first)
			XFreeColors(dpy, colormap, &c->pixel, 1, 0);
		*c = newcolor;
	}
}

#ifdef USE_XFT
void set_xft_color(XftColor *xftcolor, XColor xcolor) {
	xftcolor->pixel = xcolor.pixel;
	xftcolor->color.red = xcolor.red;
	xftcolor->color.green = xcolor.green;
	xftcolor->color.blue = xcolor.blue;
	xftcolor->color.alpha = USHRT_MAX;
}
#endif

void str_bool(char *str, bool *b) {
	if(strcmp(str, "false") == 0)
		*b = false;
	if(strcmp(str, "true") == 0)
		*b = true;
}

void str_fsstacking(char *str, int *s) {
	if(strcmp(str, "normal") == 0)
		*s = FS_NORMAL;
	if(strcmp(str, "ontop") == 0)
		*s = FS_ONTOP;
	if(strcmp(str, "always_ontop") == 0)
		*s = FS_ALWAYS_ONTOP;
}

KeySym str_key(char **str, unsigned int *mask) {
	int mod;
	char *k;
	*mask = 0;
	while(*str) {
		k = eat(str, "\t ");
		mod = str_modifier(k);
		if(!mod)
			return XStringToKeysym(k);
		*mask = *mask | mod;
	}
	return NoSymbol;
}

unsigned int str_modifier(char *name) {
	if(strcmp(name, "shift") == 0)
		return ShiftMask;
	if(strcmp(name, "lock") == 0)
		return LockMask;
	if(strcmp(name, "control") == 0)
		return ControlMask;
	if(strcmp(name, "mod1") == 0)
		return Mod1Mask;
	if(strcmp(name, "mod2") == 0)
		return Mod2Mask;
	if(strcmp(name, "mod3") == 0)
		return Mod3Mask;
	if(strcmp(name, "mod4") == 0)
		return Mod4Mask;
	if(strcmp(name, "mod5") == 0)
		return Mod5Mask;
	return 0;
}

void str_action(char *str, action **ret) {
	char *act = eat(&str, " \t");
	free(*ret);
	*ret = _malloc(sizeof(action));
	if(strcmp(act, "next") == 0)
		(*ret)->code = A_NEXT;
	else if(strcmp(act, "prev") == 0)
		(*ret)->code = A_PREV;
	else if(strcmp(act, "iconify") == 0)
		(*ret)->code = A_ICONIFY;
	else if(strcmp(act, "iconify_all") == 0)
		(*ret)->code = A_ICONIFY_ALL;
	else if(strcmp(act, "maximize") == 0)
		(*ret)->code = A_MAXIMIZE;
	else if(strcmp(act, "fullscreen") == 0)
		(*ret)->code = A_FULLSCREEN;
	else if(strcmp(act, "expand") == 0)
		(*ret)->code = A_EXPAND;
	else if(strcmp(act, "sticky") == 0)
		(*ret)->code = A_STICKY;
	else if(strcmp(act, "title") == 0)
		(*ret)->code = A_TITLE;
	else if(strcmp(act, "to_border") == 0)
		(*ret)->code = A_TO_BORDER;
	else if(strcmp(act, "close") == 0)
		(*ret)->code = A_CLOSE;
	else if(strcmp(act, "exec") == 0)
		(*ret)->code = A_EXEC;
	else if(strcmp(act, "next_desktop") == 0)
		(*ret)->code = A_NEXT_DESKTOP;
	else if(strcmp(act, "prev_desktop") == 0)
		(*ret)->code = A_PREV_DESKTOP;
	else if(strcmp(act, "ontop") == 0)
		(*ret)->code = A_ONTOP;
	else if(strcmp(act, "below") == 0)
		(*ret)->code = A_BELOW;
	else if(strcmp(act, "raise") == 0)
		(*ret)->code = A_RAISE;
	else if(strcmp(act, "lower") == 0)
		(*ret)->code = A_LOWER;
	else if(strcmp(act, "move") == 0)
		(*ret)->code = A_MOVE;
	else if(strcmp(act, "resize") == 0)
		(*ret)->code = A_RESIZE;
	else if(strcmp(act, "quit") == 0)
		(*ret)->code = A_QUIT;
	else {
		free(*ret);
		*ret = NULL;
		return;
	}
	if(str) {
		while(*str == ' ' || *str == '\t')
			str++;
		(*ret)->arg = (char *) _malloc(strlen(str) + 1);
		strncpy((*ret)->arg, str, strlen(str) + 1);
	} else if((*ret)->code == A_EXEC) {
		free(*ret);
		*ret = NULL;
	}	else (*ret)->arg = NULL;
}

int str_wbutton(char *button) {
	if(strcmp(button, "close") == 0)
		return B_CLOSE;
	if(strcmp(button, "maximize") == 0)
		return B_MAXIMIZE;
	if(strcmp(button, "expand") == 0)
		return B_EXPAND;
	if(strcmp(button, "iconify") == 0)
		return B_ICONIFY;
	if(strcmp(button, "sticky") == 0)
		return B_STICKY;
	if(strcmp(button, "ontop") == 0)
		return B_ONTOP;
	if(strcmp(button, "below") == 0)
		return B_BELOW;
	if(strcmp(button, "fullscreen") == 0)
		return B_FULLSCREEN;
	return B_NONE;
}

void str_buttons(char *str, int **buttons, int *nbuttons) {
	*nbuttons = 0;
	while(str) {
		*buttons = (int *) _realloc((void *) *buttons, sizeof(int) * (*nbuttons + 1));
		(*buttons)[*nbuttons] = str_wbutton(eat(&str, " \t"));
		if((*buttons)[*nbuttons] != B_NONE)
			(*nbuttons)++;
	}
}
