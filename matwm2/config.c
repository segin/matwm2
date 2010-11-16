#include "matwm.h"

XColor bg, ibg, fg, ifg;
GC gc, igc, bgc, ibgc;
int border_width, text_height, title_height, title_spacing, center_title, center_wlist_items, button_size, snapat, button1, button2, button3, button4, button5, click_focus, click_raise, focus_new, taskbar_ontop, dc, first = 1, *buttons_right = NULL, nbuttons_right, *buttons_left = NULL, nbuttons_left, doubleclick_time, double1, double2, double3, double4, double5, fullscreen_stacking;
XFontStruct *font = NULL;
char *no_title = NO_TITLE;

void cfg_read(int initial) {
	char *home = getenv("HOME");
	char *cfg, *cfgfn;
	XGCValues gv;
	cfg_parse_defaults(initial);
	if(read_file(GCFGFN, &cfg) > 0) {
		cfg_parse(cfg, initial);
		free((void *) cfg);
	}
	if(home) {
		cfgfn = (char *) malloc(strlen(home) + strlen(CFGFN) + 2);
		strncpy(cfgfn, home, strlen(home) + 1);
		strncat(cfgfn, "/", 1);
		strncat(cfgfn, CFGFN, strlen(CFGFN));
		if(read_file(cfgfn, &cfg) > 0) {
			cfg_parse(cfg, initial);
			free((void *) cfg);
		}
		free((void *) cfgfn);
	}
	if(!font) {
		fprintf(stderr, "error: font not found\n");
		qsfd_send(ERROR);
	}
	keys_update();
	text_height = font->max_bounds.ascent + font->max_bounds.descent;
	title_height = text_height + title_spacing;
	button_size = text_height - ((text_height % 2) ? 0 : 1);
	gv.line_width = 1;
	gv.font = font->fid;
	gv.foreground = fg.pixel;
	gc = XCreateGC(dpy, root, GCLineWidth | GCForeground | GCFont, &gv);
	gv.foreground = ifg.pixel;
	igc = XCreateGC(dpy, root, GCLineWidth | GCForeground | GCFont, &gv);
	gv.foreground = bg.pixel;
	bgc = XCreateGC(dpy, root, GCLineWidth | GCForeground | GCFont, &gv);
	gv.foreground = ibg.pixel;
	ibgc = XCreateGC(dpy, root, GCLineWidth | GCForeground | GCFont, &gv);
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
	first = 0;
}

