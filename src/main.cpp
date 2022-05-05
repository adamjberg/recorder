#include <Arduino.h>
#include <SD.h>

const char filename[] = "demo.txt";

File txtFile;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!SD.begin(D8)) {
    Serial.println("Card failed, or not present");
    while (1);
  }

  txtFile = SD.open(filename, FILE_WRITE);
  txtFile.println("Hello World!");
  txtFile.close();
}

void loop() {

}
