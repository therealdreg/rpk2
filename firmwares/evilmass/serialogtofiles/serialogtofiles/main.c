// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
serialogtofiles - MIT License - Copyright 2020
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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#define COMSRC // ONLY DEFINE THIS ONE TIME IN THE FULL PROJECT, THE OTHERS MUST BE USED ONLY: #include <comsrc.h>
#include <comsrc.h>

#include <windows.h>

int serialogtofiles_f(char* dir_name, FILE* report, FILE* log)
{
    char read_line[500];

    fseek(log, 0L, SEEK_SET);
    uint32_t last_file_id = 0;
    uint32_t last_nr_block = -1;
    FILE* last_file = NULL;
    uint32_t adler32 = 1;
    uint32_t truncate_size = 0;
    static char new_file_name[4096];

    while (!feof(log))
    {
        memset(read_line, 0, sizeof(read_line));
        fgets(read_line, sizeof(read_line) - 1, log);
        if (read_line[0] == '\0')
        {
            break;
        }
        puts(read_line);
        if (memcmp(read_line, "sdatarecv", sizeof("sdatarecv") - 1) == 0)
        {
loopez:
            memset(read_line, 0, sizeof(read_line));
            fgets(read_line, sizeof(read_line) - 1, log);
            if (read_line[0] == '\0')
            {
                break;
            }
            if (read_line[0] == '\r' || read_line[0] == '\n')
            {
                goto loopez;
            }
            printf("found: %s\n", read_line);
            printf("converting to binary....\n");
            char buff[500] = { 0 };
            char* curr_char = strtok(read_line, " ");
            if (NULL == curr_char)
            {
                break;
            }
            int x = 0;
            do
            {
                char* ptr;
                buff[x++] = (char) strtoul(curr_char, &ptr, 16);
            } while ((curr_char = strtok(NULL, " ")) != NULL);
            puts("done!");
            EXFL_BLK_t* blk = (EXFL_BLK_t*) buff;

            printf("\n"
                   "exfstr: %s\n"
                   "fileid: %d\n"
                   "curr_block_nr: %d\n"
                   "last_block: %d\n",
                   blk->exfstr,
                   blk->fileid,
                   blk->curr_block_nr,
                   blk->last_block);

            printf("data:\n");
            for (int i = 0; i < sizeof(blk->data); i++)
            {
                printf("0x%02X ", blk->data[i]);
                if ((i + 1) % 5 == 0)
                {
                    printf("    %c%c%c%c%c",
                           isprint(blk->data[i - 4]) ? blk->data[i - 4] : '.',
                           isprint(blk->data[i - 3]) ? blk->data[i - 3] : '.',
                           isprint(blk->data[i - 2]) ? blk->data[i - 2] : '.',
                           isprint(blk->data[i - 1]) ? blk->data[i - 1] : '.',
                           isprint(blk->data[i]) ? blk->data[i] : '.');
                    printf("\n");
                }
            }
            puts("\n-\n");

            if (blk->fileid != last_file_id)
            {
                if (NULL != last_file)
                {
                    fprintf(report, "adler32 calculated: 0x%08X\r\n", adler32);
                    fprintf(report, "nr last block: %d\r\n\r\n", last_nr_block);
                    printf("closing last file\n");
                    fclose(last_file);
                    if (truncate_size != 0)
                    {
                        static cmds[5000] = { 0 };
                        sprintf(cmds, "FSUTIL file seteof %s %d", new_file_name, truncate_size);
                        printf("truncating file to original size...\n%s\n", cmds);
                        system(cmds);
                    }
                    last_file = NULL;
                }

                last_file_id = blk->fileid;
                last_nr_block = -1;
                adler32 = 1;
                truncate_size = 0;


                memset(new_file_name, 0, sizeof(new_file_name));

                sprintf(new_file_name, "%s%d", dir_name, blk->fileid);
                printf("new file_id detected, creating file...: %s\n", new_file_name);
                last_file = fopen(new_file_name, "wb+");
                if (NULL == last_file)
                {
                    perror(last_file);
                    break;
                }
                fprintf(report, "%s\r\n", new_file_name);
            }
            if (blk->last_block)
            {
                puts("skipping last block\n");
                fprintf(report, "found last block\r\n");
                LAST_BLOCK_DATA_t* lst = (LAST_BLOCK_DATA_t*) blk->data;
                fprintf(report, "last block adler32: 0x%08X\r\n"
                        "last block size of file: 0x%08X\r\n"
                        "last block tag name: %" xstr(SIZE_TAG_NAME) "s\r\n",
                        lst->adler32_full_blocks,
                        lst->size_file,
                        lst->tag_name);
                if (adler32 != lst->adler32_full_blocks)
                {
                    fprintf(report, "incorrect adler32\r\n");
                    fprintf(stderr, "incorrect adler32\r\n");
                }
                else
                {
                    fprintf(report, "correct adler32\r\n");
                    printf( "correct adler32\r\n");
                }

                truncate_size = lst->size_file;
            }
            else
            {
                if (last_nr_block == blk->curr_block_nr)
                {
                    fprintf(stderr, "\n\nWARNING: the same block again! skipping...\n\n");
                }
                else
                {
                    printf("writing block nr: %d\n", blk->curr_block_nr);
                    fwrite(blk->data, sizeof(blk->data), 1, last_file);
                    adler32 = update_adler32(adler32, blk->data, sizeof(blk->data));
                    last_nr_block = blk->curr_block_nr;
                }
            }
        }
    }

    if (NULL != last_file)
    {
        fclose(last_file);
        fprintf(report, "adler32 calculated: 0x%08X\r\n", adler32);
        fprintf(report, "nr last block: %d\r\n\r\n", last_nr_block);
        if (truncate_size != 0)
        {
            static cmds[5000] = { 0 };
            sprintf(cmds, "FSUTIL file seteof %s %d", new_file_name, truncate_size);
            printf("truncating file to original size...\n%s\n", cmds);
            system(cmds);
        }
    }


    return 0;
}

