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

#include "com.h"

#include <stdio.h>

bool invalidate_fat_cache_method2(char letter)
{
    char path[100] = { 0 };

    sprintf(path, "\\\\.\\%c:", letter);

    printf("\ninvalidating cache ... %s\n", path);


    HANDLE hFile = CreateFileA(path,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_NO_BUFFERING,
                               NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("error!\n");
        return false;
    }

    CloseHandle(hFile);

    FILE* file = fopen(path, "rb+");
    if (NULL != file)
    {
        fwrite("C", 1, 1, file);
        fclose(file);
    }

    puts("\nOK! fat cache invalidated! method2");

    return true;
}

bool invalidate_fat_cache_method1(char letter)
{
    char path[100] = { 0 };

    sprintf(path, "\\\\.\\%c:", letter);

    printf("\ninvalidating cache ... %s\n", path);

    HANDLE hFile = CreateFileA(path,
                               FILE_READ_DATA,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    else
    {
        DWORD le = GetLastError();
        if (le != ERROR_SHARING_VIOLATION && le != ERROR_ACCESS_DENIED)
        {
            fprintf(stderr, "Error %d clearing cache: %s\n", path);
            return false;
        }
    }

    puts("\nOK! fat cache invalidated! method1");

    return true;
}

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_PRIVILEGE_NOT_HELD        ((NTSTATUS)0xC0000061L)

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemMemoryListInformation = 80, // 80, q: SYSTEM_MEMORY_LIST_INFORMATION; s: SYSTEM_MEMORY_LIST_COMMAND (requires SeProfileSingleProcessPrivilege)
} SYSTEM_INFORMATION_CLASS;

// private
typedef enum _SYSTEM_MEMORY_LIST_COMMAND
{
    MemoryCaptureAccessedBits,
    MemoryCaptureAndResetAccessedBits,
    MemoryEmptyWorkingSets,
    MemoryFlushModifiedList,
    MemoryPurgeStandbyList,
    MemoryPurgeLowPriorityStandbyList,
    MemoryCommandMax
} SYSTEM_MEMORY_LIST_COMMAND;

BOOL GetPrivilege(HANDLE TokenHandle, LPCSTR lpName, int flags)
{
    BOOL bResult;
    DWORD dwBufferLength;
    LUID luid;
    TOKEN_PRIVILEGES tpPreviousState;
    TOKEN_PRIVILEGES tpNewState;

    dwBufferLength = 16;
    bResult = LookupPrivilegeValueA(0, lpName, &luid);
    if (bResult)
    {
        tpNewState.PrivilegeCount = 1;
        tpNewState.Privileges[0].Luid = luid;
        tpNewState.Privileges[0].Attributes = 0;
        bResult = AdjustTokenPrivileges(TokenHandle, 0, &tpNewState,
                                        (LPBYTE) & (tpNewState.Privileges[1]) - (LPBYTE)&tpNewState, &tpPreviousState,
                                        &dwBufferLength);
        if (bResult)
        {
            tpPreviousState.PrivilegeCount = 1;
            tpPreviousState.Privileges[0].Luid = luid;
            tpPreviousState.Privileges[0].Attributes = flags != 0 ? 2 : 0;
            bResult = AdjustTokenPrivileges(TokenHandle, 0, &tpPreviousState,
                                            dwBufferLength, 0, 0);
        }
    }
    return bResult;
}

bool invalidate_fat_cache_method3(void)
{
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hToken;

    puts("\nWARNING: this method should run as admin\n");
    if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        fprintf(stderr, "Can't open current process token\n");
        return false;
    }

    if (!GetPrivilege(hToken, "SeProfileSingleProcessPrivilege", 1))
    {
        fprintf(stderr, "Can't get SeProfileSingleProcessPrivilege\n");
        return false;
    }

    CloseHandle(hToken);

    HMODULE ntdll = LoadLibrary(L"ntdll.dll");
    if (!ntdll)
    {
        fprintf(stderr, "Can't load ntdll.dll, wrong Windows version?\n");
        return false;
    }

    typedef DWORD NTSTATUS; // ?
    NTSTATUS(WINAPI * NtSetSystemInformation)(INT, PVOID,
            ULONG) = (NTSTATUS(WINAPI*)(INT, PVOID, ULONG))GetProcAddress(ntdll,
                     "NtSetSystemInformation");
    NTSTATUS(WINAPI * NtQuerySystemInformation)(INT, PVOID, ULONG,
            PULONG) = (NTSTATUS(WINAPI*)(INT, PVOID, ULONG, PULONG))GetProcAddress(ntdll,
                      "NtQuerySystemInformation");
    if (!NtSetSystemInformation || !NtQuerySystemInformation)
    {
        fprintf(stderr, "Can't get function addresses, wrong Windows version?\n");
        return false;
    }

    SYSTEM_MEMORY_LIST_COMMAND command = MemoryPurgeStandbyList;
    NTSTATUS status = NtSetSystemInformation(
                          SystemMemoryListInformation,
                          &command,
                          sizeof(SYSTEM_MEMORY_LIST_COMMAND)
                      );
    if (status == STATUS_PRIVILEGE_NOT_HELD)
    {
        fprintf(stderr, "Insufficient privileges to execute the memory list command");
    }
    else if (!NT_SUCCESS(status))
    {
        fprintf(stderr, "Unable to execute the memory list command %p", status);
    }

    puts("\nOK! fat cache invalidated! method3");

    return NT_SUCCESS(status) ? true : false;
}

