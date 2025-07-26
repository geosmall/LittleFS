/*
  Sustained write speed test

  This program repeated writes 512K of random data to a flash memory chip,
  while measuring the write speed.  A blank chip will write faster than
  one filled with old data, where sectors must be erased.

  The main purpose of this example is to verify sustained write performance
  is fast enough for USB MTP.  Microsoft Windows requires at least
  87370 bytes/sec speed to avoid timeout and cancel of MTP SendObject.
    https://forum.pjrc.com/threads/68139?p=295294&viewfull=1#post295294

  This example code is in the public domain.
*/

#include <LittleFS.h>

void Local_Error_Handler()
{
    asm("BKPT #0\n"); // break into the debugger
}

#if defined(ARDUINO_BLACKPILL_F411CE)
//              MOSI  MISO  SCLK
SPIClass SPIbus(PA7,  PA6,  PA5);
#define CS_PIN PA4
#else
//              MOSI  MISO  SCLK
SPIClass SPIbus(PC12, PC11, PC10);
#define CS_PIN PD2
#endif

LittleFS_SPIFlash myfs;

void setup() {
  bool res;
  Serial.begin(115200);
  while (!Serial) delay(100); // wait until Serial/monitor is opened
  Serial.println("SPI Flash speed test...");
  // ensure the CS pin is pulled HIGH
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);
  delay(10); // Wait a bit to make sure w25qxx chip is ready
  res = myfs.begin(CS_PIN, SPIbus);
  if (!res) {
    Serial.println("initialization failed!");
    Local_Error_Handler();
  }
  Serial.printf("Volume size %d MByte\n", myfs.totalSize() / 1048576);
}

uint64_t total_bytes_written = 0;

void loop() {
  unsigned long buf[1024];
  File myfile = myfs.open("WriteSpeedTest.bin", FILE_WRITE_BEGIN);
  if (myfile) {
    const int num_write = 128;
    Serial.printf("Writing %d byte file... ", num_write * 4096);
    uint32_t val = analogRead(0);
    randomSeed(val);
    uint32_t start = getCurrentMillis();
    for (int n=0; n < num_write; n++) {
      for (int i=0; i<1024; i++) buf[i] = random();
      myfile.write(buf, 4096);
    }
    myfile.close();
    uint32_t ms = getCurrentMillis() - start;
    total_bytes_written = total_bytes_written + num_write * 4096;
    Serial.printf(" %d ms, bandwidth = %d bytes/sec", ms, num_write * 4096 * 1000 / ms);
    myfs.remove("WriteSpeedTest.bin");
  }
  Serial.println();
  delay(2000);
  /* Do not write forever, possibly reducing the chip's lifespan */
  if (total_bytes_written >= myfs.totalSize() * 100) {
    Serial.println("End test, entire flash has been written 100 times");
    while (1) ; // stop here
  }
}
