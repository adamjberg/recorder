#include <Arduino.h>
#include <SD.h>

const char filename[] = "demo.txt";

File txtFile;
char dataBuffer[2048];
unsigned long startMillis = 0;
int position = 0;

void setup()
{
  Serial.begin(9600);

  while (!Serial)
    ;

  if (!SD.begin(D8))
  {
    Serial.println("Card failed, or not present");
    while (1)
      ;
  }

  txtFile = SD.open(filename, FILE_WRITE);
  startMillis = millis();
}

void loop()
{
  if (millis() - startMillis >= 1000)
  {
    txtFile.close();
  }
  else
  {
    int sensorValue = analogRead(A0);
    dataBuffer[position] = 'a';
    position = position + 1;

    if (position == 2048)
    {
      txtFile.write(dataBuffer, 2048);
      position = 0;
    }
  }
}
