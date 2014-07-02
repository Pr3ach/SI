#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <TlHelp32.h>	//For CreateToolhelp32Snapshot()
#include <CommCtrl.h>
#include <string.h>		
#include "main.h"
#include "core.h"
#include "option.h"
#include "init.h"

int init(HWND hwnd)
{
	HANDLE hFile;
	char iniLastDllPath[1024]={0};
	char iniLastProcessPath[1024]={0};
	WNDCLASS dlgc={0};
	keepOnTop=FALSE;
	closeWhenDone=FALSE;
	saveLogToDisk=FALSE;

	// register dlg class	
	dlgc.hInstance=hInst;
	dlgc.lpfnWndProc=dlgProc;
	dlgc.lpszClassName=(LPCSTR)"dlg";
	dlgc.hbrBackground=CreateSolidBrush(RGB(250,250,250));
	
	if(!RegisterClass(&dlgc))
		exit(1);		

	GetCurrentDirectory(sizeof(startDir),startDir);//Save original directory path (cuz it changes after GetOpenFileName)

	memcpy(iniPath,startDir,strlen(startDir));
	strcat(iniPath,"\\SI.ini");

	memcpy(logPath,startDir,strlen(startDir));
	strcat(logPath,"\\SI.log");
	
	/* .ini check */
		//Make sure to use the right share access when creating
	hFile=CreateFile(iniPath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	
	if((long)hFile==ERROR_FILE_NOT_FOUND) // Dont check .ini if it doesnt exists yet, save time/optimization++
		return 0;

	if(GetPrivateProfileInt("Options","alwaysTop",0,iniPath)==1)
	{
		SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
		keepOnTop=TRUE;
	}
	
	if(GetPrivateProfileInt("Options","closeAfter",0,iniPath)==1)
		closeWhenDone=TRUE;	// Value to check later at the end of injections
	
	if(GetPrivateProfileInt("Options","saveLog",0,iniPath)==1)	
		saveLogToDisk=TRUE;
	
	GetPrivateProfileString("Options","lastDllPath","",iniLastDllPath,sizeof(iniLastDllPath),iniPath);
	if(strlen(iniLastDllPath)!=0)	
		SendMessage(dllEdit,WM_SETTEXT,0,(LPARAM)iniLastDllPath);	

	GetPrivateProfileString("Options","lastProcessPath","",iniLastProcessPath,sizeof(iniLastProcessPath),iniPath);
	if(strlen(iniLastProcessPath)!=0)	
		SendMessage(createProcessEdit,WM_SETTEXT,0,(LPARAM)iniLastProcessPath);	

	CloseHandle(hFile);

	return 0;
}
