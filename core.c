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


int injectByCreating(HWND hwnd) 
{
	int* pLoadLib=NULL;
	void* pAllocatedSpace=NULL;
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	HANDLE snapshot;
	PROCESSENTRY32 ep={0};
	char addr[96]; 
	char fileTitle[1024]={0};
	char info[2048]={0};
	char dllName[1024]={0};
	unsigned int securityLoop=0;
	char tempBuff[2048]={0};

	if(strlen(fileNameDll)==0)
		return 1;

	if(strlen(fileNameProcess)==0)
		return 1;

	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)""); //empty the log window

	pLoadLib=(int*)GetProcAddress(GetModuleHandle("Kernel32.dll"),"LoadLibraryA"); 

	if(pLoadLib==NULL)	//If no address is returned
	{
		sprintf(logBuff,"%s","[!] Unable to find LoadLibraryA address !");
		strcat(logBuff," Error GetProcAddress(),GetModuleHandle(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		if(saveLogToDisk==TRUE)
			writeLog(hwnd);

		return 1;
	}
	
	sprintf(addr,"0x%0x",pLoadLib);		
	sprintf(logBuff,"%s","[+] kernel32!LoadLibraryA found at ");
	strcat(logBuff,addr);
	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	si.cb=sizeof(STARTUPINFO);

	if(!CreateProcess(0,fileNameProcess,0,0,TRUE,CREATE_SUSPENDED,0,0,&si,&pi))
	{	
		strcat(logBuff,"\r\n[!] Unable to create process !");
		strcat(logBuff," Error CreateProcess(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)
		writeLog(hwnd);


		return 1;
	}

		strcat(logBuff,"\r\n[+] Process created");
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		pAllocatedSpace=VirtualAllocEx(pi.hProcess,0,strlen(fileNameDll),MEM_COMMIT,PAGE_EXECUTE_READWRITE); 

		if(pAllocatedSpace==NULL)
		{
		strcat(logBuff,"\r\n[!] Unable to allocate memory in the child process !");
		strcat(logBuff," Error VirtualAllocEx(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)
		writeLog(hwnd);

		return 1;
		}

		strcat(logBuff,"\r\n[+] Memory allocated");
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		//Write the dll path in the process's space allocated previously

		if(!WriteProcessMemory(pi.hProcess,pAllocatedSpace,fileNameDll,strlen(fileNameDll),0))
		{
		strcat(logBuff,"\r\n[!] Unable to write data into the child process !");
		strcat(logBuff," Error WriteProcessMemory(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)
		writeLog(hwnd);
	

		return 1;
		}

		strcat(logBuff,"\r\n[+] Data written into the child process");
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		if(!CreateRemoteThread(pi.hProcess,0,0,(LPTHREAD_START_ROUTINE)pLoadLib,pAllocatedSpace,0,0))
		{
		strcat(logBuff,"\r\n[!] Unable to create a thread into the child process. Injection failed !");
		strcat(logBuff," Error CreateRemoteThread(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)
		writeLog(hwnd);

		return 1;
		}

		ResumeThread(pi.hThread);

		strcat(logBuff,"\r\n[+] Thread created into the child process\r\n[+] Thread resumed\r\n[+] Done, dll successfully injected.");

		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		GetFileTitle(fileNameProcess,fileTitle,sizeof(fileTitle)); //Extract the file name from the full path, except extension so need to strcat it
		strcat(fileTitle,".exe");								  // add extension


					// get info

		snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); //Take a snapshot of all the running process
		ep.dwSize=sizeof(PROCESSENTRY32);
		Process32First(snapshot,&ep);

		while(stricmp(ep.szExeFile,fileTitle)!=0) // stricmp: compare without case sensitivity
		{
			Process32Next(snapshot,&ep);//Browse the processes in the snapshot, and retrieve info 'bout em in the PROCESSENTRY32 struct

			securityLoop++;
			if(securityLoop>=300) // To avoid infinite loop, browse 300 process
				return 1;
		}

		GetFileTitle(fileNameDll,dllName,sizeof(dllName));		

		sprintf(info,"\r\n\r\nProcess info:\r\n-------------\r\nName: %s\r\nPID....: %d\r\nPPID..: %d (%s)\r\n\r\nDLL info:\r\n-----------\r\nName: %s\r\nSize...: %d bytes",ep.szExeFile,ep.th32ProcessID,ep.th32ParentProcessID, get_name_from_ppid(ep.th32ParentProcessID), dllName, get_file_size(fileNameDll));
		strcat(logBuff,info);				

		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		// Frees resources

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(snapshot);

		if(saveLogToDisk==TRUE)
			writeLog(hwnd);		

		if(closeWhenDone==TRUE)
			PostQuitMessage(0);

	return 0;
}


int injectByOpening(HWND hwnd)
{
	HANDLE snap=NULL;
	PROCESSENTRY32 pe32={0};
	int securedLoop=0;
	int* pLoadLib=NULL;
	char dllName[1024]={0};
	char info[2048]={0};
	void* pAlloc=NULL;
	HANDLE hOpenProcess=NULL;
	char addr[96]={0};
	char tempBuff[2048]={0};
	unsigned int i=0;

	if(strlen(fileNameDll)==0)
		return 1;

	if(strlen(fileNameProcess)==0)
		return 1;

	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)""); //empty log window
	
	pLoadLib=(int*)GetProcAddress(GetModuleHandle("kernel32.dll"),"LoadLibraryA");	//get function addr

	if(pLoadLib==NULL)
	{
		sprintf(logBuff,"%s","[!] Unable to find kernel32!LoadLibraryA address !");

			strcat(logBuff," Error GetModuleHandle(), GetProcAddress(), code: ");
			sprintf(tempBuff,"%d",GetLastError());
			strcat(logBuff,tempBuff);
			SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

			if(saveLogToDisk==TRUE)
				writeLog(hwnd);			

		return 1;
	}
	
	sprintf(addr,"0x%0x",pLoadLib);		//Convert int to char to enable to display it
	sprintf(logBuff,"%s","[+] kernel32!LoadLibraryA found at ");
	strcat(logBuff,addr);
	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	pe32.dwSize=sizeof(PROCESSENTRY32);			// Without this, the struct doesn't work


	snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	Process32First(snap,&pe32);		// The struct won't work without this

	do
	{
		Process32Next(snap,&pe32);
		i++;

		if(i>=300)
		{
			strcat(logBuff,"\r\n[!] The process ");
			strcat(logBuff,fileNameProcess);
			strcat(logBuff," can't be found");
			SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)
		writeLog(hwnd);		

			return 1;
		}

	}while(stricmp(pe32.szExeFile,fileNameProcess)!=0);

		strcat(logBuff,"\r\n[+] The process ");
		strcat(logBuff,fileNameProcess);
		strcat(logBuff," was found");
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	hOpenProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pe32.th32ProcessID);

	if(hOpenProcess==NULL)
	{
		strcat(logBuff,"\r\n[!] Unable to open the process !");
		strcat(logBuff," Error OpenProcess(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)		
		writeLog(hwnd);		

		return 1;
	}

	strcat(logBuff,"\r\n[+] Process opened");
	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);


	if(pAlloc==NULL)
	{
		strcat(logBuff,"\r\n[!] Unable to allocate memory in the process !");
		strcat(logBuff," Error VirtualAllocEx(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)		
		writeLog(hwnd);		

		return 1;
	}

	strcat(logBuff,"\r\n[+] Memory allocated");
	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(!WriteProcessMemory(hOpenProcess,pAlloc,fileNameDll,strlen(fileNameDll),0))
	{
		strcat(logBuff,"\r\n[!] Unable to write data into the process !");
		strcat(logBuff," Error WriteProcessMemory(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(saveLogToDisk==TRUE)		
		writeLog(hwnd);		

		return 1;
	}

	strcat(logBuff,"\r\n[+] Data written into the process");
	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	if(!CreateRemoteThread(hOpenProcess,0,0,(LPTHREAD_START_ROUTINE)pLoadLib,pAlloc,0,0))
	{
		strcat(logBuff,"\r\n[!] Unable to create a thread in the process. Injection failed!");
		strcat(logBuff," Error CreateRemoteThread(), code: ");
		sprintf(tempBuff,"%d",GetLastError());
		strcat(logBuff,tempBuff);
		SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

		if(saveLogToDisk==TRUE)	
			writeLog(hwnd);		

		return 1;
	}

	strcat(logBuff,"\r\n[+] Thread created into the process\r\n[+] Done, dll successfully injected.");
	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

			// get info

	GetFileTitle(fileNameDll,dllName,sizeof(dllName));		

	sprintf(info,"\r\n\r\nProcess info:\r\n-------------\r\nName: %s\r\nPID....: %d\r\nPPID..: %d (%s)\r\n\r\nDLL info:\r\n-----------\r\nName: %s\r\nSize...: %d bytes",pe32.szExeFile,pe32.th32ProcessID,pe32.th32ParentProcessID, get_name_from_ppid(pe32.th32ParentProcessID), dllName, get_file_size(fileNameDll));
	strcat(logBuff,info);				

	SendMessage(logEdit,WM_SETTEXT,0,(LPARAM)logBuff);

	CloseHandle(hOpenProcess);
	CloseHandle(snap);

	if(saveLogToDisk==TRUE)		
		writeLog(hwnd);

	if(closeWhenDone==TRUE)
		PostQuitMessage(0);		
	
	return 0;
}


int writeLog(HWND hwnd)
{
		SYSTEMTIME st={0};
		char month[32]={0},day[32]={0},year[32]={0},hour[32]={0},minute[32]={0},second[32]={0};
		HANDLE hFile=NULL;
		DWORD bytesWritten=0;
				
		GetSystemTime(&st);

		sprintf(month,"%d",st.wMonth);
		sprintf(day,"%d",st.wDay);
		sprintf(year,"%d",st.wYear);
		sprintf(hour,"%d",st.wHour);
		sprintf(minute,"%d",st.wMinute);
		sprintf(second,"%d",st.wSecond);

		hFile=CreateFile(logPath,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);

		if(hFile==NULL)
		{
			MessageBox(hwnd,"Error creating/opening the SI.log !","Error - Storm Inj3ctor",MB_ICONERROR);
			CloseHandle(hFile);

			return 1;
		}		

		// If SI.log doesn't exists yet
		strcat(logBuff,"\r\n\r\n; ");
		strcat(logBuff,month);
		strcat(logBuff,"/");
		strcat(logBuff,day);
		strcat(logBuff,"/");
		strcat(logBuff,year);
		strcat(logBuff," ");
		strcat(logBuff,hour);
		strcat(logBuff,":");
		strcat(logBuff,minute);
		strcat(logBuff," ");
		strcat(logBuff,second);
		strcat(logBuff,"s");
		strcat(logBuff,"\r\n\r\n\r\n\r\n");

		SetFilePointer(hFile,0,0,FILE_END); // This way we always append data

		WriteFile(hFile,&logBuff,strlen(logBuff),&bytesWritten,0);

		if(bytesWritten!=strlen(logBuff))
		{
			MessageBox(hwnd,"Error writing to the log file!","Error - Storm Inj3ctor",MB_ICONERROR);
			CloseHandle(hFile);

			return 1;
		}

		CloseHandle(hFile);

		return 0;
}


char *get_name_from_ppid(int ppid)
{
	PROCESSENTRY32 pe={0};
	HANDLE snap=NULL;
	unsigned int i=0;

	pe.dwSize=sizeof(PROCESSENTRY32);

	snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

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

	}while(i<600); // browse 600 process

	CloseHandle(snap);

	return (char*)"not found";
}

int get_file_size(char *filename)
{
	HANDLE file=NULL;
	LARGE_INTEGER file_size={0};

	file=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

	if(file==(HANDLE)ERROR_FILE_NOT_FOUND)
	{
		CloseHandle(file);
		return -1;
	}

	GetFileSizeEx(file,&file_size);

	CloseHandle(file);

	return file_size.LowPart;
}
