#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <TlHelp32.h>	//For CreateToolhelp32Snapshot()
#include <CommCtrl.h>
#include <string.h>		//Not sure if really needed ...
#include "main.h"
#include "core.h"
#include "option.h"
#include "init.h"


void checkStayOnTop(HWND hdlg)
{
	HANDLE hFile;

	if(SendMessage(stayOnTop,BM_GETCHECK,0,0)==BST_CHECKED)
	{
		SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

		hFile=CreateFile(iniPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ |FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile!=0)
		{		
			WritePrivateProfileString("Options","alwaysTop","1",iniPath);//Current dir -> ".\\"
			keepOnTop=TRUE;
			CloseHandle(hFile);
		}

		else
			MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
	}

	/*BST_UNCHECKED*/
	else
	{
		SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

		hFile=CreateFile(iniPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ |FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile!=0)
		{		
			WritePrivateProfileString("Options","alwaysTop","0",iniPath);
			keepOnTop=FALSE;
			CloseHandle(hFile);
		}

		else
			MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
	}

	return;
}


void checkCloseWhenDone(HWND hdlg)
{
	HANDLE hFile;

	if(SendMessage(closeAfter,BM_GETCHECK,0,0)==BST_CHECKED)
	{
		
		hFile=CreateFile(iniPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ |FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile!=0)
		{		
			WritePrivateProfileString("Options","closeAfter","1",iniPath);//Current dir -> ".\\"
			closeWhenDone=TRUE;
			CloseHandle(hFile);
		}

		else
			MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
	}

	/*BST_UNCHECKED*/
	else
	{

		hFile=CreateFile(iniPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ |FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile!=0)
		{		
			WritePrivateProfileString("Options","closeAfter","0",iniPath);
			closeWhenDone=FALSE;
			CloseHandle(hFile);
		}

		else
			MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
	}

	return;
}



void checkSaveLog(HWND hdlg)
{
	HANDLE hFile;

	if(SendMessage(saveLog,BM_GETCHECK,0,0)==BST_CHECKED)
	{
		
		hFile=CreateFile(iniPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ |FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile!=0)
		{		
			WritePrivateProfileString("Options","saveLog","1",iniPath);
			saveLogToDisk=TRUE;
			CloseHandle(hFile);
		}

		else
			MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
	}

	/*BST_UNCHECKED*/
	else
	{

		hFile=CreateFile(iniPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ |FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile!=0)
		{		
			WritePrivateProfileString("Options","saveLog","0",iniPath);
			saveLogToDisk=FALSE;
			CloseHandle(hFile);
		}

		else
			MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
	}
	
	return;
}


void saveLastPaths(HWND hwnd) 
{
	HANDLE hFile=NULL;

	hFile=CreateFile(iniPath,GENERIC_ALL,FILE_SHARE_READ | FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

	if(hFile==NULL)
	{
		MessageBox(hwnd,"Error configuring SI.ini. CreateFile() failed.","Error - Storm Inj3ctor",MB_ICONERROR);
		CloseHandle(hFile);

		return;
	}

	WritePrivateProfileString("Options","lastDllPath",fileNameDll,iniPath);
	WritePrivateProfileString("Options","lastProcessPath",fileNameProcess,iniPath);

	CloseHandle(hFile);

	return;

}