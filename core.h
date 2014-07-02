#ifndef core
#define core

// prototypes
int injectByCreating(HWND hwnd);
int injectByOpening(HWND hwnd);
int writeLog(HWND);
char *get_name_from_ppid(int ppid);
int get_file_size(char *);

// variables
char logBuff[7168];

#endif