#include <Arduino.h>

//SGP30 needs 15 seconds to initialize calibration after power on.
//The screen will display TVOC and CO2

#include <M5Stack.h>
#include "Adafruit_SGP30.h"
#include <WiFi.h>
#include "params.h"
#include <Ambient.h>

//
//#define SDA_PIN 21
//#define SCL_PIN 22

Adafruit_SGP30 sgp;
int i = 15;
int count = 0;
long last_millis = 0;
int ambiCount = 0;
int avr_eCO2 =0;  //initialize avr_eCO2

WiFiClient client;
Ambient ambient;

void header(const char *string, uint16_t color)
{
    M5.Lcd.fillScreen(color);
    M5.Lcd.setRotation(3);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.fillRect(0, 0, 320, 30, TFT_BLACK);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(string, 160, 3, 4); 
}



void setup() {
  M5.begin(true, false, true, true);
//  Wire.begin(SDA_PIN, SCL_PIN);        // join i2c bus (address optional for master)
  header("SGP30 TEST",TFT_BLACK);
  Serial.begin(115200);
      WiFi.begin(ssid, password);  //  Wi-Fi APに接続
    while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
        delay(100);
    }

  ambient.begin(channelId, writeKey, &client);


  Serial.println("SGP30 test");
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
      while (count < 10 ){
        M5.Lcd.setTextColor(RED);
        M5.Lcd.drawString("SGP30 not Working!", 50, 60, 4);
        delay(1000);
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.drawString("SGP30 not Working!", 50, 60, 4);
        delay(1000);
       count++;      
      }
    while (1);
  }
  
  M5.Lcd.drawString("TVOC:", 50, 40, 4);
  M5.Lcd.drawString("eCO2:", 50, 80, 4);
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  M5.Lcd.drawString("Initialization...", 140, 120, 4);
  while(i > 0) { 
    if(millis()- last_millis > 1000) {
      last_millis = millis();
      i--;
      M5.Lcd.fillRect(0, 120, 40, 20, TFT_BLACK);
      M5.Lcd.drawNumber(i, 20, 120, 4);
    }
  }
  M5.Lcd.fillRect(0, 120, 300, 30, YELLOW);
}

void loop() {
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  if(millis()- last_millis > 20000) {
    last_millis = millis();  
    i = 200;
    while (i > 0){
    M5.Lcd.setBrightness(i);
    i--;
    delay(10);
    }
    M5.Lcd.sleep();
    M5.Lcd.setBrightness(0);
  }

  M5.update();
  if (M5.BtnA.wasReleased()) {
    last_millis = millis(); //Timer clear
    M5.Lcd.wakeup();
    M5.Lcd.setBrightness(200);
    delay(1000);
  }

  M5.Lcd.fillRect(96, 36, 72, 70, BLUE);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.drawNumber(sgp.TVOC, 120, 40 , 4);
  M5.Lcd.drawString("ppb", 200, 40, 4);
  M5.Lcd.drawNumber(sgp.eCO2, 120, 80, 4);
  M5.Lcd.drawString("ppm", 200, 80, 4);
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  delay(1000);
  ambiCount++;
      Serial.println(";  ambiCount = " + String(ambiCount));
      avr_eCO2 = avr_eCO2 + sgp.eCO2;
      if (ambiCount == 60) { // 60秒に一回Ambidataに送信。
        avr_eCO2 = avr_eCO2 / ambiCount; //avarage of eCO2 of 60 seconds
        ambient.set(2, avr_eCO2); // Dataセット2にデータ送信。データがint型かfloat型であれば、直接セット
        if (ambient.send()){
          Serial.print("Succeeded sending co2 data=");
          Serial.println(avr_eCO2);
          M5.Lcd.fillRect(0, 120, 300, 30, YELLOW);
          M5.Lcd.setTextColor(BLACK);
          M5.Lcd.setTextFont(2);
          M5.Lcd.drawString("Sent : ",50, 124,4);
          M5.Lcd.drawString (String(avr_eCO2),120, 124,4);

        } else {
          Serial.println("Something went wrong sending data to AmbiData");

        }
        ambiCount =1;
        avr_eCO2 = 0;
      }

}