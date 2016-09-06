#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define LEDCNT 4
#define PIN 0
#define WAIT 50

class MyColor {
  public:
    byte r;
    byte g;
    byte b;
    byte a;
    MyColor(byte _r = 0, byte _g = 0, byte _b = 0, byte _a = 255) {
      r = _r;
      g = _g;
      b = _b;
      a = _a;
    }
};

class MyFlame {
  public:
    MyFlame() : MyFlame(0) {}
    MyFlame(int num) {
      colors = (MyColor* ) malloc(sizeof(MyColor) * num);
      numLeds = num;
    }
    ~MyFlame() {
      free(colors);
    }
    MyColor* color(int index) {
      return &colors[index];
    }
    int numPixels() {
      return numLeds;
    }

  private:
    MyColor* colors;
    int numLeds;
};

class MyFullColorLedStrip {
  public:
    MyFullColorLedStrip() {}
    MyFullColorLedStrip(Adafruit_NeoPixel* _pStrip);
    virtual ~MyFullColorLedStrip();
    virtual void reset();
    virtual void updateFrame();
    virtual void delayFrame();
    virtual void updateOneCycle();
    int numPixels();
    void updateStrip();
  protected:
    Adafruit_NeoPixel* pStrip;
    MyColor* backColors;
    MyColor* foreColors;
    int brightness;
    int frameWait;  // delay(frameWait);
    int currentFrame;
};

enum AuraMode {
  StartToEnd = 0,
  CenterToEdge = 1,
  EdgeToCenter = 2
};

class AuraStrip : public MyFullColorLedStrip {
  public:
    AuraStrip(Adafruit_NeoPixel* _pStrip) : MyFullColorLedStrip(_pStrip){
    }
    void set(AuraMode _auraMode, MyColor _baseColor, MyColor _flashColor) {
      auraMode = _auraMode;
      baseColor = _baseColor;
      flashColor = _flashColor;
    }
    //    virtual void reset();
    virtual void updateFrame() {
      switch (auraMode) {
        case StartToEnd:
          updateFrameStartToEnd();
          break;
      }
    }
    //    virtual void delayFrame();
    //    virtual void updateOneCycle();
  private:
    AuraMode auraMode;
    MyColor baseColor;
    MyColor flashColor;
    void updateFrameStartToEnd() {
      if (numPixels() <= currentFrame) {
        reset();
      }

      // TODO ベース色に揺らぎを加えたい
      for (int i = 0; i < numPixels(); i++) {
        backColors[i] = baseColor;
        foreColors[i] = flashColor;
        foreColors[i].a = 0;
      }
      foreColors[currentFrame].a = 255;

      currentFrame++;
      if (numPixels() <= currentFrame) {
        reset();
      }
    }
};

MyFullColorLedStrip::MyFullColorLedStrip(Adafruit_NeoPixel* _pStrip)
  : pStrip(_pStrip), brightness(0), frameWait(50), currentFrame(0)
{
  backColors = (MyColor*)malloc(sizeof(MyColor) * numPixels());
  foreColors = (MyColor*)malloc(sizeof(MyColor) * numPixels());
}

MyFullColorLedStrip::~MyFullColorLedStrip() {
  free(backColors);
  free(foreColors);
}

void MyFullColorLedStrip::reset() {
  currentFrame = 0;
}

void MyFullColorLedStrip::updateFrame() {
}

void  MyFullColorLedStrip::delayFrame() {
  delay(frameWait);
}

void MyFullColorLedStrip::updateOneCycle() {
  do {
    updateFrame();
    updateStrip();
    delayFrame();
  } while (currentFrame > 0);
}

int MyFullColorLedStrip::numPixels() {
  return pStrip->numPixels();
}

void MyFullColorLedStrip::updateStrip() {
  if (brightness) {
    pStrip->setBrightness(brightness);
  }
  for (int i = 0; i < numPixels(); i++) {
    MyColor c;
    MyColor* pc = &c;
//    Serial.print("fore#a =  ");
//    Serial.print(foreColors[i].a, HEX);
//    Serial.print(": ");
    if (foreColors[i].a == 255) {
      pc = &foreColors[i];
    } else if (foreColors[i].a == 0) {
      pc = &backColors[i];
    } else {
      MyColor* pc1 = &backColors[i];
      MyColor* pc2 = &foreColors[i];
      pc->r =  ((pc1->r * (255 - pc2->a)) >> 8) + ((pc2->r * (int)pc2->a) >> 8);
      pc->g =  ((pc1->g * (255 - pc2->a)) >> 8) + ((pc2->g * (int)pc2->a) >> 8);
      pc->b =  ((pc1->b * (255 - pc2->a)) >> 8) + ((pc2->b * (int)pc2->a) >> 8);
    }
//    Serial.print("(");
//    Serial.print(pc->r, HEX);
//    Serial.print(", ");
//    Serial.print(pc->g, HEX);
//    Serial.print(", ");
//    Serial.print(pc->b, HEX);
//    Serial.print(", ");
//    Serial.print(pc->a, HEX);
//    Serial.println(")");
    pStrip->setPixelColor(i, pc->r, pc->g, pc->b);
  }
  pStrip->show();
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDCNT, PIN, NEO_GRB + NEO_KHZ800);
MyFullColorLedStrip* pMyStrip;

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
  pMyStrip = new MyFullColorLedStrip(&strip);

  Serial.println("End setup.");
}

void loop() {
  Serial.println("Start loop.");
  //  Serial.println(++loopCount, DEC);

  selectStrip();

  pMyStrip->updateOneCycle();
  //  pMyStrip->updateFrame() ;
  //  pMyStrip->updateStrip() ;
  //  pMyStrip->delayFrame() ;

  //  Serial.println("End loop.");
}

void selectStrip() {
  // TODO 入力の割り込みの有無を見る
  // 入力があった場合
  // TODO strip を入力に応じて更新する
  delete  pMyStrip;
  AuraStrip* pAuraStrip = new AuraStrip(&strip);
  pAuraStrip->set(StartToEnd, MyColor(0, 127, 64), MyColor(192, 248, 224));
  pMyStrip = pAuraStrip;
}

