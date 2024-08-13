///////////////////////////////////////////////////////////////////////////////////////DO NOT TOUCH//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Adafruit_GFX.h>
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif
#include <string.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

struct Joystick {
  uint8_t parameterIdx;
  uint8_t pin;
  uint8_t pinPrevious;
  String type;
} Joy1, Joy2, Joy3;

struct Parameter {
	String name;
  String display;
  String Selection;
  int Page;
  float value;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////EDIT HERE!////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t PARAMETER_MAX = 50; // number of parameters must even
struct Parameter parameter[PARAMETER_MAX] = {
	{"none", " ---", " ---", 1},

  {"Pan", "Pan", "INTENS", 1}, // LEFT RIGHT
  {"Tilt", "Tilt", "INTENS", 1}, // UP DOWN
	{"Intens", "Intens", "INTENS", 1}, // TURN

///////////////////////////////////////////////////////////////////

	{"Zoom", "Zoom", "FOCUS1", 1}, // LEFT RIGHT
	{"Edge", "Edge", "FOCUS1", 1}, // UP DOWN
	{"Iris", "Iris", "FOCUS1", 1}, // TURN

	{"Diffusion", "Diffus", "FOCUS2", 2},
  {"Diffusion 2", "Diffus 2", "FOCUS2", 2},

//////////////////////////////////////////////////////////////////

	{"Red", "Red", "COLOR1", 1},
	{"Green", "Green", "COLOR1", 1},
	{"Blue", "Blue", "COLOR1", 1},

	{"White", "White", "COLOR2", 2},
	{"Amber", "Amber", "COLOR2", 2},
	{"UV", "UV", "COLOR2", 2},

  {"Cyan", "Cyan", "COLOR3", 3},
	{"Magenta", "Magenta", "COLOR3", 3},
	{"Yellow", "Yellow", "COLOR3", 3},

	{"cto", "CTO", "COLOR4", 4},
  {"Hue", "Hue", "COLOR4", 4},
	{"Saturation", "Saturatn", "COLOR4", 4},

  {"Color Select", "Select1", "COLORWL", 5},
	{"Color Select_2", "Select2", "COLORWL", 5},

//////////////////////////////////////////////////////////////////

  {"Gobo Ind\\Spd", "GoboSPD", "GOBO", 1},
  {"Gobo Select", "GoboSEL", "GOBO", 1},
	{"Gobo Wheel Mode", "GoboMODE", "GOBO", 1},

//////////////////////////////////////////////////////////////////

  {"Beam Fx Ind\\Spd", "BeamSPD", "BEAMFX1", 1},
  {"Beam Fx Select", "BeamSEL", "BEAMFX1", 1},
	{"Beam Fx Ind\\Spd 2", "BeamSP2", "BEAMFX1", 1},

  {"Beam Fx Select 2", "BeamSEL2", "BEAMFX2", 2},

////////////////////////////////////////////////////////////////////

	{"frame_assembly", "Assembly", "FRAME1", 1},
	{"thrust_A", "ThrustA", "FRAME1", 1},
	{"angle_A", "AngleA", "FRAME1", 1},

	{"thrust_B", "ThrustB", "FRAME2", 2},
	{"angle_B", "AngleB", "FRAME2", 2},

	{"thrust_C", "ThrustC", "FRAME3", 3},
	{"angle_C", "AngleC", "FRAME3", 3},

	{"thrust_D", "ThrustD", "FRAME4", 4},
	{"angle_D", "AngleD", "FRAME4", 4},

  ///////////////////////////////////////////////////////////////////

