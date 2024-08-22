void WheelUpdate(OSCMessage& msg, int addressOffset) {
  tft.fillRect(0, 250, 400, 25, BLACK);
  tft.setCursor(0, 250);
  tft.setTextSize(1);
  tft.print("WHEEL TRIGGERED!");

  tft.fillRect(0, 300, 400, 25, BLACK);
  tft.setCursor(0, 300);
  tft.setTextSize(1);
  char str[100];
  msg.getString(0, str);
  tft.print(str);
}

void ChannelUpdate(OSCMessage& msg, int addressOffset) {
  tft.fillRect(0, 350, 400, 25, BLACK);
  tft.setCursor(0, 250);
  tft.setTextSize(1);
  tft.print("CHANNEL TRIGGERED!");

  tft.fillRect(0, 400, 400, 25, BLACK);
  tft.setCursor(0, 300);
  tft.setTextSize(1);
  char str[100];
  msg.getString(0, str);
  tft.print(str);
}

void issueFilters() {
  OSCMessage filter("/eos/filter/add");
  filter.add("/eos/out/active/chan");
  filter.add("/eos/out/active/wheel/*");
  filter.add("/eos/out/ping");

  SLIPSerial.beginPacket();
  filter.send(SLIPSerial);
  SLIPSerial.endPacket();
}

void parseOSCMessage(String& msg) {
  if (msg.indexOf(HANDSHAKE_QUERY) != -1) {
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();

    issueFilters();
    connectedToEos = true;
  } else {
    OSCMessage oscmsg;
    oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());
    oscmsg.route(EOS_CHAN_OUT.c_str(), ChannelUpdate);
    oscmsg.route(EOS_WHEEL_OUT.c_str(), WheelUpdate);
  }
}
