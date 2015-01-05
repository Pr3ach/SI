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

int init(HWND hwnd)
{
  HANDLE hFile = NULL;
  char iniLastDllPath[1024] = {0};
  char iniLastProcessPath[1024] = {0};
  WNDCLASS dlgc = {0};
  keepOnTop = FALSE;
  closeWhenDone = FALSE;
  saveLogToDisk = FALSE;

  /* register dlg class */
  dlgc.hInstance = hInst;
  dlgc.lpfnWndProc = dlgProc;
  dlgc.lpszClassName = (LPCSTR)"dlg";
  dlgc.hbrBackground = CreateSolidBrush(RGB(250, 250, 250));

  if(!RegisterClass(&dlgc))
    exit(-1);

  /* Save original directory path (cuz it changes after GetOpenFileName) */
  GetCurrentDirectory(sizeof(startDir), startDir);

  memcpy(iniPath, startDir, strlen(startDir));
  strcat(iniPath, "\\SI.ini");

  memcpy(logPath, startDir, strlen(startDir));
  strcat(logPath, "\\SI.log");

  /* .ini check */
  hFile = CreateFile(iniPath, GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

  if((long)hFile == ERROR_FILE_NOT_FOUND)
    return 0;

  if(GetPrivateProfileInt("Options", "alwaysTop", 0, iniPath) == 1)
  {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    keepOnTop = TRUE;
  }

  if(GetPrivateProfileInt("Options", "closeAfter", 0, iniPath) == 1)
    closeWhenDone = TRUE;

  if(GetPrivateProfileInt("Options", "saveLog", 0, iniPath) == 1)
    saveLogToDisk = TRUE;

  GetPrivateProfileString("Options", "lastDllPath", "", iniLastDllPath, sizeof(iniLastDllPath), iniPath);

  if(strlen(iniLastDllPath) && *iniLastDllPath)
    SendMessage(dllEdit, WM_SETTEXT, 0, (LPARAM)iniLastDllPath);

  GetPrivateProfileString("Options", "lastProcessPath", "", iniLastProcessPath, sizeof(iniLastProcessPath), iniPath);

  if(strlen(iniLastProcessPath) && *iniLastProcessPath)
    SendMessage(createProcessEdit, WM_SETTEXT, 0, (LPARAM)iniLastProcessPath);

  CloseHandle(hFile);

  return 0;
}
