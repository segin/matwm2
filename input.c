#include "matwm.h"

unsigned int mousemodmask, nosnapmodmask, *mod_ignore = NULL;
XModifierKeymap *modmap;
keybind *keys = NULL;
int keyn = 0, nmod_ignore = 0;

void keys_alloc(int n) {
	keys = (keybind *) _realloc((void *) keys, n * sizeof(keybind));
}

void key_bind(char *str) {
	keybind k;
	int i;
	k.sym = str_key(&str, &k.mask);
	k.a = NULL;
	str_action(str, &k.a);
	if(!k.a)
		return;
	if((k.a->code == A_NEXT || k.a->code == A_PREV) && !k.mask) {
		key_free(&k);
		return;
	}
	for(i = 0; i < keyn; i++)
		if(keys[i].sym == k.sym && keys[i].mask == k.mask) {
			key_free(&keys[i]);
			keys[i] = k;
			return;
		}
	keys_alloc(keyn + 1);
	keys[keyn] = k;
	keyn++;
}

void keys_update(void) {
	int i;
	modmap = XGetModifierMapping(dpy);
	for(i = 0; i < keyn; i++) {
		keys[i].code = XKeysymToKeycode(dpy, keys[i].sym);
		key_grab(keys[i]);
	}
}

void keys_ungrab(void) {
	int i;
	for(i = 0; i < keyn; i++)
		key_ungrab(keys[i]);
}

void key_free(keybind *k) {
	free((void *) k->a->arg);
	free((void *) k->a);
}

void keys_free(void) {
	int i;
	for(i = 0; i < keyn; i++)
		key_free(&keys[i]);
	free((void *) keys);
	keys = NULL;
	keyn = 0;
}

void key_grab(keybind key) {
	int i;
	XGrabKey(dpy, key.code, key.mask, root, True, GrabModeAsync, GrabModeAsync);
	for(i = 0; i < nmod_ignore; i++)
		XGrabKey(dpy, key.code, key.mask | mod_ignore[i], root, True, GrabModeAsync, GrabModeAsync);
}

void key_ungrab(keybind key) {
	int i;
	XUngrabKey(dpy, key.code, key.mask, root);
	for(i = 0; i < nmod_ignore; i++)
		XUngrabKey(dpy, key.code, key.mask | mod_ignore[i], root);
}

action *buttonaction(int button, int is_double) {
	if(is_double)
		switch(button) {
			case Button1:
				return double1;
			case Button2:
				return double2;
			case Button3:
				return double3;
			case Button4:
				return double4;
			case Button5:
				return double5;
		}
	else
		switch(button) {
			case Button1:
				return button1;
			case Button2:
				return button2;
			case Button3:
				return button3;
			case Button4:
				return button4;
			case Button5:
				return button5;
		}
	return NULL;
}

action *root_buttonaction(int root_button, int is_double) {
	if(is_double)
		switch(root_button) {
			case Button1:
				return root_double1;
			case Button2:
				return root_double2;
			case Button3:
				return root_double3;
			case Button4:
				return root_double4;
			case Button5:
				return root_double5;
		}
	else
		switch(root_button) {
			case Button1:
				return root_button1;
			case Button2:
				return root_button2;
			case Button3:
				return root_button3;
			case Button4:
				return root_button4;
			case Button5:
				return root_button5;
		}
	return NULL;
}

action *keyaction(XEvent *ev) {
	int i;
	for(i = 0; i < keyn; i++)
		if(keys[i].code == ev->xkey.keycode && cmpmodmask(keys[i].mask, ev->xkey.state))
			return keys[i].a;
	return NULL;
}

int key_to_mask(KeyCode key) {
	int i;
	for(i = 0; i < 8 * modmap->max_keypermod; i++)
		if(modmap->modifiermap[i] == key)
			return 1 << (i / modmap->max_keypermod);
	return 0;
}

void button_grab(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask) {
	int i;
	XGrabButton(dpy, button, modmask, w, False, event_mask, GrabModeAsync, GrabModeAsync, None, CurrentTime);
	for(i = 0; i < nmod_ignore; i++)
		XGrabButton(dpy, button, modmask | mod_ignore[i], w, False, event_mask, GrabModeAsync, GrabModeAsync, None, CurrentTime);
}

void button_ungrab(Window w, unsigned int button, unsigned int modmask) {
	int i;
	XUngrabButton(dpy, button, modmask, w);
	for(i = 0; i < nmod_ignore; i++)
		XUngrabButton(dpy, button, modmask | mod_ignore[i], w);
}

int cmpmodmask(int m1, int m2) {
	int i;
	for(i = 0; i < nmod_ignore; i++)
		m2 ^= m2 & mod_ignore[i];
	return m1 == m2;
}
