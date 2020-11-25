#include "FS.h"
#include "SPIFFS.h"
#include "M5StickC.h"
#include "M5Display.h"

#include <HTTPClient.h>
#include <ArduinoJson.h> 
char str[100];
const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=tokyo,jp&APPID=";
const String key = "YOURKEY";

String getWeather(){
  HTTPClient http;
  http.begin(endpoint + key); //URLを指定
  int httpCode = http.GET();  //GETリクエストを送信
  String weather;
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
//ここから参考にしたM5stackの関数群
uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
void drawBmpFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y) {
  if ((x >= 160) || (y >= 80)) return;

  // Open requested file on SD card
  File bmpFS = fs.open(path, "r");

  if (!bmpFS) {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42) {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) {
      y += h - 1;

      M5.Lcd.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (col = 0; col < w; col++) {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        M5.Lcd.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      Serial.print("Loaded in "); Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}
void showWeather(){
  String weather = getWeather();
  Serial.println(weather);
  M5.Lcd.fillScreen(BLACK);
  if (weather=="Clear"){
    drawBmpFile(SPIFFS, "/sunny.bmp", 50, 20);
  }else if(weather=="Clouds"){
    drawBmpFile(SPIFFS, "/cloudy.bmp", 50, 20);
  }else if(weather=="Rain"){
    drawBmpFile(SPIFFS, "/rainy.bmp", 50, 20);
  }else{
    M5.Lcd.print(weather);
  }
  String temp = String(getTemperature()-273.15) + " C";
  M5.Lcd.setCursor(0,0);
  M5.Lcd.print(temp);
}
//参考にした関数終わり
void setup(){
    M5.begin();
    M5.Axp.ScreenBreath(10);          // 7-12で明るさ設定
    M5.Lcd.setRotation(3);            // 0-3で画面の向き  
    M5.Lcd.setTextSize(2);
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
    drawBmpFile(SPIFFS, "/logo.bmp", 0, 0);
}

void loop(){
  showWeather();
  delay(1000*600);
}
