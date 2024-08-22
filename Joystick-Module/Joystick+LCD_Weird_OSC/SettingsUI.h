void OpenSettingsPage() {
  tft.fillScreen(BLACK);

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
  tft.print("Settings");

  tft.drawFastHLine(0, 80, 320, WHITE);

  tft.setCursor(0, 90);
  tft.print("Box Name / Type: " + VERSION);

  tft.setCursor(0, 110);
  tft.print("Eos Version: V1");

  tft.setCursor(0, 130);
  tft.print("External Modules: NONE");

  tft.drawFastHLine(0, 150, 320, WHITE);

  PcMode_btn.initButton(&tft, 60, 190, 100, 40, WHITE, CYAN, BLACK, "PC Mode", 2);
  EosMode_btn.initButton(&tft, 60, 250, 100, 40, WHITE, CYAN, BLACK, "EOS Mode", 2);

  PcMode_btn.drawButton(true);
  EosMode_btn.drawButton(true);

  SettingsBack_btn.initButton(&tft, 60, 450, 100, 40, WHITE, CYAN, BLACK, "Back", 2);
  SettingsBack_btn.drawButton(true);
}
