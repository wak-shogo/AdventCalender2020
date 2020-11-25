#include "FS.h"
#include "SPIFFS.h"
#include <stdio.h>
#include "M5StickC.h"
#include "M5Display.h"

#include <HTTPClient.h>
#include <ArduinoJson.h> 
char str[100];
const char* ssid = "YOURSSID";
const char* password = "YOURPASS";

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=tokyo,jp&APPID=";
const String key = "YOURKEY";


String getWeather(){
  HTTPClient http;
  http.begin(endpoint + key); //URLを指定
  int httpCode = http.GET();  //GETリクエストを送信
  String weather="Error";
  if (httpCode > 0) { //返答がある場合

      String payload = http.getString();  //返答（JSON形式）を取得
      Serial.println(httpCode);
      Serial.println(payload);

      //jsonオブジェクトの作成
      DynamicJsonBuffer jsonBuffer;
      String json = payload;
      JsonObject& weatherdata = jsonBuffer.parseObject(json);

      //パースが成功したかどうかを確認
      if(!weatherdata.success()){
        Serial.println("parseObject() failed");
      }

      //各データを抜き出し
       weather = weatherdata["weather"][0]["main"].as<char*>();
      const double temp = weatherdata["main"]["temp"].as<double>();
      }
  return weather;
}
double getTemperature(){
  HTTPClient http;
  http.begin(endpoint + key); //URLを指定
  int httpCode = http.GET();  //GETリクエストを送信
  if (httpCode > 0) { //返答がある場合

      String payload = http.getString();  //返答（JSON形式）を取得
      Serial.println(httpCode);
      Serial.println(payload);

      //jsonオブジェクトの作成
      DynamicJsonBuffer jsonBuffer;
      String json = payload;
      JsonObject& weatherdata = jsonBuffer.parseObject(json);

      //パースが成功したかどうかを確認
      if(!weatherdata.success()){
        Serial.println("parseObject() failed");
      }
      const double temp = weatherdata["main"]["temp"].as<double>();
      return temp;
  }else{
    return -88.88;
  }
}

void showWeather(){
  M5.Lcd.fillScreen(BLACK);
  String weather = getWeather();
  String temp = String(getTemperature()-273.15) + " C";
  Serial.println(weather);
  M5.Lcd.setCursor(0,10);
  M5.Lcd.print(weather);
  M5.Lcd.setCursor(0,40);
  M5.Lcd.print(temp);
}
void setup(){
    M5.begin();
    M5.Axp.ScreenBreath(12);          // 7-12で明るさ設定
    M5.Lcd.setRotation(3);            // 0-3で画面の向き  
    M5.Lcd.setTextSize(3);
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    delay(1000);
}

void loop(){
  showWeather();
  delay(1000*600);//10分待機
}
