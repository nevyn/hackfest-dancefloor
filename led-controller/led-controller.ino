#include <MIDI.h>

#include <Button.h>
#include <Adafruit_NeoPixel.h>
#include <OverAnimate.h>



//////////////////////////////////////////////////////
//// Settings
//////////////////////////////////////////////////////
static const int ledsPerPad = 10;
static const int padCount = 6;
static const int ledsPin = 6;


//////////////////////////////////////////////////////
//// Global app state
//////////////////////////////////////////////////////

typedef struct {
  byte on;
  byte firstPin;
} Pad;

Pad pads[6] = {
 {
    true,
    0
 },
 {
    true,
    10,
 },
 {
    true,
    20
 },
 {
    true,
    30
 },
 {
    true,
    40,
 },
 {
    true,
    50
 }
};

Button buttons[6] = {
  Button(8, PULLUP),
  Button(9, PULLUP),
  Button(10, PULLUP),
  Button(11, PULLUP),
  Button(13, PULLUP),
  Button(12, PULLUP),
};

/// ----

AnimationSystem anims;
Adafruit_NeoPixel leds = Adafruit_NeoPixel(ledsPerPad*padCount, ledsPin, NEO_GRB + NEO_KHZ800);

MIDI_CREATE_DEFAULT_INSTANCE();
void controlCallback(byte channel, byte number, byte value);

//////////////////////////////////////////////////////
//// Setup and run
//////////////////////////////////////////////////////

int RXLED = 17;
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // used to show if headlight is on
  //pinMode(RXLED, OUTPUT); // used to show if headlight is ons
  leds.begin();
  leds.show();

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  Serial.begin(115200);
  MIDI.setHandleControlChange(controlCallback);
  MIDI.turnThruOff();

  for(int i = 8; i < 14; i++) {
    pinMode (i, INPUT_PULLUP);
  }

  rainbow(&leds, 10);

  MIDI.sendNoteOn(42, 127, 1);
}

unsigned long lastMillis;

void loop()
{
  
  unsigned long now = millis();
  if(!lastMillis) {
    lastMillis = now;
  }
  unsigned long diff = now - lastMillis;
  lastMillis = now;
  TimeInterval delta = diff/1000.0;
  
  //handleButtons();
  //handleState();
  anims.playElapsedTime(delta);
  
  // Read incoming messages
  MIDI.read();


  for(int i = 0; i < padCount; i++)  {
    Pad *pad = &pads[i];
    for(int j = pad->firstPin; j < pad->firstPin + ledsPerPad; j++) {
      uint32_t onColor = Wheel(&leds, (i*ledsPerPad+j) & 255);
      leds.setPixelColor(j,  pad->on ? onColor : 0);
    }
    leds.show();
  }

  for(int i = 0; i < 6; i++) {
    if(buttons[i].uniquePress()) {
      MIDI.sendNoteOn(i+42, 127, 1);
    } else if(!buttons[i].isPressed() && pads[i].on) {
      MIDI.sendNoteOff(i+42, 127, 1);
    }
    pads[i].on = buttons[i].isPressed();
  }
  

  digitalWrite(LED_BUILTIN, pads[0].on); // status led on board
}

//////////////////////////////////////////////////////
//// Input
//////////////////////////////////////////////////////

void controlCallback(byte channel, byte number, byte value /* 0-127 */)
{
  switch(number) {
    case 22:
      pads[0].on = value;
      break;
    case 23:
      pads[1].on = value;
      break;
    case 24:
      pads[2].on = value;
      break;
  }
}

//////////////////////////////////////////////////////
//// State
//////////////////////////////////////////////////////


//////////////////////////////////////////////////////
//// Output
//////////////////////////////////////////////////////


void rainbow(Adafruit_NeoPixel *strip, uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<64; j++) {
    for(i=0; i<strip->numPixels(); i++) {
      strip->setPixelColor(i, Wheel(strip, (i+j) & 255));
    }
    strip->show();
    delay(wait);
  }

  // reset afterwards
  for(i=0; i<strip->numPixels(); i++) {
    strip->setPixelColor(i, 0);
  }
  strip->show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(Adafruit_NeoPixel *strip, byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
