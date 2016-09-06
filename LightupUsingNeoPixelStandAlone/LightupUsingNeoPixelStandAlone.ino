#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

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
    MyColor(const char* colorCode) {
      Serial.print(colorCode);
      Serial.print("->");
      if (strlen(colorCode) < 7 || colorCode[0] != '#') {
        // XXX error
        return;
      }
      r = hexToInt(&colorCode[1]);
      g = hexToInt(&colorCode[3]);
      b = hexToInt(&colorCode[5]);
      if (strlen(colorCode) >= 9) {
        // XXX alpha
        a = hexToInt(&colorCode[7]);
      } else {
        a = 255;
      }
      Serial.print("(");
      Serial.print(r, HEX);
      Serial.print(", ");
      Serial.print(g, HEX);
      Serial.print(", ");
      Serial.print(b, HEX);
      Serial.print(", ");
      Serial.print(a, HEX);
      Serial.println(")");
    }
    static byte hexToInt(const char* hex) {
      return (hexToInt(hex[0]) << 4) + hexToInt(hex[1]);
    }
    static byte hexToInt(const char hex) {
      return hex >= 'a' ? hex - 'a' + 10 : hex - '0';
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
    //
    void setFrameWait(int _frameWait) {
      frameWait = _frameWait;
    }
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
    AuraStrip(Adafruit_NeoPixel* _pStrip) : MyFullColorLedStrip(_pStrip) {
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
MyFullColorLedStrip* pMyStrip = NULL;
MyFullColorLedStrip* pMyNextStrip = NULL;

const char *ssid = "MyStrip";
const char *password = "";
ESP8266WebServer server(80);

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  // For loging
  Serial.begin(9600);

  initializeServer();

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pMyStrip = new MyFullColorLedStrip(&strip);

  Serial.println("End setup.");
}

void loop() {
  //  Serial.println("Start loop.");
  //  Serial.println(++loopCount, DEC);
  server.handleClient();
  selectStrip();

  pMyStrip->updateOneCycle();
  //  pMyStrip->updateFrame() ;
  //  pMyStrip->updateStrip() ;
  //  pMyStrip->delayFrame() ;

  //  Serial.println("End loop.");
}

void selectStrip() {
  // 入力があった場合
  if (pMyNextStrip) {
    // strip を入力に応じて更新する
    delete  pMyStrip;
    pMyStrip = pMyNextStrip;
    pMyNextStrip = NULL;
  }
}

void initializeServer() {
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", handleRoot);
  server.begin();
}

const char* form = "<html>"
                   "<head>"
                   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                   "<style>input { display: block; margin: 4px; }</style>"
                   "</head>"
                   "<body>"
                   "<form method=\"post\">"
                   "<input type=\"color\" name=\"basecolor\" value=\"#008040\" >"
                   "<input type=\"color\" name=\"flashcoolor\" value=\"#c0f0e0\" >"
                   "<input type=\"number\" name=\"delay\" value=\"50\" >"
                   "<input type=\"submit\" value=\"Send\" >"
                   "</form>"
                   "</body>"
                   "</html>"
                   ;

void handleRoot() {
  Serial.println("handleRoot");
  if (server.method() != HTTP_GET) {
    String baseColor = "#008040";
    String flashColor = "#c0f0e0";
    int frameWait = 50;

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      //      Serial.print(server.argName(i));
      //      Serial.print("=");
      //      Serial.print(server.arg(i));
      if (server.argName(i)[0] == 'b') {
        baseColor = server.arg(i);
        // baseColor.toLower();
      }
      if (server.argName(i)[0] == 'f') {
        flashColor = server.arg(i);
      }
      if (server.argName(i)[0] == 'd') {
        frameWait = server.arg(i).toInt();
      }
    }
    AuraStrip* pAuraStrip = new AuraStrip(&strip);
    pAuraStrip->set(StartToEnd, MyColor(baseColor.c_str()), MyColor(flashColor.c_str()));
    pAuraStrip->setFrameWait(frameWait);
    pMyNextStrip = pAuraStrip;
  }
  server.send(200, "text/html", form);
}

