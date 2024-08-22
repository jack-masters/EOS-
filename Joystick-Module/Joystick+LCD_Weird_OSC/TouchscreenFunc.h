#include "UIColors.h"

bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);      //because TFT control pins
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

void centerprint(const char *s, int y) {
  int len = strlen(s) * 6;
  tft.setTextColor(WHITE, RED);
  tft.setCursor((tft.width() - len) / 2, y);
  tft.print(s);
}

void centertitle(const char *s) {
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, 100, 14, RED);
  centerprint(s, 1);
  tft.fillRect(0, 14, 100, 1, WHITE);
  tft.setCursor(0, 30);
  tft.setTextColor(WHITE, BLACK);
}

void SetupTouchscreen() {
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) { ID = 0x9486; }
  tft.begin(ID);
  tft.setRotation(0);
}