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

#if !defined(CORE_H)
#define CORE_H

#define MAX_LOG_BUF 65536

int injectByCreating(HWND hwnd);
int injectByOpening(HWND hwnd);
int writeLog(HWND);
char *get_name_from_ppid(int ppid);
int get_file_size(char *);

char logBuff[MAX_LOG_BUF];
int sz;

#endif /* !CORE_H */
