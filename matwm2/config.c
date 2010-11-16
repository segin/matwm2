#include "matwm.h"

XColor bg, ibg, fg, ifg;
GC gc, igc, bgc, ibgc;
int border_width, text_height, title_height, button_parent_width, snapat, button1, button2, button3, button4, button5, click_focus, click_raise, focus_new, taskbar_ontop, dc, first = 0;
XFontStruct *font = NULL;
char *no_title = NO_TITLE;

void cfg_read(int initial) {
	char *home = getenv("HOME");
	char *cfg, *cfgfn;
	XGCValues gv;
	cfg = (char *) malloc(strlen(DEF_CFG) + 1);
	strncpy(cfg, DEF_CFG, strlen(DEF_CFG));
	cfg_parse(cfg, initial);
	free((void *) cfg);
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
		exit(1);
	}
	keys_update();
	text_height = font->max_bounds.ascent + font->max_bounds.descent;
	title_height = text_height + 2;
	button_parent_width = (text_height * 4) + 6;
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

void cfg_parse(char *cfg, int initial) {
	char *opt, *key;
	int i;
	while(cfg) {
		opt = eat(&cfg, "\n");
		opt = eat(&opt, "#");
		key = eat(&opt, "\t ");
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
	XColor dummy;
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
	if(strcmp(key, "snap") == 0)
		snapat = strtol(value, NULL, 0);
	if(strcmp(key, "desktops") == 0) {
		i = strtol(value, NULL, 0);
		if(i > STICKY)
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
	if(strcmp(key, "click_focus") == 0)
		str_bool(value, &click_focus);
	if(strcmp(key, "click_raise") == 0)
		str_bool(value, &click_raise);
	if(strcmp(key, "focus_new") == 0)
		str_bool(value, &focus_new);
	if(strcmp(key, "taskbar_ontop") == 0)
		str_bool(value, &taskbar_ontop);
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
}

void cfg_reinitialize(void) {
	int i;
	for(i = 0; i < cn; i++) {
		client_update(clients[i]);
		(clients[i] == current) ? client_set_bg(clients[i], bg, fg) : client_set_bg(clients[i], ibg, ifg);
		if(clients[i]->flags & IS_TASKBAR)
			client_set_layer(clients[i], taskbar_ontop ? TOP : NORMAL);
		XMoveWindow(dpy, clients[i]->title, border_width - 1, border_width - 1);
		XUngrabButton(dpy, AnyButton, AnyModifier, clients[i]->parent);
		client_grab_buttons(clients[i]);
	}
}

void str_color(char *str, XColor *c) {
	XColor newcolor, dummy;
	if(!first && XAllocNamedColor(dpy, DefaultColormap(dpy, screen), str, &newcolor, &dummy)) {
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

