/* This example shows how to display a moving rainbow pattern on
 * an APA102-based LED strip. */

/* By default, the APA102 library uses pinMode and digitalWrite
 * to write to the LEDs, which works on all Arduino-compatible
 * boards but might be slow.  If you have a board supported by
 * the FastGPIO library and want faster LED updates, then install
 * the FastGPIO library and uncomment the next two lines: */
// #include <FastGPIO.h>
// #define APA102_USE_FAST_GPIO

#include <APA102.h>

//#define XCOORD (shapes[x][1] >> 4)
//#define YCOORD (shapes[x][1] && 0b00001111)

//uint8_t points[32] = {0, 6, 10, 12, 18, 22, 16, 14, 10, 4, 2, 6, 8, 14, 18, 20, 3, 5, 9, 15, 17, 21, 19, 13, 9, 7, 1, 5, 11, 13, 17, 23};

// Define which pins to use.
const uint8_t dataPin = 3;
const uint8_t clockPin = 4;
volatile int mode = 0;
volatile int x = 0;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

// Create a buffer for holding the colors (3 bytes per color).
volatile rgb_color colors[24];

// Set the brightness to use (the maximum is 31).
const uint8_t brightness = 1;

void setup()
{
  GIMSK = 0b00100000;    // turns on pin change interrupts
  PCMSK = 0b00000001;    // turn on interrupts on pins PB0
  sei();                 // enables interrupts
  pinMode(1, INPUT);
  randomSeed(analogRead(1));
  mode = 0;
  //for(uint8_t i = 0; i < 24; i++){
  //  colors[i] = rgb_color(0, 0, 0);
  //}
  //ledStrip.write(colors, 24, brightness);
  //delay(1000);
}

/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. */
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    //h=h%360;
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}

ISR(PCINT0_vect){
  mode++;
  if(mode == 4){
    mode = 0;
  }
  x = 0;
  for(uint8_t i = 0; i < 24; i++){
    colors[i] = rgb_color(0, 0, 0);
  }
}

int offset = 0;
int y = 0;
void loop(){
  switch(mode){
    case 0:
      for(int i = 0; i < 24; i++){
        if(y){
          if(i <= x){
            colors[23-i] = hsvToRgb(offset+4*(i+1), 255, 255);
          } else {
            colors[23-i] = hsvToRgb(offset+4*(25-i)+300, 255, 255);
          }
        } else {
          if(i <= x){
            colors[i] = hsvToRgb(offset+4*(i+1), 255, 255);
          } else {
            colors[i] = hsvToRgb(offset+4*(25-i)+300, 255, 255);
          }
        }
      }
      ledStrip.write(colors, 24, brightness);
      x++;
      delay(50);
      if(x == 24){
        x = 0;
        y = 1-y;
        offset = offset + 60;
        offset = offset % 360;
      }
      break;
      
    case 1:
      for(uint8_t i = 0; i < 24; i++){
        colors[i] = hsvToRgb((x + i * 8)%360, 255, 255);
      }
      x = x%360;
      x = x+8;
      ledStrip.write(colors, 24, brightness);
      delay(10);
      break;

    case 2:
      for(uint8_t i = 0; i < 24; i++){
        colors[i] = rgb_color(0, 0, 0);
      }
      colors[(x+23)%24] = hsvToRgb(15*(x-1)+offset, 50, 50);
      colors[x] = hsvToRgb(15*x+offset, 255, 255);
      colors[(x+1)%24] = hsvToRgb(15*(x-1)+offset, 50, 50);
      
      colors[(x+11)%24] = hsvToRgb(15*(x-1)+offset+180, 50, 50);
      colors[(x+12)%24] = hsvToRgb(15*x+offset+180, 255, 255);
      colors[(x+13)%24] = hsvToRgb(15*(x-1)+offset+180, 50, 50);

      ledStrip.write(colors, 24, brightness);
      delay(50);
      x++;
      if(x==24){
        x = 0;
        offset = random(0, 360);
      }
      break;
  }
}
