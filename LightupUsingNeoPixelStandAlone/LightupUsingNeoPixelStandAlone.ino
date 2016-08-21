#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LEDCNT 4
#define PIN 0
#define WAIT 50

#define BASE_COLOR BuildMyColor(0, 127, 0)
#define FLASH_COLOR BuildMyColor(192, 255, 224)
#define BLANK_COLOR BuildMyColor(0, 0, 0)

struct MyColor {
  byte r;
  byte g;
  byte b;
};

struct ForeColor {
  MyColor c;
  bool enable;
};

// XXX 構造体のコンストラクタは？　クラスにした方が良い？
MyColor BuildMyColor(byte _r, byte _g, byte _b) {
  MyColor c;
  c.r = _r;
  c.g = _g;
  c.b = _b;
  return c;
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDCNT, PIN, NEO_GRB + NEO_KHZ800);

// プログラム中で操作しやすいようにバッファはライブラリの内外で二重持ちする
MyColor baseColors[LEDCNT];
ForeColor foreColors[LEDCNT];
int flashIndex;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  // For loging
  Serial.begin(9600);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  initFlame();
  
  Serial.println("End setup.");
}

void loop() {
//  Serial.print("Start loop.");
//  Serial.println(++loopCount, DEC);

  nextFlame();
  updateFlame();
  strip.show();
  delay(WAIT);

//  Serial.println("End loop.");
}

void initFlame() {
  for (int i = 0; i < LEDCNT; i++) {
      foreColors[i].c = FLASH_COLOR;
      foreColors[i].enable = false;
      baseColors[i] = BASE_COLOR;
    }
  flashIndex = 0;
}

void nextFlame() {
  int lastFlashIndex = flashIndex;
  flashIndex++;
  if (flashIndex >= LEDCNT) {
    flashIndex = 0;
  }
  foreColors[lastFlashIndex].enable = false;
  foreColors[flashIndex].enable = true;
}

void updateFlame() {
  for (int i = 0; i < LEDCNT; i++) {
    if (foreColors[i].enable) {
      MyColor c = foreColors[i].c;
      strip.setPixelColor(i, c.r, c.g, c.b);
    } else {
      MyColor c = baseColors[i];
      strip.setPixelColor(i, c.r, c.g, c.b);
    }
  }
}

