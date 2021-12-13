#include "at90lib.h"

#include <string.h>


int main(int argc, char** argv)
{
    uint8_t current_sector[512] = { 0 };
    uint8_t zero_sector[512] = { 0 };
    char ret;

    ignore_fuses();
    ioinit();
    init_uart();
    on_sd();

    uart_putsnl("\r\n\r\nhellow dreg");


    /*
    uart_putsnl("hellow dreg, press any text + enter to continue \n");
    uint8_t buff[10 + 1] = { 0 };
    uart_readbuff_until_newline(buff, 10, true);
    uart_puts("you pressed: ");
    uart_putsnl(buff);
    */

    SPI_init();

    ret = SD_init();
    if (ret)
    {
        uart_puts("SD err: ");
        uart_printhexchar(ret);
        return -1;
    }

    uart_puts("SD init sucesss!\r\n");

    uart_puts("number of sectors in sd: 0x");
    uint32_t total_sectors = 0;
    if (sd_get_number_of_sectors(&total_sectors))
    {
        uart_printhexint32(total_sectors);
        uart_puts("(");
        uart_printundecint32(total_sectors);
        uart_puts(")\r\n");
        if (total_sectors == 3921920)
        {
            uart_putsnl("2GB SD detected!\r\n");
        }
    }
    else
    {
        uart_putsnl("\r\nERROR sd_get_number_of_sectors\r\n");
    }

    uart_puts("writing data to sector 1 (based on current data stored)\r\n");
    memset(current_sector, 0, 512);
    SD_read_sector(1, current_sector);
    memset(current_sector, current_sector[0] + 1, 512);
    SD_write_sector(1, current_sector);

    uart_puts("READING 5 MULTIPLE BLOCKS IN SD from SECTOR 1\r\n");
    static uint8_t five[512 * 5];

    memset(five, 0, 512 * 5);

    sd_raw_initmultiread(1);
    for (int i = 0; i < 5; i++)
    {
        read_multi_buff(five + (512 * i));
    }
    for (int i = 0; i < 5; i++)
    {

        uart_puts("\r\ncontent of sector: ");
        uart_printdecint32(i + 1);
        uart_puts("\r\n");
        uart_printhexdump(five + (512 * i), 512);

    }
    sd_raw_stopmultiread();

    uart_putsnl("\r\n----");

    uart_puts("\r\nREADING BLOCK BY BLOCK IN SD\r\n");
    for (int i = 0; i < 1000; i++)
    {
        memset(current_sector, 0, 512);
        SD_read_sector(i, current_sector);
        if (i != 0 && i != 1 && memcmp(current_sector, zero_sector, 512) == 0)
        {
            /*
            uart_puts("\r\nall zeros sector: ");
            uart_printdecint32(i);
            uart_puts("\r\n");
            */
        }
        else
        {
            uart_puts("\r\ncontent of sector: ");
            uart_printdecint32(i);
            uart_puts("\r\n");
            uart_printhexdump(current_sector, 512);

            // _delay_ms(5000);
        }
    }




    return 0;
}
