#include <Arduino.h>
extern "C"
{
#include "user_interface.h"
}
#include <SDFat.h>
#include "sdios.h"
#include <iostream>
#include <string>


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

SdFat sd;

FsFile txtFile;
char dataBuffer[2048];
unsigned long startMillis = 0;
int position = 0;

struct soundhdr {
    char  riff[4];        /* "RIFF"                                  */
    long  flength;        /* file length in bytes                    */
    char  wave[4];        /* "WAVE"                                  */
    char  fmt[4];         /* "fmt "                                  */
    long  chunk_size;     /* size of FMT chunk in bytes (usually 16) */
    short format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
    short num_chans;      /* 1=mono, 2=stereo                        */
    long  srate;          /* Sampling rate in samples per second     */
    long  bytes_per_sec;  /* bytes per second = srate*bytes_per_samp */
    short bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
    short bits_per_samp;  /* Number of bits per sample               */
    char  data[4];        /* "data"                                  */
    long  dlength;        /* data length in bytes (filelength - 44)  */
};

#define AUDIO_SIZE 2048 * 80
struct soundhdr wavh;

void setup()
{
  Serial.begin(9600);

  while (!Serial)
    ;

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }

  strncpy(wavh.riff,"RIFF",4);
  strncpy(wavh.wave,"WAVE",4);
  strncpy(wavh.fmt,"fmt ",4);
  strncpy(wavh.data,"data",4);

  wavh.chunk_size = 16;
  wavh.format_tag = 1;
  wavh.num_chans = 1;
  wavh.srate = 8000;
  wavh.bits_per_samp = 16;

  wavh.flength = AUDIO_SIZE + 44;
  wavh.dlength = AUDIO_SIZE;

  randomSeed(analogRead(1));
  int r = random(1000);
  Serial.println(r);


  std::string filename = "recording" + std::to_string(r) + ".wav";

  txtFile.open(filename.c_str(), FILE_WRITE);
  txtFile.write((byte *) &wavh, 44);
  startMillis = millis();

  Serial.println("START");
}

int numWritten = 0;

void loop()
{
  int startLoopMicroseconds = micros();
  if (numWritten >= AUDIO_SIZE)
  {
    Serial.println("DONE");
    txtFile.close();
  }
  else
  {
    uint16_t result16;
    system_adc_read_fast(&result16, 1, 8);

    uint16_t shifted_result = result16 << 6;

    memcpy(&dataBuffer[position], &shifted_result, sizeof(uint16_t));
    position = position + 2;

    if (position == 2048)
    {
      txtFile.write(dataBuffer, 2048);
      position = 0;
      numWritten += 2048;
    }
  }
  int endLoopMicroseconds = micros();
  int diffMicroseconds = endLoopMicroseconds - startLoopMicroseconds;
  int delayMs = 120 - diffMicroseconds;
  if (delayMs > 0) {
    delayMicroseconds(delayMs);
  }
}