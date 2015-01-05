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

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include <string.h>
#include "../inc/main.h"
#include "../inc/core.h"
#include "../inc/option.h"
#include "../inc/init.h"

int injectByCreating(HWND hwnd)
{
  int* pLoadLib = NULL;
  void* pAllocatedSpace = NULL;
  PROCESS_INFORMATION pi = {0};
  STARTUPINFO si = {0};
  HANDLE snapshot;
  PROCESSENTRY32 ep = {0};
  char fileTitle[1024] = {0};
  char dllName[1024] = {0};
  unsigned int securityLoop = 0;

  if(!strlen(fileNameDll) || !strlen(fileNameProcess))
    return 1;

  /* empty the log window */
  SendMessage(logEdit, WM_SETTEXT, 0, (LPARAM)"");

  pLoadLib = (int*)GetProcAddress(GetModuleHandle("Kernel32.dll"), "LoadLibraryA");

  if(!pLoadLib)
  {
    log("[!] Unable to find LoadLibraryA address ! Error GetProcAddress, GetModuleHandle: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("[+] kernel32!LoadLibraryA found at address 0x%x", pLoadLib);

  si.cb = sizeof(STARTUPINFO);

  if(!CreateProcess(0, fileNameProcess, 0, 0, TRUE, CREATE_SUSPENDED, 0, 0, &si, &pi))
  {
    log("\r\n[!] Unable to create process ! Error CreateProcess: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Process created");

  pAllocatedSpace = VirtualAllocEx(pi.hProcess, 0, strlen(fileNameDll), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

  if(!pAllocatedSpace)
  {
    log("\r\n[!] Unable to allocate memory in the child process ! Error VirtualAllocEx: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Memory allocated");

  if(!WriteProcessMemory(pi.hProcess, pAllocatedSpace, fileNameDll, strlen(fileNameDll), 0))
  {
    log("\r\n[!] Unable to write data into the child process ! Error WriteProcessMemory: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Data written into the child process");

  if(!CreateRemoteThread(pi.hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pLoadLib, pAllocatedSpace, 0, 0))
  {
    log("\r\n[!] Unable to create a thread into the child process. Injection failed ! Error CreateRemoteThread: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  ResumeThread(pi.hThread);

  log("\r\n[+] Thread created into the child process\r\n[+] Thread resumed\r\n[+] Done, dll successfully injected.");

  GetFileTitle(fileNameProcess, fileTitle, sizeof(fileTitle));
  strcat(fileTitle,".exe");

  /* gather info */
  snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  ep.dwSize = sizeof(PROCESSENTRY32);
  Process32First(snapshot, &ep);

  while(stricmp(ep.szExeFile, fileTitle) != 0)
  {
    Process32Next(snapshot, &ep);
    securityLoop++;
    if(securityLoop>=300)
      return 1;
  }

  GetFileTitle(fileNameDll,dllName,sizeof(dllName));

  log("\r\n\r\nProcess info:\r\n-------------\r\nName: %s\r\nPID....: %d\r\nPPID..: %d (%s)\r\n\r\nDLL info:\r\n-----------\r\nName: %s\r\nSize...: %d bytes\r\n\r\n",ep.szExeFile, ep.th32ProcessID, ep.th32ParentProcessID, get_name_from_ppid(ep.th32ParentProcessID), dllName, get_file_size(fileNameDll));

  /* free resources */
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(snapshot);

  if(saveLogToDisk)
    writeLog(hwnd);

  if(closeWhenDone)
    PostQuitMessage(0);

  return 0;
}

int injectByOpening(HWND hwnd)
{
  HANDLE snap = NULL;
  PROCESSENTRY32 pe32 = {0};
  int securedLoop = 0;
  int* pLoadLib = NULL;
  char dllName[1024] = {0};
  void* pAlloc = NULL;
  HANDLE hOpenProcess = NULL;
  unsigned int i = 0;

  if(!strlen(fileNameDll) || !strlen(fileNameProcess))
    return 1;

  /* empty log window */
  SendMessage(logEdit, WM_SETTEXT, 0, (LPARAM)"");

  pLoadLib = (int*)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

  if(!pLoadLib)
  {
    log("[!] Unable to find kernel32!LoadLibraryA address ! Error GetProcAddress, GetModuleHandle: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("[+] kernel32!LoadLibraryA found at 0x%x", pLoadLib);

  pe32.dwSize=sizeof(PROCESSENTRY32);

  snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  /* The struct won't work without this */
  Process32First(snap, &pe32);

  do
  {
    Process32Next(snap, &pe32);
    i++;

    if(i >= 300)
    {
      log("\r\n[!] The process %s can't be found", fileNameProcess);

      if(saveLogToDisk)
        writeLog(hwnd);

      return 1;
    }

  }while(stricmp(pe32.szExeFile, fileNameProcess) != 0);

  log("\r\n[+] Process found ");

  hOpenProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);

  if(!hOpenProcess)
  {
    log("\r\n[!] Unable to open the process ! Error OpenProcess: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Process opened");

  if(!pAlloc)
  {
    log("\r\n[!] Unable to allocate memory in the process ! Error VirtualAllocEx: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Memory allocated");

  if(!WriteProcessMemory(hOpenProcess, pAlloc, fileNameDll, strlen(fileNameDll), 0))
  {
    log("\r\n[!] Unable to write data into the process ! Error WriteProcessMemory: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Data written into the process");

  if(!CreateRemoteThread(hOpenProcess, 0, 0, (LPTHREAD_START_ROUTINE)pLoadLib, pAlloc, 0, 0))
  {
    log("\r\n[!] Unable to create a thread in the process. Injection failed ! Error CreateRemoteThread: 0x%x (%d)", GetLastError(), GetLastError());

    if(saveLogToDisk)
      writeLog(hwnd);

    return 1;
  }

  log("\r\n[+] Thread created into the process\r\n[+] Done, dll successfully injected.");

  /* gather info */
  GetFileTitle(fileNameDll,dllName,sizeof(dllName));

  log("\r\n\r\nProcess info:\r\n-------------\r\nName: %s\r\nPID....: %d\r\nPPID..: %d (%s)\r\n\r\nDLL info:\r\n-----------\r\nName: %s\r\nSize...: %d bytes\r\n\r\n",pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID, get_name_from_ppid(pe32.th32ParentProcessID), dllName, get_file_size(fileNameDll));

  CloseHandle(hOpenProcess);
  CloseHandle(snap);

  if(saveLogToDisk)
    writeLog(hwnd);

  if(closeWhenDone)
    PostQuitMessage(0);

  return 0;
}

int writeLog(HWND hwnd)
{
  SYSTEMTIME st = {0};
  HANDLE hFile = NULL;
  DWORD bytesWritten = 0;

  GetSystemTime(&st);

  hFile = CreateFile(logPath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

  if(!hFile)
  {
    MessageBox(hwnd, "Error creating/opening SI.log !", "Error - Storm Inj3ctor", MB_ICONERROR);
    return 1;
  }


  raw("%s\r\n\r\n; %d/%d/%d %d:%d:%d\r\n\r\n\r\n\r\n", logBuff, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
  SetFilePointer(hFile, 0, 0, FILE_END);
  WriteFile(hFile, raw_buf, strlen(raw_buf), &bytesWritten, 0);

  if(bytesWritten != strlen(raw_buf))
  {
    MessageBox(hwnd, "Error writing to the log file!", "Error - Storm Inj3ctor", MB_ICONERROR);
    CloseHandle(hFile);

    return 1;
  }

  CloseHandle(hFile);

  return 0;
}

char *get_name_from_ppid(int ppid)
{
  PROCESSENTRY32 pe = {0};
  HANDLE snap = NULL;
  unsigned int i = 0;

  pe.dwSize = sizeof(PROCESSENTRY32);

  snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

  if(!Process32First(snap,&pe))
    return (char*)"error";

  do
  {
    if(pe.th32ProcessID==ppid)
    {
      CloseHandle(snap);
      return (char*)pe.szExeFile;
    }

    Process32Next(snap,&pe);

    i++;
  }while(i<600);

  CloseHandle(snap);

  return (char*)"not found";
}

int get_file_size(char *filename)
{
  FILE * fd = NULL;

  sz = 0;
  fd = fopen(filename, "r");

  if(!fd)
  {
    return -1;
  }

  fseek(fd, 0, SEEK_END);
  sz = ftell(fd);
  fclose(fd);

  return sz;
}
