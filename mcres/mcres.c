/* mcres.c - Minecraft Resolution helper 
 * Copyright (c) 2011, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * vim: ai ts=4 sw=4 encoding=utf-8
 */
 
#include <windows.h>
#include <shellapi.h>
#include "mcres-rc.h"

HMENU hMenu;

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	);

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg) {
		case WM_CREATE:
			hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, IDM_ABOUT, "&About");
			AppendMenu(hMenu, MF_STRING, IDM_QUIT, "E&xit");
			break;
		case WM_DESTROY:
			DestroyMenu(hMenu);
			PostQuitMessage(0);
			break;
		// case WM_SHELLNOTIFY:
		//	break;
		default:
			DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
	}
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{
	MSG msg;
	NOTIFYICONDATA trayicon;
	WNDCLASSEX wc;
	HWND hwnd;
	HANDLE prochandle = GetModuleHandle(NULL);
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = &WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = prochandle;	
	wc.hIcon = LoadIcon(prochandle, NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = COLOR_APPWORKSPACE;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "McResHelper";
	
	
	trayicon.cbSize = sizeof(NOTIFYICONDATA);
	

}
