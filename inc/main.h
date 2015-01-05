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

#if !defined MAIN_H
#define MAIN_H

#define VERSION "1.7"
#define MAX_RAW_BUF 65536+4096

LRESULT CALLBACK mainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dlgProc(HWND hdlg,UINT uMsg,WPARAM wParam, LPARAM lParam);
int WMC(HWND hwnd);
int userMsg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int openFileDll(HWND hwnd);
int openFileProcess(HWND hwnd);
int makeInjectionChoice(HWND hwnd);
void editsKeydown(WPARAM wParam);
void options(void);
void log(char *fmt, ...);

HINSTANCE hInst;
HWND hwnd;
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
char raw_buf[MAX_RAW_BUF];

#endif /* !MAIN_H */
