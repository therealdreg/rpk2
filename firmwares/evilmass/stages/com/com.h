// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
MIT License - Copyright 2020
-
David Reguera Garcia aka Dreg - dreg@fr33project.org
-
http://www.rootkit.es/
http://github.com/David-Reguera-Garcia-Dreg/
http://www.fr33project.org/
-
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

WARNING! this is a POC, the code is CRAP
*/

#ifndef COM_H_
#define COM_H_

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#define MALWARE_FILE_NAME "d.exe"
#define FILE_INFO_NAME "r"
#define STAGE1_MUTEX_NAME L"drgmlwsf"
#define STAGE2_MUTEX_NAME L"roaptsdrgmlw"

bool invalidate_fat_cache_method2(char letter);
bool invalidate_fat_cache_method1(char letter);
bool invalidate_fat_cache_method3(void);
void print_disk_info(HANDLE hFile);
bool update_phys_prop(int number_phys_disk);
void get_cmdex_shex_curr_dir(char* cmdex, char* shex, char* curr_dir);
char get_root_letter_from_path(char* file_path);

#endif /* COM_H_ */
