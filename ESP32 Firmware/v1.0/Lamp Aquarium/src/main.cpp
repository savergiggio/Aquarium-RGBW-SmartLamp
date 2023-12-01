#include "BluetoothSerial.h"
#include "time.h"
#include <Preferences.h>
#include <main.h>
#include <ESP32Time.h>

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// update variables
const char *ssid = "LAMP_updater";
const char *password = "aquariumlife";
AsyncWebServer server(80);

ESP32Time rtc(0); // offset in seconds GMT+1
Preferences preferences;
BluetoothSerial SerialBT;

// riceve da android l'orario e la data corrente
int hour, minute, day, month, year;
// Imposta la durata dell'effetto alba/tramonto in secondi
unsigned int fadeDuration = 1;
// Imposta l'ora di accensione del LED di default
int onHour = 8;
int onMinute = 0;

// Imposta l'ora di spegnimento del LED di default
int offHour = 12;
int offMinute = 0;

// schedule on/off state
bool manual = 1;
volatile bool flagtask;

int updater = 0;

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    String ip = "power:12:\n";
    String myString = ip;                          // Stringa da inviare
    byte myBytes[myString.length()];               // Array di byte che conterrà la stringa
    myString.getBytes(myBytes, myString.length()); // Converti la stringa in un array di byte
    SerialBT.write(myBytes, myString.length());    // Invia l'array di byte
    delay(100);
  }
}

unsigned long minutiToMillisecondi(unsigned int minuti)
{
  return minuti * 60000UL;
}

const int freq = 5000; // 5000
const int ledChannelw = 0;
const int ledChannelr = 1;
const int ledChannelg = 2;
const int ledChannelb = 3;
const int res = 13;
byte str_in[4];
#define ledw 5 // 14
#define ledr 18 // 25
#define ledg 19  // 12
#define ledb 21 // 13
#define fan 23
#define tempb 22
#define templ 32
#define tempw 33
OneWire oneWireb(tempb);
OneWire oneWirel(templ);
OneWire oneWirew(tempw);
DallasTemperature DS18B20b(&oneWireb);
DallasTemperature DS18B20l(&oneWirel);
DallasTemperature DS18B20w(&oneWirew);
int pwm_w, pwm_r, pwm_g, pwm_b, power_w;

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  btStop();
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
  Serial.println("OTA update finished successfully!");
  updater = 0;
  preferences.putUInt("updater", updater);
  } else {
    Serial.println("There was an error during OTA update!");
  updater = 0;
  preferences.putUInt("updater", updater);
  }
  delay(1000);
  ESP.restart();
}

void setup(void)
{
  Serial.begin(115200);
  digitalWrite(23, HIGH);
  ElegantOTA.setAutoReboot(true);
  setCpuFrequencyMhz(240);
  delay(100);
  preferences.begin("led", false);
  btStart();
  DS18B20b.begin();
  DS18B20l.begin();
  DS18B20w.begin();
  SerialBT.begin("Aquarium WRGB Lamp"); // Bluetooth device name
  SerialBT.register_callback(callback);

  updater = preferences.getUInt("updater", 0);

  manual = preferences.getUInt("manual", 0);
  onHour = preferences.getUInt("onHour", 0);
  onMinute = preferences.getUInt("onMinute", 0);
  offHour = preferences.getUInt("offHour", 0);
  offMinute = preferences.getUInt("offMinute", 0);
  fadeDuration = preferences.getUInt("fade", 0);

  hour = preferences.getUInt("hour", 0);
  minute = preferences.getUInt("minute", 0);
  day = preferences.getUInt("day", 0);
  month = preferences.getUInt("month", 0);
  year = preferences.getUInt("year", 0);


  pinMode(fan, OUTPUT);
  pinMode(tempb, INPUT);
  pinMode(templ, INPUT);
  pinMode(tempw, INPUT);
  ledcAttachPin(ledw, ledChannelw);
  ledcAttachPin(ledr, ledChannelr);
  ledcAttachPin(ledg, ledChannelg);
  ledcAttachPin(ledb, ledChannelb);
  ledcSetup(ledChannelw, freq, res);
  ledcSetup(ledChannelr, freq, res);
  ledcSetup(ledChannelg, freq, res);
  ledcSetup(ledChannelb, freq, res);
  ledcWrite(ledChannelw, 0);
  ledcWrite(ledChannelr, 0);
  ledcWrite(ledChannelg, 0);
  ledcWrite(ledChannelb, 0);
  setPwmValues();
  Serial.println(fadeDuration);
  Serial.println(updater);
  rtc.setTime(0, minute, hour, day, month, year); // 17th Jan 2021 15:24:30
}

