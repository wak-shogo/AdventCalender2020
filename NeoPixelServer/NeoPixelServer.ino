#include "WiFi.h"
#include <M5StickC.h>
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <Adafruit_NeoPixel.h>
#define PIN 33 //INが接続されているピンを指定
#define NUMPIXELS 30 //LEDの数を指定
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //800kHzでNeoPixelを駆動
int saturation = 100;
int value = 0;
int brightness = 1;
int HUE = 32768;
int lednum = 1;
const char* ssid = "ESP32_RGB";
const char* pass = "esp32";
const IPAddress ip(192,168,0,1);
const IPAddress subnet(255,255,255,0);
// ポート80にサーバーを設定
AsyncWebServer server(80);
// 実際のピン出力によってhtmlファイル内のSTATEの文字を変える
String processor(const String& var){
  return String();
}

void setup(){
  M5.begin();
  Serial.begin(115200);
  pixels.begin(); //NeoPixelを開始
  // SPIFFSのセットアップ
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //AP（親機）モードの場合
  WiFi.softAP("esp32_rgb","password");
  delay(100);
  WiFi.softAPConfig(ip,ip,subnet);
  Serial.println(WiFi.softAPIP());
  delay(1000);
  // サーバーのルートディレクトリにアクセスされた時のレスポンス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    int paramsNr = request->params();
    Serial.println(paramsNr);
    for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->name() =="HUE"){
          HUE = p->value().toInt();
        }else if(p->name() == "Brightness"){
          brightness = p->value().toInt();
        }else if(p->name() == "Saturation"){
          saturation = p->value().toInt();
        }else if(p->name() == "lednum"){
          lednum = p->value().toInt();
        }
        OnPixels(HUE,saturation,brightness,lednum);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // style.cssにアクセスされた時のレスポンス
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
    // style.cssにアクセスされた時のレスポンス
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  });
  // サーバースタート
  server.begin();
  OnPixels(HUE,saturation,brightness,lednum);
}

void loop(){
}

void OnPixels(int hue,int saturation, int brightness,int lednum){
  for(int i=0; i<NUMPIXELS; i++) { //LEDを１つずつ制御 
    if(i < lednum){
      pixels.setPixelColor(i, pixels.ColorHSV(hue, saturation, brightness)); //LEDの色を設定
    }else{
      pixels.setPixelColor(i, pixels.ColorHSV(hue, saturation, 0)); //LEDの色を設定
    }
      pixels.show();   //LEDに色を反映 
      delay(40);
  }
}
void FirePixels(int hue,int saturation){
  delay(5000);
  for(int value=0;value<255;value++){
    for(int i=0; i<NUMPIXELS; i++) { //LEDを１つずつ制御  
      pixels.setPixelColor(i, pixels.ColorHSV(hue + i*200, saturation, value)); //LEDの色を設定
      pixels.show();   //LEDに色を反映
      delay(80);
    }
    delay(80);
  }
  delay(10000);
  for(value=255;value>0;value--){
    for(int i=0; i<NUMPIXELS; i++) { //LEDを１つずつ制御  
      pixels.setPixelColor(i, pixels.ColorHSV(hue, saturation, value)); //LEDの色を設定
      pixels.show();   //LEDに色を反映
    }
    delay(20);
  }
  pixels.clear(); //NeoPixelの出力をリセット
  pixels.show();
}
