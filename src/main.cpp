#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#define PINPV D4
#define PINBAT D3

const char ssid[] = WIFI_SSID;
const char password[] = WIFI_PASSWD;
WiFiClient wifiClient;
Adafruit_NeoPixel PVstrip = Adafruit_NeoPixel(19, PINPV, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel Batstrip = Adafruit_NeoPixel(19, PINBAT, NEO_GRB + NEO_KHZ800);
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

  //clear strip
  for (int i = 0; i < 19; i++){
    PVstrip.setPixelColor(i, PVstrip.Color(0, 0, 0));
  }
  PVstrip.show();
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
  http.begin(wifiClient,"http://192.168.178.59/");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST("optType=ReadRealTimeData&pwd=SRCMLTKJL9");
  String payload = http.getString();
  //Serial.println(httpCode);
  //Serial.println(payload);
  http.end();
  //convert payload to JSON
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, payload);
  


  int pv1 = doc["Data"][14];
  int pv2 = doc["Data"][15];
  int cons = doc["Data"][47];
  int batTotal = doc["Data"][103];
  int pvTotal = pv1 + pv2;
  pvTotal = 850;


  // Serial.println(pv1);
  // Serial.println(pv2);
  Serial.println(cons);



  // int pvTotal = 300;
  // int batTotal = 43;
  if (pvTotal < 1001){
    //pvtotal mod 200
    numLed = pvTotal / 200;
    LastLedBrightness = (pvTotal % 200)*255/200;
  }
  else{
    numLed = 5+ (pvTotal/1000);
    LastLedBrightness = (pvTotal % 1000)*255/1000;
  }


  if (cons < 1001){
    //pvtotal mod 200
    numConsumptionLED = cons / 200;
    LastConsumptionLedBrightness = (cons % 200)*255/200;
  }
  else{
    numConsumptionLED = 5 + (cons/1000);
    LastConsumptionLedBrightness = 5 +  (cons % 1000)*255/1000;
  }


  batLedNum = (batTotal-10) / 9;
  batLedBrightness = (batTotal-10) % 9 * 255/9;

  Serial.print("PV LEDs: ");
  Serial.println(numLed);
  Serial.print("Cons LEDS: ");
  Serial.println(numConsumptionLED);
  Serial.println("-----");
  Serial.print("LastLedBrightness: ");
  Serial.println(LastLedBrightness);
 
  
  
  
  
  //clear strip
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


  //wait 10 seconds
  delay(10000);

}








