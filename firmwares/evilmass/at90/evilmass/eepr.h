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


#ifndef EEPR_H_
#define EEPR_H_

#include <avr/eeprom.h>
#include <stdbool.h>
#include "Config/AppConfig.h"
#include <LUFA/Drivers/Peripheral/Serial.h>

#define STAG_1        0
#define STAG_2        1
#define STAG_EXFILL   2
#define STAG_ONLYMASS 3

typedef struct 
{
    uint8_t restore_defaults_on_boot;
    uint8_t curr_stage;
    uint8_t deleted_files;
    unsigned int curr_sector_stage1_deleted;
    unsigned int curr_sector_stage2_deleted;
} EPPR_VARS_t;

#define EEPR_VARS_ADDR 0
#define EEPR_VARS_SIZE sizeof(eppr_vars)
#define EPPR_VARS_END_ADDR (EEPR_VARS_ADDR + EEPR_VARS_SIZE + 1)

extern volatile EPPR_VARS_t eppr_vars;

void eepr_boot(void); // must be called with ATOMIC_BLOCK(ATOMIC_RESTORESTATE )
void save_eepr(void); // must be called with ATOMIC_BLOCK(ATOMIC_RESTORESTATE )
void load_eepr(void);
void defaults_eepr(void); // must be called with ATOMIC_BLOCK(ATOMIC_RESTORESTATE )
void print_eepr(void);

#endif /* EEPR_H_ */