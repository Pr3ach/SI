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

#if !defined(INIT_H)
#define INIT_H

int init(HWND);

char startDir[1024];
char iniPath[1024];
char logPath[1024];
BOOL keepOnTop;
BOOL closeWhenDone;
BOOL saveLogToDisk;

#endif /* !INIT_H */
