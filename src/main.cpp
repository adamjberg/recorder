#include <Arduino.h>
extern "C"
{
#include "user_interface.h"
}
#include <SDFat.h>
#include "sdios.h"


const uint8_t SD_CS_PIN = D8;
#define SPI_CLOCK SD_SCK_MHZ(21)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

const char filename[] = "demo3.txt";

SdFat sd;

FsFile txtFile;
char dataBuffer[2048];
unsigned long startMillis = 0;
int position = 0;

void setup()
{
  Serial.begin(9600);

  while (!Serial)
    ;

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }

  txtFile.open(filename, FILE_WRITE);
  startMillis = millis();
}

void loop()
{
  int startLoopMicroseconds = micros();
  if (millis() - startMillis >= 1000)
  {
    txtFile.close();
  }
  else
  {
    uint16_t result16;
    system_adc_read_fast(&result16, 1, 8);
    dataBuffer[position] = 'a';
    position = position + 1;

    if (position == 2048)
    {
      txtFile.write(dataBuffer, 2048);
      position = 0;
    }
  }
  int endLoopMicroseconds = micros();
  int diffMicroseconds = endLoopMicroseconds - startLoopMicroseconds;
  int delayMs = 125 - diffMicroseconds;
  if (delayMs > 0) {
    delayMicroseconds(delayMs);
  }
}