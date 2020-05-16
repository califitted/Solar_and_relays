#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"
#include "config.h"

//solarStuff

#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;
Adafruit_INA219 ina219_load(0x41);

// set up the 'voltage' and 'current' feeds
AdafruitIO_Feed *voltage = io.feed("solar.solar-panel-voltage");
AdafruitIO_Feed *power = io.feed("solar.solar-panel-power");
AdafruitIO_Feed *current = io.feed("solar.solar-panel-current");
AdafruitIO_Feed *voltage_load = io.feed("solar.solar-panel-voltage-load");
AdafruitIO_Feed *power_load = io.feed("solar.solar-panel-power-load");
AdafruitIO_Feed *current_load = io.feed("solar.solar-panel-current-load");

//final working code for Nodemcu v3 and compatible for 8 Channel Power Relay..
//fb.com/insider7enjoy
//programmed on 13-Sep-2017
//PhantomCluster.com

// prototypes
boolean connectWifi();

//on/off callbacks
void lightOneOn();
void lightOneOff();
void lightTwoOn();
void lightTwoOff();
void lightThreeOn();
void lightThreeOff();
void lightFourOn();
void lightFourOff();
void outletOneOn();
void outletOneOff();
void outletTwoOn();
void outletTwoOff();
void outletThreeOn();
void outletThreeOff();
void outletFourOn();
void outletFourOff();

// Change this before you flash
const char* ssid = "alexa";
const char* password = "alexa@IoT";

boolean wifiConnected = false;

UpnpBroadcastResponder upnpBroadcastResponder;

Switch *lightOne = NULL;
Switch *lightTwo = NULL;
Switch *lightThree = NULL;
Switch *lightFour = NULL;
Switch *outletOne = NULL;
Switch *outletTwo = NULL;
Switch *outletThree = NULL;
Switch *outletFour = NULL;

//relay pin setup for funct
int relayOne = 5;
int relayTwo = 4;
int relayThree = 0;
int relayFour = 2;
int relayFive = 14;
int relaySix = 12;
int relaySeven = 13;
int relayEight = 15;


void setup() {

 Serial.begin(115200);

  // Initialise wifi connection
  wifiConnected = connectWifi();

  if(wifiConnected){
    upnpBroadcastResponder.beginUdpMulticast();

    // Define your switches here. Max 14
    // Format: Alexa invocation name, local port no, on callback, off callback
    lightOne = new Switch("Light One", 80, lightOneOn, lightOneOff);
    lightTwo = new Switch("Light Two", 81, lightTwoOn, lightTwoOff);
    lightThree = new Switch("Light Three", 82, lightThreeOn, lightThreeOff);
    lightFour = new Switch("Light Four", 83, lightFourOn, lightFourOff);
    outletOne = new Switch("Outlet One", 84,outletOneOn, outletOneOff);
    outletTwo = new Switch("Outlet Two", 85, outletTwoOn, outletTwoOff);
    outletThree = new Switch("Outlet Three", 86, outletThreeOn, outletThreeOff);
    outletFour = new Switch("Outlet Four", 87,outletFourOn, outletFourOff);

    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*lightOne);
    upnpBroadcastResponder.addDevice(*lightTwo);
    upnpBroadcastResponder.addDevice(*lightThree);
    upnpBroadcastResponder.addDevice(*lightFour);
    upnpBroadcastResponder.addDevice(*outletOne);
    upnpBroadcastResponder.addDevice(*outletTwo);
    upnpBroadcastResponder.addDevice(*outletThree);
    upnpBroadcastResponder.addDevice(*outletFour);

    //relay pins setup i Used D1,D2,D3,D4,D5,D6,D7,D8 followed as assigned below, if you are willing to change Pin or planning to use extra please Check Image in Github File..:)
    pinMode (5, OUTPUT);
    pinMode (4, OUTPUT);
    pinMode (0, OUTPUT);
    pinMode (2, OUTPUT);
    pinMode (14, OUTPUT);
    pinMode (12, OUTPUT);
    pinMode (13, OUTPUT);
    pinMode (15, OUTPUT);
    digitalWrite (5,LOW);
    digitalWrite (4,LOW);
    digitalWrite (0,LOW);
    digitalWrite (2,LOW);
    digitalWrite (14,LOW);
    digitalWrite (12,LOW);
    digitalWrite (13,LOW);
    digitalWrite (15,LOW);
  }
    // start the serial connection
  Serial.begin(115200);
  // wait for serial monitor to open
  while(! Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    // while (1) { delay(10); }
  }
  if (! ina219_load.begin()) {
    Serial.println("Failed to find INA219 load chip");
    // while (1) { delay(10); }
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();
  
  Serial.println("Measuring voltage and current with INA219 ...");
}

