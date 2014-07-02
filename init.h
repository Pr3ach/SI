#ifndef initialize
#define initialize

int init(HWND);

char startDir[1024];
char iniPath[1024]; // iniPath = startDir + "\\SI.ini"
char logPath[1024]; // logPath = startDir + "\\SI.log"
BOOL keepOnTop;
BOOL closeWhenDone;
BOOL saveLogToDisk;

#endif
