#include "bitmaps.h"

void OpenWheels() {
  tft.fillScreen(BLACK);

  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 15);
  tft.println("   || |--- |---|  /---  OSC BASED NOMAD");
  tft.setCursor(10, 25);
  tft.println("   || |__  || || |__    INTERFACE");
  tft.setCursor(10, 35);
  tft.println("|| || |    || ||    |   C2024 - Jack Masters");
  tft.setCursor(10, 45);
  tft.println("|___| |___ |___| ___/ ");

  tft.drawFastHLine(0, 60, 320, WHITE);

  tft.setCursor(0, 68);
  tft.setTextSize(1);
  tft.print("Wheels");

  tft.drawFastHLine(0, 80, 320, WHITE);
  
  tft.fillRect(10, 123, 300, 105, BLACK);
  Intens_btn.initButton(&tft, 60, 150, 85, 40, WHITE, CYAN, BLACK, "Intens", 2);
  Focus_btn.initButton(&tft, 160, 150, 85, 40, WHITE, CYAN, BLACK, "Focus", 2);
  Color_btn.initButton(&tft, 260, 150, 85, 40, WHITE, CYAN, BLACK, "Color", 2);
  Shutter_btn.initButton(&tft, 60, 205, 85, 40, WHITE, CYAN, BLACK, "Shutter", 2);
  Image_btn.initButton(&tft, 160, 205, 85, 40, WHITE, CYAN, BLACK, "Image", 2);
  Form_btn.initButton(&tft, 260, 205, 85, 40, WHITE, CYAN, BLACK, "Form", 2);


  PageUp_btn.initButton(&tft, 90, 280, 115, 40, WHITE, CYAN, BLACK, "Page Up", 2);
  PageDown_btn.initButton(&tft, 230, 280, 115, 40, WHITE, CYAN, BLACK, "Page Down", 2);
  WheelBack_btn.initButton(&tft, 290, 102, 85, 25, WHITE, CYAN, BLACK, "Back", 2);

  Intens_btn.drawButton(true); 
  Focus_btn.drawButton(true);
  Color_btn.drawButton(true);
  Shutter_btn.drawButton(true);
  Image_btn.drawButton(true);
  Form_btn.drawButton(true);

  PageUp_btn.drawButton(true);
  PageDown_btn.drawButton(true);
  WheelBack_btn.drawButton(true);

  tft.writeFastHLine(0, 335, 320, WHITE);

  Home1_btn.initButton(&tft, 57, 363, 30, 30, BLACK, BLACK, BLACK, "", 0);
  Home1_btn.drawButton(true);
  tft.drawBitmap(43, 350, house_bitmap, 27, 27, WHITE);

  Home2_btn.initButton(&tft, 157, 363, 30, 30, BLACK, BLACK, BLACK, "", 0);
  Home2_btn.drawButton(true);
  tft.drawBitmap(143, 350, house_bitmap, 27, 27, WHITE);

  Home3_btn.initButton(&tft, 257, 363, 30, 30, BLACK, BLACK, BLACK, "", 0);
  Home3_btn.drawButton(true);
  tft.drawBitmap(243, 350, house_bitmap, 27, 27, WHITE);

  tft.setCursor(35, 445);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print("Left - Right");

  tft.setCursor(140, 445);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print("Up - Down");

  tft.setCursor(235, 445);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print("TurnL - TurnR");
}

void ChangeWheelsValue(String Wheel1, String Wheel2, String Wheel3) {
  tft.fillRect(0, 390, 400, 10, BLACK);

  tft.setCursor(35, 390);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(Wheel1);

  tft.setCursor(140, 390);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(Wheel2);

  tft.setCursor(235, 390);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(Wheel3);
}

void ChangeWheelNames(String Name1, String Name2, String Name3) {
  tft.fillRect(0, 420, 400, 10, BLACK);

  tft.setCursor(35, 420);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print(Name1);

  tft.setCursor(140, 420);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print(Name2);

  tft.setCursor(235, 420);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print(Name3);
}

void ShowErrorCodeWheels(String Text1, String Text2) {
  tft.fillRect(0, 390, 400, 40, BLACK);

  tft.setCursor(10, 390);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(Text1);

  tft.setCursor(10, 415);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(Text2);
}