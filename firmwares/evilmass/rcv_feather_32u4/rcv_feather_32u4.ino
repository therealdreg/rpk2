/* WARNING ! WARNING ! WARNING ! WARNING ! WARNING ! WARNING ! 
 * YOU MUST USE ADAFRUIT RADIO HEAD FORK: 
 * https://github.com/adafruit/RadioHead/archive/master.zip 
 * WARNING ! WARNING ! WARNING ! WARNING ! WARNING ! 
*/

#include <SPI.h>
#include <RH_RF69.h>

/*
  Arduino UNO -> RFM69HCW adafruit
    VIN -> 5V
    GND -> GND
    G0 -> 2
    SCK -> 13
    MISO -> 12
    MOSI -> 11
    CS -> 10
    RST -> 4

#define RFM69_INT     2 
#define RFM69_CS      10 
#define RFM69_RST     4 
#define LED           13
*/

 #if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

#define RF69_FREQ 434.0

RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup() 
{
  Serial.begin(9600);
  while (!Serial) { delay(1); }   // wait until serial console is open, remove if not tethered to computer

 pinMode(A5, OUTPUT);     
 digitalWrite(A5, HIGH);

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("\r\n\r\nRFM69 RX!");
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
  
  pinMode(LED, OUTPUT);

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}


void loop() {
  loopez:
  if (rf69.available()) {
    // Should be a message for us now   
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      if (len == 3)
      {
        Serial.print("\r\n\r\nPING FROM VICTIM RECVED!\r\n\r\n");
        uint8_t data[] = "ok";
        rf69.send(data, sizeof(data));
        rf69.waitPacketSent();
        Serial.println("Sent a reply");
        goto loopez;
      }
      buf[len] = 0;
      Serial.print("\r\nPacket len: "); Serial.println(len); 
      Serial.print("\r\nPacket nr: "); 
      Serial.print(packetnum++);
      Serial.print("\r\nReceived [");
      Serial.print(len);
      Serial.print(" bytes]: ");
      Serial.print("RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);

      Serial.print("\r\nsdatarecv\r\n");
      for (int a = 0; a < len; a++)
      {
        { 
          char buff[4] = { 0 }; 
          sprintf(buff,"%02X ",buf[a]); 
          Serial.print(buff); 
        }
      }
      Serial.println("\r\nedatarecv\r\n");
      
      // Send a reply!
      uint8_t data[] = "ok";
      rf69.send(data, sizeof(data));
      rf69.waitPacketSent();
      Serial.println("Sent a reply");
  }
 }
}
