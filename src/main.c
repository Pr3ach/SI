/*
 * This file is part of SI
 * Copyright (C) 2015, Preacher
 * All rights reserved.
 *
 * SI is a free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SI.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _WIN32_WINNT 0x0500

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include "../inc/main.h"
#include "../inc/core.h"
#include "../inc/option.h"
#include "../inc/init.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevHinstance, LPSTR cmdLine, int show)
{
  MSG uMsg;
  WNDCLASS wc = {0};
  int argc = 0;

  wc.hInstance = hInstance;
  wc.lpszClassName = "class";
  wc.lpfnWndProc = mainProc;
  wc.hbrBackground = CreateSolidBrush(RGB(250, 250, 250));
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));

  if(!RegisterClass(&wc))
    return 1;

  hInst = hInstance;

  hwnd = CreateWindowEx(0, "class", raw("Storm Inj3ctor %s - By Preacher", VERSION), WS_SYSMENU | WS_MINIMIZEBOX, 300, 300, 400, 365, 0, 0, hInstance, 0);

  if(!hwnd)
    exit(-1);

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  while(GetMessage(&uMsg, 0, 0, 0))
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

      /* Message sent by user action, wParam and lParam contain info about the message */
    case WM_COMMAND:
      userMsg(hwnd, uMsg, wParam, lParam);
      break;

      /* Sets background and text color */
    case WM_CTLCOLORSTATIC:
      hdc = (HDC)wParam;
      if((HWND)lParam == logEdit)
      {
        back = CreateSolidBrush(RGB(45, 45, 45));
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkColor(hdc, RGB(45, 45, 45));

        /* When handling this msg (WM_CTLCOLORSTATIC), we gotta return a bkg brush, cast INT_PTR = int for x86/x64 */
        return (INT_PTR)back;
        ReleaseDC(hwnd,hdc);
      }
      break;

    case WM_DESTROY:
      /* restore original WndProc of dllEdit */
      SetWindowLongPtr(dllEdit, GWL_WNDPROC, (WNDPROC)wndpOldDllEditProc);
      SetWindowLongPtr(createProcessEdit, GWL_WNDPROC, (WNDPROC)wndpOldProcessEditProc);
      ExitProcess(0);
      break;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK dlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_CREATE:
      stayOnTop = CreateWindowEx(0, "button", "Keep main window on top", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 8, 200, 23, hdlg, (HMENU)1, 0, 0);
      closeAfter = CreateWindowEx(0, "button","Close SI when injection is done", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 33, 200, 23, hdlg, (HMENU)2, 0, 0);
      saveLog = CreateWindowEx(0, "button","Save the log to the disk", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 58, 200, 23, hdlg, (HMENU)3, 0, 0);

      SendMessage(stayOnTop, WM_SETFONT, (WPARAM)font, TRUE);
      SendMessage(closeAfter, WM_SETFONT, (WPARAM)font, TRUE);
      SendMessage(saveLog, WM_SETFONT, (WPARAM)font, TRUE);

      //check if autocheckboxes are checked/unchecked
      if(keepOnTop)
        SendMessage(stayOnTop, BM_CLICK, 0, 0);
      if(saveLogToDisk)
        SendMessage(saveLog, BM_CLICK, 0, 0);
      if(closeWhenDone)
        SendMessage(closeAfter, BM_CLICK, 0, 0);
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

  return DefWindowProc(hdlg, uMsg, wParam, lParam);
}

/* custom dllEdit proc to handle WM_KEYUP and thus ENTER/TAB keys */
LRESULT CALLBACK dllEditProc(HWND dllEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_KEYDOWN:
      editsKeydown(wParam);
      break;
  }

  return CallWindowProc(wndpOldDllEditProc, dllEdit, uMsg, wParam, lParam);
}

