#include <QueueArray.h>
#include <USBHID.h>
#include "SPI.h"
#include <Adafruit_GFX_AS.h>    // Core graphics library, with extra fonts.
#include <Adafruit_ILI9341_STM.h> // STM32 DMA Hardware-specific library
#include <Wire.h>
//#include <Fonts/FreeMonoBold12pt7b.h>

#define TFT_CS         PA4                  
#define TFT_DC         PA3                
#define TFT_RST        PA2 

#define MODE           PB11
#define BATTERY        PA1
#define NUMLK          PB10
#define SDA            PB7
#define SCL            PB6
#define INT            PB5

#define ADDR0          32
#define ADDR1          33


Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST); // Use hardware SPI

bool mode = 1; //1 = numpad, 0 = calc
bool numlk = 0;
const uint8_t lookupTable[2][22] = {{KEY_F1, KEY_F2, KEY_F3, 0, 0, '/', '*', KEY_BACKSPACE, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', KEY_RETURN, '0', '.'},
                                    {KEY_F1, KEY_F2, KEY_F3, 0, 0, '/', '*', KEY_BACKSPACE, KEY_HOME, KEY_UP_ARROW, KEY_PAGE_UP, '-', KEY_LEFT_ARROW, '5', KEY_RIGHT_ARROW, '+', KEY_END, KEY_DOWN_ARROW, KEY_PAGE_DOWN, KEY_RETURN, KEY_INSERT, KEY_DELETE}};
QueueArray <uint8_t> keyQueue;

String labels[4] = {"F1", "F2", "F3", "LIGHTING"};

String expressions[6];

void setup() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  //tft.setFont(&FreeMonoBold12pt7b);
  tft.setTextWrap(false);
  pinMode(INT, INPUT);
  attachInterrupt(INT, ISR, FALLING);
  pinMode(MODE, INPUT);
  //mode = digitalRead(MODE);
  pinMode(BATTERY, INPUT);
  Wire.begin();
  
  Wire.beginTransmission(ADDR0);
  Wire.write(0b00000110);
  Wire.write(0b11111111);
  Wire.write(0b11111111);
  Wire.endTransmission();

  Wire.beginTransmission(ADDR1);
  Wire.write(0b00000110);
  Wire.write(0b11111111);
  Wire.write(0b11111111);
  Wire.endTransmission();

  if(mode){
    USBHID_begin_with_serial(HID_BOOT_KEYBOARD);
    BootKeyboard.begin();
    delay(1000);
    functionTabs();
    //tft.drawFastHLine
  } else {
    labels[0] = "SYMBOLS";
    labels[1] = "TRIGONOMETRY";
    labels[2] = "VARIABLES";
    labels[3] = "MENU";
    
  }
}

void ISR(){
  byte data[3] = {0b11111111, 0b11111111, 0b11111111};
  Wire.beginTransmission(ADDR0);
  Wire.write(0b00000000);
  Wire.endTransmission();
  Wire.requestFrom(ADDR0, 2);
  if(Wire.available()) {
    data[0] =  Wire.read();
  }
  if(Wire.available()) {
    data[1] = Wire.read();
  }
    
  Wire.beginTransmission(ADDR1);
  Wire.write(0b00000000);
  Wire.endTransmission();
  Wire.requestFrom(ADDR1, 1);
  if(Wire.available()) {
    data[2] = Wire.read();
  }
    
  uint8_t x = 0;
  for(int i = 0; i < 3; i++){
    while(data[i] & 0b00000001 != 0){
      data[i] = data[i] >> 1;
      x++;
    }
    if(x != 8*(i+1)){
      break;
    }
  }
  if(x != 24){
    keyQueue.push(x);
  }
}

void functionTabs(){
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.fillRect(0, 228, 320, 12, tft.color565(150, 150, 150));
  tft.fillTriangle(75, 228, 80, 240, 85, 228, ILI9341_BLACK);
  tft.fillTriangle(155, 228, 160, 240, 165, 228, ILI9341_BLACK);
  tft.fillTriangle(235, 228, 240, 240, 245, 228, ILI9341_BLACK);
  for(int x = 0; x < 4; x++){
    tft.setCursor(x*80+40-(int)(strlen(labels[x].c_str())*2.5), 230);
    tft.print(labels[x]);
  }
}

void header(){
  float battery = analogRead(BATTERY);
  uint16_t color;
  tft.setTextSize(1);
  tft.fillRect(0, 0, 320, 12, tft.color565(150, 150, 150));
  tft.drawRect(2, 2, 16, 8, ILI9341_WHITE);
  tft.fillRect(18, 4, 2, 4, ILI9341_WHITE);
  tft.setCursor(28, 2);
  if(battery >= 2900){
    if(battery >= 3351){
      color = tft.color565((int)(255*(3910-battery)/1118), 255, 0);
    } else {
      color = tft.color565(255, (int)(255*(3910-battery)/1118), 0);
    }
    tft.fillRect(3, 3, (int)((battery-2792)/1118*14), 6, color);
    tft.setTextColor(color);
    tft.print((battery-2792)/1118*100);
    tft.print("% ");
  } else {
    tft.fillRect(3, 3, (int)((battery-2792)/1118*14), 6, ILI9341_RED);
    tft.drawRect(2, 2, 16, 8, ILI9341_RED);
    tft.fillRect(18, 4, 2, 4, ILI9341_RED);
    tft.setTextColor(ILI9341_RED);
    tft.print((battery-2792)/1118*100);
    tft.print("% LOW BATTERY! ");
  }
  tft.setTextColor(ILI9341_BLACK);

  if(mode){
    tft.print("MODE: NUMPAD ");
    if(numlk){
      tft.print("NUMLOCK OFF");
    } else {
      tft.print("NUMLOCK ON");
    }
  } else {
    tft.print("MODE: CALCULATOR ");
  }
}

void symbolMenu(){
  tft.fillRect(20, 20, 280, 200, tft.color565(150, 150, 150));
}

unsigned long t = millis();

void loop(){
  if(mode){
    while(1){
      if(!keyQueue.isEmpty()){
        if(keyQueue.peek() == 3){
          digitalWrite(NUMLK, 1);
          delay(10);
          digitalWrite(NUMLK, 0);  
          keyQueue.pop();      
        } else if(keyQueue.peek() == 4){
          numlk = 1-numlk;
          header();
          keyQueue.pop();
        } else {
          BootKeyboard.press(lookupTable[numlk][keyQueue.peek()]);
          delay(10);
          BootKeyboard.release(lookupTable[numlk][keyQueue.pop()]);
          delay(10);
        }
      }
      if(millis() - t >= 500){
        header();
        t = millis();
      }
    }
  } else {
    while(1){
      header();
      functionTabs();
      
    }
  }
}

