#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define toogle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))



/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

/* WARNING ! WARNING ! WARNING ! WARNING ! WARNING ! WARNING ! 
 * YOU MUST USE ADAFRUIT RADIO HEAD FORK: 
 * https://github.com/adafruit/RadioHead/archive/master.zip 
 * WARNING ! WARNING ! WARNING ! WARNING ! WARNING ! 
*/

#include <SPI.h>
#include <RH_RF69.h>

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
#define LED_1           8
#define LED_2           9
#define Serial Serial1

RH_RF69 rf69(RFM69_CS, RFM69_INT);

String serial_input = "";       // Hold serial input
uint8_t rec_buf[RH_RF69_MAX_MESSAGE_LEN + 1] = { 0 }; // Received data through RF
uint8_t rec_buf_len = sizeof(rec_buf);
char rfdata[RH_RF69_MAX_MESSAGE_LEN + 1] = { 0 }; // Data to be sent through RF


void switch_usb_to_at90(void)
{
    set_bit(DDRD, DDD7);
    delay(500);
    clear_bit(PORTD, PD7);
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
}

void setup() 
{
  Serial.begin(9600);

 switch_usb_to_at90();
    
 pinMode(A5, OUTPUT);     
 digitalWrite(A5, HIGH);

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
      
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("\r\n\r\nArduino Leonardo RFM69 RX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  


  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

void loop() {

  // Check if there is any character to read
	while (Serial.available()) {
		// Read 1 byte
		char input_char = (char)Serial.read();
		    if (input_char != '\n'  && input_char != '\r') {
			    serial_input += input_char;
		    }
				// If eol clean input and send it through rf
				if (input_char == '\n') {
					// Cast input buffer to char array
					serial_input.toCharArray(rfdata, RH_RF69_MAX_MESSAGE_LEN);
					// Send the data
					rf69.send((uint8_t *)rfdata, strlen(rfdata));
					rf69.waitPacketSent();
					Serial.print("Leo: "); Serial.println(serial_input);
					// Clear input buffer
					serial_input = "";
					memset(rfdata, 0, strlen(rfdata));
				}
	}
	
  // Check if there is anything to receive
  if (rf69.recv(rec_buf, &rec_buf_len)) {
	  if (!rec_buf_len) return;
	  Serial.print("Feather: "); Serial.println((char*)rec_buf);
		memset(rec_buf, 0, rec_buf_len);
  }

	// Delay between loop iterations
	delay(500);
}
