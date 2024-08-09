#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#include <SLIPEncodedSerial.h>
#include <LiquidCrystal.h>
#include <string.h>

// LCD
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
int LCD_CHARS = 16;
int LCD_LINES = 2;

// JOYSTICK
const byte Analog_Pan_pin = A0;
const byte Analog_Tilt_pin = A1;
const byte Analog_Other_pin = A2;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Analog_Pan = 0;
int Analog_Tilt = 0;
int Analog_Other = 0;
enum WHEEL_TYPE { TILT, PAN, INTENS };
enum WHEEL_MODE { COARSE, FINE };

int EOS_pan = 0;
int EOS_tilt = 0;

float timeNow;

enum ConsoleType


{
  ConsoleNone,
  ConsoleEos,
  ConsoleCobalt,
  ConsoleColorSource
};

ConsoleType connectedToConsole = ConsoleNone;

#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);

//osc
#define OSC_BUF_MAX_SIZE    512

const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

#define VERSION_STRING      "2"
#define BOX_NAME_STRING     "EOS Joystick"
#define PING_AFTER_IDLE_INTERVAL    2500
#define TIMEOUT_AFTER_IDLE_INTERVAL 5000

#define SUBSCRIBE           ((int32_t)1)
#define UNSUBSCRIBE         ((int32_t)0)

bool updateDisplay = false;
unsigned long lastMessageRxTime = 0;
bool timeoutPingSent = false;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadAnalog()
{
  
  Analog_Pan = analogRead(Analog_Pan_pin);  
  delay(10); //allowing a little time between two readings
  Analog_Tilt = analogRead(Analog_Tilt_pin);    
  delay(10); //allowing a little time between two readings
  Analog_Other = analogRead(Analog_Other_pin);    
  
}

void issueEosSubscribes()
{
  // Add a filter so we don't get spammed with unwanted OSC messages from Eos
  OSCMessage filter("/eos/filter/add");
  filter.add("/eos/out/param/*");
  filter.add("/eos/out/ping");
  SLIPSerial.beginPacket();
  filter.send(SLIPSerial);
  SLIPSerial.endPacket();

  // subscribe to Eos pan & tilt updates
  OSCMessage subPan("/eos/subscribe/param/pan");
  subPan.add(SUBSCRIBE);
  SLIPSerial.beginPacket();
  subPan.send(SLIPSerial);
  SLIPSerial.endPacket();

  OSCMessage subTilt("/eos/subscribe/param/tilt");
  subTilt.add(SUBSCRIBE);
  SLIPSerial.beginPacket();
  subTilt.send(SLIPSerial);
  SLIPSerial.endPacket();
}

void parseEos(OSCMessage& msg, int addressOffset)
{
  // If we don't think we're connected, reconnect and subscribe
  if (connectedToConsole != ConsoleEos)
  {
    issueEosSubscribes();
    connectedToConsole = ConsoleEos;
    updateDisplay = true;
  }
}

void parseOSCMessage(String& msg)
{
  // check to see if this is the handshake string
  if (msg.indexOf(HANDSHAKE_QUERY) != -1)
  {
    // handshake string found!
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();

    // An Eos would do nothing until subscribed
    // Let Eos know we want updates on some things
    issueEosSubscribes();

    updateDisplay = true;
  }
  else
  {
    // prepare the message for routing by filling an OSCMessage object with our message string
    OSCMessage oscmsg;
    oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());
    // route pan/tilt messages to the relevant update function

    // Try the various OSC routes
    if (oscmsg.route("/eos", parseEos))
      return;
  }
}

void displayStatus()
{
  lcd.clear();

  switch (connectedToConsole)
  {
    case ConsoleNone:
      {
        // display a splash message before the Eos connection is open
        lcd.setCursor(0, 0);
        lcd.print(BOX_NAME_STRING " v" VERSION_STRING);
        lcd.setCursor(0, 1);
        lcd.print("Connecting..");
      } break;

    case ConsoleEos:
      {
        // put the cursor at the begining of the first line
        lcd.setCursor(0, 0);
        lcd.print("Pan:  ");
        lcd.print(Analog_Pan);

        lcd.setCursor(8, 0);
        lcd.print("INT:  ");
        lcd.print(Analog_Other);

        // put the cursor at the begining of the second line
        lcd.setCursor(0, 1);
        lcd.print("Tilt: ");
        lcd.print(Analog_Tilt);
      } break;
  }

  updateDisplay = false;
}

void sendOscMessage(const String &address, float value)
{
  OSCMessage msg(address.c_str());
  msg.add(value);
  SLIPSerial.beginPacket();
  msg.send(SLIPSerial);
  SLIPSerial.endPacket();
}

