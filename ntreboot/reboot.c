#include <stdio.h>
#include <windows.h>


typedef enum _SHUTDOWN_ACTION {
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
}SHUTDOWN_ACTION;

DWORD NTAPI NtShutdownSystem(IN SHUTDOWN_ACTION Action);

int main(int argc, char *argv)
{
	HANDLE token;
	if (OpenProcessToken(GetCurrentProcess(),
	TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES, &token))
	{
	    TOKEN_PRIVILEGES priv;

	    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &priv.Privileges[0].Luid);

	    priv.PrivilegeCount = 1;
	    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	    AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, 0);
	    CloseHandle(token);
	}
	NtShutdownSystem(ShutdownReboot);
}
