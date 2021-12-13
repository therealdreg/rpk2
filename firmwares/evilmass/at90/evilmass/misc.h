/*
 * misc.h
 *
 * Created: 2021-10-10 22:43:20
 *  Author: Dreg
 */ 


#ifndef MISC_H_
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

#define MISC_H_

#include <stdint.h>
#include <stdbool.h>
#include "comsrc.h"

struct CMD_s
{
    char* cmd;
    unsigned char opt_data[70];
};

typedef struct CMD_s CMD_t;

bool recv_from_serial(unsigned char* buffer, uint32_t nbytes, uint32_t max_tries);
bool recv_new_line_from_serial(unsigned char* buffer, uint32_t max_nbytes, uint32_t max_tries);
bool recv_from_serial_until(unsigned char* buffer, uint32_t max_nbytes, unsigned char* pattern, uint32_t pattern_size, uint32_t max_tries);
unsigned int send_cmd(CMD_t* cmd, void* ext);
void flush_serial_input(void);
bool clear_new_line_serial(uint32_t max_tries);


#endif /* MISC_H_ */
