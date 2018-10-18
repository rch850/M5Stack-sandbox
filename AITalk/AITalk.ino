#include <M5Stack.h>

#include <WiFi.h>
#include "AudioFileSourceHTTPStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// Enter your WiFi setup here:
const char *SSID = "";
const char *PASSWORD = "";

// mp3 url
const char *URL="https://webapi.aitalk.jp/webapi/v2/ttsget.php?username=&password=&speaker_name=nozomi&ext=mp3&text=%E3%81%8A%E3%82%84%E3%81%A4%E3%83%A9%E3%83%B3%E3%83%89%E3%81%B8%E3%82%88%E3%81%86%E3%81%93%E3%81%9D";

AudioGeneratorMP3 *mp3;
AudioFileSourceHTTPStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2S *out;

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}


void setup()
{
  M5.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Connecting to WiFi");

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextDatum(MC_DATUM);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.drawString("Connecting to WiFi", 160, 120);

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);

  WiFi.begin(SSID, PASSWORD);

  // Try forever
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...Connecting to WiFi");
    delay(1000);
  }
  Serial.println("Connected");

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.drawString("Connected", 160, 120);

  file = new AudioFileSourceHTTPStream(URL);
  buff = new AudioFileSourceBuffer(file, 2048);
  buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  mp3->begin(buff, out);

  M5.Lcd.fillScreen(BLACK);
}


void loop()
{
  static int lastms = 0;

  M5.Lcd.drawString("Welcome to Oyatu Land", 160, 120);

  if (mp3->isRunning()) {
    if (millis()-lastms > 1000) {
      lastms = millis();
      Serial.printf("Running for %d ms...\n", lastms);
      Serial.flush();
     }
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.printf("MP3 done\n");
    delay(1000);
  }
}