int serialogtofiles(int argc, char* argv[])
{
    int retf = 1;
#define SYNTAX_USE "serialogtofiles.exe file_serial_log report_file dir_output\\\n"

    puts("\n-\n"
         "serialogtofiles (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
         "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
         "    syntax: " SYNTAX_USE " - examples:\n"
         "    serialogtofiles.exe teraterm.log report_file.txt ./files_recv/\n\n"
         "\n-\n");

    for (int i = 1; i < argc; i++)
    {
        for (int j = 1; j < argc; j++)
        {
            if (strcmpi(argv[i], argv[j]) == 0)
            {
                if (j == i)
                {
                    continue;
                }
                fprintf(stderr, "error, dont use the same file names: %s %s\n",
                        argv[i],
                        argv[j]);
                return 1;
            }
        }
    }

    if (4 != argc)
    {
        fprintf(stderr, "\nerror: bad args, use: " SYNTAX_USE "\n");
        return 1;
    }

    printf("creating directory: %s\n", argv[3]);
    if (!CreateDirectoryA(argv[3], NULL) || ERROR_ALREADY_EXISTS == GetLastError())
    {
        fprintf(stderr, "error, I cant create the directory. btw must be no exist\n");
        return 1;
    }

    if (NULL != fopen(argv[2], "rb"))
    {
        fprintf(stderr, "error, report file must be no exist\n");
        return 1;
    }

    FILE* log = fopen(argv[1], "rb");
    if (NULL == log)
    {
        perror(log);
        return 1;
    }
    printf("open: %s\n", argv[1]);
    FILE* report = fopen(argv[2], "wb+");
    if (NULL != report)
    {
        printf("open report file: %s\n", argv[2]);
        retf = serialogtofiles_f(argv[3], report, log);
        fclose(report);
    }


    fclose(log);

    return retf;
}



int main(int argc, char* argv[])
{
    int retf;

    retf = serialogtofiles(argc, argv);

    puts("\nPRESS ENTER TO EXIT\n");
    getchar();

    return retf;
}