LRESULT CALLBACK processEditProc(HWND createProcessEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_KEYDOWN:
      editsKeydown(wParam);
      break;
  }

  return CallWindowProc(wndpOldProcessEditProc, createProcessEdit, uMsg, wParam, lParam);
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

  font = CreateFont(16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Calibri Light");

  groupBox = CreateWindowEx(0, "button", "Library", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 3, 3, 388, 50, hwnd, (HMENU)-1, 0, 0);
  dllEdit = CreateWindowEx(0, "edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 8, 23, 340, 22, hwnd, (HMENU)-1, 0, 0);
  browseDll = CreateWindowEx(0, "button", "...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 355, 23, 30, 23, hwnd, (HMENU)0, 0, 0);
  groupBox1 = CreateWindowEx(0, "button", "Process", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 3, 55, 388, 50, hwnd, (HMENU)-1, 0, 0);
  createProcessEdit = CreateWindowEx(0, "edit", "", WS_CHILD | WS_VISIBLE |WS_BORDER | ES_AUTOHSCROLL, 8, 75, 340, 22, hwnd, (HMENU)-1, 0, 0);
  browseExe = CreateWindowEx(0, "button", "...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 355, 75, 30, 23, hwnd, (HMENU)1, 0, 0);
  groupBox2 = CreateWindowEx(0, "button", "Log", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 3, 108, 388, 196, hwnd, (HMENU)-1, 0, 0);
  logEdit = CreateWindowEx(0, "edit", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_READONLY, 8, 125, 381, 175, hwnd, (HMENU)-1, 0, 0);

  inject = CreateWindowEx(0, "button", "Inject", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 8, 311, 80, 23, hwnd, (HMENU)2, 0, 0);
  options = CreateWindowEx(0, "button", "Options", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 304, 311, 80, 23, hwnd, (HMENU)5, 0, 0);

  SendMessage(groupBox, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(groupBox1, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(dllEdit, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(dllEdit, 0x1501, (WPARAM)TRUE, (LPARAM)L"Browse for a DLL to inject");
  SendMessage(createProcessEdit, 0x1501, (WPARAM)TRUE, (LPARAM)L"Browse for an exe' file or enter the name of a running process");
  SendMessage(dllEdit, EM_LIMITTEXT, 2048,0);
  SendMessage(createProcessEdit, EM_LIMITTEXT, 2048, 0);
  SendMessage(createProcessEdit, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(groupBox2, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(logEdit, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(inject, WM_SETFONT, (WPARAM)font, TRUE);
  SendMessage(options, WM_SETFONT, (WPARAM)font, TRUE);

  /*
   * Set new procedure for DllEdit to handle ENTER key, cuz WM_KEYUP is not sent to parent window proc,
   * but to the window proc directly (so dllEdit's WndProc handled by Windows)
   *
   * returned value is the original/old window proc (so function pointer) of the control, we'll use it for msgs we dont handle
   */
  wndpOldDllEditProc = (WNDPROC)SetWindowLongPtr(dllEdit, GWL_WNDPROC, (WNDPROC)dllEditProc);
  wndpOldProcessEditProc = (WNDPROC)SetWindowLongPtr(createProcessEdit, GWL_WNDPROC, (WNDPROC)processEditProc);
  SetFocus(dllEdit);

  DragAcceptFiles(dllEdit, TRUE);
  DragAcceptFiles(createProcessEdit, TRUE);

  init(hwnd);

  return 0;
}

int userMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(LOWORD(wParam))
  {
    /* browse for .dll */
    case 0:
      openFileDll(hwnd);
      break;

      /* browse for .exe */
    case 1:
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
  OPENFILENAME ofn = {0};

  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = fileNameDll;
  ofn.lpstrFilter = "Dynamic Link Library\0*.dll\0\0";
  ofn.nMaxFile = sizeof(fileNameDll);
  ofn.lStructSize = sizeof(OPENFILENAME);

  if(GetOpenFileName(&ofn))
    SendMessage(dllEdit,WM_SETTEXT,0,(LPARAM)fileNameDll); /* Send path to edit */

  return 0;
}

int openFileProcess(HWND hwnd)
{
  OPENFILENAME ofn = {0};

  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = fileNameProcess;
  ofn.lpstrFilter = "Executable\0*.exe\0\0";
  ofn.nMaxFile = sizeof(fileNameProcess);
  ofn.lStructSize = sizeof(OPENFILENAME);

  if(GetOpenFileName(&ofn))
    SendMessage(createProcessEdit,WM_SETTEXT,0,(LPARAM)fileNameProcess);

  return 0;
}

int makeInjectionChoice(HWND hwnd)
{
  HANDLE hThread = NULL;

  GetWindowText(createProcessEdit, fileNameProcess, sizeof(fileNameProcess));
  GetWindowText(dllEdit, fileNameDll, sizeof(fileNameDll));

  /* Checks if the edit contains slash '\' to know whether it's a path or a process name */
  if(!strchr(fileNameProcess,'\\'))
    hThread=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)injectByOpening, hwnd, 0, 0);
  else
    hThread=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)injectByCreating, hwnd, 0, 0);

  /* Save dll and process paths/name */
  saveLastPaths(hwnd);

  CloseHandle(hThread);

  return 0;
}

void editsKeydown(WPARAM wParam)
{
  GetWindowText(createProcessEdit, fileNameProcess, sizeof(fileNameProcess));
  GetWindowText(dllEdit, fileNameDll, sizeof(fileNameDll));

  switch(wParam)
  {
    case VK_RETURN:
      if(strlen(fileNameProcess) && strlen(fileNameDll) && *fileNameDll && fileNameProcess)
        makeInjectionChoice(hwnd);
      break;

    case VK_TAB:
      if(GetFocus() == dllEdit)
        SetFocus(createProcessEdit);
      else
        SetFocus(dllEdit);
      break;
  }

  return;
}

void options(void)
{
  hdlg = CreateWindowEx(0x0188, "dlg", "Storm Inj3ctor Options", WS_VISIBLE | WS_SYSMENU, 300, 200, 300, 120, 0, 0, hInst, 0);
  SetWindowPos(hdlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void log(char *fmt, ...)
{
  char buf[2048];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(buf, 2047, fmt, ap);
  va_end(ap);

  sprintf(logBuff, "%s%s", logBuff, buf);
  SendMessage(logEdit, WM_SETTEXT, 0, (LPARAM)logBuff);
}

void raw(char *fmt, ...)
{
  char tmp_buf[MAX_RAW_BUF];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(tmp_buf, MAX_RAW_BUF - 1, fmt, ap);
  va_end(ap);

  memset(raw_buf, 0, MAX_RAW_BUF);

  return strncpy(raw_buf, tmp_buf, MAX_RAW_BUF - 1);
}
