/*

Storm Inj3ctor By Pr3acher @__Pr3__

v1.6 
- Info (in log file/window) about injected dll [OK]
- Added PPID name [OK]
- Options checkbox in separated window [OK]

v1.5

- Extend writeLog call to errors etc. [OK]
- Change writeLog: append text with SetFilePointer() [OK]
- Store last dll/exe paths in SI.ini [OK]
- Edit DLL is no longer readonly [OK]
- Edits support RETURN and TAB keys [OK]


v1.4

- Fixed several bugs concerning SI.ini config file
- Added ability to save a log
- Fixed a bug in date/time format 

*/

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <TlHelp32.h>	
#include <CommCtrl.h>
#include "main.h"
#include "core.h"
#include "option.h"
#include "init.h"

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevHinstance,LPSTR cmdLine,int show)
{
	
	MSG uMsg;
	WNDCLASS wc={0};
	char isDbg=0;
	int argc=0;

	wc.hInstance=hInstance;
	wc.lpszClassName="class";
	wc.lpfnWndProc=mainProc;	
	wc.hbrBackground=CreateSolidBrush(RGB(250,250,250));
	wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(1));

	if(!RegisterClass(&wc))
		return 1;

	hInst=hInstance;

	// quick Anti-Debug tricks

	__asm
	{
		mov eax,fs:[30h] // PEB base addr
		mov ebx,[eax+2]	 // PEB.BeingDebugged
		mov isDbg,bl
	}

	if(isDbg)
		exit(-2);

	hwnd=CreateWindowEx(0,"class","Storm Inj3ctor 1.6 - By Pr3acher",WS_SYSMENU | WS_MINIMIZEBOX,300,300,400,365,0,0,hInstance,0);

	if(!hwnd)
	exit(-1);

	ShowWindow(hwnd,SW_SHOW);
	UpdateWindow(hwnd);

	while(GetMessage(&uMsg,0,0,0))
	{
		TranslateMessage(&uMsg);
		DispatchMessage(&uMsg);
	}

	return uMsg.wParam;

}


LRESULT CALLBACK mainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HBRUSH back;

	switch(uMsg)
	{
	case WM_CREATE:
		WMC(hwnd);
		break;

	case WM_COMMAND: // Message sent by user action, wParam and lParam contain info about the message
		userMsg(hwnd,uMsg,wParam,lParam);
		break;

	case WM_CTLCOLORSTATIC: // Sets background and text color
		hdc=(HDC)wParam;
    if((HWND)lParam==logEdit)
    {		
	back=CreateSolidBrush(RGB(45,45,45));
    SetTextColor(hdc,RGB(255,255,255));
    SetBkColor(hdc,RGB(45,45,45));
    return (INT_PTR)back; /* When handling this msg (WM_CTLCOLORSTATIC), we gotta return a bkg brush, cast INT_PTR = int for x86/x64 */
    ReleaseDC(hwnd,hdc);
    }
		 break;

	case WM_DESTROY:
		SetWindowLongPtr(dllEdit,GWL_WNDPROC,(WNDPROC)wndpOldDllEditProc); // restore original WndProc of dllEdit
		SetWindowLongPtr(createProcessEdit,GWL_WNDPROC,(WNDPROC)wndpOldProcessEditProc);
		ExitProcess(0);
		break;
	}
	
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


