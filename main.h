#ifndef main
#define main

// Prototypes
LRESULT CALLBACK mainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dlgProc(HWND hdlg,UINT uMsg,WPARAM wParam, LPARAM lParam);
int WMC(HWND hwnd);
int userMsg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int openFileDll(HWND hwnd);
int openFileProcess(HWND hwnd);
int makeInjectionChoice(HWND hwnd);
void editsKeydown(WPARAM wParam);
void options();

// Globals
HINSTANCE hInst;
HWND hwnd; // Main Window
HWND dllEdit;
HWND createProcessEdit;
HFONT font;
HWND logEdit;
HWND stayOnTop;
HWND closeAfter;
HWND saveLog;
char fileNameProcess[2048];
char fileNameDll[2048];
WNDPROC wndpOldDllEditProc;
WNDPROC wndpOldProcessEditProc;
HWND hdlg;

#endif