void loop(void)
{
  
    
    ElegantOTA.loop();
    while (SerialBT.available() > 0)
    {
      
      String str_in = SerialBT.readStringUntil(';');

      char c = str_in[0];
      Serial.println(c);
      String r = str_in.substring(1, 4);
      unsigned int pwm = r.toInt();
      Serial.println(pwm);

      if (manual == 0)
      {
        if (c == 'W')
        {
          int pwm_w = map(pwm, 0, 100, 0, 1000); //4096
          ledcWrite(ledChannelw, pwm_w);
          preferences.putUInt("w", pwm_w);
        }
        if (c == 'R')
        {
          int pwm_r = map(pwm, 0, 100, 0, 1000); //4096
          ledcWrite(ledChannelr, pwm_r);
          preferences.putUInt("r", pwm_r);
        }
        if (c == 'G')
        {
          int pwm_g = map(pwm, 0, 100, 0, 1000);  //4096
          Serial.println(pwm_g);
          ledcWrite(ledChannelg, pwm_g);
          preferences.putUInt("g", pwm_g);
        }
        if (c == 'B')
        {
          int pwm_b = map(pwm, 0, 100, 0, 1000);  //4096
          ledcWrite(ledChannelb, pwm_b);
          preferences.putUInt("b", pwm_b);
        }
      }
      if (c == 'F')
      {
        unsigned int fadeDuration = minutiToMillisecondi(pwm);
        Serial.println(fadeDuration); // Stampa il risultato in millisecondi
        preferences.putUInt("fade", fadeDuration);
      }
      if (c == 'A')
      {
        onHour = pwm;
        preferences.putUInt("onHour", onHour);
      }
      if (c == 'C')
      {
        onMinute = pwm;
        preferences.putUInt("onMinute", onMinute);
      }
      if (c == 'D')
      {
        offHour = pwm;
        preferences.putUInt("offHour", offHour);
      }
      if (c == 'H')
      {
        offMinute = pwm;
        preferences.putUInt("offMinute", offMinute);
      }

      if (c == 'M')
      {
        manual = 0;
        flagtask = true;
        preferences.putUInt("manual", manual);
      }

      if (c == 'T')
      {
        manual = 1;
        flagtask = false;
        preferences.putUInt("manual", manual);
      }

      if (c == 'K')
      {
        digitalWrite(fan, HIGH);
      }

      if (c == 'J')
      {
        digitalWrite(fan, LOW);
      }

      if (c == 'I')
      {
        getInfo();
      }

      if (c == 'U')
      {
        updater_firmware();
        //updater = 1;
        //preferences.putUInt("updater", updater); 
        //ESP.restart();
      }

      if (str_in.indexOf(':') != -1)
      {
        char separator = ':';
        char inputBuffer[64]; // Cambia la dimensione se necessario
        str_in.toCharArray(inputBuffer, sizeof(inputBuffer));
        char *token = strtok(inputBuffer, ":");

        if (token != NULL)
        {
          hour = atoi(token);
          preferences.putUInt("hour", hour);
          token = strtok(NULL, ":");
        }
        if (token != NULL)
        {
          minute = atoi(token);
          preferences.putUInt("minute", minute);
          token = strtok(NULL, ":");
        }
        if (token != NULL)
        {
          day = atoi(token);
          preferences.putUInt("day", day);
          token = strtok(NULL, ":");
        }
        if (token != NULL)
        {
          month = atoi(token);
          preferences.putUInt("month", month);
          token = strtok(NULL, ":");
        }
        if (token != NULL)
        {
          year = atoi(token);
          preferences.putUInt("year", year);
        }
        struct tm timeinfo = rtc.getTimeStruct();
        rtc.setTime(27, minute, hour, day, month, year);
      }
    }
    if (manual == 1)
    {
      scheduledOnOff();
    }
    // Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));  // (String) returns time with specified format
    // delay(1000);
  led_temp();
}

