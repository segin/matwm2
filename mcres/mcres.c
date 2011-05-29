/* mcres.c - Minecraft Resolution helper 
 * ISC license goes here
 * vim: ai ts=4 sw=4 encoding=utf-8
 */
 
#include <windows.h>
#include <shellapi.h>
#include "mcres-rc.h"

HMENU hMenu;

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
			break;
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
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = &WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = GetModuleHandle(NULL);	
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = COLOR_APPWORKSPACE;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "McResHelper";
	
	
	trayicon.cbSize = sizeof(NOTIFYICONDATA);
	

}