void cfg_set_opt(char *key, char *value, int initial) {
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
	if(strcmp(key, "font") == 0) {
		newfont = XLoadQueryFont(dpy, value);
		if(newfont) {
			if(font)
				XFreeFont(dpy, font);
			font = newfont;
		}
	}
	if(strcmp(key, "border_width") == 0) {
		i = strtol(value, NULL, 0);
		if(i > 0)
		  border_width = i;
	}
	if(strcmp(key, "title_spacing") == 0) {
		i = strtol(value, NULL, 0);
		if(i > 0)
			title_spacing = i;
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
	if(strcmp(key, "button1") == 0)
		button1 = str_buttonaction(value);
	if(strcmp(key, "button2") == 0)
		button2 = str_buttonaction(value);
	if(strcmp(key, "button3") == 0)
		button3 = str_buttonaction(value);
	if(strcmp(key, "button4") == 0)
		button4 = str_buttonaction(value);
	if(strcmp(key, "button5") == 0)
		button5 = str_buttonaction(value);
	if(strcmp(key, "double1") == 0)
		double1 = str_buttonaction(value);
	if(strcmp(key, "double2") == 0)
		double2 = str_buttonaction(value);
	if(strcmp(key, "double3") == 0)
		double3 = str_buttonaction(value);
	if(strcmp(key, "double4") == 0)
		double4 = str_buttonaction(value);
	if(strcmp(key, "double4") == 0)
		double4 = str_buttonaction(value);
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
			mod_ignore = (unsigned int *) realloc((void *) mod_ignore, (nmod_ignore + nmod_ignore + 2) * sizeof(unsigned int));
			if(!mod_ignore)
				error();
			mod_ignore[nmod_ignore] = str_modifier(eat(&value, " \t"));
			if(mod_ignore[nmod_ignore] == None)
				continue;
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
	XFreeGC(dpy, gc);
	XFreeGC(dpy, igc);
	XFreeGC(dpy, bgc);
	XFreeGC(dpy, ibgc);
	keys_free();
	cfg_read(0);
	for(i = 0; i < cn; i++) {
		XDestroyWindow(dpy, clients[i]->button_parent_left);
		XDestroyWindow(dpy, clients[i]->button_parent_right);
		free((void *) clients[i]->buttons);
		buttons_create(clients[i]);
		client_update(clients[i]);
		client_update_name(clients[i]);
		(clients[i] == current) ? client_set_bg(clients[i], bg, fg) : client_set_bg(clients[i], ibg, ifg);
		if(clients[i]->flags & IS_TASKBAR)
			client_set_layer(clients[i], taskbar_ontop ? TOP : NORMAL);
		XUngrabButton(dpy, AnyButton, AnyModifier, clients[i]->parent);
		client_grab_buttons(clients[i]);
		if(clients[i]->flags & FULLSCREEN && clients[i]->layer <= NORMAL && fullscreen_stacking != FS_NORMAL)
			client_update_layer(clients[i], (fullscreen_stacking == FS_ALWAYS_ONTOP) ? NORMAL : TOP);
		ewmh_update_extents(clients[i]);
	}
	ewmh_update_number_of_desktops();
}

void str_color(char *str, XColor *c) {
	XColor newcolor, dummy;
	if(XAllocNamedColor(dpy, DefaultColormap(dpy, screen), str, &newcolor, &dummy)) {
		if(!first)
			XFreeColors(dpy, colormap, &c->pixel, 1, 0);
		*c = newcolor;
	}
}

void str_bool(char *str, int *b) {
	if(strcmp(str, "false") == 0)
		*b = 0;
	if(strcmp(str, "true") == 0)
		*b = 1;
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

int str_buttonaction(char *str) {
	if(strcmp(str, "move") == 0)
		return BA_MOVE;
	if(strcmp(str, "resize") == 0)
		return BA_RESIZE;
	if(strcmp(str, "raise") == 0)
		return BA_RAISE;
	if(strcmp(str, "lower") == 0)
		return BA_LOWER;
	if(strcmp(str, "maximize") == 0)
		return BA_MAXIMIZE;
	if(strcmp(str, "expand") == 0)
		return BA_EXPAND;
	if(strcmp(str, "iconify") == 0)
		return BA_ICONIFY;
	if(strcmp(str, "close") == 0)
		return BA_CLOSE;
	return BA_NONE;
}

int str_keyaction(char *str) {
	if(strcmp(str, "next") == 0)
		return KA_NEXT;
	if(strcmp(str, "prev") == 0)
		return KA_PREV;
	if(strcmp(str, "iconify") == 0)
		return KA_ICONIFY;
	if(strcmp(str, "iconify_all") == 0)
		return KA_ICONIFY_ALL;
	if(strcmp(str, "maximize") == 0)
		return KA_MAXIMIZE;
	if(strcmp(str, "fullscreen") == 0)
		return KA_FULLSCREEN;
	if(strcmp(str, "expand") == 0)
		return KA_EXPAND;
	if(strcmp(str, "sticky") == 0)
		return KA_STICKY;
	if(strcmp(str, "title") == 0)
		return KA_TITLE;
	if(strcmp(str, "to_border") == 0)
		return KA_TO_BORDER;
	if(strcmp(str, "close") == 0)
		return KA_CLOSE;
	if(strcmp(str, "exec") == 0)
		return KA_EXEC;
	if(strcmp(str, "next_desktop") == 0)
		return KA_NEXT_DESKTOP;
	if(strcmp(str, "prev_desktop") == 0)
		return KA_PREV_DESKTOP;
	if(strcmp(str, "ontop") == 0)
		return KA_ONTOP;
	if(strcmp(str, "below") == 0)
		return KA_BELOW;
	if(strcmp(str, "raise") == 0)
		return KA_RAISE;
	if(strcmp(str, "lower") == 0)
		return KA_LOWER;
	return KA_NONE;
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
	return B_NONE;
}

void str_buttons(char *str, int **buttons, int *nbuttons) {
	*nbuttons = 0;
	while(str) {
		*buttons = (int *) realloc((void *) *buttons, sizeof(int) * (*nbuttons + 1));
		if(!buttons)
			error();
		(*buttons)[*nbuttons] = str_wbutton(eat(&str, " \t"));
		if((*buttons)[*nbuttons] != B_NONE)
			(*nbuttons)++;
	}
}