  {"Shutter Strobe", "Strobe", "OTHER", 1},
  {"Haze", "Haze", "OTHER", 1},
	{"Blower", "Blower", "OTHER", 1},
};


Adafruit_GFX_Button Mode_btn, Intens_btn, Focus1_btn, Focus2_btn, Color1_btn, Color2_btn, Color3_btn, Color4_btn, ColorWL_btn, Gobo_btn, BeamFx1_btn, BeamFx2_btn, Frame1_btn, Frame2_btn, Frame3_btn, Frame4_btn, Other_btn;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP=8,XM=A2,YP=A3,YM=9; //320x480 ID=0x9486
const int TS_LEFT=121,TS_RT=899,TS_TOP=950,TS_BOT=95;


// JOYSTICK
const byte Analog_LR_pin = A0;
const byte Analog_UD_pin = A1;
const byte Analog_T_pin = A2;
int Analog_LR = 0;
int Analog_UD = 0;
int Analog_T = 0;

float timeNow;

enum ConsoleType {
  ConsoleNone,
  ConsoleEos,
  ConsoleCobalt,
  ConsoleColorSource
};
ConsoleType connectedToConsole = ConsoleNone;

//osc
#define OSC_BUF_MAX_SIZE    512
#define PING_AFTER_IDLE_INTERVAL    2500
#define TIMEOUT_AFTER_IDLE_INTERVAL 5000

#define SUBSCRIBE				1
#define UNSUBSCRIBE			0

bool updateDisplay = false;
bool connectedToEos = false;
uint32_t lastMessageRxTime = 0;
bool timeoutPingSent = false;
int8_t idx = 0; // start with parameter index 2 must even

const String NO_PARAMETER = "none";
const String VERSION = "Jacks EOS Box   V1.0.0.9";
const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";
const String PING_QUERY = "box2b_hello";
const String SUBSCRIBE_QUERY = "/eos/subscribe/param/";
const String UNSUBSCRIBE_QUERY = "/eos/subscribe/param/*";
const String PARAMETER_QUERY = "/eos/out/param/";

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

void centerprint(const char *s, int y)
{
    int len = strlen(s) * 6;
    tft.setTextColor(WHITE, RED);
    tft.setCursor((tft.width() - len) / 2, y);
    tft.print(s);
}

void centertitle(const char *s){
    tft.fillScreen(BLACK);
    tft.fillRect(0, 0, 100, 14, RED);
    centerprint(s, 1);
    tft.fillRect(0, 14, 100, 1, WHITE);
    tft.setCursor(0, 30);
    tft.setTextColor(WHITE, BLACK);
}

///////////////////////////////////EOS FUNCS////////////////////////////////////////////////////////////

void issueFilters() {
	OSCMessage filter("/eos/filter/add");
	filter.add("/eos/out/param/*");
	filter.add("/eos/out/ping");
	SLIPSerial.beginPacket();
	filter.send(SLIPSerial);
	SLIPSerial.endPacket();
}

void issueSubscribes() {
	// unsubscribes all parameters
	String unsubMsg = UNSUBSCRIBE_QUERY;
	OSCMessage unsubscribe(unsubMsg.c_str());
	unsubscribe.add(UNSUBSCRIBE);
	SLIPSerial.beginPacket();
	unsubscribe.send(SLIPSerial);
	SLIPSerial.endPacket();

	// subscribes the displayed parameters, exept the parameter with keyword none
	String subMsg;
	if (parameter[idx].name == NO_PARAMETER) {
		subMsg = SUBSCRIBE_QUERY + parameter[idx + 1].name;
		}
	else if (parameter[idx + 1].name == NO_PARAMETER) {
		subMsg = SUBSCRIBE_QUERY + parameter[idx].name;
		}
	else {
		subMsg = SUBSCRIBE_QUERY + parameter[idx].name + "/" + parameter[idx + 1].name;
		}
	OSCMessage subscribe(subMsg.c_str());
	subscribe.add(SUBSCRIBE);
	SLIPSerial.beginPacket();
	subscribe.send(SLIPSerial);
	SLIPSerial.endPacket();
}

void sendPing() {
	OSCMessage ping("/eos/ping");
	ping.add(PING_QUERY.c_str());
	SLIPSerial.beginPacket();
	ping.send(SLIPSerial);
	SLIPSerial.endPacket();
	timeoutPingSent = true;
}

void initEOS() {
	SLIPSerial.beginPacket();
	SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
	SLIPSerial.endPacket();
	issueFilters();
	issueSubscribes();
}

String ModeSPD = "Coarse";

void displayStatus() {
	tft.fillScreen(BLACK);
	if (!connectedToEos) {
		// display a splash message before the Eos connection is open
		centertitle("Waiting For EOS..");
		tft.println(VERSION);
	}
	else {
		// put the cursor at the begining of the first line

    //////////////////////////////////////////////////////////////////////////EDIT HERE!//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Intens_btn.initButton(&tft,  60, 45, 85, 40, WHITE, CYAN, BLACK, "Intens", 2);

    Focus1_btn.initButton(&tft, 160, 45, 85, 40, WHITE, CYAN, BLACK, "Focus1", 2);
    Focus2_btn.initButton(&tft, 260, 45, 85, 40, WHITE, CYAN, BLACK, "Focus2", 2);

    Color1_btn.initButton(&tft, 60, 95, 85, 40, WHITE, CYAN, BLACK, "Color1", 2);
    Color2_btn.initButton(&tft, 160, 95, 85, 40, WHITE, CYAN, BLACK, "Color2", 2);
    Color3_btn.initButton(&tft, 260, 95, 85, 40, WHITE, CYAN, BLACK, "Color3", 2);
    Color4_btn.initButton(&tft, 60, 145, 85, 40, WHITE, CYAN, BLACK, "Color4", 2);
    ColorWL_btn.initButton(&tft, 160, 145, 85, 40, WHITE, CYAN, BLACK, "ColorWL", 2);

    Gobo_btn.initButton(&tft, 260, 145, 85, 40, WHITE, CYAN, BLACK, "Gobo", 2);

    BeamFx1_btn.initButton(&tft, 60, 195, 85, 40, WHITE, CYAN, BLACK, "BeamFX1", 2);
    BeamFx2_btn.initButton(&tft, 160, 195, 85, 40, WHITE, CYAN, BLACK, "BeamFX2", 2);

    Frame1_btn.initButton(&tft, 260, 195, 85, 40, WHITE, CYAN, BLACK, "Frame1", 2);
    Frame2_btn.initButton(&tft, 60, 245, 85, 40, WHITE, CYAN, BLACK, "Frame2", 2);
    Frame3_btn.initButton(&tft, 160, 245, 85, 40, WHITE, CYAN, BLACK, "Frame3", 2);
    Frame4_btn.initButton(&tft, 260, 245, 85, 40, WHITE, CYAN, BLACK, "Frame4", 2);

    Other_btn.initButton(&tft, 60, 295, 85, 40, WHITE, CYAN, BLACK, "Other", 2);

    /////////////////////////////////////Draw Buttons (Editable)//////////////////////////////////////////////////////////////////

    Intens_btn.drawButton(true);


    Focus1_btn.drawButton(true);
    Focus2_btn.drawButton(true);


    Color1_btn.drawButton(true);
    Color2_btn.drawButton(true);
    Color3_btn.drawButton(true);
    Color4_btn.drawButton(true);
    ColorWL_btn.drawButton(true);


    Gobo_btn.drawButton(true);

    BeamFx1_btn.drawButton(true);
    BeamFx2_btn.drawButton(true);


    Frame1_btn.drawButton(true);
    Frame2_btn.drawButton(true);
    Frame3_btn.drawButton(true);
    Frame4_btn.drawButton(true);


    Other_btn.drawButton(true);

    /////////////////////////////////////////////////////////////////////////////////////////////////DONT TOUCH BELOW///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //  JOYSTICK SELECTION SHOW

    tft.drawFastHLine(0, 400, 350, WHITE);

    tft.setCursor(15, 415);
    tft.setTextSize(1.5);
    tft.println("Left - Right");

    tft.setCursor(110, 415);
    tft.setTextSize(1.5);
    tft.println("Up - Down");

    tft.setCursor(210, 415);
    tft.setTextSize(1.5);
    tft.println("Turn L - Turn R");



    tft.setCursor(5, 445);
    tft.setTextSize(2);
    tft.println(parameter[Joy1.parameterIdx].display);

    tft.setCursor(110, 445);
    tft.setTextSize(2);
    tft.println(parameter[Joy2.parameterIdx].display);

    tft.setCursor(210, 445);
    tft.setTextSize(2);
    tft.println(parameter[Joy3.parameterIdx].display);

    ///  SELECTION INDICATORS

    tft.setCursor(15, 385);
    tft.setTextSize(1.5);
    tft.println("Selected Menu: " + parameter[Joy1.parameterIdx].Selection);

    tft.setCursor(190, 385);
    tft.setTextSize(1.5);
    tft.println("Selected Page: " + String(parameter[Joy1.parameterIdx].Page));

    tft.setCursor(190, 360);
    tft.setTextSize(1.5);
    tft.println("Selected Mode: " + ModeSPD);

    // Mode Select

    Mode_btn.initButton(&tft, 45, 355, 85, 30, WHITE, CYAN, BLACK, "Modes", 2);
    Mode_btn.drawButton(true);

		updateDisplay = false;
  }
}

void parseOSCMessage(String& msg) {
	// check to see if this is the handshake string
	if (msg.indexOf(HANDSHAKE_QUERY) != -1) {
		initEOS();
		connectedToEos = true;
		updateDisplay = true;
		}

	else {
		// checks if there is a message with data of your parameter list
		OSCMessage oscmsg;
		oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());
		for (int i = 0; i < PARAMETER_MAX; i++) {
			String parseMsg = PARAMETER_QUERY + parameter[i].name;
			if(msg.indexOf(parseMsg) != -1) {
				parameter[i].value = oscmsg.getOSCData(0)->getFloat(); // get the value
				connectedToEos = true; // Update this here just in case we missed the handshake
				updateDisplay = true;
				}
			}
		}
	}