void sendEosWheelMove(WHEEL_TYPE type, float ticks)
{
  String wheelMsg("/eos/wheel");

  //if (digitalRead(SHIFT_BTN) == LOW)
    //wheelMsg.concat("/fine");
  //else
    //wheelMsg.concat("/coarse");

  wheelMsg.concat("/coarse");

  if (type == PAN)
    wheelMsg.concat("/pan");
  else if (type == TILT)
    wheelMsg.concat("/tilt");
  else if (type == INTENS)
    wheelMsg.concat("/intens");
  else
    // something has gone very wrong
    return;

  sendOscMessage(wheelMsg, ticks);
}

void sendWheelMove(WHEEL_TYPE type, float ticks)
{
  switch (connectedToConsole)
  {
    default:
    case ConsoleEos:
      sendEosWheelMove(type, ticks);
      break;
  }
}

void setup()
{
  SLIPSerial.begin(115200);
  // This is a hack around an Arduino bug. It was taken from the OSC library
  //examples
#ifdef BOARD_HAS_USB_SERIAL`
  while (!SerialUSB);
#else
  while (!Serial);
#endif

  // This is necessary for reconnecting a device because it needs some time
  // for the serial port to open. The handshake message may have been sent
  // from the console before #lighthack was ready

  SLIPSerial.beginPacket();
  SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
  SLIPSerial.endPacket();

  // If it's an Eos, request updates on some things
  issueEosSubscribes();

  lcd.clear();
  lcd.begin(16,2);
  displayStatus();
}

void loop()
{
  static String curMsg;
  int size;
  // get the updated state of each pot
  ReadAnalog();


  /////////PAN/////////////
  int Las_Pan = Analog_Pan;
  if (Las_Pan >= 1000) {
    Analog_Pan = 1000;
  }
  if (Las_Pan >= 600) {
    Analog_Pan = -5;
  } else if (Las_Pan <= 500) {
    Analog_Pan = 5;
  } else {
    Analog_Pan = 0;
  }

  if (Analog_Pan != 0) {
    sendWheelMove(PAN, Analog_Pan);
  }

  /////////TILT/////////////
  int Las_Tilt = Analog_Tilt;
  if (Las_Tilt >= 1000) {
    Analog_Tilt = 1000;
  }
  if (Las_Tilt >= 600) {
    Analog_Tilt = -5;
  } else if (Las_Tilt <= 400) {
    Analog_Tilt = 5;
  } else {
    Analog_Tilt = 0;
  }

  if (Analog_Tilt != 0) {
    sendWheelMove(TILT, Analog_Tilt);
  }

    /////////INTENSITY/////////////
  int Las_INT = Analog_Other;
  if (Las_INT >= 1000) {
    Analog_Other = 1000;
  }
  if (Las_INT >= 600) {
    Analog_Other = 5;
  } else if (Las_INT <= 400) {
    Analog_Other = -5;
  } else {
    Analog_Other = 0;
  }

  if (Analog_Other != 0) {
    sendWheelMove(INTENS, Analog_Other);
  }

  displayStatus();
  // Then we check to see if any OSC commands have come from Eos
  // and update the display accordingly.
  size = SLIPSerial.available();
  if (size > 0)
  {
    // Fill the msg with all of the available bytes
    while (size--)
      curMsg += (char)(SLIPSerial.read());
  }
  if (SLIPSerial.endofPacket())
  {
    parseOSCMessage(curMsg);
    lastMessageRxTime = millis();
    // We only care about the ping if we haven't heard recently
    // Clear flag when we get any traffic
    timeoutPingSent = false;
    curMsg = String();
  }

  if (lastMessageRxTime > 0)
  {
    unsigned long diff = millis() - lastMessageRxTime;
    //We first check if it's been too long and we need to time out
    if (diff > TIMEOUT_AFTER_IDLE_INTERVAL)
    {
      connectedToConsole = ConsoleNone;
      lastMessageRxTime = 0;
      updateDisplay = true;
      timeoutPingSent = false;
    }

    //It could be the console is sitting idle. Send a ping once to
    // double check that it's still there, but only once after 2.5s have passed
    if (!timeoutPingSent && diff > PING_AFTER_IDLE_INTERVAL)
    {
      OSCMessage ping("/eos/ping");
      ping.add(BOX_NAME_STRING "_hello"); // This way we know who is sending the ping
      SLIPSerial.beginPacket();
      ping.send(SLIPSerial);
      SLIPSerial.endPacket();
      timeoutPingSent = true;
    }
  }
}
