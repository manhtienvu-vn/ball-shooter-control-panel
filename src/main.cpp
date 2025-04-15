#include <Arduino.h>
#include <ESPUI.h>
#include <DNSServer.h>
#include <WiFi.h>

const byte DNS_PORT = 53; //setup standard DNS port, which listens for DNS queries
IPAddress apIP(192,168,4,1);
DNSServer dnsServer;

uint16_t powerLabel, modeLabel, angleYLabel, angleZLabel, buttonGameStatus, buttonResetGameStatus, turnTimeLabel, gameTimeLabel;
int16_t angleY, angleZ, inputAngleY, inputAngleZ, cnt = 0;
int16_t currentTime, previousTime, gameTime = 120, turnTime = 20;

char powerStatus[8], modeStatus[10], gameStatus[8], resetGameStatus[8];
bool power = false, shootTrigger = false, play = false;

const char* ssid = "rmlab-controlpanel";
const char* password = "12345678";
const char* hostname = "rmlab";


void powerSwitcherCallback(Control *sender, int type){
  //int switchval = sender->value.toInt();
  switch (type)
  {
  case S_INACTIVE:
    power = false;
    strcpy(powerStatus, "OFF");
    ESPUI.getControl(powerLabel)->color = ControlColor::Alizarin;
    ESPUI.updateLabel(powerLabel, powerStatus);
    break;
  case S_ACTIVE:
    power = true;
    strcpy(powerStatus, "ON");
    ESPUI.getControl(powerLabel)->color = ControlColor::Emerald;
    ESPUI.updateLabel(powerLabel, powerStatus);
    break;
  }

  Serial.print("Power Switch: ID:  ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(String(type));
  Serial.println(powerStatus);

}

void modeSwitcherCallback(Control *sender, int type){
  switch (type)
  {
  case S_INACTIVE:
    strcpy(modeStatus,"MANUAL");
    ESPUI.getControl(modeLabel)->color = ControlColor::Sunflower;
    ESPUI.updateLabel(modeLabel, modeStatus);
    break;
  
  case S_ACTIVE:
    strcpy(modeStatus, "AUTO");
    ESPUI.getControl(modeLabel)->color = ControlColor::Peterriver;
    ESPUI.updateLabel(modeLabel, modeStatus);
    break;
  }

  Serial.print("Mode Switch: ID:  ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(String(type));
  Serial.println(modeStatus);

}

void padWithCenterCallback(Control *sender, int value){

  switch (value)
  {
  case P_LEFT_DOWN:
    angleZ -=1;
    Serial.print("Angle Z:");
    Serial.print("  ");
    Serial.print(angleZ);
    Serial.print("  ");
    Serial.print("LEFT pressed");
    ESPUI.updateLabel(angleZLabel,String(angleZ));
    break;

  case P_LEFT_UP:
    Serial.print("LEFT released");
    break;

  case P_RIGHT_DOWN:
    angleZ += 1;
    Serial.print("Angle Z:");
    Serial.print("  ");
    Serial.print(angleZ);
    Serial.print("  ");
    Serial.print("RIGHT pressed");
    ESPUI.updateLabel(angleZLabel,String(angleZ));
    break;

  case P_RIGHT_UP:
    Serial.print("RIGHT released");
    break;

  case P_FOR_DOWN:
    angleY += 1;
    Serial.print("Angle Y:");
    Serial.print("  ");
    Serial.print(angleY);
    Serial.print("  ");
    Serial.print("UP pressed");
    ESPUI.updateLabel(angleYLabel,String(angleY));
    break;

  case P_FOR_UP:
    Serial.print("UP released");
    break;

  case P_BACK_DOWN:
    angleY -= 1;
    Serial.print("Angle Y:");
    Serial.print("  ");
    Serial.print(angleY);
    Serial.print("  ");
    Serial.print("DOWN pressed");
    ESPUI.updateLabel(angleYLabel,String(angleY));
    break;

  case P_BACK_UP:
    Serial.print("DOWN released");
    break;

  case P_CENTER_DOWN:
    shootTrigger = true;
    Serial.print("Fire the shooter!");
    Serial.print("  ");
    Serial.print("CENTER pressed");
    break;

  case P_CENTER_UP:
    shootTrigger = false;
    Serial.print("Done!");
    Serial.print("  ");
    Serial.print("CENTER released");
    break;
  }

  Serial.print(" ");
  Serial.println(sender->id);
  
}

void inputAngleYCallback(Control *sender, int value){
  int16_t inputAngleY_Val = sender->value.toInt();

  angleY = inputAngleY_Val;
  ESPUI.updateLabel(angleYLabel,String(angleY));

  Serial.println(sender->id);
  Serial.print("Angle Y was changed to: ");
  Serial.print(angleY);

}

void inputAngleZCallback(Control*sender, int value){
  int16_t inputAngleZ_Val = sender->value.toInt();

  angleZ = inputAngleZ_Val;
  ESPUI.updateLabel(angleZLabel,String(angleZ));

  Serial.println(sender->id);
  Serial.print("Angle Z was changed to: ");
  Serial.print(angleZ);
}

void gameStatusCallback(Control *sender, int value){
  switch (value)
  {
  case B_DOWN:
    cnt += 1;
    play =! play;
    if(play == false && cnt != 0){
      strcpy(gameStatus,"RESUME");
      ESPUI.updateButton(buttonGameStatus,gameStatus);
      Serial.println(sender->id);
      Serial.print("Game Status: PAUSING");
      ESPUI.updateLabel(turnTimeLabel, String(turnTime));
    }
    else if(play == true && cnt != 0){
      strcpy(gameStatus,"PAUSE");
      ESPUI.updateButton(buttonGameStatus,gameStatus);
      Serial.println(sender->id);
      Serial.print("Game Status: STARTED");
      ESPUI.updateLabel(turnTimeLabel, String(turnTime));
    }
    break;
  
  case B_UP:
    break;
  }

}

void resetGameStatusCallback(Control *sender, int value){
  switch (value)
  {
  case B_DOWN:
      play = false;
      gameTime = 120;
      turnTime = 20;
      cnt = 0;
      strcpy(gameStatus,"START");
      ESPUI.updateLabel(turnTimeLabel, String(turnTime));
      ESPUI.updateLabel(gameTimeLabel, String(gameTime));
      ESPUI.updateLabel(buttonGameStatus, String(gameStatus));

    break;
  
  case B_UP:
    break;
  }

}

void updateTurnTime(){
  if(play == false && cnt ==0){
    gameTime = 120;
    turnTime = 20;
    currentTime = 0;
    previousTime = 0;
  }

  else if(play == true && cnt != 0){
    currentTime = millis();
    if(currentTime - previousTime >= 1000){
      gameTime -= 1;
      turnTime -= 1;
      ESPUI.updateLabel(gameTimeLabel, String(gameTime));
      ESPUI.updateLabel(turnTimeLabel, String(turnTime));
      Serial.print("Time: ");
      Serial.println(turnTime);
      previousTime = currentTime;
    }
  }

  else if(play == false && cnt != 0){
    turnTime = 20;
    currentTime = 0;
    previousTime = 0;
    ESPUI.updateLabel(turnTimeLabel, String(turnTime));
  }

}

void initUI(){
  auto maintab = ESPUI.addControl(Tab, "Control", "Control");
  //Create first section, started by adding a seperator
  ESPUI.addControl(Separator,"Power","",None, maintab);

  ESPUI.addControl(ControlType::Switcher, "OFF / ON", "0", Alizarin, maintab, powerSwitcherCallback);
  powerLabel = ESPUI.addControl(Label, "Power Status", powerStatus, Emerald, maintab, powerSwitcherCallback);
  ESPUI.addControl(ControlType::Switcher, "MANUAL / AUTO", "0", Alizarin, maintab, modeSwitcherCallback);
  modeLabel = ESPUI.addControl(Label, "Operation Mode", modeStatus, Sunflower, maintab, modeSwitcherCallback);
  

  ESPUI.addControl(Separator, "Basic Control", "", None, maintab);
  ESPUI.addControl(PadWithCenter, "Turret Control", "", Dark, maintab, padWithCenterCallback);

  angleYLabel = ESPUI.addControl(Label, "Angle Y Value", String(angleY), Carrot, maintab, padWithCenterCallback);
  angleZLabel = ESPUI.addControl(Label, "Angle Z Value", String(angleZ), Carrot, maintab, padWithCenterCallback);

  ESPUI.addControl(Separator, "Input Control", "", None, maintab);
  inputAngleY = ESPUI.addControl(Number, "Angle Y Input", "0", Emerald, maintab, inputAngleYCallback);
  inputAngleZ = ESPUI.addControl(Number,"Angle Z Input", "0", Emerald, maintab, inputAngleZCallback);
	ESPUI.addControl(Min, "", "0", None, inputAngleY);
	ESPUI.addControl(Max, "", "180", None, inputAngleY);

  auto trackingtab = ESPUI.addControl(Tab,"Position Tracking","Position Tracking");
  ESPUI.addControl(Separator,"Robot Position Tracking", "", None, trackingtab);

  auto gametab = ESPUI.addControl(Tab, "Game Cycle", "Game Cycle");
  ESPUI.addControl(Separator, "Scoreboard", "", None, gametab);
  
  ESPUI.addControl(Separator, "Score Clock", "", None, gametab);

  buttonGameStatus = ESPUI.addControl(Button, "START / PAUSE", "START", Sunflower, gametab, gameStatusCallback);
  gameTimeLabel = ESPUI.addControl(Label, "Game Time", String(gameTime), Peterriver, gametab, gameStatusCallback);
  buttonResetGameStatus = ESPUI.addControl(Button,"RESET GAME", "Click to Reset!", Alizarin, gametab, resetGameStatusCallback);
  turnTimeLabel = ESPUI.addControl(Label, "Turn Time", String(turnTime), Emerald, gametab, gameStatusCallback);


}

void setup() {
  Serial.begin(115200);
  WiFi.setHostname(hostname);
  WiFi.mode(WIFI_AP); //set the esp32 wifi to an access point
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0)); //configure the IP address of the esp32 in the access point mode
  WiFi.softAP(ssid,password); //create a Wifi network in Access Point mode

  dnsServer.start(DNS_PORT, "*", apIP); 
  //start a DNS server that redirects all domain name (when users try to access to abc.com) to the esp32's IP pre-set address 
  //DNS Server is configured as a captive portal

  Serial.println("\n\nWiFi parameters:");
  Serial.print("Mode: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
  Serial.print("IP address: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
  ESPUI.begin("___[RM_DT02] Ball Shooter Control Panel___"  );
  initUI();
}

void loop() {
  dnsServer.processNextRequest(); 
  updateTurnTime();
}