void updateEncoder(struct Joystick* joystick) {
	int Analog_read = analogRead(joystick->pin);

  int Las_Rec = Analog_read;
  if (Las_Rec >= 1000) {
    Analog_read = 1000;
  }

  int SetValue = 0;
  String Setting = "";

  if (ModeSPD == "Large") {
    SetValue = 4;
    Setting = "Coarse";
  } else if (ModeSPD == "Coarse") {
    SetValue = 1;
    Setting = "Coarse";
  } else if (ModeSPD == "Fine") {
    SetValue = 3;
    Setting = "Fine";
  } else if (ModeSPD == "XFine") {
    SetValue = 1;
    Setting = "Fine";
  }

  if (joystick->type == "LR") {
    if (Las_Rec >= 600) {
      Analog_read = -SetValue;
    } else if (Las_Rec <= 400) {
      Analog_read = SetValue;
    } else {
      Analog_read = 0;
    }
  } else if (joystick->type == "UD") {
    if (Las_Rec >= 600) {
      Analog_read = -SetValue;
    } else if (Las_Rec <= 400) {
      Analog_read = SetValue;
    } else {
      Analog_read = 0;
    }
  } else if (joystick->type == "T") {
    if (Las_Rec >= 600) {
      Analog_read = SetValue;
    } else if (Las_Rec <= 400) {
      Analog_read = -SetValue;
    } else {
      Analog_read = 0;
    }
  }

  if (joystick->pinPrevious != Analog_read) {
    if ((Analog_read != 0) && (joystick->parameterIdx >= 0)) {
      if (parameter[joystick->parameterIdx].name == NO_PARAMETER) return;
		  String wheelMsg("/eos/wheel");
      
      if (Setting == "Coarse") {
        wheelMsg.concat("/coarse");
      } else if (Setting == "Fine") {
        wheelMsg.concat("/fine");
      };

      wheelMsg.concat('/' + parameter[joystick->parameterIdx].name);
		  OSCMessage wheelUpdate(wheelMsg.c_str());
		  wheelUpdate.add(Analog_read);
		  SLIPSerial.beginPacket();
		  wheelUpdate.send(SLIPSerial);
		  SLIPSerial.endPacket();
    }
  }
}

