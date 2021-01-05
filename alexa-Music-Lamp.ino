#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Espalexa.h>
#include "reactive_common.h"
#include <FastLED.h>
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#define DATAPIN  4 // GPIO pin used to drive the LED strip
#define ANALOG_PIN A0
#define ANALOG_THRESHOLD 512

// change wifi ssid and password
#define WIFI_SSID "MY WIFI"
#define WIFI_PASSWORD "MY WIFI PASSWORD"

#define TIMER_MS 3000
#define MIC_LOW 0
#define MIC_HIGH 644

#define SAMPLE_SIZE 20
#define LONG_TERM_SAMPLES 250
#define BUFFER_DEVIATION 400
#define BUFFER_SIZE 3

// change LED CONFIG here
#define NUMLEDS 75 // number of LEDs on the strip
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
/*
    define your ws2812fx presets
*/
struct averageCounter *samples;
struct averageCounter *longTermSamples;
struct averageCounter* sanityBuffer;

unsigned long last_trigger = 0;
unsigned long now = 0;
char Music = 'F';// F= music mode off O= music mode on


float globalHue;
float globalBrightness = 255;
int hueOffset = 120;
float fadeScale = 2.3;
float hueIncrement = 0.7;

struct CRGB leds[NUMLEDS];

Espalexa espalexa;


void setup() {
  Serial.begin(115200);
  Serial.println("\r\n");

  globalHue = 0;
  samples = new averageCounter(SAMPLE_SIZE);
  longTermSamples = new averageCounter(LONG_TERM_SAMPLES);
  sanityBuffer    = new averageCounter(BUFFER_SIZE);

  while (sanityBuffer->setSample(250) == true) {}
  while (longTermSamples->setSample(200) == true) {}
  delay(1000);

  // init WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  Serial.print("Connecting to " WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nServer IP is ");
  Serial.println(WiFi.localIP());

  // add your alexa virtual devices giving them a name and associated callback
  espalexa.addDevice("lamp", lampCallback);
  espalexa.addDevice("Disco", musicCallback);

  espalexa.begin();
  
  FastLED.addLeds<LED_TYPE, DATAPIN, COLOR_ORDER>(leds, NUMLEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness (0);
  fill_solid(leds, NUMLEDS, CRGB::Black); 
  delay(5);
  FastLED.show();
}

void loop() {
  espalexa.loop();
  delay(1);

  if ( Music == 'O') {

    int sanityValue = sanityBuffer->computeAverage();
    int analogRaw = analogRead(ANALOG_PIN);
    Serial.println(analogRaw);
    Serial.print(" ");
    if (!(abs(analogRaw - sanityValue) > BUFFER_DEVIATION)) {
      sanityBuffer->setSample(analogRaw);
    }

    analogRaw = fscale(MIC_LOW, MIC_HIGH, MIC_LOW, MIC_HIGH, analogRaw, 0.5);
    //    Serial.println(analogRaw);
    //    Serial.print(" ");
    if (samples->setSample(analogRaw))
    {
      return;
    }

    uint16_t longTermAverage = longTermSamples->computeAverage();
    uint16_t useVal = samples->computeAverage();
    longTermSamples->setSample(useVal);

    int diff = (useVal - longTermAverage);
    if (diff > 5)
    {
      if (globalHue < 235)
      {
        globalHue += hueIncrement;
      }
    }
    else if (diff < -5)
    {
      if (globalHue > 2)
      {
        globalHue -= hueIncrement;
      }
    }


    int curshow = fscale(MIC_LOW, MIC_HIGH, 0.0, (float)NUMLEDS, (float)useVal, 0);
    //int curshow = map(useVal, MIC_LOW, MIC_HIGH, 0, NUM_LEDS)
    // Serial.println(curshow);
    //    Serial.print(" curshow");
    if (curshow > 2) {
      fadeToBlackBy( leds, NUMLEDS, 10);
      addGlitter(NUMLEDS - curshow);
      FastLED.show();
    }
    for (int i = 0; i < NUMLEDS; i++)
    {

      if (i < curshow)
      {
        leds[i] = CHSV(globalHue + hueOffset + (i * 2), 255, 255);
      }
      else
      {
        leds[i] = CRGB(leds[i].r / fadeScale, leds[i].g / fadeScale, leds[i].b / fadeScale);
      }

    }
    delay(5);
    FastLED.show();

  }
}
void addGlitter( fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {

    leds[ random16(NUMLEDS) ] += CHSV( globalHue + hueOffset, 255, 255);
  }
}
/*
   our callback functions
*/

void lampCallback(uint8_t brightness, uint32_t rgb) {
  Music = 'F';
  Serial.print("Loading lamp preset "); Serial.println(brightness);
  Serial.println(rgb);
  fill_solid(leds, NUMLEDS, rgb);
  FastLED.setBrightness (brightness);
  FastLED.show();
  //  loadPreset(lamp_preset, brightness, rgb);
}
void musicCallback(uint8_t brightness) {
  //  ws2812fx.setMode(FX_MODE_RANDOM_COLOR);
  Serial.print("Loading Music preset ");
  Serial.println(brightness);
  Music = 'F';
  if (brightness > 0) {
    Music = 'O';
    FastLED.setBrightness(globalBrightness);
    //    ws2812fx.start();
  }
}




float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve)
{

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;

  // condition curve parameter
  // limit range

  if (curve > 10)
    curve = 10;
  if (curve < -10)
    curve = -10;

  curve = (curve * -.1);  // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  // Check for out of range inputValues
  if (inputValue < originalMin)
  {
    inputValue = originalMin;
  }
  if (inputValue > originalMax)
  {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin)
  {
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal = zeroRefCurVal / OriginalRange; // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax)
  {
    return 0;
  }

  if (invFlag == 0)
  {
    rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin;
  }
  else // invert the ranges
  {
    rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