LRESULT CALLBACK dlgProc(HWND hdlg,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		stayOnTop=CreateWindowEx(0,"button","Keep main window on top",WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,8,8,200,23,hdlg,(HMENU)1,0,0);
		closeAfter=CreateWindowEx(0,"button","Close SI when injection is done",WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,8,33,200,23,hdlg,(HMENU)2,0,0);
		saveLog=CreateWindowEx(0,"button","Save the log to the disk",WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,8,58,200,23,hdlg,(HMENU)3,0,0);

		SendMessage(stayOnTop,WM_SETFONT,(WPARAM)font,TRUE);
		SendMessage(closeAfter,WM_SETFONT,(WPARAM)font,TRUE);
		SendMessage(saveLog,WM_SETFONT,(WPARAM)font,TRUE);

		//check if autocheckboxes are checked/unchecked
		if(keepOnTop==TRUE)
			SendMessage(stayOnTop,BM_CLICK,0,0);
		if(saveLogToDisk==TRUE)
			SendMessage(saveLog,BM_CLICK,0,0);
		if(closeWhenDone==TRUE)
			SendMessage(closeAfter,BM_CLICK,0,0);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 1:
			checkStayOnTop(hdlg);
			break;

		case 2:
			checkCloseWhenDone(hdlg);
			break;

		case 3:
			checkSaveLog(hdlg);
			break;
		}
		break;

	case WM_DESTROY:
		DestroyWindow(hdlg);
		break;
	}

	return DefWindowProc(hdlg,uMsg,wParam,lParam);
}

// Custom dllEdit proc to handle WM_KEYUP and thus ENTER/TAB keys
LRESULT CALLBACK dllEditProc(HWND dllEdit,UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
		editsKeydown(wParam);
		break;
	}

	return CallWindowProc(wndpOldDllEditProc,dllEdit,uMsg,wParam,lParam); // For msgs we dont deal with
}


LRESULT CALLBACK processEditProc(HWND createProcessEdit,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
		editsKeydown(wParam);
		break;
	}

	return CallWindowProc(wndpOldProcessEditProc,createProcessEdit,uMsg,wParam,lParam);
}