void initJoy(struct Joystick* joystick, uint8_t pinA, String Type) {
	joystick->pin = pinA;

	pinMode(pinA, INPUT_PULLUP);

	joystick->pinPrevious = analogRead(pinA);

  joystick->type = Type;
}

void ChangeWheels(String Wheel) {
  for (int i = 0; i < PARAMETER_MAX; ++i) {
    if (parameter[i].Selection == Wheel) {
      Joy1.parameterIdx = i;

      if (parameter[i + 1].Selection == Wheel) {
        Joy2.parameterIdx = i + 1;
      } else {
        Joy2.parameterIdx = 0;
      }

      if (parameter[i + 2].Selection == Wheel) {
        Joy3.parameterIdx = i + 2;
      } else {
        Joy3.parameterIdx = 0;
      }

      idx = i;

      displayStatus();

      int AmmountParams = 0;
      
      for (int i = 0; i < PARAMETER_MAX; ++i) {
         if (parameter[i].Selection == Wheel) {
          AmmountParams = AmmountParams + 1;
         }
      }

      break;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) ID = 0x9486;
  tft.begin(ID);
  tft.setRotation(0);
  tft.fillScreen(BLACK);

  SLIPSerial.begin(115200);

	#ifdef BOARD_HAS_USB_SERIAL
	 while (!SerialUSB);
	 #else
	 while (!Serial);
	#endif

	initEOS(); // for hotplug with Arduinos without native USB like UNO

	// init of hardware elements
	initJoy(&Joy1, A13, "LR");
	initJoy(&Joy2, A14, "UD");
  initJoy(&Joy3, A15, "T");
	Joy1.parameterIdx = idx;
	Joy2.parameterIdx = idx + 1;
  Joy3.parameterIdx = idx + 2;

  ChangeWheels("INTENS");

	displayStatus();
}

bool UpdatedDisp = false;

void loop() {
  if (connectedToEos == true) {
    if (UpdatedDisp == false) {
      displayStatus();
      UpdatedDisp = true;
    };
  }
  bool down = Touch_getXY();

//////////////////////////////////////////////////////////////////////////////////////////EDIT HERE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Intens_btn.press(down && Intens_btn.contains(pixel_x, pixel_y));

  Focus1_btn.press(down && Focus1_btn.contains(pixel_x, pixel_y));
  Focus2_btn.press(down && Focus2_btn.contains(pixel_x, pixel_y));

  Color1_btn.press(down && Color1_btn.contains(pixel_x, pixel_y));
  Color2_btn.press(down && Color2_btn.contains(pixel_x, pixel_y));
  Color3_btn.press(down && Color3_btn.contains(pixel_x, pixel_y));
  Color4_btn.press(down && Color4_btn.contains(pixel_x, pixel_y));
  ColorWL_btn.press(down && ColorWL_btn.contains(pixel_x, pixel_y));

  Gobo_btn.press(down && Gobo_btn.contains(pixel_x, pixel_y));

  BeamFx1_btn.press(down && BeamFx1_btn.contains(pixel_x, pixel_y));
  BeamFx2_btn.press(down && BeamFx2_btn.contains(pixel_x, pixel_y));

  Frame1_btn.press(down && Frame1_btn.contains(pixel_x, pixel_y));
  Frame2_btn.press(down && Frame2_btn.contains(pixel_x, pixel_y));
  Frame3_btn.press(down && Frame3_btn.contains(pixel_x, pixel_y));
  Frame4_btn.press(down && Frame4_btn.contains(pixel_x, pixel_y));

  Other_btn.press(down && Other_btn.contains(pixel_x, pixel_y));

  if (Intens_btn.justPressed()) {
    ChangeWheels("INTENS");
  }


  if (Focus1_btn.justPressed()) {
    ChangeWheels("FOCUS1");
  }
  if (Focus2_btn.justPressed()) {
    ChangeWheels("FOCUS2");
  }


  if (Color1_btn.justPressed()) {
    ChangeWheels("COLOR1");
  }
  if (Color2_btn.justPressed()) {
    ChangeWheels("COLOR2");
  }
  if (Color3_btn.justPressed()) {
    ChangeWheels("COLOR3");
  }
  if (Color4_btn.justPressed()) {
    ChangeWheels("COLOR4");
  }
  if (ColorWL_btn.justPressed()) {
    ChangeWheels("COLORWL");
  }


  if (Gobo_btn.justPressed()) {
    ChangeWheels("GOBO");
  }


  if (BeamFx1_btn.justPressed()) {
    ChangeWheels("BEAMFX1");
  }
  if (BeamFx2_btn.justPressed()) {
    ChangeWheels("BEAMFX2");
  }


  if (Frame1_btn.justPressed()) {
    ChangeWheels("FRAME1");
  }
  if (Frame2_btn.justPressed()) {
    ChangeWheels("FRAME2");
  }
  if (Frame3_btn.justPressed()) {
    ChangeWheels("FRAME3");
  }
  if (Frame4_btn.justPressed()) {
    ChangeWheels("FRAME4");
  }


  if (Other_btn.justPressed()) {
    ChangeWheels("OTHER");
  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Mode_btn.press(down && Mode_btn.contains(pixel_x, pixel_y));

  if (Mode_btn.justPressed()) {
    if (ModeSPD == "Large") {
      ModeSPD = "Coarse";
    } else if (ModeSPD == "Coarse") {
      ModeSPD = "Fine";
    } else if (ModeSPD == "Fine") {
      ModeSPD = "XFine";
    } else if (ModeSPD == "XFine") {
      ModeSPD = "Large";
    };
    displayStatus();
  }

  static String curMsg;

	// Then we check to see if any OSC commands have come from Eos
	// and update the display accordingly.
	int size = SLIPSerial.available();
	if (size > 0) {
		while (size--) curMsg += (char)(SLIPSerial.read());
		}
	if (SLIPSerial.endofPacket()) {
		parseOSCMessage(curMsg);
		lastMessageRxTime = millis();
		// We only care about the ping if we haven't heard recently
		// Clear flag when we get any traffic
		timeoutPingSent = false;
		curMsg = String();
		}

	// check for next/last updates
	updateEncoder(&Joy1);
	updateEncoder(&Joy2);
  updateEncoder(&Joy3);

	if(lastMessageRxTime > 0) {
		unsigned long diff = millis() - lastMessageRxTime;
		//We first check if it's been too long and we need to time out
		if (diff > TIMEOUT_AFTER_IDLE_INTERVAL) {
			connectedToEos = false;
			lastMessageRxTime = 0;
			updateDisplay = true;
			timeoutPingSent = false;
      displayStatus();
		}

		// It could be the console is sitting idle. Send a ping once to
		// double check that it's still there, but only once after 2.5s have passed
		if (!timeoutPingSent && diff > PING_AFTER_IDLE_INTERVAL) {
			sendPing();
		}
	};
}