void setPwmValues()
{
  pwm_w = preferences.getUInt("w", 0);
  pwm_r = preferences.getUInt("r", 0);
  pwm_g = preferences.getUInt("g", 0);
  pwm_b = preferences.getUInt("b", 0);
  ledcWrite(ledChannelb, pwm_b);
  ledcWrite(ledChannelg, pwm_g);
  ledcWrite(ledChannelr, pwm_r);
  ledcWrite(ledChannelw, pwm_w);
}

void savePwmValues()
{
  preferences.putUInt("w", pwm_w);
  preferences.putUInt("b", pwm_b);
  preferences.putUInt("r", pwm_r);
  preferences.putUInt("g", pwm_g);
}

void getPwmValues()
{
  Serial.println(pwm_w);
  Serial.println(pwm_r);
  Serial.println(pwm_g);
  Serial.println(pwm_b);
}

void scheduledOnOff()
{
  Serial.println("Schedule Mode");
  fadeDuration = preferences.getUInt("fade", 0);
  pwm_w = preferences.getUInt("w", 0);
  pwm_r = preferences.getUInt("r", 0);
  pwm_g = preferences.getUInt("g", 0);
  pwm_b = preferences.getUInt("b", 0);

  struct tm timeinfo = rtc.getTimeStruct();
  int nowhour = rtc.getHour(true);
  int nowminute = rtc.getMinute();

  Serial.print(nowhour);
  Serial.print(":");
  Serial.println(nowminute);
  unsigned long previousMillis = 0;
  const unsigned long interval = 1000; // Intervallo di controllo dei dati ogni 1 secondo

  // Serial.println(&timeinfo, "%H.%M");
  int elapsedTime = (nowhour - onHour) * 3600 + (nowminute - onMinute) * 60;

  // Accendi il LED con effetto alba se l'ora corrente corrisponde all'ora di accensione
  if (elapsedTime == 0)
  {

    Serial.println(pwm_g);
    unsigned long startTime = millis(); // Salva il tempo di inizio

    while (true)
    {
      unsigned long currentTime = millis();
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval)
      {
        // Verifica se sono disponibili nuovi dati Bluetooth da leggere
        while (SerialBT.available() > 0)
        {

          String str_in = SerialBT.readStringUntil(';');
          char c = str_in[0];
          Serial.println(c);

          String r = str_in.substring(1, 4);
          unsigned int pwm = r.toInt();
          Serial.println(pwm);

          if (c == 'M')
          {
            manual = 0;
            preferences.putUInt("manual", manual);
            return loop();
          }

          if (c == 'I')
          {
            //getInfo();
          }
          previousMillis = currentMillis;
        }
      }
      float elapsedMillis = currentTime - startTime;

      if (elapsedMillis > fadeDuration)
      {
        break;
      }

      float progress = elapsedMillis / fadeDuration;
      Serial.print(fadeDuration);
      Serial.print(":");
      Serial.println(progress);

      float pwm_blue = progress * pwm_b;
      float pwm_green = progress * pwm_g;
      float pwm_red = progress * pwm_r;
      float pwm_white = progress * pwm_w;

      ledcWrite(ledChannelb, (int)(pwm_blue));
      ledcWrite(ledChannelg, (int)(pwm_green));
      ledcWrite(ledChannelr, (int)(pwm_red));
      ledcWrite(ledChannelw, (int)(pwm_white));

      // Esegui altre operazioni durante il fading

      // delay(1000); // Aggiungi una breve pausa per evitare un'elaborazione eccessivamente veloce
    }
    savePwmValues();
    setPwmValues();
  }

  int remainingTime = ((offHour - nowhour) * 3600 + (offMinute - nowminute) * 60);

  if (remainingTime == 0)
  {

    Serial.println(pwm_g);
    unsigned long startTime = millis(); // Salva il tempo di inizio

    while (true)
    {
      unsigned long currentTime = millis();
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval)
      {
        while (SerialBT.available() > 0)
        {

          String str_in = SerialBT.readStringUntil(';');
          char c = str_in[0];
          Serial.println(c);

          String r = str_in.substring(1, 4);
          unsigned int pwm = r.toInt();
          Serial.println(pwm);

          if (c == 'M')
          {
            manual = 0;
            preferences.putUInt("manual", manual);
            return loop();
          }

          if (c == 'I')
          {
            //getInfo();
          }
          // Leggi i dati Bluetooth disponibili
          // Esegui le operazioni necessarie con i dati ricevuti

          // Aggiorna previousMillis
          previousMillis = currentMillis;
        }
      }
      float elapsedMillis = currentTime - startTime;

      if (elapsedMillis > fadeDuration)
      {
        break; // Esci dal ciclo quando il tempo di fading è trascorso
      }

      float progress = elapsedMillis / fadeDuration;
      Serial.print(fadeDuration);
      Serial.print(":");
      Serial.println(progress);

      float pwm_blue = (1 - progress) * pwm_b; // Inverti l'ordine per l'effetto tramonto
      float pwm_green = (1 - progress) * pwm_g;
      float pwm_red = (1 - progress) * pwm_r;
      float pwm_white = (1 - progress) * pwm_w;

      ledcWrite(ledChannelb, (int)(pwm_blue));
      ledcWrite(ledChannelg, (int)(pwm_green));
      ledcWrite(ledChannelr, (int)(pwm_red));
      ledcWrite(ledChannelw, (int)(pwm_white));

      // Esegui altre operazioni durante il fading

      // delay(1000); // Aggiungi una breve pausa per evitare un'elaborazione eccessivamente veloce
    }

    savePwmValues();
    setPwmValues();
  }

  // Verifica se l'ora corrente è compresa nell'intervallo di accensione/spegnimento
  else if (elapsedTime > 0 && remainingTime > 0)
  {
    // Accendi il LED al massimo della luminosità
    setPwmValues();
  }
  else
  {
    // Spegni il LED
    ledcWrite(ledChannelb, 0);
    ledcWrite(ledChannelg, 0);
    ledcWrite(ledChannelr, 0);
    ledcWrite(ledChannelw, 0);
  }
  delay(1000);
}

