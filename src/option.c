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
#include <string.h>
#include "../inc/main.h"
#include "../inc/core.h"
#include "../inc/option.h"
#include "../inc/init.h"

void checkStayOnTop(HWND hdlg)
{
  HANDLE hFile = NULL;

  if(SendMessage(stayOnTop, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    hFile = CreateFile(iniPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile)
    {
      WritePrivateProfileString("Options", "alwaysTop", "1", iniPath); /* Current dir -> ".\\" */
      keepOnTop=TRUE;
      CloseHandle(hFile);
    }

    else
      MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
  }

  /* BST_UNCHECKED */
  else
  {
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    hFile = CreateFile(iniPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile)
    {
      WritePrivateProfileString("Options", "alwaysTop", "0", iniPath);
      keepOnTop = FALSE;
      CloseHandle(hFile);
    }

    else
      MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
  }

  return;
}

void checkCloseWhenDone(HWND hdlg)
{
  HANDLE hFile = NULL;

  if(SendMessage(closeAfter, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {

    hFile = CreateFile(iniPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile)
    {
      WritePrivateProfileString("Options", "closeAfter", "1", iniPath);
      closeWhenDone = TRUE;
      CloseHandle(hFile);
    }

    else
      MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
  }

  /* BST_UNCHECKED */
  else
  {

    hFile = CreateFile(iniPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile)
    {
      WritePrivateProfileString("Options", "closeAfter", "0", iniPath);
      closeWhenDone = FALSE;
      CloseHandle(hFile);
    }

    else
      MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
  }

  return;
}

void checkSaveLog(HWND hdlg)
{
  HANDLE hFile = NULL;

  if(SendMessage(saveLog, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {

    hFile = CreateFile(iniPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile)
    {
      WritePrivateProfileString("Options", "saveLog", "1", iniPath);
      saveLogToDisk = TRUE;
      CloseHandle(hFile);
    }

    else
      MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
  }

  /* BST_UNCHECKED */
  else
  {
    hFile = CreateFile(iniPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile)
    {
      WritePrivateProfileString("Options", "saveLog", "0", iniPath);
      saveLogToDisk = FALSE;
      CloseHandle(hFile);
    }

    else
      MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
  }

  return;
}

void saveLastPaths(HWND hwnd)
{
  HANDLE hFile = NULL;

  hFile = CreateFile(iniPath, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

  if(!hFile)
  {
    MessageBox(hwnd, "Error configuring SI.ini. CreateFile() failed.", "Error - Storm Inj3ctor", MB_ICONERROR);
    CloseHandle(hFile);

    return;
  }

  WritePrivateProfileString("Options", "lastDllPath", fileNameDll, iniPath);
  WritePrivateProfileString("Options", "lastProcessPath", fileNameProcess, iniPath);

  CloseHandle(hFile);

  return;
}
