/*
 * this is a wrapper for the cor port i/o functions for WinNT/2000/XP.
 * this is to be replaced by some functions that are platform/interface
 * specific to access the data lines.
 * for now, this is only for parport access, but in future there will be a way
 * to plug this on the usb bus.
 * if the interface changes, only this file has to be adopted for the target system
 */
#if defined __WIN32__ && defined Z_WANT_LPT

#include <stdio.h>
#include <windows.h>

int read_parport(int port);
void write_parport(int port, int value);
int open_port(int port);

static BOOL bPrivException = FALSE;

int read_parport(int port)
{
	unsigned char value;
#ifdef _MSC_VER
	__asm mov edx,port
	__asm in al,dx
	__asm mov value,al
#else
    /* hmm, i should read some documentation about inline assembler */
    post("lpt: cannot read from parport (recompile!)");
        return 0;
#endif
	return (int)value;
}

void write_parport(int port, int invalue)
{
  /* _outp((unsigned short)port, value); */
  BYTE value = (BYTE)invalue;
#ifdef _MSC_VER
  __asm mov edx,port
  __asm mov al,value
  __asm out dx,al
#else
    /*
     * hmm, i should read some documentation about inline assembler
     * and probably about assembler in general...
     */
    post("lpt: cannot write to parport (recompile!)");
    /*
    asm(
        "mov %%edx,%0\n"
        "mov %%al,%1\n"
        "out %%dx,%%al\n"
        :
        : "a"(port),"b"(value)
        );
    */
#endif
}

static LONG WINAPI HandlerExceptionFilter ( EXCEPTION_POINTERS *pExPtrs )
{

	if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
	{
		pExPtrs->ContextRecord->Eip ++; /* Skip the OUT or IN instruction that caused the exception */
		bPrivException = TRUE;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
		return EXCEPTION_CONTINUE_SEARCH;
}

static BOOL StartUpIoPorts(UINT PortToAccess, BOOL bShowMessageBox, HWND hParentWnd)
{
	HANDLE hUserPort;

	hUserPort = CreateFile("\\\\.\\UserPort", GENERIC_READ, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(hUserPort); /* Activate the driver */
	Sleep(100); /* We must make a process switch */

	SetUnhandledExceptionFilter(HandlerExceptionFilter);
	
	bPrivException = FALSE;
	read_parport(PortToAccess);  /* Try to access the given port address */

	if (bPrivException)
	{
		if (bShowMessageBox)
		{
#if 0
    		MessageBox(hParentWnd,"Privileged instruction exception has occured!\r\n\r\n"
								  "To use this external under Windows NT, 2000 or XP\r\n"
								  "you need to install the driver 'UserPort.sys' and grant\r\n"
								  "access to the ports used by this program.\r\n\r\n"
								  "See the file README for further information!\r\n", NULL, MB_OK);
#endif
		}
		return FALSE;
	}
	return TRUE;
}
	/* check if we are running NT/2k/XP */
static int IsWinNT(void)
{
	OSVERSIONINFO OSVersionInfo;
	OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&OSVersionInfo);

	return OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

	/* open parport */
int open_port(int port)
{
	if(IsWinNT())	/* we are under NT and need kernel driver */
	{
		if(StartUpIoPorts(port, 1, 0))
			return(0);
		return(-1);
	}
	else	/* no need to use kernel driver */
	{
		return(0);
	}
}
#endif /* __WIN32__ & Z_WANT_LPT */