void loop() {

    // wait 5 seconds (5000 milliseconds == 5 seconds)
  delay(15000);
  //  delay(1000);

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  // float voltage_value = 12.0;
  
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
  float power_W = 0.0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  power_W = power_mW/1000.0;
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  current->save(current_mA); 
  power->save(power_W);
  voltage->save(loadvoltage);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

  float shuntvoltage_load = 0;
  float busvoltage_load = 0;
  float current_mA_load = 0;
  float loadvoltage_load = 0;
  float power_mW_load = 0;
  float power_W_load = 0.0;

  shuntvoltage_load = ina219_load.getShuntVoltage_mV();
  busvoltage_load = ina219_load.getBusVoltage_V();
  current_mA_load = ina219_load.getCurrent_mA();
  power_mW_load = ina219_load.getPower_mW();
  power_W_load = power_mW_load/1000.0;
  loadvoltage_load = busvoltage_load + (shuntvoltage_load / 1000);

  current_load->save(current_mA_load); 
  power_load->save(power_W_load);
//  voltage_load->save(loadvoltage_load);
  
  Serial.print("Bus Voltage Load:   "); Serial.print(busvoltage_load); Serial.println(" V");
  Serial.print("Shunt Voltage Load: "); Serial.print(shuntvoltage_load); Serial.println(" mV");
  Serial.print("Load Voltage Load:  "); Serial.print(loadvoltage_load); Serial.println(" V");
  Serial.print("Current Load:       "); Serial.print(current_mA_load); Serial.println(" mA");
  Serial.print("Power Load:         "); Serial.print(power_mW_load); Serial.println(" mW");
  Serial.println("");


  //delay(2000);

  {
   if(wifiConnected){
      upnpBroadcastResponder.serverLoop();

      lightOne->serverLoop();
      lightTwo->serverLoop();
      lightThree->serverLoop();
      lightFour->serverLoop();
      outletOne->serverLoop();
      outletTwo->serverLoop();
      outletThree->serverLoop();
      outletFour->serverLoop();
   }
}

void lightOneOff() {
    Serial.print("Switch 1 turn off ...");
    digitalWrite(relayOne, LOW);   // sets relayOne off
}

void lightOneOn() {
    Serial.print("Switch 1 turn on ...");
    digitalWrite(relayOne, HIGH);   // sets relayOne on
}

void lightTwoOff() {
    Serial.print("Switch 2 turn off ...");
    digitalWrite(relayTwo, LOW);   // sets relayOne o
}

void lightTwoOn() {
  Serial.print("Switch 2 turn on ...");
  digitalWrite(relayTwo, HIGH);   // sets relayOne on
}

void lightThreeOff() {
    Serial.print("Switch 3 turn off ...");
    digitalWrite(relayThree, LOW);   // sets relayOne on
}

void lightThreeOn() {
  Serial.print("Switch 3 turn on ...");
  digitalWrite(relayThree, HIGH);   // sets relayOne on
}

void lightFourOff() {
    Serial.print("Switch4 turn off ...");
    digitalWrite(relayFour, LOW);   // sets relayOne on
}

void lightFourOn() {
  Serial.print("Switch 4 turn on ...");
  digitalWrite(relayFour, HIGH);   // sets relayOne on
}

//sockets

void outletOneOff() {
    Serial.print("Socket 1 turn off ...");
    digitalWrite(relayFive, LOW);   // sets relayOne on
}

void outletOneOn() {
    Serial.print("Socket 1turn on ...");
    digitalWrite(relayFive, HIGH);   // sets relayOne off
}

void outletTwoOff() {
    Serial.print("Socket 2 turn off ...");
    digitalWrite(relaySix, LOW);   // sets relayOne on
}

void outletTwoOn() {
  Serial.print("Socket 2 turn on ...");
  digitalWrite(relaySix, HIGH);   // sets relayOne on
}

void outletThreeOff() {
    Serial.print("Socket 3 turn off ...");
    digitalWrite(relaySeven, LOW);   // sets relayOne on
}

void outletThreeOn() {
    Serial.print("Socket 3 turn on ...");
    digitalWrite(relaySeven, HIGH);   // sets relayOne off
}

void outletFourOff() {
    Serial.print("Socket  4 turn off ...");
    digitalWrite(relayEight, LOW);   // sets relayOne on
}

void outletFourOn() {
  Serial.print("Socket 4 turn on ...");
  digitalWrite(relayEight, HIGH);   // sets relayOne on
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }

  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}
