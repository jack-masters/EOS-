// LIBRARIES
#include <Adafruit_GFX.h>
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#include <string.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

MCUFRIEND_kbv tft;

#ifdef BOARD_HAS_USB_SERIAL
    #include <SLIPEncodedUSBSerial.h>
    SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
    #include <SLIPEncodedSerial.h>
    SLIPEncodedSerial SLIPSerial(Serial);
#endif

/////////////////////////////////////////////////
// TOUCHSCREEN CONFIGURATION
const int XP = 8, XM = A2, YP = A3, YM = 9; // 320x480 ID=0x9486
const int TS_LEFT = 121, TS_RT = 899, TS_TOP = 950, TS_BOT = 95;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int pixel_x, pixel_y; // Global vars updated by Touch_getXY()

// Button declarations
Adafruit_GFX_Button Intens_btn, Focus_btn, Color_btn, Shutter_btn, Image_btn, Form_btn;
Adafruit_GFX_Button PageUp_btn, PageDown_btn, WheelBack_btn, Home1_btn, Home2_btn, Home3_btn;
Adafruit_GFX_Button Settings_btn, PanTilt_btn, Wheels_btn;
Adafruit_GFX_Button PcMode_btn, EosMode_btn, SettingsBack_btn;

/////////////////////////////////////////////////
// VARIABLES
String CurrentPage = "Connecting";
bool connectedToEos = false;

const String VERSION = "Jacks EOS Box V1.0.0.9";
const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

const String EOS_CHAN_OUT = "/eos/out/active/chan";
const String EOS_WHEEL_OUT = "/eos/out/active/wheel/*";

const String EOS_INTENSITY = "Intensity";
const String EOS_FOCUS = "Focus";
const String EOS_COLOR = "Color";
const String EOS_IMAGE = "Image";
const String EOS_FORM = "Form";
const String EOS_SHUTTER = "Shutter";

char ErrorMessageLine1[7][41] = {
    "This fixture has literally no parameters",
    "This fixture has no intensity parameters",
    "This fixture has no focus parameters",
    "This fixture has no color parameters",
    "This fixture has no image parameters",
    "This fixture has no form parameters",
    "This fixture has no shutter parameters"
};

char ErrorMessageLine2[7][40] = {
    "       ...maybe patch something?",
    "maybe it's a hazer?",
    "you should probably go get a ladder.",
    "maybe add a cut of L202?",
    "I hear templates are out of style.",
    "you probably need a different light.",
    "have you tried a square template?"
};

const String CategoryKeyCommand[6] = {
    EOS_INTENSITY, EOS_FOCUS, EOS_COLOR,
    EOS_IMAGE, EOS_FORM, EOS_SHUTTER
};

bool updateWheelsName = false;

/////////////////////////////////////////////////
// MODULES
#include "TouchscreenFunc.h"
#include "UIController.h"
#include "UIColors.h"
#include "Parameters.h"
#include "OSC.h"

/////////////////////////////////////////////////

void setup() {
    SLIPSerial.begin(115200);

    #ifdef BOARD_HAS_USB_SERIAL
        while (!SerialUSB);
    #else
        while (!Serial);
    #endif

    SetupTouchscreen();
    ChangeUIPage("Connecting");
}

bool UpdatedDisp = false;

void loop() {
    if (connectedToEos && !UpdatedDisp) {
        ChangeUIPage("Home");
        UpdatedDisp = true;
    }

    ////////////////////////////Button Triggers////////////////////////////////////
    bool down = Touch_getXY();

    Wheels_btn.press(down && Wheels_btn.contains(pixel_x, pixel_y));
    PanTilt_btn.press(down && PanTilt_btn.contains(pixel_x, pixel_y));
    Settings_btn.press(down && Settings_btn.contains(pixel_x, pixel_y));

    SettingsBack_btn.press(down && SettingsBack_btn.contains(pixel_x, pixel_y));
    WheelBack_btn.press(down && WheelBack_btn.contains(pixel_x, pixel_y));

    if (Wheels_btn.justPressed()) {
        ChangeUIPage("Wheels");
        CurrentPage = "Wheels";
    }
    if (PanTilt_btn.justPressed()) {
        ChangeUIPage("PanTiltWheels");
        CurrentPage = "PanTiltWheels";
    }
    if (Settings_btn.justPressed()) {
        ChangeUIPage("Settings");
        CurrentPage = "Settings";
    }

    if (SettingsBack_btn.justPressed() || WheelBack_btn.justPressed()) {
        if (connectedToEos) {
            ChangeUIPage("Home");
            CurrentPage = "Home";
        } else {
            ChangeUIPage("Connecting");
            CurrentPage = "Connecting";
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////

    static String curMsg;
    int size = SLIPSerial.available();  //Check for incoming OSC
    if (size > 0)
      while (size--)
        curMsg += (char)(SLIPSerial.read());
    if (SLIPSerial.endofPacket()) {
      parseOSCMessage(curMsg);
      curMsg = String("");
    }
}