void print_disk_info(HANDLE hFile)
{
    static STORAGE_PROPERTY_QUERY spq = { StorageDeviceProperty, PropertyStandardQuery };

    union
    {
        PVOID buf;
        PSTR psz;
        PSTORAGE_DEVICE_DESCRIPTOR psdd;
    } tagg;

    ULONG size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 0x100;

    ULONG dwError;

    do
    {
        dwError = ERROR_NO_SYSTEM_RESOURCES;

        if (tagg.buf = LocalAlloc(0, size))
        {
            ULONG BytesReturned;

            if (DeviceIoControl(hFile,
                                IOCTL_STORAGE_QUERY_PROPERTY,
                                &spq,
                                sizeof(spq),
                                tagg.buf,
                                size, &BytesReturned, 0))
            {
                if (tagg.psdd->Version >= sizeof(STORAGE_DEVICE_DESCRIPTOR))
                {
                    if (tagg.psdd->Size > size)
                    {
                        size = tagg.psdd->Size;
                        dwError = ERROR_MORE_DATA;
                    }
                    else
                    {
                        dwError = ERROR_NO_DATA;
                        if (tagg.psdd->ProductRevisionOffset)
                        {
                            printf("ProductRevisionOffset = %s\n",
                                   tagg.psz + tagg.psdd->ProductRevisionOffset);
                            dwError = NOERROR;
                        }
                        if (tagg.psdd->SerialNumberOffset)
                        {
                            printf("SerialNumber = %s\n", tagg.psz + tagg.psdd->SerialNumberOffset);
                            dwError = NOERROR;
                        }
                        if (tagg.psdd->ProductIdOffset)
                        {
                            printf("ProductIdOffset = %s\n", tagg.psz + tagg.psdd->ProductIdOffset);
                            dwError = NOERROR;
                        }
                        if (tagg.psdd->VendorIdOffset)
                        {
                            printf("VendorIdOffset = %s\n", tagg.psz + tagg.psdd->VendorIdOffset);
                            dwError = NOERROR;
                        }
                    }
                }
                else
                {
                    dwError = ERROR_GEN_FAILURE;
                }
            }
            else
            {
                dwError = GetLastError();
            }

            LocalFree(tagg.buf);
        }
    } while (dwError == ERROR_MORE_DATA);
}

bool update_phys_prop(int number_phys_disk)
{
    bool retf = false;
    DWORD BytesReturned = 0;
    char driv[200] = { 0 };
    HANDLE hdrive;

    puts("\nWARNING: this should run as admin\n");

    sprintf(driv, "\\\\.\\PhysicalDrive%d", number_phys_disk);

    hdrive = CreateFileA(driv,
                         (GENERIC_READ | GENERIC_WRITE),
                         (FILE_SHARE_READ | FILE_SHARE_WRITE),
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_WRITE_THROUGH,
                         NULL);
    if (hdrive == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "getlasterr: %d\n", GetLastError());
        return false;
    }

    printf("open %s\n", driv);
    if (DeviceIoControl(
                (HANDLE)hdrive,
                IOCTL_DISK_UPDATE_PROPERTIES,
                NULL,
                0,
                NULL,
                0,
                &BytesReturned,
                NULL
            ))
    {
        puts("updated!");
        retf = true;
    }

    print_disk_info(hdrive);

    CloseHandle(hdrive);

    return retf;
}

void get_cmdex_shex_curr_dir(char* cmdex, char* shex, char* curr_dir)
{
    if (NULL == curr_dir)
    {
        return;
    }

    GetCurrentDirectoryA(MAX_PATH, curr_dir);
    if (NULL != cmdex)
    {
        sprintf(cmdex, "%s\\%s", curr_dir, FILE_INFO_NAME);
    }
    if (NULL != shex)
    {
        sprintf(shex, "%s\\%s", curr_dir, MALWARE_FILE_NAME);
    }
}


#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

char get_root_letter_from_path(char* file_path)
{
    char buff[4] = { 0 };

    int drv = PathGetDriveNumberA(file_path);
    if (drv == -1)
    {
        return 0;
    }
    PathBuildRootA(buff, drv);

    return buff[0];
}