#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#define PINPV D3
#define PINBAT D4
#define LEDS 19

const char ssid[] = WIFI_SSID;
const char password[] = WIFI_PASSWD;
WiFiClient wifiClient;
Adafruit_NeoPixel PVstrip = Adafruit_NeoPixel(LEDS, PINPV, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel Batstrip = Adafruit_NeoPixel(LEDS, PINBAT, NEO_GRB + NEO_KHZ800);
int numLed = 0;
int LastLedBrightness = 0;
int numConsumptionLED = 0;
int LastConsumptionLedBrightness = 0;

int batLedNum = 0;
int batLedBrightness = 0;

void connectToWiFi(const char* ssid, const char* password){
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP-Address: ");
  Serial.println(WiFi.localIP());

}


void setup() {

  PVstrip.begin();
  PVstrip.setBrightness(50);
  Batstrip.begin();
  Batstrip.setBrightness(50);


  //clear strip
  for (int i = 0; i < 19; i++){
    PVstrip.setPixelColor(i, PVstrip.Color(0, 0, 0));
    Batstrip.setPixelColor(i, PVstrip.Color(0, 0, 0));
  }
  PVstrip.show();
  Batstrip.show();
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello World!");
  //connect to WiFi
  connectToWiFi(ssid, password);

}


void loop() {
  // connect ws2812b


  // put your main code here, to run repeatedly:
  while(WiFi.status() != WL_CONNECTED){
    connectToWiFi(ssid, password);
  }

  //POST request
  HTTPClient http;
  http.begin(wifiClient,"http://espressif/");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST("optType=ReadRealTimeData&pwd=SRCMLTKJL9");
  String payload = http.getString();
  //Serial.println(httpCode);
  //Serial.println(payload);
  if (httpCode != 200){
    Serial.println("Error on HTTP request");
    return;
  }

  http.end();
  //convert payload to JSON
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, payload);
  
  //print each value
/*   for (int i = 0; i < doc["Data"].size(); i++){
    Serial.print("\n" + String(i) + ": ");
    int x = doc["Data"][i];
    Serial.print(x);
  } */


  int pv1 = doc["Data"][14];
  int pv2 = doc["Data"][15];
  int cons = doc["Data"][47];
  int batTotal = doc["Data"][103];
  int pvTotal = pv1 + pv2;


  Serial.println(pvTotal);
  Serial.println(cons);
  Serial.println(batTotal);



  // int pvTotal = 300;
  // int batTotal = 43;
  if (pvTotal < 1001){
    //pvtotal mod 200
    numLed = pvTotal / 200;
    LastLedBrightness = (pvTotal % 200)*255/200;
  }
  else{
    numLed = 4+ (pvTotal/1000);
    LastLedBrightness = (pvTotal % 1000)*255/1000;
  }
  



  if (cons < 1001){
    //pvtotal mod 200
    numConsumptionLED = cons / 200;
    LastConsumptionLedBrightness = (cons % 200)*255/200;
  }
  else{
    numConsumptionLED = 4 + (cons/1000);
    LastConsumptionLedBrightness = 4 +  (cons % 1000)*255/1000;
  }

  Serial.println(numLed);
  Serial.println(numConsumptionLED);

  
  batLedNum = ((batTotal-10) /90.0)*LEDS;
  batLedBrightness = (batTotal-10) % 9 * 255/9;

  for (int i = 0; i < 19; i++){
    PVstrip.setPixelColor(i, PVstrip.Color(0, 0, 0));
  }
  
  for (int i = 0; i < max(numConsumptionLED,numLed); i++)
  {
    if (i < numLed && i < numConsumptionLED)
    {
      PVstrip.setPixelColor(i, PVstrip.Color(0, 0, 255));
    }
    else if (i < numLed)
    {
      PVstrip.setPixelColor(i, PVstrip.Color(0, 255, 0));
    }
    else if (i < numConsumptionLED)
    {
      Serial.print("i: ");
      PVstrip.setPixelColor(i, PVstrip.Color(255, 0, 0));
    }
  }
  
  //set overlapping Pixel
  if (numConsumptionLED == numLed)
  {
    PVstrip.setPixelColor(numLed, PVstrip.Color(0,0, LastConsumptionLedBrightness ) );
  }else if (numLed<numConsumptionLED)
  {
    Serial.println("numLed < numConsumptionLED");
    PVstrip.setPixelColor(numConsumptionLED, PVstrip.Color(LastConsumptionLedBrightness,0, 0));
    PVstrip.setPixelColor(numLed, PVstrip.getPixelColor(numLed) + PVstrip.Color(0, 0, LastLedBrightness));
  }
  else{
    PVstrip.setPixelColor(numLed, PVstrip.Color(0,LastLedBrightness, 0));
    PVstrip.setPixelColor(numConsumptionLED, PVstrip.getPixelColor(numConsumptionLED) + PVstrip.Color(0,0,LastConsumptionLedBrightness));
  }


  PVstrip.show();

  //clear strip
  for (int i = 0; i < 19; i++){
    Batstrip.setPixelColor(i, Batstrip.Color(0, 0, 0));
  }

  //Show battery
  for (int i = 0; i < batLedNum; i++){
    Batstrip.setPixelColor(i, Batstrip.Color(0, 255, 0));
  }
  Batstrip.setPixelColor(batLedNum, Batstrip.Color(0, batLedBrightness, 0));
  Batstrip.show();

  Serial.println("---------------------");
  //wait 10 seconds
  delay(10000);

}








