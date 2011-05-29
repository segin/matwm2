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

#define WM_SHELLNOTIFY WM_USER+5

HMENU hMenu; HANDLE thread;
NOTIFYICONDATA trayicon; int die = 0;

enum
{
	MINECRAFT_WIDTH = 800,
	MINECRAFT_HEIGHT = 600,
	MINECRAFT_DEPTH = 32
};

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	);

int IsMinecraftRunning(void)
{
	return FindWindow("SunAwtFrame", "Minecraft") != NULL || FindWindow("SunAwtFrame", "Minecraft Launcher") != NULL;
}
	
DWORD WINAPI McResThread(LPVOID lpParam)	
{
	int isRunning, wasRunning;
	DEVMODE old, new;
	while (!die) {
		Sleep(100);
		isRunning = IsMinecraftRunning();
		if (isRunning && !wasRunning) {
			old.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &old);
			new.dmSize = sizeof(DEVMODE);
			new.dmPelsWidth = MINECRAFT_WIDTH;
			new.dmFields |= DM_PELSWIDTH;
			new.dmPelsHeight = MINECRAFT_HEIGHT;
			new.dmFields |= DM_PELSHEIGHT;
			new.dmBitsPerPel = MINECRAFT_DEPTH;
			new.dmFields |= DM_BITSPERPEL;
			int ret = ChangeDisplaySettings(&new, 0);
			if (!ret) { 
				char msg[256];
				ret = GetLastError(); 
				
			}
		}
		if (!isRunning && wasRunning)
			ChangeDisplaySettings(&old, 0);
		wasRunning = isRunning;
	};
	ExitThread(0);
}
	
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
			DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
		case WM_DESTROY:
			die = 1;
			DestroyMenu(hMenu);
			Shell_NotifyIcon(NIM_DELETE, &trayicon);
			PostQuitMessage(0);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDM_ABOUT:
					MessageBox(hWnd, "Minecraft Resolution Helper, v0.2\n\nCopyright © 2011, Kirn Gill <segin2005@gmail.com>", "About Minecraft Resolution Helper", MB_OK | MB_ICONINFORMATION);
					break;
				case IDM_QUIT:
					die = 1;
					DestroyMenu(hMenu);
					Shell_NotifyIcon(NIM_DELETE, &trayicon);
					PostQuitMessage(0);
					break;
			}
		case WM_SHELLNOTIFY:
			switch (lParam) {
				case WM_RBUTTONDOWN:
					;
					POINT pt;
					GetCursorPos(&pt);
					TrackPopupMenu(hMenu, TPM_RIGHTALIGN, pt.x, pt.y, 0, hWnd, NULL);
					PostMessage(hWnd,WM_NULL,0,0);
					break;
				case WM_MOUSEMOVE:
					break;
			}
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
	}
	return 0;
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{
	MSG msg;
	int ret;
	WNDCLASSEX wc;
	HWND hwnd;
	HANDLE prochandle = GetModuleHandle(NULL);
	char t[32];
	static const GUID myGUID = {
		0xf90b590c, 0xbea2, 0x4cb2, 
		{ 0x9c, 0x43, 0xb8, 0x69, 0x8c, 0x57, 0xc4, 0xdd }
	};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = prochandle;
	wc.hIcon = wc.hIconSm = 0;
	wc.hCursor = 0;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "McResHelper";
	
	ret = RegisterClassEx(&wc);
	hwnd = CreateWindowEx(0, "McResHelper", "Minecraft Resolution Helper", 
		0, 0, 0, 0, 0, 0, 0, hInstance, 0); 

	trayicon.cbSize = sizeof(NOTIFYICONDATA);
	trayicon.hWnd = hwnd;
	trayicon.uID = IDI_MCRES;
	trayicon.uCallbackMessage = WM_SHELLNOTIFY;
	trayicon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	trayicon.hIcon = LoadIcon(prochandle, MAKEINTRESOURCE(IDI_MCRES));
	strcpy(trayicon.szTip,"Minecraft Resolution Helper");
	
	Shell_NotifyIcon(NIM_ADD, &trayicon);
	
	while (1)
	{
		ret = GetMessage(&msg, NULL, 0, 0);
		if (!ret) return;
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	} 
	
	Shell_NotifyIcon(NIM_DELETE, &trayicon);	
	return msg.wParam;
}