int WMC(HWND hwnd)
{
	HWND groupBox;
	HWND browseDll;
	HWND groupBox1;
	HWND browseExe;
	HWND groupBox2;
	HWND inject;
	HWND options;

	font=CreateFont(16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Calibri Light");

	groupBox=CreateWindowEx(0,"button","Library",WS_CHILD | WS_VISIBLE | BS_GROUPBOX,3,3,388,50,hwnd,(HMENU)-1,0,0);
	dllEdit=CreateWindowEx(0,"edit","",WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,8,23,340,22,hwnd,(HMENU)-1,0,0);
	browseDll=CreateWindowEx(0,"button","...",WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,355,23,30,23,hwnd,(HMENU)0,0,0);
	groupBox1=CreateWindowEx(0,"button","Process",WS_CHILD | WS_VISIBLE | BS_GROUPBOX,3,55,388,50,hwnd,(HMENU)-1,0,0);
    createProcessEdit=CreateWindowEx(0,"edit","",WS_CHILD | WS_VISIBLE |WS_BORDER | ES_AUTOHSCROLL,8,75,340,22,hwnd,(HMENU)-1,0,0);
	browseExe=CreateWindowEx(0,"button","...",WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,355,75,30,23,hwnd,(HMENU)1,0,0);
	groupBox2=CreateWindowEx(0,"button","Log",WS_CHILD | WS_VISIBLE | BS_GROUPBOX,3,108,388,196,hwnd,(HMENU)-1,0,0);
	logEdit=CreateWindowEx(0,"edit","",WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_READONLY,8,125,381,175,hwnd,(HMENU)-1,0,0);

    inject=CreateWindowEx(0,"button","Inject",WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,8,311,80,23,hwnd,(HMENU)2,0,0);
	options=CreateWindowEx(0,"button","Options",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,304,311,80,23,hwnd,(HMENU)5,0,0);

	SendMessage(groupBox,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(groupBox1,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(dllEdit,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(dllEdit,EM_SETCUEBANNER,(WPARAM)TRUE,(LPARAM)L"Browse for a DLL to inject");
	SendMessage(createProcessEdit,EM_SETCUEBANNER,(WPARAM)TRUE,(LPARAM)L"Browse for an exe' file or enter the name of a running process");
	SendMessage(dllEdit,EM_LIMITTEXT,2048,0);
	SendMessage(createProcessEdit,EM_LIMITTEXT,2048,0);
	SendMessage(createProcessEdit,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(groupBox2,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(logEdit,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(inject,WM_SETFONT,(WPARAM)font,TRUE);
	SendMessage(options,WM_SETFONT,(WPARAM)font,TRUE);

	// Set new procedure for DllEdit to handle ENTER key, cuz WM_KEYUP is not sent to parent window proc, but to the window proc directly (so dllEdit's WndProc handled by Windows)
	// returned value is the original/old window proc (so function pointer) of the control, we'll use it for msgs we dont handle
	wndpOldDllEditProc=(WNDPROC)SetWindowLongPtr(dllEdit,GWL_WNDPROC,(WNDPROC)dllEditProc);
	wndpOldProcessEditProc=(WNDPROC)SetWindowLongPtr(createProcessEdit,GWL_WNDPROC,(WNDPROC)processEditProc);
	SetFocus(dllEdit);

	DragAcceptFiles(dllEdit,TRUE);
	DragAcceptFiles(createProcessEdit,TRUE);
	
	init(hwnd); // initialize SI.ini, paths ...

	return 0;
}


int userMsg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	UINT id=LOWORD(wParam);

	switch(id)
	{
	case 0:		 //browse for .dll
		openFileDll(hwnd);
		break;

	case 1:		 //browse for .exe	
		openFileProcess(hwnd);
		break;

	case 2:
		makeInjectionChoice(hwnd);
		break;

	case 5:
		options();
		break;
	}

	return 0;
}

int openFileDll(HWND hwnd)
{
	OPENFILENAME ofn={0};

	ofn.Flags=OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.hwndOwner=hwnd;
	ofn.lpstrFile=fileNameDll; // See main.h, variable is in the globals
	ofn.lpstrFilter="Dynamic Link Library\0*.dll\0\0";
	ofn.nMaxFile=sizeof(fileNameDll);
	ofn.lStructSize=sizeof(OPENFILENAME);

	if(GetOpenFileName(&ofn)!=0)// !=0 means user selected a file
	{
		SendMessage(dllEdit,WM_SETTEXT,0,(LPARAM)fileNameDll);//Send path to edit
	}

	return 0;
}

int openFileProcess(HWND hwnd)
{
	OPENFILENAME ofn={0};

	ofn.Flags=OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.hwndOwner=hwnd;
	ofn.lpstrFile=fileNameProcess; // See main.h, variable is in the globals
	ofn.lpstrFilter="Executable\0*.exe\0\0";
	ofn.nMaxFile=sizeof(fileNameProcess);
	ofn.lStructSize=sizeof(OPENFILENAME);

	if(GetOpenFileName(&ofn)!=0)	
	{
		SendMessage(createProcessEdit,WM_SETTEXT,0,(LPARAM)fileNameProcess);	//Send path to edit
	}

	return 0;
}


int makeInjectionChoice(HWND hwnd)
{
	HANDLE hThread=NULL;

	GetWindowText(createProcessEdit,fileNameProcess,sizeof(fileNameProcess));
	GetWindowText(dllEdit,fileNameDll,sizeof(fileNameDll));

	// Checks if the edit contains slash '\' to know whether it's a path or a process name
	if(strchr(fileNameProcess,'\\')==NULL)	
		hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)injectByOpening,hwnd,0,0);	
	else
		hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)injectByCreating,hwnd,0,0);	

	saveLastPaths(hwnd);	// Save dll and process paths/name

	CloseHandle(hThread);

	return 0;
}

void editsKeydown(WPARAM wParam)
{
	GetWindowText(createProcessEdit,fileNameProcess,sizeof(fileNameProcess));
	GetWindowText(dllEdit,fileNameDll,sizeof(fileNameDll));

	switch(wParam)
	{
	case VK_RETURN:
		if(strlen(fileNameProcess)!=0 && strlen(fileNameDll)!=0)
			makeInjectionChoice(hwnd);
		break;

	case VK_TAB:
		if(GetFocus()==dllEdit)
			SetFocus(createProcessEdit);
		else
			SetFocus(dllEdit);
		break;
	}

	return;
}


void options()
{
	hdlg=CreateWindowEx(0x0188,"dlg","Storm Inj3ctor Options",WS_VISIBLE | WS_SYSMENU,300,200,300,120,0,0,hInst,0);
	SetWindowPos(hdlg,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);

	return;
}
