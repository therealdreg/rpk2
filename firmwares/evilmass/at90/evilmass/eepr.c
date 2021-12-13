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

#include "eepr.h"

volatile EPPR_VARS_t eppr_vars;

void defaults_eepr(void)
{
    memset((void*)&eppr_vars, 0, sizeof(eppr_vars));
    
    save_eepr();
}

void print_eepr(void)
{
    printf_P(PSTR("eppr_vars addr: 0x%08X\r\n"
                  "eppr_vars size: %d\r\n"
                  "eppr_vars.restore_defaults_on_boot = %d\r\n"
                  "eppr_vars.curr_stage = %d\r\n"
                  "eppr_vars.deleted_files = %d\r\n"
                  "eppr_vars.curr_sector_stage1_deleted = %u\r\n"
                  "eppr_vars.curr_sector_stage2_deleted = %u\r\n"), 
                  EEPR_VARS_ADDR, 
                  sizeof(eppr_vars),
                  eppr_vars.restore_defaults_on_boot, 
                  eppr_vars.curr_stage,
                  eppr_vars.deleted_files,
                  eppr_vars.curr_sector_stage1_deleted,
                  eppr_vars.curr_sector_stage2_deleted);
}

void eepr_boot(void)
{
    load_eepr();
    if (eppr_vars.restore_defaults_on_boot)
    {
        defaults_eepr();
    }
}

void save_eepr(void)
{
    if (eppr_vars.curr_stage == STAG_2)
    {
        eppr_vars.curr_stage = STAG_1; // RACE CONDITION, THE USER DISCONNECTED THE DEVICE BETWEEN STAGES....
    }
    eeprom_write_block((void*)&eppr_vars, (void*)EEPR_VARS_ADDR, sizeof(eppr_vars));
}

void load_eepr(void)
{
    eeprom_read_block((void*)&eppr_vars, (void*)EEPR_VARS_ADDR, sizeof(eppr_vars));
    if (eppr_vars.curr_stage == STAG_2)
    {
        eppr_vars.curr_stage = STAG_1; // RACE CONDITION, THE USER DISCONNECTED THE DEVICE BETWEEN STAGES....
    }
}