void led_temp()
{

  DS18B20b.requestTemperatures();       // send the command to get temperatures
  float tempB = DS18B20b.getTempCByIndex(0);
  DS18B20l.requestTemperatures();       // send the command to get temperatures
  float tempL = DS18B20l.getTempCByIndex(0);
  DS18B20w.requestTemperatures();       // send the command to get temperatures
  float tempW = DS18B20w.getTempCByIndex(0);
/*
  Serial.print("Board:");
  Serial.println(tempB);
  Serial.print("LED:");
  Serial.println(tempL);
  Serial.print("water:");
  Serial.println(tempW);
  */

  //delay(1000);

}

void updater_firmware()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo.");
  });


  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  server.begin();
  Serial.println("HTTP server started");
  //delay(1000);
}

void getInfo()
{
  uint8_t *uint8Message;
  size_t messageLength;
  if (!WL_CONNECTED)
  {
    String ip = "no:\n";
    String myString = ip;            // Stringa da inviare
    byte myBytes[myString.length()]; // Array di byte che conterrà la stringa
    // Converti la stringa in un array di byte
    myString.getBytes(myBytes, myString.length());
    // Invia l'array di byte
    SerialBT.write(myBytes, myString.length());
    //delay(500);
  }
  else
  {
    // Ottenimento del valore di TcL
    DS18B20b.requestTemperatures();       // send the command to get temperatures
    float tempB = DS18B20b.getTempCByIndex(0);
    
    struct tm timeinfo = rtc.getTimeStruct();
    int nowhour = rtc.getHour(true);
    int nowminute = rtc.getMinute();
    String add = "192.168.4.1";
    // Costruzione del messaggio
    String ip = "info:" + String(add) + ":" + String(nowhour) + "." + String(nowminute) + ":" + "v2.0" + ":" +  String(tempB, 1) + ":";
    // String currentTime = ":" + String(nowhour) + "." + String(nowminute)+ ":";
    // String tempMessage = ":" + String(TcL, 1) + "::";
    // String versionMessage = ":v1.0";

    // Calcola la lunghezza totale del messaggio
    messageLength = ip.length();

    // Alloca spazio per l'array di byte del messaggio
    uint8Message = new uint8_t[messageLength];

    // Copia le stringhe nel messaggio di byte
    ip.getBytes(uint8Message, ip.length());
    // currentTime.getBytes(uint8Message + ip.length(), currentTime.length());
    // tempMessage.getBytes(uint8Message + ip.length(), tempMessage.length());
    // versionMessage.getBytes(uint8Message + ip.length(), tempMessage.length(), versionMessage.length());

    // Invia il messaggio tramite Bluetooth
    SerialBT.write(uint8Message, messageLength);
    Serial.println((char *)uint8Message);

    // Libera la memoria allocata per il messaggio
    // delete[] uint8Message;
  }
}
