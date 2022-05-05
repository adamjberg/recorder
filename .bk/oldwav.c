#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

void handleRoot() {
  Serial.println("Root");
  server.send(200, "text/plain", "hello from esp8266!\r\n");
}

#define AUDIO_SIZE 20000

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
    char  audio_data[AUDIO_SIZE];
};

#define MIC A0

struct soundhdr *wavh;

void setup(void) {
  pinMode(MIC, INPUT);

  wavh = (struct soundhdr *)malloc(sizeof(struct soundhdr));
  
  // It's easy enough to initialize the strings
  strncpy(wavh->riff,"RIFF",4);
  strncpy(wavh->wave,"WAVE",4);
  strncpy(wavh->fmt,"fmt ",4);
  strncpy(wavh->data,"data",4);
  
  // size of FMT chunk in bytes
  wavh->chunk_size = 16;
  wavh->format_tag = 1; // PCM
  wavh->num_chans = 1; // mono
  // This is easier than converting to hex and then to bytes :)
  wavh->srate = 8000;
  wavh->bits_per_samp = 8;

  wavh->flength = AUDIO_SIZE + 44;
  wavh->dlength = AUDIO_SIZE;
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/gif", []() {
    server.send(200, "audio/wav", (byte *) wavh, wavh->flength);
  });

  server.begin();
  Serial.println("HTTP server started");
}

long cur_pos = 0;

void loop(void) {
  server.handleClient();
  MDNS.update();

  if (cur_pos == 0) {
    Serial.println("START!"); 
  }

  if (cur_pos < AUDIO_SIZE) {
    int readVal = analogRead(MIC);
    if (readVal < 255) {
      wavh->audio_data[cur_pos] = readVal; 
    } else {
      Serial.println(readVal);
      wavh->audio_data[cur_pos] = 255;
    }

    cur_pos += 1;
  }

  if (cur_pos == AUDIO_SIZE) {
    Serial.println("FINISH!");
    cur_pos += 1; 
  }
}