#include "WheelsUI.h"
#include "SettingsUI.h"

void ChangeUIPage(String page) {
  if (page == "Home") {
    tft.fillRect(0,0,500,500,BLACK);

    tft.setTextColor(WHITE);
    tft.setTextSize(1);
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
    tft.print("Home");

    tft.setCursor(150, 68);
    tft.setTextSize(1);
    tft.print(VERSION);

    tft.drawFastHLine(0, 80, 320, WHITE);

    tft.setCursor(0, 465);
    tft.setTextSize(1);
    tft.print("Sucsessfully Connected With EOS");

    Wheels_btn.initButton(&tft, 60, 120, 95, 40, WHITE, CYAN, BLACK, "Wheels", 2);
    PanTilt_btn.initButton(&tft, 170, 120, 95, 40, WHITE, CYAN, BLACK, "PanTilt", 2);
    Settings_btn.initButton(&tft, 60, 175, 100, 40, WHITE, CYAN, BLACK, "Settings", 2);

    Wheels_btn.drawButton(true);
    PanTilt_btn.drawButton(true);
    Settings_btn.drawButton(true);
  } else if (page == "Wheels") {
    OpenWheels();
  } else if (page == "PanTiltWheels") {

  } else if (page == "Settings") {
    OpenSettingsPage();
  } else if (page == "Connecting") {
    tft.fillScreen(BLACK);
	  centertitle("Waiting For EOS..");
	  tft.println(VERSION);

    tft.setCursor(40, 55);
    tft.setTextSize(2);
    tft.println("   || |--- |---|  /--- ");
    tft.setCursor(40, 75);
    tft.println("   || |__  || || |__  ");
    tft.setCursor(40, 95);
    tft.println("|| || |    || ||    | ");
    tft.setCursor(40, 115);
    tft.println("|___| |___ |___| ___/ ");

    tft.setTextSize(1);

    Settings_btn.initButton(&tft, 60, 450, 100, 40, WHITE, CYAN, BLACK, "Settings", 2);
    Settings_btn.drawButton(true);
  }

  CurrentPage = page;
}